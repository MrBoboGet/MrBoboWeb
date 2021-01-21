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
	MBSockets::Init();
	MBSockets::HTTPConnectSocket TestSocket("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	TestSocket.Connect();
	TestSocket.EstablishSecureConnetion();
	std::cout<<TestSocket.GetDataFromRequest("GET", "/")<<std::endl;

	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	MrPostOGet::HTTPServer TestServer("./ServerResources/MrBoboGet/HTMLResources/", 80);
	TestServer.AddRequestHandler(DefaultSearch);
	TestServer.StartListening();
	return(0);
}