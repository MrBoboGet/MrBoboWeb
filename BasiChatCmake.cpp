#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets.h>
#include <Crawler.h>
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MrPostOGet/SearchEngineImplementation.h>
#include <MrPostOGet/TLSHandler.h>
#include <MrBoboChatt/MrBoboChatt.h>
#include <time.h>
void ListeningPart2(std::string PortToListenTo)
{
	MBSockets::ServerSocket ServerSocketen(PortToListenTo, MBSockets::TraversalProtocol::TCP);
	ServerSocketen.Bind();
	ServerSocketen.Listen();
	ServerSocketen.Accept();
	char RecievadData[500];
	int LengthOfData;
	while((LengthOfData = ServerSocketen.RecieveData(RecievadData,500)) > 0)
	{
		std::cout << std::string(RecievadData, LengthOfData) << std::endl;
	}
}
void SendingPart2(std::string IPAdressToConnectTo,std::string Port)
{
	MBSockets::ConnectSocket ConnectSocketen(IPAdressToConnectTo, Port, MBSockets::TraversalProtocol::TCP);
	ConnectSocketen.Connect();
	while(true)
	{
		std::string DataAttSkicka;
		std::getline(std::cin, DataAttSkicka);
		ConnectSocketen.SendData(DataAttSkicka.c_str(), DataAttSkicka.length());
	}
}
std::mutex WorkerTestMutex;
void WorkerTestFunction(int Id)
{
	std::lock_guard<std::mutex> Locken(WorkerTestMutex);
	std::cout << "Worker function " << Id << " printar detta" << std::endl;
}
int main()
{
	//initialiserar lite lokala variablar så vi kan få grejer att connecta utan att jag specificerar 
	//std::fstream TestFil("TestTest.txt", std::ios::binary | std::ios::out);
	//TestFil << "HejHej";
	//TestFil.close();
	//std::cout << "Testfile is created" << std::endl;
	
	//MBSockets::Init();
	//TestMBChatGrejer();
	//MrBoboChat TestGrej = MrBoboChat();
	//TestGrej.MainLoop();

	///*
	
	
	//mrbigint test grejer
	MrBigInt TestInt(256);
	//std::cout << "256^256 " << std::endl << TestInt.Pow(256).GetString() << std::endl;
	//std::cout << "123123^123%123321"<<std::endl<<MrBigInt(123123).PowM(MrBigInt(123), MrBigInt(123321)).GetString() << std::endl;
	std::cout << MrBigInt(5).Pow(100).GetString() << std::endl;


	MBSockets::Init();
	std::cout << "Skapar socketen" << std::endl;
	MBSockets::HTTPConnectSocket TestSocket("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	std::cout << "Börjar TLS connection" << std::endl;
	TestSocket.Connect();
	TestSocket.EstablishSecureConnetion();
	std::cout << "Börjar Get Request" << std::endl;
	std::cout<< TestSocket.GetDataFromRequest("GET", "/")<<std::endl;

	//test kryptering
	exit(0);
	unsigned char IVToTest[16] = { 0x86, 0xd3, 0xe4, 0xd5, 0x0a, 0x45, 0x9b, 0x3e, 0x83, 0x5c, 0xef, 0x41, 0xa8, 0x96, 0x15, 0x74 };
	char ClientWriteMacKey[] = { 0x97, 0xd9, 0xb1, 0x06, 0x06, 0xc6, 0x75, 0xe8, 0x6e, 0x52, 0x94, 0x8b, 0xc5, 0x9f, 0x51, 0x26, 0x97, 0x8c, 0x8e, 0xf1, 0x63, 0x0b,0x27, 0x9d,0x4e,0xd6,0x28,0x6f,0xc3,0x96,0x67,0x90 };
	char ClientWriteKey[] = {
		0x62,
		0x95,
		0x4b,
		0x70,
		0xdc,
		0xaf,
		0xaf,
		0x23,
		0x0d,
		0x85,
		0xc7,
		0x00,
		0x14,
		0xa3,
		0xc6,
		0xd2 };
	char RecordData[] = {
		0x14,
		0x00,
		0x00,
		0x0c,
		0x0d,
		0x43,
		0xd5,
		0xd7,
		0x1e,
		0x47,
		0x1d,
		0x7a,
		0xcd,
		0xb2,
		0xf0,
		0x63, };
	TLS1_2::TLS1_2GenericRecord Record;
	Record.Type = TLS1_2::handshake;
	Record.Protocol = { 3,3 };
	Record.Length = 4 + 12;
	Record.Data = std::string(RecordData, sizeof(RecordData));

	TLSHandler TestHandler;
	TestHandler.TestEncryptRecord(IVToTest, 0,std::string(ClientWriteMacKey, sizeof(ClientWriteMacKey)), std::string(ClientWriteKey, sizeof(ClientWriteKey)),Record);
	//host test
	//MBSockets::ServerSocket TestHostSocket = MBSockets::ServerSocket("443", MBSockets::TraversalProtocol::TCP);
	//TestHostSocket.Bind();
	//TestHostSocket.Listen();
	//TestHostSocket.Accept();
	//TestHostSocket.EstablishSecureConnection();


	//MrBigInt Out;
	//MrBigInt::Pow(MrBigInt(12312323), 123, Out);
	//*/
	
	
	/*
	clock_t OriginalTime = clock();
	clock_t Timer = clock();
	int SendIntervall = 1;
	while (true)
	{
		if ((clock() - Timer) / float(CLOCKS_PER_SEC) > SendIntervall)
		{
			Timer = clock();
			std::cout << "Offset since start is: " << (clock()-OriginalTime) / float(CLOCKS_PER_SEC)<<std::endl;
		}
	}
	*/
	/*
	MBSockets::UDPSocket Test("127.0.0.1", "7331", MBSockets::TraversalProtocol::TCP);
	Test.Bind(7331);
	std::string TestString = "Test";
	Test.UDPSendData(TestString, "127.0.0.1", 7331);
	std::cout << "Test Fungerade" << std::endl;
	*/
	/*
	MBSockets::Init();
	MBSockets::HTTPConnectSocket ConnectSocket("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	ConnectSocket.Connect();
	std::cout << ConnectSocket.GetIpOfConnectedSocket() << std::endl;
	ConnectSocket.EstablishSecureConnetion();
	std::string DataFromRequest = ConnectSocket.GetDataFromRequest("GET", "/");
	std::cout<< DataFromRequest <<std::endl;
	int a = 2;
	*/
	//std::cout << "Hello world" << std::endl;
	return(0);
}