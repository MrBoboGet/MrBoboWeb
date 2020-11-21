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
#include <sys/select.h>
#include <sys/stat.h>
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
enum class MBCPMessageType : uint8_t
{
	InitiateConnection, AcceptConnection = 2, Null
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
		for (int i = 3; i >= 0; i--)
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
		ReturnValue += char(PortSuggestion >> 8);
		ReturnValue += char(PortSuggestion % 256);
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
enum class MBTCPRecordType : uint8_t
{
	InitiateMessageTransfer,
	MessageData, 
	RequestResponse,
	NATBreakMessage,
	Null
};
enum class MBTCPError : uint8_t
{
	OK,Error
};
enum class MBTCPRecordSendType : uint8_t
{
	Send,
	Respond,Null
};
class MBTCPRecord
{
protected:
	int ParseOffset = 0;
public:
	MBTCPRecordType RecordType = MBTCPRecordType::Null;
	MBTCPRecordSendType SendType = MBTCPRecordSendType::Null;
	uint32_t RecordNumber = 0;
	MBTCPRecord() {};
	MBTCPRecord(std::string StringToParse)
	{
		TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
		DataHandler.SetPosition(ParseOffset);

		RecordType = (MBTCPRecordType) DataHandler.Extract8();
		SendType = (MBTCPRecordSendType)DataHandler.Extract8();
		RecordNumber = DataHandler.Extract32();

		ParseOffset = DataHandler.GetPosition();
	}
	virtual std::string ToString()
	{
		char DataBuffer[6];
		TLS1_2::NetWorkDataHandler DataHandler((uint8_t*)DataBuffer);
		DataHandler << uint8_t(RecordType);
		DataHandler << uint8_t(SendType);
		DataHandler << uint32_t(RecordNumber);
		return(std::string(DataBuffer, 6));
	}
};
class MBTCPInitatieMessageTransfer : public MBTCPRecord
{
public:
	uint32_t MessageId = 0;//sätter ett id som varje record i detta message har
	uint16_t NumberOfMessages = 0;//avgör hur många data messages som förväntas skickas med denna funktion
	uint32_t TotalDataLength = 0;//avgör förväntad total mängd data
	uint8_t DataCheckIntervall = 10;//avgör hur många protokoll max som kan skickas innan en integirets check förväntas
	MBTCPError RespondError = MBTCPError::OK;
	MBTCPInitatieMessageTransfer() {};
	MBTCPInitatieMessageTransfer(std::string StringToParse) : MBTCPRecord(StringToParse)
	{
		TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
		DataHandler.SetPosition(ParseOffset);

		MessageId = DataHandler.Extract32();
		NumberOfMessages = DataHandler.Extract16();
		TotalDataLength = DataHandler.Extract32();
		DataCheckIntervall = DataHandler.Extract8();
		RespondError = (MBTCPError)DataHandler.Extract8();

		ParseOffset = DataHandler.GetPosition();
	}
	bool ParametersMatch(MBTCPInitatieMessageTransfer ObjectToCompare)
	{
		if (MessageId != ObjectToCompare.MessageId)
		{
			return(false);
		}
		if (NumberOfMessages != ObjectToCompare.NumberOfMessages)
		{
			return(false);
		}
		if (TotalDataLength != ObjectToCompare.TotalDataLength)
		{
			return(false);
		}
		if (DataCheckIntervall != ObjectToCompare.DataCheckIntervall)
		{
			return(false);
		}
		return(true);
	}
	std::string ToString() override
	{
		std::string ReturnValue = MBTCPRecord::ToString();
		char DataBuffer[12];
		TLS1_2::NetWorkDataHandler DataHandler((uint8_t*)DataBuffer);
		DataHandler << uint32_t(MessageId);
		DataHandler << uint16_t(NumberOfMessages);
		DataHandler << uint32_t(TotalDataLength);
		DataHandler << uint8_t(DataCheckIntervall);
		DataHandler << uint8_t(RespondError);
		ReturnValue += std::string(DataBuffer, 12);
		return(ReturnValue);
	}
};
class MBTCPMessage : public MBTCPRecord
{
public:
	uint32_t MessageId = 0;
	uint16_t MessageNumber = 1;
	uint16_t DataLength = 0;
	std::string Data;

