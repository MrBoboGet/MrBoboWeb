#pragma once
#include <vector>
#include <string>
#include <MBErrorHandling.h>
#include <unordered_map>
#include <MBAlgorithms.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <array>
#include <map>

#include <MBSearchEngine/BooleanParse.h>
#include <MrPostOGet/MBHTMLParser.h>
#include <MBSearchEngine/MBISave.h>
typedef unsigned int DocID;
typedef unsigned int PostingListID;
//MBI Save grejer

namespace MBSearchEngine
{
	class SearchToken
	{
	public:
		std::string TokenName = "";
		bool operator==(const SearchToken& other) const;
		SearchToken(std::string const& WordToNormalize);
		std::string GetTokenString() const
		{
			return(TokenName);
		}
		static std::vector<SearchToken> TokenizeString(std::string const& TextdataToTokenize);
	};
	class Posting
	{
		friend void MBI_SaveObjectToFile<Posting>(std::fstream&, Posting&);
		friend Posting MBI_ReadObjectFromFile<Posting>(std::fstream&);
	public:
		Posting();
		DocID DocumentReference = -1;
		int NumberOfOccurances = 0;
		std::vector<int> m_DocumentPositions = {};
		Posting(DocID NewDocumentReference);
		void Update(int TokenPosition);
		void Print() const;
		bool operator<(Posting const& RightPosting) const;
		bool operator<(DocID IDToCompare) const;
		bool operator==(DocID IDToCompare) const;
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
	class TokenDictionaryEntry
	{
		friend void MBI_SaveObjectToFile<TokenDictionaryEntry>(std::fstream&, TokenDictionaryEntry&);
		friend TokenDictionaryEntry MBI_ReadObjectFromFile<TokenDictionaryEntry>(std::fstream&);
	public:
		std::string TokenData = "";
		PostingListID PostingIndex = 0;
		TokenDictionaryEntry() {};
		TokenDictionaryEntry(std::string const& NewTokenData, PostingListID NewPostingIndex);
		bool operator<(std::string const& StringToCompare) const;
		bool operator==(std::string const& StringToCompare) const;
		bool operator<(TokenDictionaryEntry const& DictionaryEntryToCompare) const;
		bool operator==(TokenDictionaryEntry const& DictionaryEntryToCompare) const;
	};
	class TokenDictionaryIterator
	{
		friend class TokenDictionary;
	private:
		//int m_Offset = 0;
		//int m_MaxElements = 0;
		//std::vector<TokenDictionaryEntry>const* m_DictionaryData;
		std::map<std::string, TokenDictionaryEntry>::iterator _InternalIterator;
		TokenDictionaryIterator(std::map<std::string, TokenDictionaryEntry>::iterator const& InitialIterator);
	public:
		//TokenDictionaryIterator(std::vector<TokenDictionaryEntry> const& DictionaryData);
		//TokenDictionaryIterator(TokenDictionaryIterator const& IteratorToCopy);
		bool operator==(TokenDictionaryIterator const& RightIterator) const;
		bool operator!=(TokenDictionaryIterator const& RightIterator);
		TokenDictionaryIterator& operator++();
		TokenDictionaryIterator& operator++(int);
		TokenDictionaryEntry& operator*() const;
		TokenDictionaryEntry& operator->() const;
	};
	class TokenDictionary
	{
		friend void MBI_SaveObjectToFile<TokenDictionary>(std::fstream&,TokenDictionary&);
		friend TokenDictionary MBI_ReadObjectFromFile<TokenDictionary>(std::fstream&);
	private:
		std::map<std::string,TokenDictionaryEntry> m_TokenMapInMemory = {};
	public:
		void Save(std::fstream& OutFile);
		void Load(std::fstream& FileToReadFrom);
		PostingListID GetTokenPosting(std::string const& TokenData);
		int GetNumberOfTokens();
		PostingListID AddNewToken(std::string const& TokenData);
		TokenDictionaryIterator begin();
		TokenDictionaryIterator end();
		bool TokenInDictionary(std::string const& TokenData);
	};
	typedef Posting PostingClass;
	typedef SearchToken TokenClass;
	//template<typename PostingClass> 
	class PostingsListIterator
	{
		friend class PostingsList;
	private:
		//den triviala dumma enkla implementationen
		size_t m_PostingsListOffset = 0;
		PostingsList* m_ListReference = nullptr;
		PostingsListIterator(PostingsList*);
	public:
		PostingsListIterator(PostingsListIterator const& IteratorToCopy);
		bool operator==(PostingsListIterator const& RightIterator) const;
		bool operator!=(PostingsListIterator const& RightIterator) const;
		PostingsListIterator& operator++();
		PostingsListIterator& operator++(int);
		const Posting& operator*() const;
		const Posting& operator->() const;
	};
	class PostingsList
	{
		friend void MBI_SaveObjectToFile<PostingsList>(std::fstream&, PostingsList&);
		friend PostingsList MBI_ReadObjectFromFile<PostingsList>(std::fstream&);
	private:
		//std::vector<PostingClass> m_PostingsInMemory = {};
		std::map<size_t, PostingClass> m_PostingsInMemory = {};
		size_t m_DocumentFrequency = 0;
	public:
		size_t GetDocumentFrequency() const;
		void AddPosting(DocID DocumentID, int TokenPosition);
		int size() const;
		PostingsListIterator begin();
		PostingsListIterator end();
		PostingClass const& operator[](PostingListID Index) const;
	};
	//std::vector<SearchToken> TokenizeString(std::string const&);

