#pragma once
#include <MrBoboSockets.h>
#include <MinaStringOperations.h>
#include <MrPostOGet/TLSHandler.h>
//forwards declaration of all classes

//olika depandancies
#ifdef WIN32
#include <conio.h>
#elif __linux__
#include <unistd.h>
#include <termios.h>
#endif
class MrBoboChat;
struct MainSockPipe
{
	std::atomic<bool> RecordAdded = { false };
	std::mutex PipeResourceMutex;
	std::vector<std::string> RecievedRecords = std::vector<std::string>(0);
};
inline void TestUDPFunc(std::string Adress, std::string Port, MBSockets::UDPSocket* SocketToUse)
{
	//MBSockets::UDPSocket Test(Adress, Port, MBSockets::TraversalProtocol::TCP);
	//Test.Bind(std::stoi(Port));
	while (true)
	{
		std::cout << SocketToUse->UDPGetData() << std::endl;
	}
}
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
	std::string BOLDBLACK =  "\033[1m\033[30m";    /* Bold Black */
	std::string BOLDRED    = "\033[1m\033[31m";    /* Bold Red */
	std::string BOLDGREEN  = "\033[1m\033[32m";    /* Bold Green */
	std::string BOLDYELLOW = "\033[1m\033[33m";    /* Bold Yellow */
	std::string BOLDBLUE   = "\033[1m\033[34m";    /* Bold Blue */
	std::string BOLDMAGENTA = "\033[1m\033[35m";     /* Bold Magenta */
	std::string BOLDCYAN  =  "\033[1m\033[36m";    /* Bold Cyan */
	std::string BOLDWHITE  = "\033[1m\033[37m";    /* Bold White */
};	
enum class MBErrorType : uint64_t
{
	OK,Error,Null
};
class MBError
{
public:
	MBErrorType Type = MBErrorType::Null;
	std::string ErrorMessage = "";
	operator bool() const { return(Type == MBErrorType::OK); }
	MBError(bool BoolInitialiser)
	{
		if (true)
		{
			Type = MBErrorType::OK;
		}
		else
		{
			Type = MBErrorType::Error;
		}
	}
};
inline void TestMainFunc()
{
	/*
	std::cout << "Welcome to MrBoboChat!\nType /help for a list of commands";
	std::cout << "Enter IP adress of peer:" << std::endl;
	std::string IPAdress;
	std::cin >> IPAdress;
	std::cout << "Enter port to send and recieve data to:\n";
	std::string Port;
	std::cin >> Port;
	std::mutex TestMutex;
	MBSockets::UDPSocket Test(IPAdress, Port, MBSockets::TraversalProtocol::TCP);
	std::thread TestThread(TestUDPFunc, IPAdress, Port, &Test);
	Test.Bind(std::stoi(Port));
	while (true)
	{
		std::string StringToSend;
		std::cin >> StringToSend;
		Test.UDPSendData(StringToSend, IPAdress, std::stoi(Port));
	}
	TestThread.join();
	*/
}
enum class MrBoboChatState : uint64_t
{
	Start
};
struct MBChatConnection
{
private:
	std::mutex PeerResponseMutex;
	std::condition_variable PeerResponseConditional;
	std::mutex PeerSendMutex;
	std::condition_variable PeerSendConditional;
	std::atomic<bool> RecievedResponseMessage{ false };
	std::atomic<bool> RecievedSendMessage{ false };
	std::deque<std::string> PeerResponseMessages = {};
	std::deque<std::string> PeerSendMessages = {};
	std::thread ConnectionListenThread;
	std::string GetNextResponseMessage()
	{
		//clock_t TimeOut = 1000;
		while (!RecievedResponseMessage)
		{
			std::unique_lock<std::mutex> Lock(PeerResponseMutex);
			PeerSendConditional.wait(Lock);
		}
		//fått meddelande, recieved response message är då nästa meddelande
		std::string ReturnString = "";
		{
			std::lock_guard<std::mutex> Lock(PeerResponseMutex);
			ReturnString = PeerResponseMessages.front();
			PeerResponseMessages.pop_front();
		}
		return(ReturnString);
	}
	std::string GetNextSendMessage()
	{
		//clock_t TimeOut = 1000;
		while (!RecievedSendMessage)
		{
			std::unique_lock<std::mutex> Lock(PeerSendMutex);
			PeerSendConditional.wait(Lock);
		}
		//fått meddelande, recieved response message är då nästa meddelande
		std::string ReturnString = "";
		{
			std::lock_guard<std::mutex> Lock(PeerSendMutex);
			ReturnString = PeerSendMessages.front();
			PeerSendMessages.pop_front();
		}
		return(ReturnString);
	}
public:
	int PrivateRecordNumber = 0;
	int PeerRecordNumber = 0;
	int ConnectionPort = -1;
	int ConnectionHandle = -1;