	MBTCPMessage() {};
	MBTCPMessage(std::string StringToParse) : MBTCPRecord(StringToParse)
	{
		TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
		DataHandler.SetPosition(ParseOffset);

		MessageId = DataHandler.Extract32();
		MessageNumber = DataHandler.Extract16();
		DataLength = DataHandler.Extract16();
		
		Data = StringToParse.substr(DataHandler.GetPosition(), DataLength);

		ParseOffset = DataHandler.GetPosition() + Data.size();
	}
	std::string ToString() override
	{
		std::string ReturnValue = MBTCPRecord::ToString();

		DataLength = Data.size();
		for (int i = 3; i >= 0; i--)
		{
			ReturnValue += char((MessageId >> (i * 8)) % 256);
		}
		for (int i = 1; i >= 0; i--)
		{
			ReturnValue += char((MessageNumber >> (i * 8)) % 256);
		}
		for (int i = 1; i >= 0; i--)
		{
			ReturnValue += char((DataLength >> (i * 8)) % 256);
		}
		ReturnValue += Data;
		return(ReturnValue);
	}
};
class MBTCPMessageVerification : public MBTCPRecord
{
public:
	MBTCPError RespondError = MBTCPError::OK;
	uint32_t MessageId = 0;
	uint16_t ByteLengthOfResendRecords = 0;
	std::vector<uint16_t> RecordsToResend = {};

	MBTCPMessageVerification() {};
	MBTCPMessageVerification(std::string StringToParse) : MBTCPRecord(StringToParse)
	{
		TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
		DataHandler.SetPosition(ParseOffset);

		RespondError = (MBTCPError)DataHandler.Extract8();
		MessageId = DataHandler.Extract32();
		ByteLengthOfResendRecords = DataHandler.Extract16();
		for (size_t i = 0; i < ByteLengthOfResendRecords/2; i++)
		{
			RecordsToResend.push_back(DataHandler.Extract16());
		}

		ParseOffset = DataHandler.GetPosition();
	}
	std::string ToString() override
	{
		std::string ReturnValue = MBTCPRecord::ToString();

		ReturnValue += char(RespondError);
		for (int i = 3; i >= 0; i--)
		{
			ReturnValue += char((MessageId >> (i * 8)) % 256);
		}
		for (int i = 1; i >= 0; i--)
		{
			ReturnValue += char((ByteLengthOfResendRecords >> (i * 8)) % 256);
		}
		for (size_t i = 0; i < RecordsToResend.size(); i++)
		{
			ReturnValue += char(RecordsToResend[i] >> 8);
			ReturnValue += char(RecordsToResend[i] % 256);
		}
		return(ReturnValue);
	}
};
class MBChatConnection;
void MBChatConnection_ListenFunc(MBChatConnection*);
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
	MBSockets::UDPSocket ConnectionSocket;
	//är 16, 20 för säkerhets skull
	const std::atomic<int> MBTCPMessageHeaderLength{ 20 };
	const std::atomic<int> MessagesBeforeDisconnection{ 10 };
	const std::atomic<float> StandardResponseWait{ 0.3 };
	const std::atomic<int> MaxSendLength{ 512 - MBTCPMessageHeaderLength};
	friend void MBChatConnection_ListenFunc(MBChatConnection*);
	std::string GetNextResponseMessage(float SecondsBeforeTimeout = 1)
	{
		clock_t TimeOut = clock();
		while (!RecievedResponseMessage)
		{
			if ((clock()-TimeOut)/float(CLOCKS_PER_SEC) > SecondsBeforeTimeout)
			{
				return("");
			}
		}
		//fått meddelande, recieved response message är då nästa meddelande
		std::string ReturnString = "";
		{
			std::lock_guard<std::mutex> Lock(PeerResponseMutex);
			ReturnString = PeerResponseMessages.front();
			PeerResponseMessages.pop_front();
			if (PeerResponseMessages.size() == 0)
			{
				RecievedResponseMessage = false;
			}
		}
		return(ReturnString);
	}
	std::string GetNextSendMessage(float SecondsBeforeTimeout = 1)
	{
		clock_t TimeOut = clock();
		while (!RecievedSendMessage)
		{
			if ((clock() - TimeOut) / float(CLOCKS_PER_SEC) > SecondsBeforeTimeout)
			{
				return("");
			}
		}
		//fått meddelande, recieved response message är då nästa meddelande
		std::string ReturnString = "";
		{
			std::lock_guard<std::mutex> Lock(PeerSendMutex);
			ReturnString = PeerSendMessages.front();
			PeerSendMessages.pop_front();
			if (PeerSendMessages.size() == 0)
			{
				RecievedSendMessage = false;
			}
		}
		return(ReturnString);
	}
public:
	int PrivateRecordNumber = 0;
	int PeerRecordNumber = 0;
	int ConnectionPort = -1;
	int ConnectionHandle = -1;
	//MBChatConnection()
	//{
	//	ConnectionListenThread;
	//}
	MBChatConnection(std::string PeerIP, int Port) : ConnectionSocket(PeerIP, std::to_string(Port), MBSockets::TraversalProtocol::TCP)
	{
		ConnectionSocket.UDPMakeSocketNonBlocking(StandardResponseWait);
		ConnectionSocket.Bind(Port);
		ConnectionListenThread = std::thread(MBChatConnection_ListenFunc,this);
	}
	std::mutex ConnectionMutex;
	std::string PeerIPAddress = "";
	MrBoboChat* AssociatedChatObject;
	std::thread* ConnectionThread;
	std::deque<std::string> ConnectionInput = {};

