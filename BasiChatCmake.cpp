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
	//karatsuba test
	//hash tester
	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	char Data[48];
	std::ifstream TestFil("DebugHmac21");
	std::cout << TestFil.is_open() << std::endl;
	unsigned int BitsRead = TestFil.read(Data, 48).gcount();
	std::cout << BitsRead << std::endl;
	std::cout << HexEncodeString(std::string(Data,48)) << std::endl;
	std::cout << HexEncodeString(MBSha1(std::string(Data, 48))) << std::endl;
	std::cout << HexEncodeString(MBSha1("123123")) << std::endl;
	//exit(0);
	MrBigInt Five(5);
	MrBigInt CurrentNumber = 5;
	std::cout << HexEncodeString(MBSha1("TestTest")) << std::endl;
	for (size_t i = 0; i < 10; i++)
	{
		MrBigInt NewKaratsuba;
		MrBigInt NewLong;
		double KaratsubaTime = 0;
		double LongTime = 0;
		clock_t Timer = clock();
		MrBigInt::KaratsubaMultiplication(CurrentNumber, CurrentNumber, NewKaratsuba);
		if (i % 1 == 0)
		{
			std::cout << "Karatsuba time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
		}
		Timer = clock();
		MrBigInt::LongMultiplication(CurrentNumber, CurrentNumber, NewLong);
		if (i % 1 == 0)
		{
			std::cout << "Long time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			std::cout << "Number size: " << CurrentNumber.GetNumberOfUnits() << std::endl;
		}
		if (NewKaratsuba != NewLong)
		{
			exit(0);
		}
		CurrentNumber = NewLong;
	}
	std::cout << TLS1_2::Base64ToBinary("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=") << std::endl;
	//MrBigInt Test = MrBigInt(256).Pow(512);
	MBSockets::Init();
	MBSockets::HTTPConnectSocket TestSocket("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	TestSocket.Connect();
	TestSocket.EstablishSecureConnetion();
	std::cout<<TestSocket.GetDataFromRequest("GET", "/")<<std::endl;

	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler(DefaultSearch);
	TestServer.StartListening();
	return(0);
}