	std::mutex ConnectionMutex;
	std::string PeerIPAddress = "";
	MrBoboChat* AssociatedChatObject;
	std::thread* ConnectionThread;
	std::deque<std::string> ConnectionInput = {};

	MBError SendData(std::string DataToSend)
	{

	}
	std::string GetData()
	{

	}
	

	std::atomic<bool> ShouldStop{ false };
	std::atomic<bool> RecievedInput{ false };
};
enum class MBCPMessageType : uint8_t
{
	InitiateConnection, AcceptConnection,Null
};
class MBCMessage
{
protected:
	//variabel som underlättar parsing
	int ParseOffset = 0;
public:
	MBCPMessageType MBCMessageType = MBCPMessageType::Null;
	uint32_t RecordNumber = 0;
	MBCMessage(std::string StringToParse)
	{
		TLS1_2::NetWorkDataHandler DataExtracter(reinterpret_cast<const uint8_t*>(StringToParse.c_str()));
		MBCMessageType = MBCPMessageType(DataExtracter.Extract8());
		RecordNumber = DataExtracter.Extract32();
		ParseOffset = DataExtracter.GetPosition();
	}
	MBCMessage()
	{

	}
	virtual std::string ToString()
	{
		std::string ReturnString = "";
		ReturnString += char(MBCMessageType);
		for (int i = 3; i >=0; i--)
		{
			ReturnString += char((RecordNumber >> i * 8) % 256);
		}
		return(ReturnString);
	}
	~MBCMessage()
	{

	}
};
enum class MBCPInitatioMessageType : uint8_t
{
	FirstMessage,
	PortSuggestion,
	AcceptPort,
	Null
};
class MBCInitatieConnectionMessage : public MBCMessage
{
public:
	MBCPInitatioMessageType InitationMessageType = MBCPInitatioMessageType::Null;
	uint16_t PortSuggestion = 2700;
	std::string SendAdress = "";
	MBCInitatieConnectionMessage(std::string StringToParse) : MBCMessage(StringToParse)
	{
		uint32_t OffsetInString = ParseOffset;
		TLS1_2::NetWorkDataHandler DataExtracter(reinterpret_cast<const uint8_t*>(StringToParse.c_str()));
		DataExtracter.SetPosition(ParseOffset);
		InitationMessageType = MBCPInitatioMessageType(DataExtracter.Extract8());
		PortSuggestion = DataExtracter.Extract16();
		SendAdress = StringToParse.substr(DataExtracter.GetPosition());
		ParseOffset = DataExtracter.GetPosition();
	}
	std::string ToString() override
	{
		std::string ReturnValue = MBCMessage::ToString();
		ReturnValue += char(InitationMessageType);
		ReturnValue += char(PortSuggestion>>8);
		ReturnValue += char(PortSuggestion%256);
		ReturnValue += SendAdress;
		return(ReturnValue);
	}
	MBCInitatieConnectionMessage() : MBCMessage()
	{

	}
};
class MBCConfirmConnectionMessage : public MBCMessage
{
public:
	std::string UserName = "";
	MBCConfirmConnectionMessage(std::string StringToParse) : MBCMessage(StringToParse)
	{
		UserName = StringToParse.substr(ParseOffset);
	}
	MBCConfirmConnectionMessage()
	{

	}
	std::string ToString() override
	{
		std::string ReturnValue = MBCMessage::ToString();
		ReturnValue += UserName;
		return(ReturnValue);
	}
};
struct MainSockDataToSendStruct
{
	std::string Data = "";
	std::string Host = "";
	int Port = -1;
};
inline void ListenOnInitiationSocket(MBSockets::UDPSocket* SocketToListenOn,std::string* StringToModify,std::condition_variable* ConditionalToNotify, std::mutex* ResourceMutex)
{
	std::string RecievedDataFromSocket = SocketToListenOn->UDPGetData();
};
inline void ListenOnInitiationUserInput(std::atomic<bool>* BoolToModify,std::atomic<bool>* ShouldStop)
{
	while (!ShouldStop)
	{
		if (std::cin.rdbuf()->in_avail() > 0)
		{
			*BoolToModify = true;
			break;
		}
	}
};
inline void InitiationSendData(MBSockets::UDPSocket* SocketToSendDataOn, std::string* StringToModify, std::condition_variable* ConditionalToNotify,std::mutex* ResourceMutex)
{
	std::string DataToSend = "";
	while (true)
	{
		SocketToSendDataOn->SendData(DataToSend.data(), DataToSend.size());
	}
}
inline void ChatMainFunction_Listener(std::atomic<bool>* ShouldStop,MBSockets::UDPSocket*SocketToListenOn,std::vector<std::string>* RecievedMessages,std::atomic<bool>* RecievedMessage,std::mutex* MainFuncMutex)
{
	//std::cout << "Hej" << std::endl;
	while (!*ShouldStop)
	{
		std::string NextMessage = SocketToListenOn->UDPGetData();
		std::lock_guard<std::mutex> Lock(*MainFuncMutex);
		RecievedMessages->push_back(NextMessage);
		*RecievedMessage = true;
	}
	//std::cout << "Hej da" << std::endl;
}
void ChatMainFunction(MBChatConnection* AssociatedConnectionObject);
void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MainSocket_Sender(MrBoboChat* AssociatedChatObject);
void MainSocket_Listener(MrBoboChat* AssociatedChatObject);
void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection);
class MrBoboChat
{
	friend void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend void MainSocket_Listener(MrBoboChat* AssociatedChatObject);
	friend void MainSocket_Sender(MrBoboChat* AssociatedChatObject);
	friend void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
	friend MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection);

