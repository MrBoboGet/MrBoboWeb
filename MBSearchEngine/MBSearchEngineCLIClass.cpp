#include <MBSearchEngine/MBSearchEngineCLIClass.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <MBUnicode/MBUnicode.h>
#include <MBUtility/MBStrings.h>
#include <filesystem>
#include <codecvt>
#include <stack>
#include <MBSearchEngine/MBRecursiveDirectoryIterator.h>
namespace MBSearchEngine
{
	MBSearchEngineCLI::MBSearchEngineCLI(int argc,const char*const* argv)
	{
		//std::unordered_map<std::string, bool> CommandOptions = {};
		//std::vector<std::string> CommandMainArguments = {};
		for (size_t i = 1; i < argc; i++)
		{
			m_InitialCommand.TotalCommandTokens.push_back(argv[i]);
			std::string ArgumentString = std::string(argv[i]);
			if (ArgumentString.substr(0,2) == "--")
			{
				m_InitialCommand.CommandTopDirectives.push_back(ArgumentString);
			}
			else if(ArgumentString[0] == '-')
			{
				m_InitialCommand.CommandOptions[ArgumentString] = i;
			}
			else
			{
				m_InitialCommand.CommandArguments.push_back(ArgumentString);
			}
		}
		if (m_InitialCommand.TotalCommandTokens.size() == 0)
		{
			//ska öppna console programm grejen
			m_IsInteractiveConsole = true;
		}
	}
	int MBSearchEngineCLI::MainLoop()
	{

		int ReturnValue = 0;
		if (m_IsInteractiveConsole)
		{
			//interactive console loop
			std::cout << "Entering Interacitve mode:\nType --exit or Ctrl+D To exit\n";
			std::string CurrentInputLine = "";
			std::string CurrentStatusLine = "mbse";
			while (std::getline(std::cin,CurrentInputLine))
			{

				std::cout << CurrentStatusLine+">";
			}
		}
		else
		{
			int CommandResult = m_ExecuteCommand(m_InitialCommand);
			ReturnValue = CommandResult;
		}
		return(ReturnValue);
	}
	int MBSearchEngineCLI::m_ExecuteCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		int ReturnValue = 0;
		if (UserInput.CommandTopDirectives.size() == 0)
		{
			std::cout << "Need a top level command" << std::endl;
			ReturnValue = -1;
		}
		else if (UserInput.CommandTopDirectives.size() > 1)
		{
			std::cout << "Only one top level command is valid" << std::endl;
			ReturnValue = -1;
		}
		else if (UserInput.CommandTopDirectives[0] == "--c" || UserInput.CommandTopDirectives[0] == "--create")
		{
			ReturnValue = m_ExecuteCreateCommand(UserInput);
		}
		else if (UserInput.CommandTopDirectives[0] == "--s" || UserInput.CommandTopDirectives[0] == "--search")
		{
			ReturnValue = m_ExecuteSearchCommand(UserInput);
		}
		else if (UserInput.CommandTopDirectives[0] == "--p" || UserInput.CommandTopDirectives[0] == "--probe")
		{
			ReturnValue = m_ExecuteProbeCommand(UserInput);
		}
		else if (UserInput.CommandTopDirectives[0] == "--h" || UserInput.CommandTopDirectives[0] == "--help")
		{
			ReturnValue = m_ExecuteHelpCommand(UserInput);
		}
		else
		{
			std::cout << "Invalid top level command" << std::endl;
			ReturnValue = -1;
		}
		return(ReturnValue);
	}
	int MBSearchEngineCLI::m_ExecuteSearchCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		int ReturnValue = 0;
		if (UserInput.CommandArguments.size() != 2)
		{
			std::cout << "Invalid number of top level arguments" << std::endl;
			ReturnValue = -1;
		}
		else
		{
			size_t NumberOfMatches = 0;
			bool ShouldCountMatches = (UserInput.CommandOptions.find("-n") != UserInput.CommandOptions.end());
			bool IsBoolean = (UserInput.CommandOptions.find("-b") != UserInput.CommandOptions.end());
			MBIndex IndexToSearch = MBIndex();
			IndexToSearch.Load(UserInput.CommandArguments[0]);
			std::vector<std::string> QuerryResult = {};
			if (IsBoolean)
			{
				BooleanQuerryIterator QuerryIterator = IndexToSearch.GetBooleanQuerryIterator(UserInput.CommandArguments[1]);
				while (!QuerryIterator.HasEnded())
				{
					if (!ShouldCountMatches)
					{
						QuerryResult.push_back(IndexToSearch.GetDocumentIdentifier(*QuerryIterator));
					}
					else
					{
						NumberOfMatches += 1;
					}
					QuerryIterator++;
				}
			}
			else
			{
				QuerryResult = IndexToSearch.EvaluteVectorModelQuerry(UserInput.CommandArguments[1]);
			}
			if (!ShouldCountMatches)
			{
				for (size_t i = 0; i < QuerryResult.size(); i++)
				{
					std::cout << QuerryResult[i] << std::endl;
				}
			}
			else
			{
				std::cout << "Number of matches: " << NumberOfMatches << std::endl;
			}
		}
		return(ReturnValue);
	}
	int MBSearchEngineCLI::m_ExecuteCreateCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		int ReturnValue = 0;
		bool IsRecursive = (UserInput.CommandOptions.find("-r") != UserInput.CommandOptions.end());
		bool IsPathIndex = (UserInput.CommandOptions.find("-p") != UserInput.CommandOptions.end());
		bool IdentifiersAreRelative = (UserInput.CommandOptions.find("-rel") != UserInput.CommandOptions.end());
		if (UserInput.CommandArguments.size() != 2)
		{
			std::cout << "Invalid number of top level arguments" << std::endl;
			ReturnValue = -1;
		}
		else
		{
			std::string TopPath = UserInput.CommandArguments[0];
			std::string OutputIndexName = UserInput.CommandArguments[1];
			if (IsPathIndex)
			{
				m_CreatePathIndex(TopPath, OutputIndexName, IsRecursive, IdentifiersAreRelative);
			}
			else
			{
				m_CreateFileIndex(TopPath, OutputIndexName, IsRecursive, IdentifiersAreRelative);
			}
		}
		return(ReturnValue);
	}
	int MBSearchEngineCLI::m_ExecuteProbeCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		if (UserInput.CommandArguments.size() != 1)
		{
			std::cout << "Invalid number of arguments" << std::endl;
			return(-1);
		}
		if (UserInput.CommandOptions.size() != 0)
		{
			std::cout << "Invalid options passed" << std::endl;
			return(-1);
		}
		MBUtility::MBRecursiveDirectoryIterator TotalIterator = MBUtility::MBRecursiveDirectoryIterator(UserInput.CommandArguments[0]);
		size_t TotalNumberOfFiles = 0;
		clock_t StartTime = clock();
		size_t NumberOfErrors = 0;
		std::error_code ErrorCode;
		std::stack<std::filesystem::path> DirectorysToTraverse = std::stack<std::filesystem::path>();
		while (!TotalIterator.HasEnded())
		{
			std::filesystem::directory_entry CurrentEntry = TotalIterator.GetCurrentEntry();
			if (CurrentEntry.is_regular_file(ErrorCode))
			{
				TotalNumberOfFiles += 1;
			}
			if (CurrentEntry.is_directory(ErrorCode))
			{
				DirectorysToTraverse.push(CurrentEntry.path());
			}
			if (ErrorCode)
			{
				NumberOfErrors += 1;
			}
			TotalIterator.Increment();
		}
		clock_t EndTime = clock();
		std::cout << "Total number of files: " << TotalNumberOfFiles << std::endl;
		std::cout << "Total traversal time: " << (EndTime - StartTime) / double(CLOCKS_PER_SEC) << std::endl;
		std::cout << "Total number of errors: " << NumberOfErrors << std::endl;
		return(0);
	}
	int MBSearchEngineCLI::m_ExecuteHelpCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		std::cout << "Program to create and search in MBIndexes" << std::endl;
		std::cout << "A command consists of a top level command, which starts with --," << std::endl;
		std::cout << "followed by options starting with - with eventual option arguments and lastly the top level commands arguments" << std::endl;
		std::cout << "--------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "--c/--create (PathToProcess): creates and index of the files in the path" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "-r: searches for files in subfolders also" << std::endl;
		std::cout << "-p: instead of creating and index of the file contents it creates and index of the files path's" << std::endl;
		std::cout << "--------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "--s/--search (IndexToSearch,SearchString): search the given index with the provided string" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "-b: The search string should be intepreted as a boolean querry instead of a vector model querry" << std::endl;
		std::cout << "--------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "--p/--probe (PathToProbe): Displays info for the current path, including the number of files in the directory and subfolders" << std::endl;
		std::cout << "--------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "--h/--help: displays this text" << std::endl;
		return(0);
	}
	void MBSearchEngineCLI::m_CreatePathIndex(std::string const& PathToProcess, std::string const& IndexOutputName, bool IsRecursive, bool IdentifiersAreRelative)
	{
		MBIndex ResultIndex = MBIndex();
		std::error_code ErrorCode;
		if (IsRecursive)
		{
			MBUtility::MBRecursiveDirectoryIterator DirectoryIterator = MBUtility::MBRecursiveDirectoryIterator(PathToProcess);
			while (!DirectoryIterator.HasEnded())
			{
				std::filesystem::directory_entry CurrentEntry = DirectoryIterator.GetCurrentEntry();
				if (CurrentEntry.is_regular_file(ErrorCode))
				{
					std::string EntryToAdd = "";
					std::string EntryPath = "";
					if (IdentifiersAreRelative)
					{
						//EntryPath = MBUnicode::Convert_U16_U8((char16_t*)CurrentEntry.path().lexically_normal().c_str());
						EntryPath = MBUnicode::PathToUTF8(CurrentEntry.path().lexically_normal());
					}
					else
					{
						std::filesystem::path WorkingDirectory = std::filesystem::current_path();
						std::filesystem::path CurrentEntryPath = CurrentEntry.path();
						if (CurrentEntryPath.is_relative())
						{
							WorkingDirectory+=CurrentEntryPath;
							EntryPath = MBUnicode::PathToUTF8(WorkingDirectory.lexically_normal());
						}
						else
						{
							EntryPath = MBUnicode::PathToUTF8(CurrentEntryPath.lexically_normal());
						}
					}
					EntryToAdd = MBUtility::ReplaceAll(EntryPath, "/", " ");
					EntryToAdd = MBUtility::ReplaceAll(EntryPath, "\\", " ");
					EntryToAdd = MBUtility::ReplaceAll(EntryToAdd, "_", " ");
					EntryToAdd = MBUtility::ReplaceAll(EntryToAdd, ".", " ");
					//DEBUG
					//std::string DEBUG_StringToSearch = "emanu";
					//if (EntryPath.size() >= DEBUG_StringToSearch.size())
					//{
					//	if (EntryPath.substr(EntryPath.size() - DEBUG_StringToSearch.size()) == DEBUG_StringToSearch)
					//	{
					//		std::cout << "Hittar stringen vi vill ha: " << EntryPath << std::endl;
					//	}
					//	if (EntryPath.find(DEBUG_StringToSearch) != EntryPath.npos)
					//	{
					//		std::cout << "stringe är i pathen: " << EntryPath << std::endl;
					//	}
					//}
					ResultIndex.IndextTextData(EntryToAdd, EntryPath);
				}
				DirectoryIterator. Increment();
			}
		}
		else
		{
			std::filesystem::directory_iterator DirectoryIterator = std::filesystem::directory_iterator(PathToProcess);
			std::filesystem::directory_iterator EndIterator = std::filesystem::directory_iterator();
			while (DirectoryIterator != EndIterator)
			{
				std::filesystem::directory_entry CurrentEntry = *DirectoryIterator;
				if (CurrentEntry.is_regular_file(ErrorCode))
				{
					std::string EntryPath = MBUnicode::PathToUTF8(CurrentEntry.path().lexically_normal());
					std::string EntryToAdd = EntryPath;
					EntryToAdd = MBUtility::ReplaceAll(EntryToAdd, "/", " ");
					EntryToAdd = MBUtility::ReplaceAll(EntryToAdd, "\\", " ");
					EntryToAdd = MBUtility::ReplaceAll(EntryToAdd, "_", " ");
					ResultIndex.IndextTextData(EntryToAdd, EntryPath);
				}
				DirectoryIterator.increment(ErrorCode);
			}
		}
		ResultIndex.Finalize();
		ResultIndex.Save(IndexOutputName);
	}
	void MBSearchEngineCLI::m_CreateFileIndex(std::string const& PathToProcess, std::string const& IndexOutputName, bool IsRecursive, bool IdentifiersAreRelative)
	{
		*((int*) (0)) = 1;
	}
}

