#define NOMINMAX
#define _CRT_RAND_S 
#pragma once
#include <MrBoboSockets.h>
#include <MinaStringOperations.h>
#include <MrPostOGet/TLSHandler.h>
#include <MBRandom.h>
#include <MrBoboChatt/MBCProtocols.h>
#include <MBErrorHandling.h>
#include <MrBoboChatt/MBChatConnection.h>
#include <MrBoboChatt/MrBoboChatt.h>
//olika depandancies
struct MBChatConnection;
struct MainSockPipe
{
	std::atomic<bool> RecordAdded = { false };
	std::mutex PipeResourceMutex;
	std::vector<std::string> RecievedRecords = std::vector<std::string>(0);
};
struct MainSockDataToSendStruct
{
	std::string Data = "";
	std::string Host = "";
	int Port = -1;
};
void ListenOnInitiationSocket(MBSockets::UDPSocket* SocketToListenOn, std::string* StringToModify, std::condition_variable* ConditionalToNotify, std::mutex* ResourceMutex);
void ListenOnInitiationUserInput(std::atomic<bool>* BoolToModify, std::atomic<bool>* ShouldStop,MrBoboChat* AssociatedChatObject,bool OnMainThread);
void InitiationSendData(MBSockets::UDPSocket* SocketToSendDataOn, std::string* StringToModify, std::condition_variable* ConditionalToNotify, std::mutex* ResourceMutex);
void ChatMainFunction_Listener(MBChatConnection* AssociatedConnectionObject, std::vector<std::string>* RecievedMessages, std::atomic<bool>* RecievedMessage, std::mutex* MainFuncMutex);
class MBChatStaticResources
{
private:
	std::mutex ResourceMutex;
	std::string DiffieHellmanModolu = "87A8E61DB4B6663CFFBBD19C651959998CEEF608660DD0F25D2CEED4435E3B00E00DF8F1D61957D4FAF7DF4561B2AA3016C3D91134096FAA3BF4296D830E9A7C209E0C6497517ABD5A8A9D306BCF67ED91F9E6725B4758C022E0B1EF4275BF7B6C5BFC11D45F9088B941F54EB1E59BB8BC39A0BF12307F5C4FDB70C581B23F76B63ACAE1CAA6B7902D52526735488A0EF13C6D9A51BFA4AB3AD8347796524D8EF6A167B5A41825D967E144E5140564251CCACB83E6B486F6B3CA3F7971506026C0B857F689962856DED4010ABD0BE621C3A3960A54E710C375F26375D7014103A4B54330C198AF126116D2276E11715F693877FAD7EF09CADB094AE91E1A1597";
	std::string DiffieHellmanGenerator = "3FB32C9B73134D0B2E77506660EDBD484CA7B18F21EF205407F4793A1A0BA12510DBC15077BE463FFF4FED4AAC0BB555BE3A6C1B0C6B47B1BC3773BF7E8C6F62901228F8C28CBB18A55AE31341000A650196F931C77A57F2DDF463E5E9EC144B777DE62AAAB8A8628AC376D282D6ED3864E67982428EBC831D14348F6F2F9193B5045AF2767164E1DFC967C1FB3F2E55A4BD1BFFE83B9C80D052B985D182EA0ADB2A3B7313D3FE14C8484B1E052588B9B7D2BBD2DF016199ECD06E1557CD0915B3353BBB64E0EC377FD028370DF92B52C7891428CDC67EB6184B523D1DB246C32F63078490F00EF8D647D148D47954515E2327CFEF98C582664B4C0F6CC41659";
public:
	std::string GetDiffieHellmanModolu();
	std::string GetDiffieHellmanGenerator();
};
class MrBoboChat;
long long GetFileSize(std::string);
void ChatMainFunction(MBChatConnection* AssociatedConnectionObject);
void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MainSocket_Sender(MrBoboChat* AssociatedChatObject);
void MainSocket_Listener(MrBoboChat* AssociatedChatObject);
void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection, bool OnMainThread);

void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength);


