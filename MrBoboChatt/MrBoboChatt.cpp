#include <MrBoboChatt/MrBoboChatt.h>
#include <filesystem>
//olika depandancies
#ifdef WIN32
#include <conio.h>
#elif __linux__
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/stat.h>
#endif
//class MrBoboChat;
namespace ANSI
{
	std::string RESET = "\033[0m";
	std::string BLACK = "\033[30m";      /* Black */
	std::string RED = "\033[31m";      /* Red */
	std::string GREEN = "\033[32m";      /* Green */
	std::string YELLOW = "\033[33m";      /* Yellow */
	std::string BLUE = "\033[34m";     /* Blue */
	std::string MAGENTA = "\033[35m";      /* Magenta */
	std::string CYAN = "\033[36m";   /* Cyan */
	std::string WHITE = "\033[37m";      /* White */
	std::string BOLDBLACK = "\033[1m\033[30m";    /* Bold Black */
	std::string BOLDRED = "\033[1m\033[31m";    /* Bold Red */
	std::string BOLDGREEN = "\033[1m\033[32m";    /* Bold Green */
	std::string BOLDYELLOW = "\033[1m\033[33m";    /* Bold Yellow */
	std::string BOLDBLUE = "\033[1m\033[34m";    /* Bold Blue */
	std::string BOLDMAGENTA = "\033[1m\033[35m";     /* Bold Magenta */
	std::string BOLDCYAN = "\033[1m\033[36m";    /* Bold Cyan */
	std::string BOLDWHITE = "\033[1m\033[37m";    /* Bold White */
};
void ListenOnInitiationSocket(MBSockets::UDPSocket* SocketToListenOn, std::string* StringToModify, std::condition_variable* ConditionalToNotify, std::mutex* ResourceMutex)
{
	std::string RecievedDataFromSocket = SocketToListenOn->UDPGetData();
};
void ListenOnInitiationUserInput(std::atomic<bool>* BoolToModify, std::atomic<bool>* ShouldStop,MrBoboChat* AssociatedChatObject,bool OnMainThread)
{
	AssociatedChatObject->InitiatingCancelableInput = true;
	while (!*ShouldStop)
	{
		int InputIsAvailable = -1;
		if (OnMainThread)
		{
//			/*
		#ifdef WIN32
			InputIsAvailable = _kbhit();
		#elif __linux__
			//InputIsAvailable = select();
			fd_set rfds;
			struct timeval tv;

			// Watch stdin (fd 0) to see when it has input.
			FD_ZERO(&rfds);
			FD_SET(0, &rfds);
			// Wait up to five seconds.
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			InputIsAvailable = select(1, &rfds, NULL, NULL, &tv);
			// Don’t rely on the value of tv now!
		#endif
//*/		
			if (InputIsAvailable > 0)
			{
				//vi har fått input, och eftersom vi är på mainthreaden kan vi också ta och läsa denna för att se vad det är för något, och cancella om det är enter eller \r
				char NewInput = AssociatedChatObject->GetCharInput();
				if (NewInput == '\n' || NewInput == '\r')
				{
					*BoolToModify = true;
					break;
				}
			}
		}
		if (AssociatedChatObject->InitiatingCancelableInput == false)
		{
			*BoolToModify = true;
			break;
		}
	}
	AssociatedChatObject->InitiatingCancelableInput = false;
};
void InitiationSendData(MBSockets::UDPSocket* SocketToSendDataOn, std::string* StringToModify, std::condition_variable* ConditionalToNotify, std::mutex* ResourceMutex)
{
	std::string DataToSend = "";
	while (true)
	{
		SocketToSendDataOn->SendData(DataToSend.data(), DataToSend.size());
	}
}
void ChatMainFunction_Listener(MBChatConnection* AssociatedConnectionObject, std::vector<std::string>* RecievedMessages, std::atomic<bool>* RecievedMessage, std::mutex* MainFuncMutex)
{
	//std::cout << "Hej" << std::endl;
	while (!AssociatedConnectionObject->ShouldStop)
	{
		std::string NextMessage = AssociatedConnectionObject->GetData();
		if (NextMessage != "")
		{
			std::lock_guard<std::mutex> Lock(*MainFuncMutex);
			RecievedMessages->push_back(NextMessage);
			*RecievedMessage = true;
		}
	}
	//std::cout << "Hej da" << std::endl;
}
//class MBChatStaticResources
std::string MBChatStaticResources::GetDiffieHellmanModolu()
{
	std::lock_guard<std::mutex> Lock(ResourceMutex);
	return(DiffieHellmanModolu);
}
std::string MBChatStaticResources::GetDiffieHellmanGenerator()
{
	std::lock_guard<std::mutex> Lock(ResourceMutex);
	return(DiffieHellmanGenerator);
}


//class MBCLineObject
MBCLineObject::MBCLineObject(MrBoboChat* ChatObject)
{
	LineNumber = ChatObject->GetCurrentLineIndex();
	AssociatedChatObject = ChatObject;
	AssociatedChatObject->PrintLine("");
}
std::string MBCLineObject::GetLineData()
{
	return(LineData);
}
void MBCLineObject::SetLineData(std::string NewData)
{
	LineData = ReplaceAll(NewData,"\n","");
	AssociatedChatObject->PrintLineAtIndex(LineNumber,NewData);
}


