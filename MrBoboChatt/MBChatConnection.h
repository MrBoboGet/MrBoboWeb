#define NOMINMAX
#define _CRT_RAND_S 
#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <MrBigInt/MrBigInt.h>
#include <MrBoboSockets/MrBoboSockets.h>
#include <MBUtility/MBErrorHandling.h>
//#include <MrBoboChatt/MrBoboChatt.h>
struct MBChatConnection_SecurityParameters
{
	int KeyLength = 32;
	std::atomic<bool> ConnectionIsSecure{ false };
	std::string SharedSecret = "";
};
struct MBChatConnection;
class MrBoboChat;
void MBChatConnection_ListenFunc(MBChatConnection* AssociatedChatObject);
struct MBChatConnection
{
private:
	friend void MBChatConnection_ListenFunc(MBChatConnection*);
	MBChatConnection_SecurityParameters SecurityParameters;
	std::mutex PeerResponseMutex;
	std::condition_variable PeerResponseConditional;
	std::mutex PeerSendMutex;
	std::condition_variable PeerSendConditional;
	std::atomic<bool> RecievedResponseMessage{ false };
	std::atomic<bool> RecievedSendMessage{ false };
	std::deque<std::string> PeerResponseMessages = {};
	std::deque<std::string> PeerSendMessages = {};
	std::thread ConnectionListenThread;
	std::string ConnectionDescription = "No description available";
	MBSockets::UDPSocket ConnectionSocket;
	//är 16, 20 för säkerhets skull
	const std::atomic<int> MBTCPMessageHeaderLength{ 20 };
	const std::atomic<int> MessagesBeforeDisconnection{ 10 };
	const std::atomic<float> StandardResponseWait{ 0.3 };
	const std::atomic<int> MaxSendLength{ 512 - MBTCPMessageHeaderLength };

	std::string GetNextResponseMessage(float SecondsBeforeTimeout = 1);
	std::string GetNextSendMessage(float SecondsBeforeTimeout = 1);
	MrBigInt GenerateRandomValue(uint16_t MaxNumberOfBytes);
	bool IpAddresIsLower(std::string LeftIPAdress, std::string RightIPAdress);
	std::string EncryptData(std::string& DataToEncrypt);
	std::string DecryptData(std::string& DataToDecrypt);
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
	std::atomic<bool> ShouldStop{ false };
	std::atomic<bool> RecievedInput{ false };
	//MBChatConnection()
	//{
	//	ConnectionListenThread;
	//}

	MBChatConnection(std::string PeerIP, int Port); //: ConnectionSocket(PeerIP, std::to_string(Port), MBSockets::TraversalProtocol::TCP);
	MBError EstablishSecureConnection();
	MBError SendData(std::string DataToSend);
	std::string GetDescription();
	void SetDescription(std::string NewDescription);
	std::string GetData(float TimeoutInSeconds = 10000);
};