class MBCLineObject
{
private:
	int LineNumber = -1;
	std::string LineData = "";
	MrBoboChat* AssociatedChatObject = nullptr;
public:
	MBCLineObject(MrBoboChat* AssociatedChatObject);
	std::string GetLineData();
	void SetLineData(std::string NewData);
};

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
	friend MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection, bool OnMainThread);

	friend MBCLineObject;
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
	std::unordered_map<int, bool> UsedPorts = {};

	std::mutex PrintMutex;

	std::atomic<int> CurrentLineIndex{0};
	std::unordered_map<std::string, void (*)(std::vector<std::string>,MrBoboChat*)> CommandList
	{
		{"/help",MrBoboChatHelp},
		{"/startcon",StartCon},
		{"/setextip",SetExtIp},
		{"/setuser",SetUsername},
		{"/viewconfig",ViewConfig},
		{"/sendfile",MBCSendFile}
	};

	int GetCurrentLineIndex();
	void PrintLineAtIndex(int Index,std::string StringToPrint);
	void InitializeObject();
	int AddMainSockPipe(std::string Filter, MainSockPipe* PipeToAdd);
	//std::atomic<int> FirstAvailablePort{ -1 };
	//std::unordered_map<int, bool> UsedPorts = {};
	bool PortIsAvailable(int PortToCheck);
	int RemoveMainSockPipe(std::string Filter);
	int AddConnection(MBChatConnection* NewConnection);
	int SendDataFromMainSocket(MainSockDataToSendStruct DataToSend);

	void ParseInput(std::string StringToParse);
	MBError SendDataToConnection(std::string Data, int ConnectionHandle);
	MBError SendDataCompletely(MBSockets::UDPSocket* SocketToUse, std::string DataToSend, std::string HostAdress, int PortNumber, MBChatConnection* AssociatedConnectionObject = nullptr);
	MBError GetNextCompleteTransmission(MBSockets::UDPSocket* SocketToUse, std::string& OutString, MBChatConnection* AssociatedConnectionObject = nullptr);
	//void InitaiteConnection(std::string PeerToConnectTo);
	void LoadConfig();
	void ChangeConfig(std::string ConfigToChange, std::string& ParameterData);
public:
	MBChatStaticResources StaticResources;

	char GetCharInput();
	std::atomic<bool> InitiatingCancelableInput{ false };
	std::string GetExtIp();
	void PrintLine(std::string LineToPrint);
	void PrintString(std::string StringToPrint);
	std::string GetUsrName();
	int GetCurrentConnection();
	MrBoboChat();
	~MrBoboChat();
	void MainLoop();
};

void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength);
void MBCSendFile(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MrBoboChatHelp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void ViewConfig(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void SetUsername(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void SetExtIp(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
void MainSocket_Sender(MrBoboChat* AssociatedChatObject);
void MainSocket_Listener(MrBoboChat* AssociatedChatObject);
long long GetFileSize(std::string filename);
void ChatMainFunction(MBChatConnection* AssociatedConnectionObject);
void StartCon(std::vector<std::string> CommandWithArguments, MrBoboChat* AssociatedChatObject);
MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection,bool OnMainThread);

inline void TestMBChatGrejer()
{
	std::string DataToSend = "HejsanSvejsan";
	MBTCPInitatieMessageTransfer FirstMessage;
	FirstMessage.SendType = MBTCPRecordSendType::Send;
	FirstMessage.RecordNumber = 1;
	FirstMessage.NumberOfMessages = DataToSend.size() / 500;
	if (DataToSend.size() % 500 != 0)
	{
		FirstMessage.NumberOfMessages += 1;
	}
	FirstMessage.DataCheckIntervall = 10;
	FirstMessage.TotalDataLength = DataToSend.size();
	FirstMessage.RecordType = MBTCPRecordType::InitiateMessageTransfer;
	//vi borde faktiskt ha en bra randomfunktion
	FirstMessage.MessageId = rand();
	MBTCPInitatieMessageTransfer NewMessage = MBTCPInitatieMessageTransfer(FirstMessage.ToString());

	MBTCPMessage MessageToSend;
	MessageToSend.Data = DataToSend;
	MessageToSend.DataLength = MessageToSend.Data.size();
	MessageToSend.MessageId = FirstMessage.MessageId;
	MessageToSend.MessageNumber = 10;
	MessageToSend.RecordNumber = 10;
	MessageToSend.RecordType = MBTCPRecordType::MessageData;
	MessageToSend.SendType = MBTCPRecordSendType::Send;
	std::string MessageToSendString = MessageToSend.ToString();
	MBTCPMessage RecievedMessage = MBTCPMessage(MessageToSendString);

	MBTCPMessageVerification MissingRecordVerification;
	MissingRecordVerification.MessageId = FirstMessage.MessageId;
	MissingRecordVerification.SendType = MBTCPRecordSendType::Respond;
	MissingRecordVerification.RecordType = MBTCPRecordType::MessageData;
	MissingRecordVerification.RecordNumber = 0;
	MissingRecordVerification.RecordsToResend.push_back(2);
	MissingRecordVerification.RecordsToResend.push_back(3);
	MissingRecordVerification.ByteLengthOfResendRecords = (2 * MissingRecordVerification.RecordsToResend.size());
	std::string VeriString = MissingRecordVerification.ToString();
	MBTCPMessageVerification NewVerification = MBTCPMessageVerification(VeriString);

	MBCInitatieConnectionMessage ConnectionMessage;
	ConnectionMessage.MBCMessageType = MBCPMessageType::InitiateConnection;
	ConnectionMessage.InitationMessageType = MBCPInitatioMessageType::FirstMessage;
	ConnectionMessage.PortSuggestion = 2700;
	//adressen vi skickar från, vet inte hur man får den externa ipn
	ConnectionMessage.SendAdress = "127.0.0.1";
	std::string ConnectionMessageData = ConnectionMessage.ToString();
	MBCInitatieConnectionMessage ConnectionResponse = MBCInitatieConnectionMessage(ConnectionMessageData);
}