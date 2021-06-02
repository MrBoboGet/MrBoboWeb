#pragma once
#include <vector>
#include <unordered_map>
namespace MBSearchEngine
{
	struct MBSE_CLI_ProcessedUserInput
	{
		std::vector<std::string> TotalCommandTokens = {};
		std::unordered_map<std::string, int> CommandOptions = {};
		std::vector<std::string> CommandArguments = {};
		std::vector<std::string> CommandTopDirectives = {};
	};
	class MBSearchEngineCLI
	{
	private:
		bool m_IsInteractiveConsole = false;
		MBSE_CLI_ProcessedUserInput m_InitialCommand = {};
		void m_CreatePathIndex(std::string const& PathToProcess, std::string const& IndexOutputName, bool IsRecursive,bool IdentifiersAreRelative);
		void m_CreateFileIndex(std::string const& PathToProcess, std::string const& IndexOutputName, bool IsRecursive,bool IdentifiersAreRelative);
		int m_ExecuteCommand(MBSE_CLI_ProcessedUserInput const& CommandToExecute);
		int m_ExecuteSearchCommand(MBSE_CLI_ProcessedUserInput const& CommandToExecute);
		int m_ExecuteCreateCommand(MBSE_CLI_ProcessedUserInput const& CommandToExecute);
		int m_ExecuteProbeCommand(MBSE_CLI_ProcessedUserInput const& CommandToExecute);
		int m_ExecuteHelpCommand(MBSE_CLI_ProcessedUserInput const& CommandToExecute);
	public:
		MBSearchEngineCLI(int argc, const char*const*);
		int MainLoop();
	};
}