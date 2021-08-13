#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets/MrBoboSockets.h>
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
#include <MBDNSHandler/MBDNSHandler.h>
#include <MrBoboMail/MrBoboMail.h>
#include <MBCrypto/MBCrypto.h>
#include <MBTorrent/MBBitTorrent.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	MBSockets::Init();
	//std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	return(MBGWebsiteMain());

}