//class MrBoboChat
void MrBoboChat::PrintLineAtIndex(int Index, std::string DataToPrint)
{
	if (Index > CurrentLineIndex)
	{
		return;
	}
	std::lock_guard<std::mutex> Lock(PrintMutex);
	int IndexBeforePrint = CurrentLineIndex;
	for (size_t i = Index; i < IndexBeforePrint; i++)
	{
		//vi ska göra det här skillnaden i antal ggr
		std::cout << "\x1b[A";
	}
	std::cout << "\r";
	//först clearar vi linen
	std::cout << "\x1b[K";
	std::cout << DataToPrint;
	for (size_t i = Index; i < IndexBeforePrint; i++)
	{
		//vi ska göra det här skillnaden i antal ggr
		std::cout << std::endl;
	}
	//hoppar vår grej fram antal steg som vi har i vår 
	//nu är vi på raden vi började på, så vi tar och helt enkelt skriver ut raden igen
	std::cout << CurrentInput;
}
int MrBoboChat::GetCurrentLineIndex()
{
	return(CurrentLineIndex);
}
char MrBoboChat::GetCharInput()
{
#ifdef WIN32
	return(_getch());
#elif __linux__
	//annat förslag
	/*
	struct termios oldt,
	newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	printf("%c", ch);
	return ch;
	*/
	///*
	char buf = 0;
	struct termios old = { 0 };
	fflush(stdout);
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	//old.c_lflag |= ICANON;
	//old.c_lflag |= ECHO;
	//if (tcsetattr(0, TCSADRAIN, &old) < 0)
		//perror("tcsetattr ~ICANON");
	//printf("%c", buf);
	return buf;
	//*/
#endif
}
void MrBoboChat::InitializeObject()
{
	MBSockets::Init();
	LoadConfig();
	MSListenThread = std::thread(MainSocket_Listener, this);
	MSSendThread = std::thread(MainSocket_Sender, this);
}
int MrBoboChat::AddMainSockPipe(std::string Filter, MainSockPipe* PipeToAdd)
{
	std::lock_guard<std::mutex> Lock(MainSockListenMutex);
	if (MainSockPipesMap.find(Filter) == MainSockPipesMap.end())
	{
		MainSockPipesMap[Filter] = PipeToAdd;
	}
	else
	{
		return(1);
	}
}
bool MrBoboChat::PortIsAvailable(int PortToCheck)
{
	//MBSockets::UDPSocket SocketToTest("127.0.0.1", std::to_string(PortToCheck), MBSockets::TraversalProtocol::TCP);
	std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
	if (UsedPorts.find(PortToCheck) == UsedPorts.end())
	{
		//FirstAvailablePort += 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
int MrBoboChat::RemoveMainSockPipe(std::string Filter)
{
	std::lock_guard<std::mutex> Lock(MainSockListenMutex);
	MainSockPipesMap.erase(Filter);
	return(0);
}
int MrBoboChat::AddConnection(MBChatConnection* NewConnection)
{
	std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
	UsedPorts[NewConnection->ConnectionPort] = true;
	NewConnection->ConnectionHandle = LastConnectionNumber + 1;
	LastConnectionNumber += 1;
	ConnectionMap[NewConnection->ConnectionHandle] = NewConnection;
	ActiveConnections.push_back(NewConnection);
	return(0);
}
int MrBoboChat::SendDataFromMainSocket(MainSockDataToSendStruct DataToSend)
{
	std::lock_guard<std::mutex> Lock(MainSockSendMutex);
	MainSocketDataToSend.push_back(DataToSend);
	MainSocketSendConditional.notify_one();
	return(0);
}

void MrBoboChat::ParseInput(std::string StringToParse)
{
	if (StringToParse[0] == '/')
	{
		//vi gör ett speciall commando
		std::vector<std::string> CommandWithArguments = Split(StringToParse, " ");
		if (CommandList.find(CommandWithArguments[0]) != CommandList.end())
		{
			CommandList[CommandWithArguments[0]](CommandWithArguments, this);
		}
		else
		{
			PrintLine(CommandWithArguments[0] + " is an unrecognised command");
		}
	}
	else
	{
		if (ActiveConnectionNumber != -1)
		{
			MBError SendError = SendDataToConnection(StringToParse, ActiveConnectionNumber);
			if (!SendError)
			{
				PrintLine(SendError.ErrorMessage);
			}
		}
	}
}
MBError MrBoboChat::SendDataToConnection(std::string Data, int ConnectionHandle)
{
	MBError ReturnError(true);
	if (ConnectionHandle == -1)
	{
		ReturnError.Type = MBErrorType::Error;
		ReturnError.ErrorMessage = "Invalid connection handle";
		return(ReturnError);
	}
	MBChatConnection* RecievingConnection;
	{
		std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
		RecievingConnection = ConnectionMap[ActiveConnectionNumber];
	}
	if (RecievingConnection == nullptr)
	{
		ReturnError.Type = MBErrorType::Error;
		ReturnError.ErrorMessage = "Invalid connection handle";
		return(ReturnError);
	}
	std::lock_guard<std::mutex> Lock(RecievingConnection->ConnectionMutex);
	RecievingConnection->ConnectionInput.push_back(Data);
	RecievingConnection->RecievedInput = true;
	return(ReturnError);
}
MBError MrBoboChat::SendDataCompletely(MBSockets::UDPSocket* SocketToUse, std::string DataToSend, std::string HostAdress, int PortNumber, MBChatConnection* AssociatedConnectionObject)
{
	//ifall vi har en nullptr till connectionen är det samma sak som att datan är okrypterad
	MBError ErrorToReturn(true);
	SocketToUse->UDPSendData(DataToSend, HostAdress, PortNumber);
	return(ErrorToReturn);
}
MBError MrBoboChat::GetNextCompleteTransmission(MBSockets::UDPSocket* SocketToUse, std::string& OutString, MBChatConnection* AssociatedConnectionObject)
{
	//ifall vi har en nullptr till connectionen är det samma sak som att datan är okrypterad
	MBError ErrorToReturn(true);
	OutString = SocketToUse->UDPGetData();
	return(ErrorToReturn);
}
void MrBoboChat::LoadConfig()
{
	std::lock_guard<std::mutex> Lock(ConfigFileMutex);
	if (std::filesystem::exists("MBConfig"))
	{
		//den existarar, vi laddar in den, annars så skapar vi en ny med nya standard värden
		//vi antar att varje rad är 255 karaktärer lång
		std::fstream ConfigFile("MBConfig");
		std::string LineData;
		std::getline(ConfigFile, LineData, '\n');
		if (ConfigFile.eof() || ConfigFile.fail())
		{
			PrintLine(ConfigFile.eof() + " " + ConfigFile.fail());
		}
		Username = std::string(LineData.substr(8).c_str());
		std::getline(ConfigFile, LineData);
		ExternalIp = std::string(LineData.substr(6).c_str());
		ConfigFile.close();
	}
	else
	{
		//vi skapar en ny config fil med standard datan
		std::ofstream ConcfigFile("MBConfig", std::ios::binary | std::ios::out);
		if (ConcfigFile.good())
		{
			std::string StandardUsername = "UsrName:Default" + std::string(ConfigfileLineSize - 1 - 15, 0) + '\n';
			std::string StandardExtIp = "ExtIp:127.0.0.1" + std::string(ConfigfileLineSize - 1 - 15, 0) + '\n';
			ConcfigFile.write(StandardUsername.c_str(), StandardUsername.size());
			ConcfigFile.write(StandardExtIp.c_str(), StandardExtIp.size());
		}
		else
		{
			PrintLine("Error reading config file. Standard variables not set");
		}
		ConcfigFile.close();
	}
}
void MrBoboChat::ChangeConfig(std::string ConfigToChange, std::string& ParameterData)
{
	if (!std::filesystem::exists("MBConfig"))
	{
		LoadConfig();
	}
	std::lock_guard<std::mutex> Lock(ConfigFileMutex);
	std::fstream ConfigFile("MBConfig", std::ios::binary | std::ios::out | std::ios::in);
	if (ConfigFile.good())
	{
		std::string LineData;
		std::getline(ConfigFile, LineData);
		int CurrentLine = 0;
		while (LineData.substr(0, ConfigToChange.size()) != ConfigToChange)
		{
			CurrentLine += 1;
			std::getline(ConfigFile, LineData);
		}
		ConfigFile.seekp(CurrentLine * ConfigfileLineSize);
		std::string LineToWrite = "";
		LineToWrite += ConfigToChange + ":" + ParameterData;
		LineToWrite += std::string(ConfigfileLineSize - LineToWrite.size() - 1, 0) + "\n";
		ConfigFile.write(LineToWrite.c_str(), LineToWrite.size());
	}
	else
	{
		PrintLine("Error loading file");
	}
}
std::string MrBoboChat::GetExtIp()
{
	std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
	return(ExternalIp);
}
void MrBoboChat::PrintLine(std::string LineToPrint)
{
	PrintString(LineToPrint + "\n");
}
void MrBoboChat::PrintString(std::string StringToPrint)
{
	std::lock_guard<std::mutex> Lock(PrintMutex);
	//tar bort all data usern har just nu
	for (int i = 0; i < CurrentInput.size(); i++)
	{
		std::cout << "\b \b";
	}
	//lägger till i vår newline nummer för varje newline i den
	for (size_t i = 0; i < StringToPrint.length(); i++)
	{
		if (StringToPrint[i] == '\n')
		{
			CurrentLineIndex += 1;
		}
	}
	std::cout << StringToPrint;
	std::cout << CurrentInput;
	std::flush(std::cout);
}
std::string MrBoboChat::GetUsrName()
{
	std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
	return(Username);
}
int MrBoboChat::GetCurrentConnection()
{
	std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
	return(ActiveConnectionNumber);
}
MrBoboChat::MrBoboChat()
{
	MainSocket.Bind(StandardInitiationPort);
	InitializeObject();
	PrintLine("Welcome to MrBoboChatt!");
	PrintLine("Enter /help to get a list of commands");
	MBChatConnection* NewConnection;
	//CreateConnection("127.0.0.1", this, &NewConnection,true);
	//NewConnection->EstablishSecureConnection();
	//NewConnection1->PeerIPAddress = "127.0.0.1";
	//NewConnection2->PeerIPAddress = "127.0.0.1";
	//inline void MBCSendFile_MainLopp(MBChatConnection * AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength)
	//std::thread Sendthread(MBCSendFile_MainLopp, NewConnection, "LinusApproves.png", false, 56181);
	//std::thread RecieveThread(MBCSendFile_MainLopp, NewConnection, "RecievedLinus.png", true, 56181);
	//Sendthread.join();
	//RecieveThread.join();
}
MrBoboChat::~MrBoboChat()
{

}
void MrBoboChat::MainLoop()
{
	//spawnar alla threads, och passar sedan enbart inputen till motsvarande ställen
	while (true)
	{
		char NewInput = GetCharInput();
		//std::cout << HexEncodeByte(NewInput);
		//continue;


		if (NewInput == '\r' || NewInput == '\n')
		{
			std::cout << NewInput;
			if (InitiatingCancelableInput)
			{
				InitiatingCancelableInput = false;
				continue;
			}
			std::string CurrentInputCopy = CurrentInput;
			{
				CurrentInput = "";
				if (CurrentInput[0] != '/')
				{
					PrintLine(ANSI::GREEN + "[" + Username + "] " + ANSI::RESET + CurrentInputCopy);
				}
				else
				{
					PrintLine(CurrentInputCopy);
				}
			}
			ParseInput(CurrentInputCopy);
		}
		else if (NewInput == '\b' || NewInput == 0x7f)
		{
			//PrintString(" ");
			std::cout << NewInput;
			if (InitiatingCancelableInput)
			{
				continue;
			}
			if (NewInput == 0x7f)
			{
				printf("\b");
			}
			CurrentInput = CurrentInput.substr(0, CurrentInput.size() - 1);
			{
				//std::lock_guard<std::mutex> Lock(PrintMutex);
				printf(" \b");
			}
		}
		else
		{
			std::cout << NewInput;
			if (InitiatingCancelableInput)
			{
				continue;
			}
			CurrentInput += NewInput;
		}
		/*
		std::string StringToParse;
		std::getline(std::cin, StringToParse);
		ParseInput(StringToParse);
		*/
	}
}

void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength)
{
	/*
	MBChatConnection* NewConnection;
	std::string ActiveConnectionIp = "";
	{
		MBChatConnection* ActiveConnection;
		{
			std::lock_guard<std::mutex> Lock(AssociatedChatObject->GeneralResourceMutex);
			ActiveConnection = AssociatedChatObject->ConnectionMap[AssociatedChatObject->ActiveConnectionNumber];
		}
		if (ActiveConnection != nullptr)
		{
			std::lock_guard<std::mutex> Lock(ActiveConnection->ConnectionMutex);
			ActiveConnectionIp = ActiveConnection->PeerIPAddress;
		}
	}
	//nu skapar vi en ny connection som får handla skickandet av filer
	if (CreateConnection(ActiveConnectionIp, AssociatedChatObject, &NewConnection))
	{
		//NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, )
	}
	else
	{
		AssociatedChatObject->PrintLine("Error occured in sendfile when initiating connection. File transfer aborted.");
	}
	*/
	if (Recieving)
	{
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("Started recievieng thread.");
		std::string HandshakeData = AssociatedConnectionObject->GetData();
		//vi skickar data tills vi skickar
		std::fstream NewFile(FileToSendOrRecieve, std::ios::binary | std::ios::out);
		int DataRecieved = 0;
		int TimeoutCount = 0;
		int TimoutMax = 15;
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("File to recieve size = " + std::to_string(RecievedOrSentDataLength));
		while (DataRecieved < RecievedOrSentDataLength)
		{
			std::string NewData = AssociatedConnectionObject->GetData();
			if (NewData != "")
			{
				NewFile << NewData;
				DataRecieved += NewData.size();
				TimeoutCount = 0;
			}
			else
			{
				TimeoutCount += 1;
				if (TimeoutCount > TimoutMax)
				{
					break;
				}
			}
		}
		NewFile.close();
	}
	else
	{
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("Started sending thread.");
		AssociatedConnectionObject->SendData("Handshake");
		int ExtractedData = 0;
		std::ifstream FileToSend(FileToSendOrRecieve, std::ios::in | std::ios::binary | std::ios::out);
		if (!FileToSend.is_open())
		{
			//error handling
			//int Hej = 123;
		}
		if (!FileToSend.good())
		{
			//int Hej = 123;
		}
		int DataChunkSize = 1500;
		if (RecievedOrSentDataLength < DataChunkSize)
		{
			DataChunkSize = RecievedOrSentDataLength;
		}
		bool DataSentSuccesfully = true;
		while (ExtractedData < RecievedOrSentDataLength)
		{
			char* Data;
			if (DataSentSuccesfully)
			{
				Data = (char*)malloc(DataChunkSize);
				FileToSend.read(Data, DataChunkSize);
			}
			MBError SendError(true);
			std::string DataToSend;
			if (ExtractedData > RecievedOrSentDataLength)
			{
				DataToSend = std::string(Data, RecievedOrSentDataLength % DataChunkSize);
				//SendError = AssociatedConnectionObject->SendData();
			}
			else
			{
				DataToSend = std::string(Data, DataChunkSize);
				//SendError = AssociatedConnectionObject->SendData(std::string(Data, DataChunkSize));
			}
			SendError = AssociatedConnectionObject->SendData(DataToSend);
			if (SendError)
			{
				ExtractedData += DataChunkSize;
				free(Data);
				DataSentSuccesfully = true;
			}
			else
			{
				DataSentSuccesfully = false;
			}
		}
		FileToSend.close();
	}
	if (Recieving)
	{
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("Succesfully saved file.");
	}
	if (!Recieving)
	{
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("Succesfully tramsfered file.");
	}
}
void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	if (CommandWithArguments.size() >= 2)
	{
		//börjar med att establisha en ny connection med den aktiva chatten
		if (AssociatedChatObject->ActiveConnectionNumber == -1)
		{
			AssociatedChatObject->PrintLine("sendfile requires you to be in an active chat.");
		}
		else
		{
			MBError RecievedError = AssociatedChatObject->SendDataToConnection(std::string(1, 1) + "sendfile " + CommandWithArguments[1], AssociatedChatObject->ActiveConnectionNumber);
			if (!RecievedError)
			{
				AssociatedChatObject->PrintLine(RecievedError.ErrorMessage);
			}
		}
	}
	else
	{
		AssociatedChatObject->PrintLine("sendfile requires a filepath");
	}
}
void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	//std::cout << "(* denotes optional argument)\n";
	AssociatedChatObject->PrintLine("help - displays a list of commands");
	AssociatedChatObject->PrintLine("startcon (ipadress) - Starts a connection with specified ip adress");
	AssociatedChatObject->PrintLine("setextip (ipadress) - Sets your external ip. Correct external ip is required in order to start a connection.");
	AssociatedChatObject->PrintLine("setuser (username) - Sets your username used in chats. Change is only visible on new connections.");
	AssociatedChatObject->PrintLine("sendfile (filename) - Transfer a copy of the file to the peer in a chatt as specified by the relative path to this executable or absolute path.");
	//std::cout << "activecon - lists current active connections by IP or username\n";
}
void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	std::lock_guard<std::mutex> Lock(AssociatedChatObject->ConfigFileMutex);
	std::string LineData;
	std::fstream ConfigFile("MBConfig", std::ios::binary | std::ios::out | std::ios::in);
	if (ConfigFile.good())
	{
		while (std::getline(ConfigFile, LineData))
		{
			AssociatedChatObject->PrintLine(LineData);
		}
	}
	else
	{
		AssociatedChatObject->PrintLine("Error loading config file");
	}
}
void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	if (CommandWithArguments.size() >= 2)
	{
		{
			std::lock_guard<std::mutex> Lock(AssociatedChatObject->GeneralResourceMutex);
			AssociatedChatObject->Username = CommandWithArguments[1];
		}
		AssociatedChatObject->ChangeConfig("UsrName", CommandWithArguments[1]);
		AssociatedChatObject->PrintLine("Username set to " + CommandWithArguments[1]);
	}
	else
	{
		AssociatedChatObject->PrintLine("Command requires a username");
	}
}
void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	if (CommandWithArguments.size() < 2)
	{
		AssociatedChatObject->PrintLine("Command requires an IP adress");
	}
	else
	{
		AssociatedChatObject->ExternalIp = CommandWithArguments[1];
		AssociatedChatObject->ChangeConfig("ExtIp", CommandWithArguments[1]);
		AssociatedChatObject->PrintLine("Ip succsefully set to " + CommandWithArguments[1]);
	}
}
void MainSocket_Sender(MrBoboChat* AssociatedChatObject)
{
	while (true)
	{
		MainSockDataToSendStruct DataToSend;
		{
			std::unique_lock<std::mutex> Lock(AssociatedChatObject->MainSockSendMutex);
			while (true)
			{
				AssociatedChatObject->MainSocketSendConditional.wait(Lock);
				if (AssociatedChatObject->MainSocketDataToSend.size() > 0)
				{
					break;
				}
			}
			DataToSend = AssociatedChatObject->MainSocketDataToSend.front();
			AssociatedChatObject->MainSocketDataToSend.pop_front();
		}
		AssociatedChatObject->MainSocket.UDPSendData(DataToSend.Data, DataToSend.Host, DataToSend.Port);
	}
}
void MainSocket_Listener(MrBoboChat* AssociatedChatObject)
{
	while (true)
	{
		std::string NextRecord = AssociatedChatObject->MainSocket.UDPGetData();
		//när vi väl fått datan så pipar vi den till motsvarande pipe
		std::lock_guard<std::mutex> Lock(AssociatedChatObject->MainSockListenMutex);
		//vi antar att alla meddelanden är av typen MBCPInitationMess
		MBCInitatieConnectionMessage RecievedMessage(NextRecord);
		if (AssociatedChatObject->MainSockPipesMap.find(RecievedMessage.SendAdress) != AssociatedChatObject->MainSockPipesMap.end())
		{
			MainSockPipe* Pipe = AssociatedChatObject->MainSockPipesMap[RecievedMessage.SendAdress];
			//här antar vi att pipen inte anvönder sin mutex
			Pipe->RecievedRecords.push_back(NextRecord);
			Pipe->RecordAdded = true;
			//här tror jag felet från koden igår är, verkar som oskar lyckades skicka grejer fast det inte pipades korrekt
		}
		else
		{
			//:(
		}
	}
}
long long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}
void ChatMainFunction(MBChatConnection* AssociatedConnectionObject)
{
	//skickar handshake meddelanden så vi vet att vi holepunchat
	//MBSockets::UDPSocket SocketToUse = MBSockets::UDPSocket(AssociatedConnectionObject->PeerIPAddress, std::to_string(AssociatedConnectionObject->ConnectionPort), MBSockets::TraversalProtocol::TCP);
	//SocketToUse.Bind(AssociatedConnectionObject->ConnectionPort);
	AssociatedConnectionObject->AssociatedChatObject->PrintLine("Port for connection is " + std::to_string(AssociatedConnectionObject->ConnectionPort));
	std::mutex MainFuncMutex;
	std::atomic<bool> HandshakeComplete{ false };
	//std::atomic<bool> StopThreads{ false };
	std::string LocalUsername = AssociatedConnectionObject->AssociatedChatObject->GetUsrName();
	std::atomic<bool> RecievedMessage{ false };
	std::vector<std::string> Messages = {};
	std::string PeerUserName = "";
	int LastReadMessage = 0;
	int LastRecievedMessageIndex = -1;
	clock_t Timer = clock();
	float SendIntervall = 0.05;
	std::thread ListenThread = std::thread(ChatMainFunction_Listener, AssociatedConnectionObject, &Messages, &RecievedMessage, &MainFuncMutex);
	do
	{
		if ((clock() - Timer) / float(CLOCKS_PER_SEC) > SendIntervall)
		{
			Timer = clock();
			MBCConfirmConnectionMessage MessageToSend = MBCConfirmConnectionMessage();
			MessageToSend.MBCMessageType = MBCPMessageType::AcceptConnection;
			MessageToSend.RecordNumber = 0;
			MessageToSend.UserName = AssociatedConnectionObject->AssociatedChatObject->GetUsrName();
			AssociatedConnectionObject->SendData(MessageToSend.ToString());
			if (RecievedMessage)
			{
				RecievedMessage = false;
				MBCConfirmConnectionMessage RecievedMessage;
				{
					std::lock_guard<std::mutex> Lock(MainFuncMutex);
					RecievedMessage = MBCConfirmConnectionMessage(Messages.back());
					PeerUserName = RecievedMessage.UserName;
				}
				AssociatedConnectionObject->SendData(MessageToSend.ToString());
				HandshakeComplete = true;
			}
		}
	} while (!HandshakeComplete);
	bool PeerSentFileTransferRequest = false;
	bool SentFileTransferRequest = false;
	bool AcceptedFileTransfer = false;
	bool EnteredFileName = false;
	std::string FileLocalWantedToSend = "";
	long long SizeOfLocalFile = 0;
	long long SizeOfPeerFile = 0;
	while (!AssociatedConnectionObject->ShouldStop)
	{
		if (RecievedMessage)
		{
			RecievedMessage = false;
			if (AssociatedConnectionObject->AssociatedChatObject->GetCurrentConnection() == AssociatedConnectionObject->ConnectionHandle)
			{
				int NumberOfMessages;
				{
					std::lock_guard<std::mutex> Lock(MainFuncMutex);
					NumberOfMessages = Messages.size() - LastReadMessage;
				}
				for (size_t i = 0; i < NumberOfMessages; i++)
				{
					std::lock_guard<std::mutex> Lock(MainFuncMutex);
					MBCMessage Test(Messages[LastReadMessage]);
					std::string Data = Messages[LastReadMessage];
					//if (MBCMessage(Messages[LastReadMessage]).MBCMessageType != MBCPMessageType::AcceptConnection)
					if (MBCMessage(Data).MBCMessageType != MBCPMessageType::AcceptConnection)
					{
						if (Data[0] != 0x01)
						{
							AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + "[" + PeerUserName + "] " + ANSI::RESET + Data);
						}
						else if (Data.substr(0, 10) == "\1sendfile ")
						{
							AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET + " Wants to send you a file named " + Split(Data.substr(10), "/").back() + "\n Do you accept? [y/n]");
							SizeOfPeerFile = std::stoi(Split(Data, " ").back());
							PeerSentFileTransferRequest = true;
						}
						else
						{
							if (SentFileTransferRequest)
							{
								if (Data == "\1sendfileaccepted")
								{
									//skapa tråden jada jada
									AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET + " Accepted the file transfer.");
									MBChatConnection* NewConnection;
									CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection,false);
									//AssociatedConnectionObject->SendData("\1sendfileaccepted");
									NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, NewConnection, FileLocalWantedToSend, false, SizeOfLocalFile);
									SentFileTransferRequest = false;
								}
								if (Data == "\1sendfiledeclined")
								{
									//säg att vi inte accepterar 
									AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET + " Declined the file transfer.");
									SentFileTransferRequest = false;
								}
							}
						}
					}
					LastRecievedMessageIndex += 1;
					LastReadMessage += 1;
				}
			}
		}
		if (AssociatedConnectionObject->RecievedInput)
		{
			AssociatedConnectionObject->RecievedInput = false;
			for (size_t i = 0; i < AssociatedConnectionObject->ConnectionInput.size(); i++)
			{
				std::string Data;
				{
					std::lock_guard<std::mutex> Lock(AssociatedConnectionObject->ConnectionMutex);
					if (AssociatedConnectionObject->ConnectionInput.size() == 0)
					{
						continue;
					}
					Data = AssociatedConnectionObject->ConnectionInput.front();
					AssociatedConnectionObject->ConnectionInput.pop_front();
				}
				if (AcceptedFileTransfer)
				{
					//resettar allt, skapar objektet
					MBChatConnection* NewConnection;
					CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection,false);
					AssociatedConnectionObject->SendData("\1sendfileaccepted");
					NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, NewConnection, Data, true, SizeOfPeerFile);
					PeerSentFileTransferRequest = false;
					AcceptedFileTransfer = false;
					EnteredFileName = false;
				}
				else if (PeerSentFileTransferRequest)
				{
					if (Data == "y" || Data == "Y")
					{
						//vi skickar att vi accepterar, och gör sedan funktionen som skickar datan
						AssociatedConnectionObject->AssociatedChatObject->PrintLine("Enter filename:");
						AssociatedConnectionObject->SendData("\1sendfileaccepted");
						AcceptedFileTransfer = true;
					}
					else if (Data == "n" || Data == "N")
					{
						AssociatedConnectionObject->SendData("\1sendfiledeclined");
						PeerSentFileTransferRequest = false;
						AcceptedFileTransfer = false;
						EnteredFileName = false;
					}
					else
					{
						AssociatedConnectionObject->AssociatedChatObject->PrintLine("Input y or n");
					}
				}
				else if (Data[0] != 0x01)
				{
					AssociatedConnectionObject->SendData(Data);
					std::lock_guard<std::mutex> Lock(AssociatedConnectionObject->ConnectionMutex);
					//AssociatedConnectionObject->ConnectionInput.pop_front();
				}
				else
				{
					//vi vet att det här är ett special kommando som vi fått från någonstans. I detta fall ska vi göra ett kommandop
					if (Data.substr(1, 9) == "sendfile ")
					{
						std::string FilePath = Data.substr(10);
						std::string FileName = Split(FilePath, "/").back();
						FileLocalWantedToSend = FilePath;
						//vi skickar med file data längden här
						long long FileSize = GetFileSize(FilePath);
						AssociatedConnectionObject->SendData(Data.substr(0, 10) + FileName + " " + std::to_string(FileSize));
						SentFileTransferRequest = true;
						SizeOfLocalFile = FileSize;
					}
					else if (SentFileTransferRequest)
					{
						//makar inte mycket sense, detta borde vi ju få från vår peer
						if (Data == "\1sendfileaccepted")
						{
							//skapa tråden jada jada
							AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET + " Accepted the file transfer.");
							MBChatConnection* NewConnection;
							CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection,false);
							AssociatedConnectionObject->SendData("\1sendfileaccepted");
							NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, NewConnection, FileLocalWantedToSend, false, SizeOfLocalFile);
							SentFileTransferRequest = false;
						}
						if (Data == "\1sendfiledeclined")
						{
							//säg att vi inte accepterar 
							AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET + " Declined the file transfer.");
							SentFileTransferRequest = false;
						}
					}
				}
			}
		}
	}
	//vi borde stoppa
	ListenThread.join();
}
void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	if (CommandWithArguments.size() < 2)
	{
		AssociatedChatObject->PrintLine("Command requires an IP adress");
	}
	else
	{
		MBChatConnection* NewConnection;
		CreateConnection(CommandWithArguments[1], AssociatedChatObject, &NewConnection,true);
		//std::thread ConnectionThread = std::thread(CreateConnection, CommandWithArguments[1], AssociatedChatObject, &NewConnection);
		//ConnectionThread.join();
		if (NewConnection != nullptr)
		{
			MBError Hej = NewConnection->EstablishSecureConnection();
			NewConnection->ConnectionThread = new std::thread(ChatMainFunction, NewConnection);
			AssociatedChatObject->ActiveConnectionNumber = NewConnection->ConnectionHandle;
		}
		else
		{
			AssociatedChatObject->PrintLine(ANSI::RED + "Error occured when trying to start connection with ip " + ANSI::BLUE + CommandWithArguments[1] + ANSI::RESET);
		}
	}
}
MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection, bool OnMainThread)
{
	MBChatConnection* NewConnection;
	MBError ErrorReturnMessage = MBError(true);
	MBCLineObject InitiatingConnectionLine = MBCLineObject(AssociatedChatObject);
	InitiatingConnectionLine.SetLineData("Initiating connection     (Press enter to cancel)");
	//AssociatedChatObject->PrintLine("Press Enter to cancel");
	//AssociatedChatObject->PrintLine("Initiating connection...\nPress Enter to cancel\n");
	std::atomic<bool> UserCanceled(false);
	std::atomic<bool> StopReadThread(false);
	std::thread ReadUserInputThread(ListenOnInitiationUserInput, &UserCanceled, &StopReadThread,AssociatedChatObject,OnMainThread);

	int PeerFirstMessagePort = -1;
	int LocalFirstMessagePort = 2700;
	//std::cout << "First message port is " << LocalFirstMessagePort << std::endl;
	//int LastPeerPortSuggestion = -1;
	//den under sätter vi på något stt
	int LastLocalPortSuggestion = 5400;
	MainSockPipe Pipe;
	AssociatedChatObject->AddMainSockPipe(IPAdress, &Pipe);
	MBCInitatieConnectionMessage MessageToSend;
	MessageToSend.MBCMessageType = MBCPMessageType::InitiateConnection;
	MessageToSend.InitationMessageType = MBCPInitatioMessageType::FirstMessage;
	MessageToSend.PortSuggestion = LocalFirstMessagePort;
	//adressen vi skickar från, vet inte hur man får den externa ipn
	MessageToSend.SendAdress = AssociatedChatObject->ExternalIp;
	bool FirstErrorReported = false;
	float SendRecordDelay = 0.5;
	float UpdateDotDelay = 1;
	if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
	{
		FirstErrorReported = true;
		AssociatedChatObject->PrintLine("Felaktig port assignades i början");
	}
	int DotIndex = 21;
	while (true)
	{
		//std::unique_lock<std::mutex> WaitLock(WaitMutex);
		//StartConWaitConditional.wait(WaitLock);
		//klockan som sköter hur vi skickar record
		clock_t SendRecordTimer = clock();
		clock_t UpdateDotTimer = clock();
		while (Pipe.RecordAdded == false && UserCanceled == false)
		{
			//kollar om vi ska uppdatera initiating connection grejen
			if ((clock() - UpdateDotTimer) / float(CLOCKS_PER_SEC) > UpdateDotDelay)
			{
				std::string CurrentLine = InitiatingConnectionLine.GetLineData();
				std::string ReplacedLine = ReplaceAll(CurrentLine, ".", "");
				if(ReplacedLine.size()+3 == CurrentLine.size())
				{
					InitiatingConnectionLine.SetLineData("Initiating connection     (Press enter to cancel)");
					DotIndex = 21;
				}
				else
				{
					std::string NewInitatingConnectionLine = CurrentLine;
					NewInitatingConnectionLine[DotIndex] = '.';
					DotIndex += 1;
					InitiatingConnectionLine.SetLineData(NewInitatingConnectionLine);
				}
				UpdateDotTimer = clock();
			}
			if ((clock() - SendRecordTimer) / float(CLOCKS_PER_SEC) > SendRecordDelay)
			{
				//std::cout << (clock() - SendRecordTimer) / float(CLOCKS_PER_SEC) << std::endl;
				SendRecordTimer = clock();
				MainSockDataToSendStruct DataToSend;
				DataToSend.Data = MessageToSend.ToString();
				DataToSend.Host = IPAdress;
				DataToSend.Port = AssociatedChatObject->StandardInitiationPort;
				AssociatedChatObject->SendDataFromMainSocket(DataToSend);
				//nu ändrar vi den senaste datan vi ska skicka
			}
		}
		Pipe.RecordAdded = false;
		if (UserCanceled)
		{
			break;
		}
		//Vi vet att vi faktiskt fick datan från socketen, så parsar in den i en struct som innehåller datan
		//MBCInitatieConnectionMessage PeerMessage(Pipe.RecievedRecords.back());
		MBCInitatieConnectionMessage PeerMessage;
		{
			std::lock_guard<std::mutex> Lock(Pipe.PipeResourceMutex);
			PeerMessage = MBCInitatieConnectionMessage(Pipe.RecievedRecords.back());
		}
		if (PeerMessage.InitationMessageType == MBCPInitatioMessageType::FirstMessage)
		{
			//det första meddelandet som går igenom UDP hål punchandet
			//vi sätter första porten till det den skickade
			PeerFirstMessagePort = PeerMessage.PortSuggestion;
			int FirstPortSuggestion = LocalFirstMessagePort + PeerFirstMessagePort;
			if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
			{
				FirstErrorReported = true;
				FirstPortSuggestion += 1;
			}
			MessageToSend.InitationMessageType = MBCPInitatioMessageType::AcceptPort;
			MessageToSend.PortSuggestion = FirstPortSuggestion;
			//sätter den senaste portsuggestionen vi skickade
			LastLocalPortSuggestion = FirstPortSuggestion;
			MainSockDataToSendStruct DataToSend;
			DataToSend.Data = MessageToSend.ToString();
			DataToSend.Host = IPAdress;
			DataToSend.Port = AssociatedChatObject->StandardInitiationPort;
			AssociatedChatObject->SendDataFromMainSocket(DataToSend);
			if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
			{
				FirstErrorReported = true;
				AssociatedChatObject->PrintLine("Blev fel efter firstmessage port suggestionen");
			}
		}
		else if (PeerMessage.InitationMessageType == MBCPInitatioMessageType::AcceptPort)
		{
			if (AssociatedChatObject->PortIsAvailable(PeerMessage.PortSuggestion))
			{
				//båda parterna har kommit överents om en port som vi kan köra på, vi skapar då den andra threaden som faktiskt är connection threaden
				NewConnection = new MBChatConnection(IPAdress, PeerMessage.PortSuggestion);
				NewConnection->ConnectionPort = PeerMessage.PortSuggestion;
				NewConnection->AssociatedChatObject = AssociatedChatObject;
				NewConnection->PeerIPAddress = IPAdress;
				//NewConnection->ConnectionThread = new std::thread(ChatMainFunction, NewConnection);
				MessageToSend.InitationMessageType = MBCPInitatioMessageType::AcceptPort;
				MessageToSend.PortSuggestion = PeerMessage.PortSuggestion;
				MainSockDataToSendStruct DataToSend;
				DataToSend.Data = MessageToSend.ToString();
				DataToSend.Host = IPAdress;
				DataToSend.Port = AssociatedChatObject->StandardInitiationPort;
				AssociatedChatObject->SendDataFromMainSocket(DataToSend);
				if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
				{
					FirstErrorReported = true;
					AssociatedChatObject->PrintLine("Blev fel då peer:en skickade en felaktig port");
				}
				StopReadThread = true;
				break;
			}
			else
			{
				//vi kan inte använda denna port så vi tar ock skickar ett annart förslag
				int NewPortSuggestion = PeerMessage.PortSuggestion + 1;
				while (!AssociatedChatObject->PortIsAvailable(NewPortSuggestion))
				{
					NewPortSuggestion += 1;
				}
				MessageToSend.InitationMessageType = MBCPInitatioMessageType::AcceptPort;
				MessageToSend.PortSuggestion = NewPortSuggestion;
				MainSockDataToSendStruct DataToSend;
				DataToSend.Data = MessageToSend.ToString();
				DataToSend.Host = IPAdress;
				DataToSend.Port = AssociatedChatObject->StandardInitiationPort;
				if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
				{
					FirstErrorReported = true;
					AssociatedChatObject->PrintLine("Blev då den föreslagna porten inte matchade");
				}
				AssociatedChatObject->SendDataFromMainSocket(DataToSend);
			}
		}
	}
	ReadUserInputThread.join();
	AssociatedChatObject->RemoveMainSockPipe(IPAdress);
	if (UserCanceled)
	{
		*OutConnection = nullptr;
		InitiatingConnectionLine.SetLineData("Cancelled connection to " + IPAdress);
		//delete NewConnection;
		ErrorReturnMessage.Type = MBErrorType::Error;
		return(ErrorReturnMessage);
	}
	else
	{
		//AssociatedChatObject->PrintLine("Connection port " + std::to_string(NewConnection->ConnectionPort));
		//AssociatedChatObject->PrintLine("Successfully connected to " + IPAdress);
		InitiatingConnectionLine.SetLineData("Successfully connected to " + IPAdress);
		AssociatedChatObject->AddConnection(NewConnection);
		//vi gör current connection till den vi addade
		*OutConnection = NewConnection;
		return(ErrorReturnMessage);
	}
}