private:
	std::string CurrentInput = "";
	std::string ExternalIp = "";
	std::atomic<int> ActiveConnectionNumber = {-1};
	std::atomic<int> LastConnectionNumber = 0;
	const std::atomic<uint32_t> StandardInitiationPort = {2700};
	MBSockets::UDPSocket MainSocket = MBSockets::UDPSocket("127.0.0.1", std::to_string(StandardInitiationPort), MBSockets::TraversalProtocol::TCP);
	std::thread MSListenThread;
	std::thread MSSendThread;
	std::mutex MainSockListenMutex;
	std::unordered_map<std::string, MainSockPipe*> MainSockPipesMap = std::unordered_map<std::string, MainSockPipe*>(0);

	std::mutex MainSockSendMutex;
	std::deque<MainSockDataToSendStruct> MainSocketDataToSend = {};
	std::condition_variable MainSocketSendConditional;

	std::mutex ConfigFileMutex;
	const std::atomic<int> ConfigfileLineSize{ 255 };

	std::mutex GeneralResourceMutex;
	std::string Username = "Default";
	std::vector<MBChatConnection*> ActiveConnections = std::vector<MBChatConnection*>(0);
	std::unordered_map<int, MBChatConnection*> ConnectionMap{};

	std::mutex PrintMutex;

	std::unordered_map<std::string, void (*)(std::vector<std::string>,MrBoboChat*)> CommandList
	{
		{"/help",MrBoboChatHelp},
		{"/startcon",StartCon},
		{"/setextip",SetExtIp},
		{"/setuser",SetUsername},
		{"/viewconfig",ViewConfig},
		{"/sendfile",MBCSendFile}
	};

	char GetCharInput()
	{
#ifdef WIN32
		return(_getche());
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
		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;
		if (tcsetattr(0, TCSADRAIN, &old) < 0)
			perror("tcsetattr ~ICANON");
		printf("%c", buf);
		return buf;
		//*/
#endif
	}


	void InitializeObject()
	{
		MBSockets::Init();
		LoadConfig();
		MSListenThread = std::thread(MainSocket_Listener, this);
		MSSendThread = std::thread(MainSocket_Sender, this);
	}
	int AddMainSockPipe(std::string Filter,MainSockPipe* PipeToAdd)
	{
		std::lock_guard<std::mutex> Lock(MainSockListenMutex);
		if(MainSockPipesMap.find(Filter) == MainSockPipesMap.end())
		{
			MainSockPipesMap[Filter] = PipeToAdd;
		}
		else
		{
			return(1);
		}
	}
	bool PortIsAvailable(int PortToCheck)
	{
		return(true);
	}
	int RemoveMainSockPipe(std::string Filter)
	{
		std::lock_guard<std::mutex> Lock(MainSockListenMutex);
		MainSockPipesMap.erase(Filter);
		return(0);
	}
	int AddConnection(MBChatConnection* NewConnection)
	{
		std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
		NewConnection->ConnectionHandle = LastConnectionNumber + 1;
		LastConnectionNumber += 1;
		ConnectionMap[NewConnection->ConnectionHandle] = NewConnection;
		ActiveConnections.push_back(NewConnection);
		return(0);
	}
	int SendDataFromMainSocket(MainSockDataToSendStruct DataToSend)
	{
		std::lock_guard<std::mutex> Lock(MainSockSendMutex);
		MainSocketDataToSend.push_back(DataToSend);
		MainSocketSendConditional.notify_one();
		return(0);
	}

	void ParseInput(std::string StringToParse)
	{
		if(StringToParse[0] == '/')
		{
			//vi gör ett speciall commando
			std::vector<std::string> CommandWithArguments = Split(StringToParse," ");
			if (CommandList.find(CommandWithArguments[0]) != CommandList.end())
			{
				CommandList[CommandWithArguments[0]](CommandWithArguments,this);
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
				MBError SendError =  SendDataToConnection(StringToParse, ActiveConnectionNumber);
				if (!SendError)
				{
					PrintLine(SendError.ErrorMessage);
				}
			}
		}
	}
	MBError SendDataToConnection(std::string Data, int ConnectionHandle)
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
	MBError SendDataCompletely(MBSockets::UDPSocket* SocketToUse,std::string DataToSend,std::string HostAdress,int PortNumber,MBChatConnection* AssociatedConnectionObject = nullptr)
	{
		//ifall vi har en nullptr till connectionen är det samma sak som att datan är okrypterad
		MBError ErrorToReturn(true);
		SocketToUse->UDPSendData(DataToSend, HostAdress, PortNumber);
		return(ErrorToReturn);
	}
	MBError GetNextCompleteTransmission(MBSockets::UDPSocket* SocketToUse, std::string& OutString,MBChatConnection* AssociatedConnectionObject = nullptr)
	{
		//ifall vi har en nullptr till connectionen är det samma sak som att datan är okrypterad
		MBError ErrorToReturn(true);
		OutString = SocketToUse->UDPGetData();
		return(ErrorToReturn);
	}
	void InitaiteConnection(std::string PeerToConnectTo)
	{
		/*
		MBChatConnection* NewConnection = new MBChatConnection;
		std::string Port = std::to_string(StandardInitiationPort);
		MBSockets::UDPSocket Test(PeerToConnectTo, Port, MBSockets::TraversalProtocol::TCP);
		std::thread ConnectionThread(TestUDPFunc, PeerToConnectTo, Port, &Test);
		Test.Bind(std::stoi(Port));
		while (true)
		{
			std::string StringToSend = "";
			Test.UDPSendData(StringToSend, PeerToConnectTo, std::stoi(Port));
		}
		ConnectionThread.join();
		*/
	}
	void LoadConfig()
	{
		std::lock_guard<std::mutex> Lock(ConfigFileMutex);
		if(std::filesystem::exists("MBConfig"))
		{
			//den existarar, vi laddar in den, annars så skapar vi en ny med nya standard värden
			//vi antar att varje rad är 255 karaktärer lång
			std::fstream ConfigFile("MBConfig");
			std::string LineData;
			std::getline(ConfigFile, LineData,'\n');
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
				std::string StandardUsername = "UsrName:Default"+std::string(ConfigfileLineSize-1 - 15, 0) + '\n';
				std::string StandardExtIp = "ExtIp:127.0.0.1" + std::string(ConfigfileLineSize-1 - 15, 0) + '\n';
				ConcfigFile.write(StandardUsername.c_str(),StandardUsername.size());
				ConcfigFile.write(StandardExtIp.c_str(), StandardExtIp.size());
			}
			else
			{
				PrintLine("Error reading config file. Standard variables not set");
			}
			ConcfigFile.close();
		}
	}
	void ChangeConfig(std::string ConfigToChange, std::string& ParameterData)
	{
		if (!std::filesystem::exists("MBConfig"))
		{
			LoadConfig();
		}
		std::lock_guard<std::mutex> Lock(ConfigFileMutex);
		std::fstream ConfigFile("MBConfig",std::ios::binary | std::ios::out | std::ios::in);
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
public:
	void PrintLine(std::string LineToPrint)
	{
		PrintString(LineToPrint + "\n");
	}
	void PrintString(std::string StringToPrint)
	{
		std::lock_guard<std::mutex> Lock(PrintMutex);
		//tar bort all data usern har just nu
		for (int i = 0; i < CurrentInput.size(); i++)
		{
			std::cout << "\b \b";
		}
		std::cout << StringToPrint;
		std::cout << CurrentInput;
		std::flush(std::cout);
	}
	std::string GetUsrName()
	{
		std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
		return(Username);
	}
	int GetCurrentConnection()
	{
		std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
		return(ActiveConnectionNumber);
	}
	MrBoboChat()
	{
		InitializeObject();
		PrintLine("Welcome to MrBoboChatt!");
		PrintLine("Enter /help to get a list of commands");
	}
	~MrBoboChat()
	{
		
	}
	void MainLoop()
	{
		//spawnar alla threads, och passar sedan enbart inputen till motsvarande ställen

		MainSocket.Bind(StandardInitiationPort);
		while (true)
		{
			char NewInput = GetCharInput();
			//std::cout << HexEncodeByte(NewInput);
			//continue;
			

			if (NewInput == '\r' || NewInput == '\n')
			{
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
				if (NewInput==0x7f)
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
				CurrentInput += NewInput;
			}
			/*
			std::string StringToParse;
			std::getline(std::cin, StringToParse);
			ParseInput(StringToParse);
			*/
		}
	}
};
inline void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject,MrBoboChat* AssociatedChatObject)
{
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
}
inline void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
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
			MBError RecievedError = AssociatedChatObject->SendDataToConnection(char(0x01) + "sendfile"+CommandWithArguments[1], AssociatedChatObject->ActiveConnectionNumber);
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
inline void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	//std::cout << "(* denotes optional argument)\n";
	AssociatedChatObject->PrintLine("help - displays a list of commands");
	AssociatedChatObject->PrintLine("startcon (ipadress) - Starts a connection with specified ip adress");
	AssociatedChatObject->PrintLine("setextip (ipadress) - Sets your external ip. Correct external ip is required in order to start a connection.");
	AssociatedChatObject->PrintLine("setuser (username) - Sets your username used in chats. Change is only visible on new connections.");
	AssociatedChatObject->PrintLine("sendfile (filename) - Transfer a copy of the file to the peer in a chatt as specified by the relative path to this executable or absolute path.");
	//std::cout << "activecon - lists current active connections by IP or username\n";
}
inline void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	std::lock_guard<std::mutex> Lock(AssociatedChatObject->ConfigFileMutex);
	std::string LineData;
	std::fstream ConfigFile("MBConfig", std::ios::binary | std::ios::out | std::ios::in);
	if (ConfigFile.good())
	{
		while (std::getline(ConfigFile,LineData))
		{
			AssociatedChatObject->PrintLine(LineData);
		}
	}
	else
	{
		AssociatedChatObject->PrintLine("Error loading config file");
	}
}
inline void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
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
inline void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
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
inline void MainSocket_Sender(MrBoboChat* AssociatedChatObject)
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
inline void MainSocket_Listener(MrBoboChat* AssociatedChatObject)
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
inline void ChatMainFunction(MBChatConnection* AssociatedConnectionObject)
{
	//skickar handshake meddelanden så vi vet att vi holepunchat
	MBSockets::UDPSocket SocketToUse = MBSockets::UDPSocket(AssociatedConnectionObject->PeerIPAddress, std::to_string(AssociatedConnectionObject->ConnectionPort), MBSockets::TraversalProtocol::TCP);
	SocketToUse.Bind(AssociatedConnectionObject->ConnectionPort);
	std::mutex MainFuncMutex;
	std::atomic<bool> HandshakeComplete{ false };
	std::atomic<bool> StopThreads{ false };
	std::string LocalUsername = AssociatedConnectionObject->AssociatedChatObject->GetUsrName();
	std::atomic<bool> RecievedMessage{ false };
	std::vector<std::string> Messages = {};
	std::string PeerUserName = "";
	int LastReadMessage = 0;
	int LastRecievedMessageIndex = -1;
	clock_t Timer = clock();
	int SendIntervall = 0.05;
	std::thread ListenThread = std::thread(ChatMainFunction_Listener, &StopThreads, &SocketToUse, &Messages, &RecievedMessage, &MainFuncMutex);
	do
	{
		if ((clock()-Timer) / float(CLOCKS_PER_SEC) > SendIntervall)
		{
			Timer = clock();
			MBCConfirmConnectionMessage MessageToSend = MBCConfirmConnectionMessage();
			MessageToSend.MBCMessageType = MBCPMessageType::AcceptConnection;
			MessageToSend.RecordNumber = 0;
			MessageToSend.UserName = AssociatedConnectionObject->AssociatedChatObject->GetUsrName();
			SocketToUse.UDPSendData(MessageToSend.ToString(), AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->ConnectionPort);
			if (RecievedMessage)
			{
				RecievedMessage = false;
				MBCConfirmConnectionMessage RecievedMessage;
				{
					std::lock_guard<std::mutex> Lock(MainFuncMutex);
					RecievedMessage = MBCConfirmConnectionMessage(Messages.back());
					PeerUserName = RecievedMessage.UserName;
				}
				SocketToUse.UDPSendData(MessageToSend.ToString(), AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->ConnectionPort);
				HandshakeComplete = true;
			}
		}
	} while (!HandshakeComplete);
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
					NumberOfMessages = Messages.size()-LastReadMessage;
				}
				for (size_t i = 0; i < NumberOfMessages; i++)
				{
					std::lock_guard<std::mutex> Lock(MainFuncMutex);
					MBCMessage Test(Messages[LastReadMessage]);
					if (MBCMessage(Messages[LastReadMessage]).MBCMessageType != MBCPMessageType::AcceptConnection)
					{
						AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + "[" + PeerUserName + "] " + ANSI::RESET+Messages.back());
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
					Data = AssociatedConnectionObject->ConnectionInput.front();
				}
				if (Data[0] != 0x00)
				{
					SocketToUse.UDPSendData(Data, AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->ConnectionPort);
					std::lock_guard<std::mutex> Lock(AssociatedConnectionObject->ConnectionMutex);
					AssociatedConnectionObject->ConnectionInput.pop_front();
				}
				else
				{
					//vi vet att det här är ett special kommando som vi fått från någonstans. I detta fall ska vi göra ett kommandop
					if(Data.substr(1,8) == "sendfile")
					{
						std::string FilePath = Data.substr(9);
						std::string FileName = Split(FilePath, "/").back();
						
					}
				}
			}
		}
	}
	//vi borde stoppa
	StopThreads = true;
	ListenThread.join();
}
inline void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject)
{
	if (CommandWithArguments.size() < 2)
	{
		AssociatedChatObject->PrintLine("Command requires an IP adress");
	}
	else
	{
		MBChatConnection* NewConnection;
		if (CreateConnection(CommandWithArguments[1], AssociatedChatObject, &NewConnection))
		{
			NewConnection->ConnectionThread = new std::thread(ChatMainFunction, NewConnection);
		}
		else
		{
			AssociatedChatObject->PrintLine(ANSI::RED + "Error occured when trying to start connection with ip " + ANSI::BLUE + CommandWithArguments[1] + ANSI::RESET);
		}
	}
}
inline MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject,MBChatConnection** OutConnection)
{
	MBChatConnection* NewConnection = new MBChatConnection;
	MBError ErrorReturnMessage = MBError(true);

	std::condition_variable StartConWaitConditional;
	std::mutex ResourceMutex;
	std::mutex WaitMutex;
	std::string RecievedData = "";
	AssociatedChatObject->PrintLine("Initiating connection...\nPress Enter to cancel\n");
	bool AgreedOnSocket = false;
	std::atomic<bool> UserCanceled(false);
	std::atomic<bool> StopReadThread(false);
	std::thread ReadUserInputThread(ListenOnInitiationUserInput, &UserCanceled, &StopReadThread);

	int PeerFirstMessagePort = -1;
	int LocalFirstMessagePort = 2700;
	int LastPeerPortSuggestion = -1;
	//den under sätter vi på något stt
	int LastLocalPortSuggestion = -1;
	MainSockPipe Pipe;
	AssociatedChatObject->AddMainSockPipe(IPAdress, &Pipe);
	MBCInitatieConnectionMessage MessageToSend;
	MessageToSend.MBCMessageType = MBCPMessageType::InitiateConnection;
	MessageToSend.InitationMessageType = MBCPInitatioMessageType::FirstMessage;
	MessageToSend.PortSuggestion = LocalFirstMessagePort;
	//adressen vi skickar från, vet inte hur man får den externa ipn
	MessageToSend.SendAdress = AssociatedChatObject->ExternalIp;
	int SendRecordDelay = 0.1;
	while (true)
	{
		//std::unique_lock<std::mutex> WaitLock(WaitMutex);
		//StartConWaitConditional.wait(WaitLock);
		//klockan som sköter hur vi skickar record
		clock_t SendRecordTimer = clock();
		while (Pipe.RecordAdded == false && UserCanceled == false)
		{
			if ((clock() - SendRecordTimer) / float(CLOCKS_PER_SEC) > SendRecordDelay)
			{
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
		MBCInitatieConnectionMessage PeerMessage(Pipe.RecievedRecords.back());
		if (PeerMessage.InitationMessageType == MBCPInitatioMessageType::FirstMessage)
		{
			//det första meddelandet som går igenom UDP hål punchandet
			//vi sätter första porten till det den skickade
			PeerFirstMessagePort = PeerMessage.PortSuggestion;
			int FirstPortSuggestion = LocalFirstMessagePort + PeerFirstMessagePort;
			while (!AssociatedChatObject->PortIsAvailable(FirstPortSuggestion))
			{
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
		}
		else if (PeerMessage.InitationMessageType == MBCPInitatioMessageType::AcceptPort)
		{
			if (PeerMessage.PortSuggestion == LastLocalPortSuggestion || AssociatedChatObject->PortIsAvailable(PeerMessage.PortSuggestion))
			{
				//båda parterna har kommit överents om en port som vi kan köra på, vi skapar då den andra threaden som faktiskt är connection threaden
				NewConnection->ConnectionPort = PeerMessage.PortSuggestion;
				NewConnection->AssociatedChatObject = AssociatedChatObject;
				NewConnection->PeerIPAddress = IPAdress;
				//NewConnection->ConnectionThread = new std::thread(ChatMainFunction, NewConnection);
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
				AssociatedChatObject->SendDataFromMainSocket(DataToSend);
			}
		}
	}
	ReadUserInputThread.join();
	AssociatedChatObject->RemoveMainSockPipe(IPAdress);
	AssociatedChatObject->PrintLine("Successfully connected to " + IPAdress);
	if (UserCanceled)
	{
		*OutConnection = nullptr;
		delete NewConnection;
		ErrorReturnMessage.Type = MBErrorType::Error;
		return(ErrorReturnMessage);
	}
	else
	{
		AssociatedChatObject->AddConnection(NewConnection);
		//vi gör current connection till den vi addade
		AssociatedChatObject->ActiveConnectionNumber = NewConnection->ConnectionHandle;
		*OutConnection = NewConnection;
		return(ErrorReturnMessage);
	}
}