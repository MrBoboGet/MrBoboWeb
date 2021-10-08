#pragma once
#include <string>
#include <vector>
#include <MrBoboSockets/MrBoboSockets.h>
#include <MBUtility/TextReader.h>
#include <MBUtility/MBStrings.h>
#include <filesystem>
#include <thread>
//tanken med search enginen är att vi ska returnera ett antal hyperlinks som innehåler data vi är intresserad av
struct URLSearchResult
{
	std::string WebsiteUrl;
	float Relevancy;
};
void SearchURLContent(std::string HTMLContentPath,std::vector<std::string> SearchWords, std::vector<URLSearchResult>* ResultVector,std::mutex* MutexToUse)
{
	URLSearchResult ReturnValue{ HTMLContentPath,0 };
	TextReader HTMLContent(HTMLContentPath);
	std::vector<int> WordOccurance = std::vector<int>(SearchWords.size(), 0);
	for (int i = 0; i < HTMLContent.Size(); i++)
	{
		for (int j = 0; j < SearchWords.size(); j++)
		{
			int Offset = 0;
			while ((Offset = HTMLContent[i].find(SearchWords[j],Offset+SearchWords[j].size())) != HTMLContent[i].npos)
			{
				WordOccurance[j] += 1;
			}
		}
	}
	for (size_t i = 0; i < WordOccurance.size(); i++)
	{
		ReturnValue.Relevancy += WordOccurance[i];
	}
	ReturnValue.Relevancy /= WordOccurance.size();
	std::lock_guard<std::mutex> Locken(*MutexToUse);
	ResultVector->push_back(ReturnValue);
}
std::vector<URLSearchResult> SearchInWebsite(std::string WebsiteToSearch, std::vector<std::string> SearchWords)
{
	std::filesystem::path WebsiteDirectoryPath(std::filesystem::current_path().concat("/"+WebsiteToSearch));
	std::filesystem::directory_iterator DirectoryToIterate(WebsiteDirectoryPath);
	MBSockets::ThreadPool ThreadPoolen(std::thread::hardware_concurrency() + 1);

	std::mutex FunctionResourcesMutex;
	std::vector<URLSearchResult> ReturnValue = std::vector<URLSearchResult>(0);
	while(DirectoryToIterate != std::filesystem::end(DirectoryToIterate))
	{
		if (DirectoryToIterate->exists())
		{
			ThreadPoolen.AddTask(SearchURLContent, DirectoryToIterate->path().string(),SearchWords,&ReturnValue,&FunctionResourcesMutex);
		}
		DirectoryToIterate++;
	}
	ThreadPoolen.Start();
	ThreadPoolen.WaitForThreadsToFinish();
	return(ReturnValue);
}
class URlSearchComparison
{
public:
	int operator()(const URLSearchResult& a, const URLSearchResult& b)
	{
		return(a.Relevancy < b.Relevancy);
	}
};
std::vector<std::string> SearchWebsites(std::string SearchString)
{
	std::vector<std::string> SearchTerms = MBUtility::Split(SearchString," ");
	//här borde vi ha några steg där vi avgör vilka hemsidor vi ens är intresserad av att söka på i första hand, men i vårt fall har vi ju indexerat så få att vi inte egentligen bryr oss
	//TODO Ha website culling

	//nu när vi har en lista av hemsidor så söker vi igenom dem
	std::vector<URLSearchResult> SearchResult = SearchInWebsite("www.remar.se", SearchTerms);
	URlSearchComparison Pr;
	std::sort(SearchResult.begin(), SearchResult.end(), Pr);
	//nu ger vi tillbaka en mängd som är på slutet
	int NumberOfResultsToReturn = 10;
	std::vector<std::string> ReturnValue = std::vector<std::string>(0);
	for (int i = 1; i <= NumberOfResultsToReturn; i++)
	{
		int ActualAdressPosition = SearchResult[SearchResult.size() - i].WebsiteUrl.find("www.remar.se\\");
		std::string HyperlinkAdress = "http://www.remar.se/" + MBUtility::ReplaceAll(MBUtility::ReplaceAll(SearchResult[SearchResult.size() - i].WebsiteUrl.substr(ActualAdressPosition + 13),"_","/"),".txt","");
		ReturnValue.push_back(HyperlinkAdress);
	}
	return(ReturnValue);
}