	MBError SendData(std::string DataToSend)
	{
		//vi initierat MBTCP protokollet
		//vi börjar med att skicka initiate meddelandet och väntar sedan på svaret från motparten
		//headers för protokollen är 16 byte, men vi tar 20 bytes för lite marginal
		MBError ReturnError(true);
		if (DataToSend == "")
		{
			ReturnError.Type = MBErrorType::Error;
			ReturnError.ErrorMessage = "No data to send";
			return(ReturnError);
		}

		MBTCPInitatieMessageTransfer FirstMessage;
		FirstMessage.SendType = MBTCPRecordSendType::Send;
		FirstMessage.RecordNumber = PrivateRecordNumber;
		PrivateRecordNumber += 1;
		FirstMessage.NumberOfMessages = DataToSend.size() / MaxSendLength;
		if (DataToSend.size()%MaxSendLength != 0)
		{
			FirstMessage.NumberOfMessages += 1;
		}
		FirstMessage.DataCheckIntervall = 10;
		if (FirstMessage.DataCheckIntervall > FirstMessage.NumberOfMessages)
		{
			FirstMessage.DataCheckIntervall = FirstMessage.NumberOfMessages;
		}
		FirstMessage.TotalDataLength = DataToSend.size();
		FirstMessage.RecordType = MBTCPRecordType::InitiateMessageTransfer;
		//vi borde faktiskt ha en bra randomfunktion
		FirstMessage.MessageId = rand();
		//nu tar vi och faktiskt skickar messaget
		bool RecievedFirstMessageResponse = false;
		int FirstMessagesSent = 0;
		std::string ResponseData = "";
		while(!RecievedFirstMessageResponse)
		{
			ConnectionSocket.UDPSendData(FirstMessage.ToString(), PeerIPAddress, ConnectionPort);
			ResponseData = GetNextResponseMessage();
			if (ResponseData != "")
			{
				break;
			}
			else
			{
				FirstMessagesSent += 1;
				if (FirstMessagesSent > MessagesBeforeDisconnection)
				{
					ReturnError.Type = MBErrorType::Error;
					ReturnError.ErrorMessage = "No response from host";
					return(ReturnError);
				}
			}
		}
		std::string DataSent = FirstMessage.ToString();
		MBTCPInitatieMessageTransfer ResponseMessage(ResponseData);
		if(ResponseMessage.RespondError != MBTCPError::OK || !FirstMessage.ParametersMatch(ResponseMessage))
		{
			ReturnError.Type = MBErrorType::Error;
			ReturnError.ErrorMessage = "Error in response message";
			return(ReturnError);
		}

		//när vi fått bekräftat att vi kan börja skicka grejer så tar vi och gör det tills vi antingen nått den bestämda MessageCheck intervallet eller skickat alla meddelanden
		bool Finished = false;
		int MessageBatchesSent = 0;
		while (!Finished)
		{
			bool AllDataRecieved = false;
			std::vector<std::string> PartitionedMessageData = std::vector<std::string>(FirstMessage.DataCheckIntervall);
			for (size_t i = 0; i < PartitionedMessageData.size(); i++)
			{
				if(i * MaxSendLength + MessageBatchesSent * FirstMessage.DataCheckIntervall > DataToSend.size())
				{
					PartitionedMessageData[i] = "";
					continue;
				}
				PartitionedMessageData[i] = DataToSend.substr(i * MaxSendLength+MessageBatchesSent*FirstMessage.DataCheckIntervall, MaxSendLength);
			}
			//först så skickar vi alla data, väntar sedan på responset och ser om vi ska skicka något igen
			for (size_t i = 0; i < PartitionedMessageData.size(); i++)
			{
				MBTCPMessage MessageToSend;
				MessageToSend.Data = PartitionedMessageData[i];
				MessageToSend.DataLength = MessageToSend.Data.size();
				MessageToSend.MessageId = FirstMessage.MessageId;
				MessageToSend.MessageNumber = i + 1 + MessageBatchesSent * FirstMessage.DataCheckIntervall;
				MessageToSend.RecordNumber = PrivateRecordNumber;
				PrivateRecordNumber += 1;
				MessageToSend.RecordType = MBTCPRecordType::MessageData;
				MessageToSend.SendType = MBTCPRecordSendType::Send;
				ConnectionSocket.UDPSendData(MessageToSend.ToString(),PeerIPAddress, ConnectionPort);
			}
			int WaitCycles = 0;
			bool PeerDisconeccted = false;
			while (!AllDataRecieved)
			{
				std::string ResponseData = GetNextResponseMessage();
				if (ResponseData == "")
				{
					//timeout, vi väntar igen en given tid
					WaitCycles += 1;
					if (WaitCycles > MessagesBeforeDisconnection)
					{
						PeerDisconeccted = true;
						break;
					}
					continue;
				}
				else
				{
					MBTCPMessageVerification VerificationResponse(ResponseData);
					if (VerificationResponse.RespondError != MBTCPError::OK || VerificationResponse.MessageId != FirstMessage.MessageId)
					{
						//vet inte vad protokollet ska göra här
					}
					else
					{
						if (VerificationResponse.RecordsToResend.size() == 0)
						{
							//alla messages har blivit reciavade, vi är klara med denna batch
							AllDataRecieved = true;
						}
						else
						{
							//vi skickar recordsen den inte fick, går till toppen av loopen och väntar på respons data
							for (size_t i = 0; i < VerificationResponse.RecordsToResend.size(); i++)
							{
								int RecordToResendIndex = VerificationResponse.RecordsToResend[i]%FirstMessage.DataCheckIntervall;
								MBTCPMessage MessageToSend;
								MessageToSend.Data = PartitionedMessageData[RecordToResendIndex-1];
								MessageToSend.DataLength = MessageToSend.Data.size();
								MessageToSend.MessageId = FirstMessage.MessageId;
								MessageToSend.MessageNumber = VerificationResponse.RecordsToResend[i];
								MessageToSend.RecordNumber = PrivateRecordNumber;
								PrivateRecordNumber += 1;
								MessageToSend.RecordType = MBTCPRecordType::MessageData;
								MessageToSend.SendType = MBTCPRecordSendType::Send;
								ConnectionSocket.UDPSendData(MessageToSend.ToString(), PeerIPAddress, ConnectionPort);
							}
						}
					}
				}
			}
			MessageBatchesSent += 1;
			if (MessageBatchesSent*FirstMessage.DataCheckIntervall >= FirstMessage.NumberOfMessages)
			{
				Finished = true;
			}
		}
		return(ReturnError);
	}
	std::string GetData(float TimeoutInSeconds = 10000)
	{
		//den första recorden får ska alltid vara en initate connection meddelande
		std::string ReturnValue = "";
		std::string PeerFirstMessageData;
		int FirstWaitTime = 0;
		//skickar lite data så vi breakar that nat
		while (true)
		{
			MBTCPRecord BreakNatMessage;
			BreakNatMessage.SendType = MBTCPRecordSendType::Respond;
			BreakNatMessage.RecordType = MBTCPRecordType::NATBreakMessage;
			BreakNatMessage.RecordNumber = 0;
			ConnectionSocket.UDPSendData(BreakNatMessage.ToString(), PeerIPAddress, ConnectionPort);
			PeerFirstMessageData = GetNextSendMessage();
			if (PeerFirstMessageData != "")
			{
				break;
			}
			else
			{
				FirstWaitTime += 1;
				if (FirstWaitTime > MessagesBeforeDisconnection)
				{
					return("");
				}
			}
		}
		MBTCPInitatieMessageTransfer FirstMessage(PeerFirstMessageData);
		MBTCPInitatieMessageTransfer FirstMessageResponse(FirstMessage.ToString());
		FirstMessageResponse.SendType = MBTCPRecordSendType::Respond;
		ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
		//vi skickar data igen om vi inte får någon data
		bool Finished = false;
		bool PeerRecievedResponseMessage = false;
		int FirstMessageResponseResends = 0;
		while (!Finished)
		{
			std::string NextSendData = GetNextSendMessage();
			if (NextSendData == "")
			{
				ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
				FirstMessageResponseResends += 1;
				if (FirstMessageResponseResends > MessagesBeforeDisconnection)
				{
					return("");
				}
			}
			else
			{
				MBTCPRecord NextResponseType(NextSendData);
				if (NextResponseType.RecordType == MBTCPRecordType::InitiateMessageTransfer)
				{
					//vi vet att vi behöver skicka får response igen, den spåg inte den
					ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
				}
				else if(NextResponseType.RecordType == MBTCPRecordType::MessageData)
				{
					//första meddelandet är bekräftat, vi går nu in i en loop där vi faktiskt ska bygga på retur värdet
					MBTCPMessage NextResponse(NextSendData);
					std::unordered_map<uint16_t, std::string> RecievedRecordsMap = {};
					RecievedRecordsMap[NextResponse.MessageNumber] = NextResponse.Data;
					int MessagesInBatchRecieved = 1;
					int CompleteBatchesRecieved = 0;
					std::string LastDataSent = "";
					while (true)
					{
						int ExpectedMessages = FirstMessage.NumberOfMessages - CompleteBatchesRecieved * FirstMessage.DataCheckIntervall;
						if (ExpectedMessages > FirstMessage.DataCheckIntervall)
						{
							ExpectedMessages = FirstMessage.DataCheckIntervall;
						}
						if (ExpectedMessages < 0)
						{
							ExpectedMessages = 1;
						}
						if (MessagesInBatchRecieved < ExpectedMessages)
						{
							NextSendData = GetNextSendMessage();
							if (NextSendData == "")
							{
								//personen har timat ut, vi ber den att skicka igen
								MBTCPMessageVerification MissingRecordVerification;
								MissingRecordVerification.MessageId = FirstMessage.MessageId;
								MissingRecordVerification.SendType = MBTCPRecordSendType::Respond;
								MissingRecordVerification.RecordType = MBTCPRecordType::MessageData;
								MissingRecordVerification.RecordNumber = 0;
								for (size_t i = 0; i < ExpectedMessages; i++)
								{
									if (RecievedRecordsMap.find(i+1+CompleteBatchesRecieved*FirstMessage.DataCheckIntervall) == RecievedRecordsMap.end())
									{
										MissingRecordVerification.RecordsToResend.push_back(i + 1 + CompleteBatchesRecieved * FirstMessage.DataCheckIntervall);
									}
								}
								MissingRecordVerification.ByteLengthOfResendRecords = (2 * MissingRecordVerification.RecordsToResend.size());
								ConnectionSocket.UDPSendData(MissingRecordVerification.ToString(), PeerIPAddress, ConnectionPort);
								LastDataSent = MissingRecordVerification.ToString();
							}
							else
							{
								//kollar om messaget vi får är av typen "ge respons"
								if (MBTCPMessage(NextSendData).RecordType == MBTCPRecordType::RequestResponse)
								{
									ConnectionSocket.UDPSendData(LastDataSent, PeerIPAddress, ConnectionPort);
									continue;
								}
								MBTCPMessage NextResponseMessage(NextSendData);
								if (RecievedRecordsMap.find(NextResponseMessage.MessageNumber) == RecievedRecordsMap.end())
								{
									RecievedRecordsMap[NextResponseMessage.MessageNumber] = NextResponseMessage.Data;
									MessagesInBatchRecieved += 1;
								}
							}
						}
						else
						{
							//vi appendar datan till vår retur string och resettar för nästa batch
							for (size_t i = 0; i < ExpectedMessages; i++)
							{
								ReturnValue += RecievedRecordsMap[i + 1 + CompleteBatchesRecieved * FirstMessage.DataCheckIntervall];
							}
							RecievedRecordsMap.clear();
							MessagesInBatchRecieved = 0;
							CompleteBatchesRecieved += 1;
							MBTCPMessageVerification MissingRecordVerification;
							MissingRecordVerification.MessageId = FirstMessage.MessageId;
							MissingRecordVerification.SendType = MBTCPRecordSendType::Respond;
							MissingRecordVerification.RecordType = MBTCPRecordType::MessageData;
							MissingRecordVerification.RecordNumber = 0;
							MissingRecordVerification.ByteLengthOfResendRecords = 0;
							ConnectionSocket.UDPSendData(MissingRecordVerification.ToString(), PeerIPAddress, ConnectionPort);
							LastDataSent = MissingRecordVerification.ToString();
							if (CompleteBatchesRecieved*FirstMessage.DataCheckIntervall >= FirstMessage.NumberOfMessages)
							{
								Finished = true;
								break;
							}
						}
					}
				}
			}
		}		
		return(ReturnValue);
	}

