#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets/MrBoboSockets.h>
#include <Crawler.h>
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboChatt/MrBoboChatt.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MBWebsite/MPGMemeSite.h>
//#include <MrBoboMedia/MBMedia.h>
#include <MBUnicode/MBUnicode.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <time.h>
#include <Crawler/MBCrawler.h>
#include <Crawler/MBCrawlerSite.h>
#include <MrPostOGet/MBHTMLParser.h>
#include <MBDNSHandler/MBDNSHandler.h>
#include <MrBoboMail/MrBoboMail.h>
#include <MBCrypto/MBCrypto.h>
#include <MBTorrent/MBBitTorrent.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
#include <MBSystem/MBSystem.h>
#include <MBCrypto/MBCrypto.h>
#include <MBUtility/MBFiles.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	MBSockets::Init();


	std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");

	std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;


	//MBUtility::MBFileInputStream TestInputStream("./Game_20200630T190553.slp");
	//
	//MBParsing::JSONObject Object = MBParsing::ParseUBJSON(&TestInputStream, nullptr);
	//
	//std::cout << Object["metadata"].ToString()<<std::endl;
	//exit(0);



	//MBSystem::SubProcess TestProcess("dir");
	//while (!TestProcess.Finished())
	//{
	//	std::cout << TestProcess.RecieveData();
	//}
	//std::cout << std::endl;
	//exit(0);
	//return(MBGWebsiteMain());
	//MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	//TestServer.LoadDomainResourcePaths("MPGDomainResourcePaths.txt");
	//TestServer.UseTLS(false);
	//MBDB_Website_GitHandler* HandlerToAdd = new MBDB_Website_GitHandler("C:/Users/emanu/Desktop/Program/",nullptr);
	//HandlerToAdd->SetURLPrefix("/");
	//TestServer.AddRequestHandler(HandlerToAdd);
	//TestServer.StartListening();
	//MBSockets::HTTPConnectSocket TestSocket("www.google.com", "443");
	//TestSocket.Connect();
	//MBError TLSResult = TestSocket.EstablishTLSConnection();
	//std::cout << TLSResult.ErrorMessage << std::endl;
	//std::cout << TestSocket.GetDataFromRequest("GET","/") << std::endl;
	
	//MBSockets::HTTPClientSocket TestSocket;
	//TestSocket.Connect("www.google.com", "443");
	//TestSocket.EstablishTLSConnection();
	//std::cout << TestSocket.GetDataFromRequest("GET", "/") << std::endl;
	//std::cout << std::filesystem::current_path() << std::endl;
	//
	//MBUnicode::CreateCodepointPropertiesHeader("../../MBUnicode/");
	std::string TestLine = "\xc3\xa5\xc3\xb6\xc3\xa4\xc3\xa5\xc3\xb6\xc3\xa4\xc3\xa5";
	//std::getline(std::cin, TestLine);
	MBUnicode::UnicodeCodepointSegmenter Segmenter;
	Segmenter.InsertData(TestLine.data(), TestLine.size());
	MBUnicode::GraphemeClusterSegmenter ClusterSegmenter;
	while (Segmenter.AvailableCodepoints() > 0)
	{
		ClusterSegmenter.InsertCodepoint(Segmenter.ExtractCodepoint());
	}
	ClusterSegmenter.Finalize();
	std::vector<MBUnicode::GraphemeCluster> ExtractedClusters;
	std::string StringToPrint;
	while (ClusterSegmenter.AvailableClusters() > 0)
	{
		ExtractedClusters.push_back(ClusterSegmenter.ExtractCluster());
		StringToPrint += ExtractedClusters.back().ToString();
	}
	std::cout << TestLine.size() << std::endl;
	std::cout << ExtractedClusters.size() << std::endl;
	std::cout << (TestLine == StringToPrint) << std::endl;
	std::cout << StringToPrint << std::endl;
	std::cout << MBUtility::HexEncodeString(TestLine) << std::endl;
	std::cout << MBUtility::HexEncodeString(StringToPrint) << std::endl;

	//exit(0);
	//return(0);z
	MBWebsite::MBGWebsiteMain();
	exit(0);
	MBTorrent::MBBitTorrentHandler TestTorrent;
	TestTorrent.LoadTorrentInfo("Serial Experiments Lain Storyboard.torrent");
	TestTorrent.StartDownload();
	return(0);
}
