#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets/MrBoboSockets.h>
#include <Crawler.h>
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboChatt/MrBoboChatt.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MrBoboMedia/MBMedia.h>
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
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	MBSockets::Init();
	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");

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
	
	MBGWebsiteMain();
	exit(0);
	MBTorrent::MBBitTorrentHandler TestTorrent;
	TestTorrent.LoadTorrentInfo("Serial Experiments Lain Storyboard.torrent");
	TestTorrent.StartDownload();
	return(0);
}
