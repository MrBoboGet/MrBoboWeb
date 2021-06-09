#include <MBSearchEngine/MBSearchEngineCLIClass.h>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <time.h>
#include <fstream>
#include <assert.h>
#include <MBSearchEngine/MBSearchEngine.h>
int main(int argc, char** argv)
{
	int ProgramReturnValue = 0;
	//std::cout << "DEBUG SESSION" << std::endl;
	std::cout << std::filesystem::current_path() << std::endl;
	//Test
	//std::fstream TestFile = std::fstream("TotalPathIndex2", std::ios::binary | std::ios::in);
	//char TestCharacter[16];
	//clock_t Timer = clock();
	//assert(TestFile.is_open() && TestFile.good());
	//while (TestFile.read(TestCharacter,16))
	//{
	//	//assert(TestFile.is_open() && TestFile.good());
	//}
	//std::cout << "Reading file byte by byte time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
	//MBSearchEngine::MBIndex Test;
	//Test.Load("TotalPathIndex2");

	argc = 5;
	const char** argv2 =(const char**) malloc(argc * sizeof(char*));
	argv2[0] = "mbse";
	argv2[1] = "--s";
	argv2[2] = "-b";
	argv2[3] = "TotalPathIndex2";
	argv2[4] = "NOT \"BasicChatCmake MBDBResources Music\" AND \"BasicChatCmake MBDBResources\"";

	//argc = 6;
	//const char** argv2 =(const char**) malloc(argc * sizeof(char*));
	//argv2[0] = "mbse";
	//argv2[1] = "--c";
	//argv2[2] = "-r";
	//argv2[3] = "-p";
	//argv2[4] = "C:\\";
	//argv2[5] = "TotalPathIndex2";
	
	//argc = 3;
	//const char** argv2 = (const char**)malloc(argc * sizeof(char*));
	//argv2[0] = "mbse";
	//argv2[1] = "--p";
	//argv2[2] = "C:\\";

	MBSearchEngine::MBSearchEngineCLI CLI(argc, argv2);
	ProgramReturnValue = CLI.MainLoop();
	return(ProgramReturnValue);
}