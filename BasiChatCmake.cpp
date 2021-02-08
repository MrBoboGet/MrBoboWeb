#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets.h>
#include <Crawler.h>
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MrPostOGet/SearchEngineImplementation.h>
#include <MrPostOGet/TLSHandler.h>
#include <MrBoboChatt/MrBoboChatt.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrBoboDatabase/MPGMemeSite.h>
#include <time.h>
int main()
{
#ifdef DNDEBUG
	std::cout << "Is Debug" << std::endl;
#endif // DEBUG
	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	///*
	std::cout << std::filesystem::current_path() << std::endl;
	const char* TestDatabas = "./TestDatabas";
	MBDB::MrBoboDatabase TestDatabase(TestDatabas,0);
	auto TestQuerry = TestDatabase.GetAllRows("SELECT * FROM Memes");
	std::tuple<int, std::string, std::string> Test = std::tuple_cat(std::make_tuple(2), std::make_tuple(std::string("Hej"), std::string("Hej")));
	std::tuple<int, std::string, std::string> MemeEntry = TestQuerry[0].GetTuple<int, std::string, std::string>();
	std::cout << std::get<0>(MemeEntry)<<" "<< std::get<1>(MemeEntry)<<" " << std::get<2>(MemeEntry) << std::endl;
	//*/
	MBSockets::Init();
	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler({ DBSite_Predicate,DBSite_ResponseGenerator });
	TestServer.StartListening();
	return(0);
	//sqlite3_prepare("HejsanSvejsan");
}
