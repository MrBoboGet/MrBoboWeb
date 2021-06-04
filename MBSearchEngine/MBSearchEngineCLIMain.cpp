#include <MBSearchEngine/MBSearchEngineCLIClass.h>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <time.h>
int main(int argc, char** argv)
{
	int ProgramReturnValue = 0;
	//std::cout << "DEBUG SESSION" << std::endl;

	argc = 5;
	const char** argv2 =(const char**) malloc(argc * sizeof(char*));
	argv2[0] = "mbse";
	argv2[1] = "--s";
	argv2[2] = "-b";
	argv2[3] = "TotalPathIndex";
	argv2[4] = ".GITIGNORE AND BUILD";

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