/*
int MBSearchEngineCLI::m_ExecuteProbeCommand(MBSE_CLI_ProcessedUserInput const& UserInput)
	{
		if (UserInput.CommandArguments.size() != 1)
		{
			std::cout << "Invalid number of arguments" << std::endl;
			return(-1);
		}
		if (UserInput.CommandOptions.size() != 0)
		{
			std::cout << "Invalid options passed" << std::endl;
			return(-1);
		}
		std::filesystem::directory_iterator TotalIterator = std::filesystem::directory_iterator(UserInput.CommandArguments[0]);
		std::filesystem::directory_iterator EndIterator = std::filesystem::directory_iterator();
		size_t TotalNumberOfFiles = 0;
		clock_t StartTime = clock();
		size_t NumberOfErrors = 0;
		std::error_code ErrorCode;
		std::stack<std::filesystem::path> DirectorysToTraverse = std::stack<std::filesystem::path>();
		while (TotalIterator != EndIterator)
		{
			std::filesystem::directory_entry CurrentEntry = *TotalIterator;
			if (CurrentEntry.is_regular_file(ErrorCode))
			{
				TotalNumberOfFiles += 1;
			}
			if (CurrentEntry.is_directory(ErrorCode))
			{
				DirectorysToTraverse.push(CurrentEntry.path());
			}
			if (ErrorCode)
			{
				NumberOfErrors += 1;
			}
			TotalIterator.increment(ErrorCode);
			if (ErrorCode)
			{
				NumberOfErrors += 1;
			}
			if (TotalIterator == EndIterator)
			{
				bool ShouldBreak = false;
				while(true)
				{
					if (DirectorysToTraverse.empty())
					{
						ShouldBreak = true;
						break;
					}
					std::filesystem::path NewFolderToTraverse = DirectorysToTraverse.top();
					DirectorysToTraverse.pop();
					TotalIterator = std::filesystem::directory_iterator(NewFolderToTraverse,ErrorCode);
					if (!ErrorCode && TotalIterator != EndIterator)
					{
						break;
					}
				}
				if (ShouldBreak)
				{
					break;
				}
			}
		}
		clock_t EndTime = clock();
		std::cout << "Total number of files: " << TotalNumberOfFiles << std::endl;
		std::cout << "Total traversal time: " << (EndTime - StartTime) / double(CLOCKS_PER_SEC) << std::endl;
		std::cout << "Total number of errors: " << NumberOfErrors << std::endl;
		return(0);
*/