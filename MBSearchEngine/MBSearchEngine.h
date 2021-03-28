#pragma once
#include <vector>
#include <string>
#include <MBErrorHandling.h>
#include <unordered_map>
namespace MBSearchEngine
{
	class SearchToken
	{
	public:
		std::string TokenName = "";
		bool operator==(const SearchToken& other) const;
		SearchToken(std::string const& WordToNormalize);
	};
	class Posting
	{
	public:
		std::string DocumentReference = "";
		int NumberOfOccurances = 0;
		Posting(std::string const& NewDocumentReference) 
		{
			DocumentReference = NewDocumentReference;
			NumberOfOccurances = 1;
		};
	};
}
namespace std
{
	template<> struct hash<MBSearchEngine::SearchToken>
	{
		std::size_t operator()(MBSearchEngine::SearchToken const& s) const noexcept
		{
			std::hash<std::string> StringHasher;
			return(StringHasher(s.TokenName));
		}
	};
}
namespace MBSearchEngine
{
	class PostingsList
	{
	private:
		std::vector<Posting> Postings = {};
	public:
		void AddPosting(SearchToken const& TokenToAdd,std::string DocumentID,int TokenPosition);
		int size() const { return(Postings.size()); };
		Posting const& operator[](int Index) const
		{
			return(Postings[Index]);
		}
	};
	std::vector<SearchToken> TokenizeString(std::string const&);
	class MBIndex
	{
	private:
		std::unordered_map<SearchToken, PostingsList> m_TokenMap = {};
		MBError UpdatePostings(std::vector<SearchToken> const&, std::string const& DocumentName);
	public:
		//MBError AddTextData(std::string const& TextData);
		MBError IndexTextDocument(std::string const& DocumentName);
		void PrintIndex();
	};
}