	std::atomic<bool> ShouldStop{ false };
	std::atomic<bool> RecievedInput{ false };
};
void MBChatConnection_ListenFunc(MBChatConnection* AssociatedChatObject)
{
	while (!AssociatedChatObject->ShouldStop)
	{
		std::string NextDataRecieved = AssociatedChatObject->ConnectionSocket.UDPGetData();
		if (NextDataRecieved != "")
		{
			MBTCPRecord DataRecord(NextDataRecieved);
			if (DataRecord.RecordType == MBTCPRecordType::NATBreakMessage)
			{
					
			}
			else if (DataRecord.SendType == MBTCPRecordSendType::Send)
			{
				std::lock_guard<std::mutex> Lock(AssociatedChatObject->PeerSendMutex);
				AssociatedChatObject->PeerSendMessages.push_back(NextDataRecieved);
				AssociatedChatObject->RecievedSendMessage = true;
			}
			else if(DataRecord.SendType == MBTCPRecordSendType::Respond)
			{
				std::lock_guard<std::mutex> Lock(AssociatedChatObject->PeerResponseMutex);
				AssociatedChatObject->PeerResponseMessages.push_back(NextDataRecieved);
				AssociatedChatObject->RecievedResponseMessage = true;
			}
		}
	}
}
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
	while (!*ShouldStop)
	{
		int InputIsAvailable = -1;
#ifdef WIN32
		InputIsAvailable = _kbhit();
#elif __linux__
		//InputIsAvailable = select();
		fd_set rfds;
		struct timeval tv;

		/* Watch stdin (fd 0) to see when it has input. */
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		/* Wait up to five seconds. */
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		InputIsAvailable = select(1, &rfds, NULL, NULL, &tv);
		/* Don’t rely on the value of tv now! */
#endif
		if (InputIsAvailable > 0)
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
inline void ChatMainFunction_Listener(MBChatConnection* AssociatedConnectionObject,std::vector<std::string>* RecievedMessages,std::atomic<bool>* RecievedMessage,std::mutex* MainFuncMutex)
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
MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject, MBChatConnection** OutConnection);
void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength);
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
	std::unordered_map<int, bool> UsedPorts = {};

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
		//old.c_lflag |= ICANON;
		//old.c_lflag |= ECHO;
		//if (tcsetattr(0, TCSADRAIN, &old) < 0)
			//perror("tcsetattr ~ICANON");
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
	std::atomic<int> FirstAvailablePort{ -1 };
	//std::unordered_map<int, bool> UsedPorts = {};
	bool PortIsAvailable(int PortToCheck)
	{
		//MBSockets::UDPSocket SocketToTest("127.0.0.1", std::to_string(PortToCheck), MBSockets::TraversalProtocol::TCP);
		std::lock_guard<std::mutex> Lock(GeneralResourceMutex);
		if(UsedPorts.find(PortToCheck) == UsedPorts.end())
		{
			//FirstAvailablePort += 1;
			return(true);
		}
		else
		{
			return(false);
		}
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
		UsedPorts[NewConnection->ConnectionPort] = true;
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
		MBChatConnection* NewConnection;
		MainSocket.Bind(StandardInitiationPort);
		//CreateConnection("127.0.0.1", this, &NewConnection);
		//inline void MBCSendFile_MainLopp(MBChatConnection * AssociatedConnectionObject, std::string FileToSendOrRecieve, bool Recieving, int RecievedOrSentDataLength)
		//std::thread Sendthread(MBCSendFile_MainLopp, NewConnection, "LinusApproves.png", false, 56181);
		//std::thread RecieveThread(MBCSendFile_MainLopp, NewConnection, "RecievedLinus.png", true, 56181);
		//Sendthread.join();
		//RecieveThread.join();
	}
	~MrBoboChat()
	{
		
	}
	void MainLoop()
	{
		//spawnar alla threads, och passar sedan enbart inputen till motsvarande ställen
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
inline void MBCSendFile_MainLopp(MBChatConnection* AssociatedConnectionObject,std::string FileToSendOrRecieve,bool Recieving,int RecievedOrSentDataLength)
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
		std::fstream NewFile(FileToSendOrRecieve,std::ios::binary|std::ios::out);
		int DataRecieved = 0;
		int TimeoutCount = 0;
		int TimoutMax = 15;
		AssociatedConnectionObject->AssociatedChatObject->PrintLine("File to recieve size = " + std::to_string(RecievedOrSentDataLength));
		while(DataRecieved < RecievedOrSentDataLength)
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
		std::ifstream FileToSend(FileToSendOrRecieve, std::ios::in | std::ios::binary |std::ios::out);
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
		while(ExtractedData < RecievedOrSentDataLength)
		{
			char* Data;
			if (DataSentSuccesfully)
			{
				Data = (char*)malloc(DataChunkSize);
				FileToSend.read(Data, DataChunkSize);
			}
			MBError SendError(true);
			std::string DataToSend;
			if(ExtractedData > RecievedOrSentDataLength)
			{
				DataToSend = std::string(Data, RecievedOrSentDataLength % DataChunkSize);
				//SendError = AssociatedConnectionObject->SendData();
			}
			else
			{
				DataToSend = std::string(Data,DataChunkSize);
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
			MBError RecievedError = AssociatedChatObject->SendDataToConnection(std::string(1,1) + "sendfile "+CommandWithArguments[1], AssociatedChatObject->ActiveConnectionNumber);
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
inline long long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}
inline void ChatMainFunction(MBChatConnection* AssociatedConnectionObject)
{
	//skickar handshake meddelanden så vi vet att vi holepunchat
	//MBSockets::UDPSocket SocketToUse = MBSockets::UDPSocket(AssociatedConnectionObject->PeerIPAddress, std::to_string(AssociatedConnectionObject->ConnectionPort), MBSockets::TraversalProtocol::TCP);
	//SocketToUse.Bind(AssociatedConnectionObject->ConnectionPort);
	AssociatedConnectionObject->AssociatedChatObject->PrintLine("Port for connection is "+std::to_string(AssociatedConnectionObject->ConnectionPort));
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
		if ((clock()-Timer) / float(CLOCKS_PER_SEC) > SendIntervall)
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
					NumberOfMessages = Messages.size()-LastReadMessage;
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
						else if(Data.substr(0,10) == "\1sendfile ")
						{
							AssociatedConnectionObject->AssociatedChatObject->PrintLine(ANSI::BLUE + PeerUserName + ANSI::RESET+" Wants to send you a file named " + Split(Data.substr(10),"/").back()+"\n Do you accept? [y/n]");
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
									CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection);
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
					CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection);
					AssociatedConnectionObject->SendData("\1sendfileaccepted");
					NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, NewConnection, Data, true,SizeOfPeerFile);
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
					else if(Data == "n" || Data == "N")
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
					if(Data.substr(1,9) == "sendfile ")
					{
						std::string FilePath = Data.substr(10);
						std::string FileName = Split(FilePath, "/").back();
						FileLocalWantedToSend = FilePath;
						//vi skickar med file data längden här
						long long FileSize = GetFileSize(FilePath);
						AssociatedConnectionObject->SendData(Data.substr(0, 10)+ FileName+" "+std::to_string(FileSize));
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
							CreateConnection(AssociatedConnectionObject->PeerIPAddress, AssociatedConnectionObject->AssociatedChatObject, &NewConnection);
							AssociatedConnectionObject->SendData("\1sendfileaccepted");
							NewConnection->ConnectionThread = new std::thread(MBCSendFile_MainLopp, NewConnection, FileLocalWantedToSend, false,SizeOfLocalFile);
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
			AssociatedChatObject->ActiveConnectionNumber = NewConnection->ConnectionHandle;
		}
		else
		{
			AssociatedChatObject->PrintLine(ANSI::RED + "Error occured when trying to start connection with ip " + ANSI::BLUE + CommandWithArguments[1] + ANSI::RESET);
		}
	}
}
inline MBError CreateConnection(std::string IPAdress, MrBoboChat* AssociatedChatObject,MBChatConnection** OutConnection)
{
	MBChatConnection* NewConnection;
	MBError ErrorReturnMessage = MBError(true);

	//std::condition_variable StartConWaitConditional;
	//std::mutex ResourceMutex;
	//std::mutex WaitMutex;
	//std::string RecievedData = "";
	AssociatedChatObject->PrintLine("Initiating connection...\nPress Enter to cancel\n");
	//bool AgreedOnSocket = false;
	std::atomic<bool> UserCanceled(false);
	std::atomic<bool> StopReadThread(false);
	std::thread ReadUserInputThread(ListenOnInitiationUserInput, &UserCanceled, &StopReadThread);

	int PeerFirstMessagePort = -1;
	int LocalFirstMessagePort = 2700;
	std::cout << "First message port is " << LocalFirstMessagePort << std::endl;
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
	if (MessageToSend.PortSuggestion == -1 && FirstErrorReported == false)
	{
		FirstErrorReported = true;
		AssociatedChatObject->PrintLine("Felaktig port assignades i början");
	}
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
				std::cout << (clock() - SendRecordTimer) / float(CLOCKS_PER_SEC) << std::endl;
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
				NewConnection = new MBChatConnection(IPAdress,PeerMessage.PortSuggestion);
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
	AssociatedChatObject->PrintLine("Connection port " + std::to_string(NewConnection->ConnectionPort));
	ReadUserInputThread.join();
	AssociatedChatObject->RemoveMainSockPipe(IPAdress);
	if (UserCanceled)
	{
		*OutConnection = nullptr;
		//delete NewConnection;
		ErrorReturnMessage.Type = MBErrorType::Error;
		return(ErrorReturnMessage);
	}
	else
	{
		AssociatedChatObject->PrintLine("Successfully connected to " + IPAdress);
		AssociatedChatObject->AddConnection(NewConnection);
		//vi gör current connection till den vi addade
		*OutConnection = NewConnection;
		return(ErrorReturnMessage);
	}
}