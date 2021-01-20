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
	//initialiserar lite lokala variablar s� vi kan f� grejer att connecta utan att jag specificerar 
	//std::fstream TestFil("TestTest.txt", std::ios::binary | std::ios::out);
	//TestFil << "HejHej";
	//TestFil.close();
	//std::cout << "Testfile is created" << std::endl;
	
	//MBSockets::Init();
	//TestMBChatGrejer();
	//MrBoboChat TestGrej = MrBoboChat();
	//TestGrej.MainLoop();

	///*
	MBSockets::Init();
	std::cout << "Skapar socketen" << std::endl;
	MBSockets::HTTPConnectSocket TestSocket("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	std::cout << "B�rjar TLS connection" << std::endl;
	TestSocket.Connect();
	TestSocket.EstablishSecureConnetion();
	std::cout << "B�rjar Get Request" << std::endl;
	std::cout<< TestSocket.GetDataFromRequest("GET", "/")<<std::endl;

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