	//template<typename TokenClass, typename PostingClass>
	enum class BoleanBinaryOperator
	{
		AND,
		OR,
		Null
	};
	struct TopQuerry
	{
		bool IsAtomic = false;
		std::string FirstOperand = "";
		BoleanBinaryOperator Operator = BoleanBinaryOperator::Null;
		std::string SecondOperand = "";
	};
	class MBIndex;
	class BooleanQuerry
	{
	private:
		MBError m_ParseStatus = MBError(true);
		bool m_IsAtomic = false;
		std::string m_QuerryString = "";
		BoleanBinaryOperator m_BinaryOperator = BoleanBinaryOperator::Null;
		std::vector<BooleanQuerry> m_SubQuerries = {};
		TopQuerry GetSubquerries(std::string const& QuerryToEvaluate);
		std::vector<DocID> EvaluateWordQuerry(MBIndex& Index,std::string const& Querry);
		std::vector<DocID> EvaluatePhraseQuerry(MBIndex& Index,std::string const& QuerryToEvaluate);
	public:
		bool IsValid()
		{
			return(m_ParseStatus);
		}
		std::string GetErrorMessage()
		{
			return(m_ParseStatus.ErrorMessage);
		}
		BooleanQuerry(std::string const& QuerryToParse);
		std::vector<DocID> Evaluate(MBIndex&);
	};
	struct DocumentIndexData
	{
		friend void MBI_SaveObjectToFile<DocumentIndexData>(std::fstream&, DocumentIndexData&);
		friend DocumentIndexData MBI_ReadObjectFromFile<DocumentIndexData>(std::fstream&);
		float DocumentLength = 0;
	};
	//class DocumentIndexDataList
	//{
	//private:
	//	std::vector<DocumentIndexData> m_DataList = {};
	//public:
	//	DocumentIndexData const& operator[](DocID);
	//};
	class MBIndex
	{
		friend class BooleanQuerry;
	private:
		std::vector<std::string> m_DocumentIDs = {};
		std::vector<DocumentIndexData> _DocumentIndexDataList = {};
		TokenDictionary m_TokenDictionary;
		std::vector<PostingsList> m_PostingsLists;
		//std::unordered_map<SearchToken, PostingsList<TokenClass,PostingClass>> m_TokenMap = {};
		
		
		//MBError UpdatePostings(std::vector<SearchToken> const& TokensToUpdate, std::string const& DocumentName);
		MBError UpdatePostings(std::vector<TokenClass> const& DocumentTokens, DocID DocumentID);
		PostingsList& GetPostinglist(PostingListID ID);
		std::string GetDocumentIdentifier(DocID DocumentID);

		DocumentIndexData m_GetDocumentIndexData(DocID ID);
		void m_CalculateDocumentLengths();
	public:
		//MBError AddTextData(std::string const& TextData);
		float GetInverseDocumentFrequency(PostingListID ID);
		MBIndex(std::string const& IndexDataPath);
		MBIndex();
		void Finalize();
		MBError Save(std::string const& OutFilename);
		MBError Load(std::string const& SavedIndexFile);
		std::vector<std::string> EvaluteBooleanQuerry(std::string const& BooleanQuerryToEvaluate);
		std::vector<std::string> EvaluteVectorModelQuerry(std::string const& VectorModelQuerry);
		MBError IndextTextData(std::string const& TextData, std::string const& DocumentIdentifier);
		MBError IndexTextDocument(std::string const& DocumentName);
		MBError IndexHTMLData(std::string const& DocumentData, std::string const& DocumentIdentifier);
		void PrintIndex();
	};
}