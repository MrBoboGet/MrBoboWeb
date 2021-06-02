#pragma once
#include <filesystem>
#include <stack>
namespace MBUtility
{
	class MBRecursiveDirectoryIterator
	{
	private:
		std::error_code ErrorCode;
		std::stack<std::filesystem::path> m_DirectorysToTraverse = std::stack<std::filesystem::path>();
		bool m_HasEnded = false;
		std::filesystem::directory_iterator TotalIterator = std::filesystem::directory_iterator();
		std::filesystem::directory_iterator EndIterator = std::filesystem::directory_iterator();
		std::filesystem::directory_entry m_CurrentDirectoryEntry = std::filesystem::directory_entry();
	public:
		MBRecursiveDirectoryIterator(std::filesystem::path PathToIterate)
		{
			TotalIterator = std::filesystem::directory_iterator(PathToIterate, ErrorCode);
			if (ErrorCode)
			{
				m_HasEnded = true;
			}
			else
			{
				Increment();
			}
		}
		void Increment()
		{
			m_CurrentDirectoryEntry = *TotalIterator;
			if (m_CurrentDirectoryEntry.is_directory(ErrorCode))
			{
				m_DirectorysToTraverse.push(m_CurrentDirectoryEntry.path());
			}
			TotalIterator.increment(ErrorCode);
			if (TotalIterator == EndIterator)
			{
				bool IterationEnded = false;
				while (true)
				{
					if (m_DirectorysToTraverse.empty())
					{
						IterationEnded = true;
						break;
					}
					std::filesystem::path NewFolderToTraverse = m_DirectorysToTraverse.top();
					m_DirectorysToTraverse.pop();
					TotalIterator = std::filesystem::directory_iterator(NewFolderToTraverse, ErrorCode);
					if (!ErrorCode && TotalIterator != EndIterator)
					{
						break;
					}
				}
				if (IterationEnded)
				{
					m_HasEnded = true;
				}
			}
		}
		std::filesystem::directory_entry GetCurrentEntry()
		{
			return(m_CurrentDirectoryEntry);
		}
		bool HasEnded()
		{
			return(m_HasEnded);
		}
	};
}