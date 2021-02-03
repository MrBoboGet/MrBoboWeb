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

int main()
{
#ifdef DNDEBUG
	std::cout << "Is Debug" << std::endl;
#endif // DEBUG
	 std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	MBSockets::Init();
	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler(DefaultSearch);
	TestServer.StartListening();
	return(0);
}
