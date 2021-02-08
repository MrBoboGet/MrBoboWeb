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
	std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	MBSockets::Init();
	///*
	UnitType DebugRemainder;
	MrBigInt DebugQuot;
	//MrBigInt::divi
	MrBigInt::DoubleUnitDivide(1, 1603210591, 3386463223,&DebugQuot,&DebugRemainder, nullptr);
	MrBigInt RegularRemainder;
	MrBigInt RegularQuot;
	MrBigInt RegularDivInt;
	RegularDivInt = 1;
	RegularDivInt <<= 32;
	RegularDivInt += 1603210591;
	RegularQuot = RegularDivInt / 3386463223;
	RegularRemainder = (RegularDivInt % 3386463223);


	//test att connecta till amazon
	MBSockets::HTTPConnectSocket AmazonConnectTest("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	AmazonConnectTest.Connect();
	AmazonConnectTest.EstablishSecureConnetion();
	std::cout << AmazonConnectTest.GetDataFromRequest("GET", "/") << std::endl;

	//std::cout << std::filesystem::current_path() << std::endl;
	std::string NumberData = std::string(1024, 0);
	std::ifstream TestData("BigIntTestData", std::ios::in | std::ios::binary);
	TestData.read((char*)NumberData.c_str(), 1024);
	//TestData << NumberData;
	//exit(0);
	MrBigInt NumberToPowM;
	NumberToPowM.SetFromBigEndianArray(NumberData.c_str(), NumberData.size());
	RSADecryptInfo DecryptInfo = TLSHandler().GetRSADecryptInfo("mrboboget.se");
	MrBigInt Result;
	MrBigInt SlidingResult;
	MrBigInt::PowM(NumberToPowM, DecryptInfo.PrivateExponent, DecryptInfo.PublicModulu, Result);
	MrBigInt::PowM_SlidinWindow(NumberToPowM, DecryptInfo.PrivateExponent, DecryptInfo.PublicModulu, SlidingResult);
	std::cout << Result.GetHexEncodedString() << std::endl;
	std::cout << SlidingResult.GetHexEncodedString() << std::endl;
	assert(SlidingResult == Result);
	const char* TestDatabas = "./TestDatabas";
	MBDB::MrBoboDatabase TestDatabase(TestDatabas,0);
	auto TestQuerry = TestDatabase.GetAllRows("SELECT * FROM Memes");
	std::tuple<int, std::string, std::string> Test = std::tuple_cat(std::make_tuple(2), std::make_tuple(std::string("Hej"), std::string("Hej")));
	std::tuple<int, std::string, std::string> MemeEntry = TestQuerry[0].GetTuple<int, std::string, std::string>();
	std::cout << std::get<0>(MemeEntry)<<" "<< std::get<1>(MemeEntry)<<" " << std::get<2>(MemeEntry) << std::endl;
	//*/
	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler({ DBSite_Predicate,DBSite_ResponseGenerator });
	TestServer.StartListening();
	return(0);
	//sqlite3_prepare("HejsanSvejsan");
}
