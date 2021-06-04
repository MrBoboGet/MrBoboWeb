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
#include <MrBoboMedia/MBMedia.h>
#include <MBSearchEngine/MBUnicode.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <time.h>
#include <Crawler/MBCrawler.h>
#include <Crawler/MBCrawlerSite.h>
#include <MrPostOGet/MBHTMLParser.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
#ifdef DNDEBUG
	std::cout << "Is Debug" << std::endl;
#endif // DEBUG
	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	MBSockets::Init();
	InitDatabase();
	//std::string FileToTest = "./remar.se/daniel/__DirectoryResource";
	//std::fstream FileToRead = std::fstream(FileToTest, std::ios::in | std::ios::binary);
	//std::string TestHTMLData = std::string(std::filesystem::file_size(FileToTest), 0);
	//FileToRead.read(&TestHTMLData[0], std::filesystem::file_size(FileToTest));
	//HTMLNode TestNode(TestHTMLData,0);
	//std::cout << TestNode.GetVisableText() << std::endl;

	//std::cout << TestDocument.ToText()->Value()<<std::endl;
	//exit(0);

	//MBError RemarIndexError = CreateWebsiteIndex("http://remar.se/", "./remar.se/", "./MBDBResources/Indexes/RemarSiteIndex");
	//exit(0);
	//MBError IndexError =CreateWebsiteIndex("./remar.se/", "RemarTestIndex2");
	//MBSearchEngine::MBIndex TestTestIndex("RemarTestIndex2");
	//std::cout << "Querry tests" << std::endl;
	//std::vector<std::string> Result = TestTestIndex.EvaluteBooleanQuerry("(iji OR iji's OR iji. OR iji,) AND (strawberry OR ludosity) AND \"slap city\"");
	//for (auto& StringToPrint: Result)
	//{
	//	std::cout << StringToPrint << std::endl;
	//}
	//Result = TestTestIndex.EvaluteVectorModelQuerry("iji game development");
	//for (auto& StringToPrint : Result)
	//{
	//	std::cout << StringToPrint << std::endl;
	//}
	
	//std::cout << "Finished indexing!" << std::endl;
	//std::cout << "Index error: " << IndexError.ErrorMessage << std::endl;
	//MrPostOGet::HTTPServer TestCopyServer = MrPostOGet::HTTPServer("RelativeRemar.se/", 443);
	//TestCopyServer.AddRequestHandler({ MBCrawlerSite_Predicate,MBCrawlerSite_ResponseGenerator });
	//TestCopyServer.StartListening();
	
	//MakeWebsiteDirectoryRelative("./remar.se", "RelativeRemar.se/", "remar.se");
	
	//IndexWebsite("remar.se", "./remar.se/",false);

	//std::string Input = "";
	//std::getline(std::cin, Input);
	//MBUnicode::CreateCMacroCodepointArrayFromPropertySpec("./MBSearchEngine/GraphemeBreakProperty.txt", "./MBSearchEngine/GraphemeBreakPropertyMacro.txt");
	//MBUnicode::CodepointRange TestRange[] = GraphemeBreakList;
	//std::cout << TestRange[15].Higher << std::endl;
	//exit(0);
	
	///*
	//test för att använda system

	//CreateHLSStream("./MBDBResources/","./Ep1",10);
	//test för att transcoda
	std::string StringToHash = std::string(std::filesystem::file_size("DebugHash.txt"), 0);
	std::ifstream DebugHash("DebugHash.txt", std::ios::in|std::ios::binary);
	DebugHash.read(&StringToHash.data()[0], std::filesystem::file_size("DebugHash.txt"));
	std::cout << ReplaceAll(HexEncodeString(MBSha1(StringToHash))," ","") << std::endl<<std::endl;
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
	//MBSockets::HTTPConnectSocket AmazonConnectTest("www.amazon.com", "443", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	//AmazonConnectTest.Connect();
	//AmazonConnectTest.EstablishSecureConnetion();
	//std::cout << AmazonConnectTest.GetDataFromRequest("GET", "/") << std::endl;

	//std::cout << std::filesystem::current_path() << std::endl;
	std::string NumberData = std::string(1024, 0);
	std::ifstream TestData("BigIntTestData", std::ios::in | std::ios::binary);
	TestData.read((char*)NumberData.c_str(), 1024);
	//TestData << NumberData;
	//exit(0);
	//MrBigInt NumberToPowM;
	//NumberToPowM.SetFromBigEndianArray(NumberData.c_str(), NumberData.size());
	//RSADecryptInfo DecryptInfo = TLSHandler().GetRSADecryptInfo("mrboboget.se");
	//MrBigInt Result;
	//MrBigInt SlidingResult;
	//MrBigInt::PowM(NumberToPowM, DecryptInfo.PrivateExponent, DecryptInfo.PublicModulu, Result);
	//MrBigInt::PowM_SlidinWindow(NumberToPowM, DecryptInfo.PrivateExponent, DecryptInfo.PublicModulu, SlidingResult);
	//std::cout << Result.GetHexEncodedString() << std::endl;
	//std::cout << SlidingResult.GetHexEncodedString() << std::endl;
	//assert(SlidingResult == Result);
	const char* TestDatabas = "./TestDatabas";
	MBDB::MrBoboDatabase TestDatabase(TestDatabas,0);
	auto TestQuerry = TestDatabase.GetAllRows("SELECT * FROM Memes");
	std::tuple<int, std::string, std::string> Test = std::tuple_cat(std::make_tuple(2), std::make_tuple(std::string("Hej"), std::string("Hej")));
	std::tuple<int, std::string, std::string> MemeEntry = TestQuerry[0].GetTuple<int, std::string, std::string>();
	std::cout << std::get<0>(MemeEntry)<<" "<< std::get<1>(MemeEntry)<<" " << std::get<2>(MemeEntry) << std::endl;
	//*/
	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler({ DBLogin_Predicate,DBLogin_ResponseGenerator });
	TestServer.AddRequestHandler({ DBSite_Predicate,DBSite_ResponseGenerator });
	TestServer.AddRequestHandler({ UploadFile_Predicate,UploadFile_ResponseGenerator });
	TestServer.AddRequestHandler({ DBGet_Predicate,DBGet_ResponseGenerator });
	TestServer.AddRequestHandler({ DBView_Predicate,DBView_ResponseGenerator });
	TestServer.AddRequestHandler({ DBViewEmbedd_Predicate,DBViewEmbedd_ResponseGenerator });
	TestServer.AddRequestHandler({ DBAdd_Predicate,DBAdd_ResponseGenerator });
	TestServer.AddRequestHandler({ DBGeneralAPI_Predicate,DBGeneralAPI_ResponseGenerator });
	TestServer.AddRequestHandler({ DBUpdate_Predicate,DBUpdate_ResponseGenerator });
	TestServer.StartListening();
	return(0);
	//sqlite3_prepare("HejsanSvejsan");
}
