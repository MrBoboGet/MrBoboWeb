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

#include <MBSearchEngine/BooleanParse.h>
typedef unsigned int DocID;
typedef unsigned int PostingListID;
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
	public:
		DocID DocumentReference = -1;
		int NumberOfOccurances = 0;
		std::vector<int> m_DocumentPositions = {};
		Posting(DocID NewDocumentReference)
		{
			DocumentReference = NewDocumentReference;
			NumberOfOccurances = 0;
		};
		void Update(int TokenPosition)
		{
			NumberOfOccurances += 1;
			m_DocumentPositions.push_back(TokenPosition);
		}
		void Print() const
		{
			std::cout << "DocumentReference: " << DocumentReference << " NumberOfOccurances: " << NumberOfOccurances << std::endl;
			std::cout << "DocumentPositions: ";
			for (size_t i = 0; i < m_DocumentPositions.size(); i++)
			{
				std::cout << m_DocumentPositions[i];
				if (i + 1 < m_DocumentPositions.size())
				{
					std::cout << ",";
				}
			}
			std::cout << std::endl;
		}
		bool operator<(Posting const& RightPosting) const
		{
			return(this->DocumentReference < RightPosting.DocumentReference);
		}
		bool operator<(DocID IDToCompare) const
		{
			return(DocumentReference < IDToCompare);
		}
		bool operator==(DocID IDToCompare) const
		{
			return(DocumentReference == IDToCompare);
		}
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
	public:
		std::string TokenData = "";
		unsigned int DocumentFrequency = 0;
		PostingListID PostingIndex = 0;
		TokenDictionaryEntry(std::string const& NewTokenData,PostingListID NewPostingIndex)
		{
			TokenData = NewTokenData;
			PostingIndex = NewPostingIndex;
			DocumentFrequency = 0;
		}
		bool operator<(std::string const& StringToCompare) const
		{
			return(TokenData < StringToCompare);
		}
		bool operator==(std::string const& StringToCompare) const
		{
			return(TokenData == StringToCompare);
		}
		bool operator<(TokenDictionaryEntry const& DictionaryEntryToCompare) const
		{
			return(TokenData < DictionaryEntryToCompare.TokenData);
		}
		bool operator==(TokenDictionaryEntry const& DictionaryEntryToCompare) const
		{
			return(TokenData == DictionaryEntryToCompare.TokenData);
		}
	};
	class TokenDictionaryIterator
	{
		friend class TokenDictionary;
	private:
		int m_Offset = 0;
		int m_MaxElements = 0;
		std::vector<TokenDictionaryEntry>const* m_DictionaryData;
	public:
		TokenDictionaryIterator(std::vector<TokenDictionaryEntry> const& DictionaryData)
			:m_DictionaryData(&DictionaryData)
		{
			m_MaxElements = DictionaryData.size();
		}
		TokenDictionaryIterator(TokenDictionaryIterator const& IteratorToCopy)
			: m_DictionaryData(IteratorToCopy.m_DictionaryData)
		{
			m_Offset = IteratorToCopy.m_Offset;
			m_MaxElements = IteratorToCopy.m_MaxElements;
		}
		bool operator==(TokenDictionaryIterator const& RightIterator) const
		{
			//kanske borde jämföra huruvida dem är har samma container också, men skitsamma så länge
			return(m_Offset == RightIterator.m_Offset);
		}
		bool operator!=(TokenDictionaryIterator const& RightIterator)
		{
			return(!(*this == RightIterator));
		}
		TokenDictionaryIterator& operator++()
		{
			m_Offset += 1;
			return(*this);
		}
		TokenDictionaryIterator& operator++(int)
		{
			m_Offset += 1;
			return(*this);
		}
		const TokenDictionaryEntry& operator*() const
		{
			return((*m_DictionaryData)[m_Offset]);
		}
		const TokenDictionaryEntry& operator->() const
		{
			return((*m_DictionaryData)[m_Offset]);
		}
	};
	class TokenDictionary
	{
	private:
		std::vector<TokenDictionaryEntry> m_TokenMapInMemory = {};
	public:
		PostingListID GetTokenPosting(std::string const& TokenData)
		{
			int MapIndex = MBAlgorithms::BinarySearch(m_TokenMapInMemory, TokenData);
			if (MapIndex != -1)
			{
				return(m_TokenMapInMemory[MapIndex].PostingIndex);
			}
		}
		int GetNumberOfTokens()
		{
			return(m_TokenMapInMemory.size());
		}
		PostingListID AddNewToken(std::string const& TokenData)
		{
			PostingListID NewListID = GetNumberOfTokens();
			m_TokenMapInMemory.push_back(TokenDictionaryEntry(TokenData, NewListID));
			//extremt långsamt och ass egentligen, men får tänka ut datastrukturen lite bättre sen
			std::sort(m_TokenMapInMemory.begin(), m_TokenMapInMemory.end());
			return(NewListID);
		}
		TokenDictionaryIterator begin()
		{
			TokenDictionaryIterator ReturnValue(m_TokenMapInMemory);
			ReturnValue.m_Offset = 0;
			return(ReturnValue);
		}
		TokenDictionaryIterator end()
		{
			TokenDictionaryIterator ReturnValue(m_TokenMapInMemory);
			ReturnValue.m_Offset = m_TokenMapInMemory.size();
			return(ReturnValue);
		}
		bool TokenInDictionary(std::string const& TokenData)
		{
			return(MBAlgorithms::BinarySearch(m_TokenMapInMemory, TokenData) != -1);
		}

	};

	typedef Posting PostingClass;
	typedef SearchToken TokenClass;
	//template<typename PostingClass> 
	class PostingsList
	{
	private:
		std::vector<PostingClass> m_PostingsInMemory = {};
	public:
		void AddPosting(DocID DocumentID, int TokenPosition)
		{
			//förutsätter att listan är sorterad
			int DocumentPostingPosition = MBAlgorithms::BinarySearch(m_PostingsInMemory, DocumentID);
			if (DocumentPostingPosition == -1)
			{
				//helt pantad algorithm egentligen, vi lägger till en på slutet och sedan sorterar vi, men kräver nog egentligen att man har lite mer eftertanke med hur
				//man vill att datastrukturen ska se ut
				m_PostingsInMemory.push_back(Posting(DocumentID));
				m_PostingsInMemory.back().Update(TokenPosition);
				std::sort(m_PostingsInMemory.begin(), m_PostingsInMemory.end());
			}
			else
			{
				m_PostingsInMemory[DocumentPostingPosition].Update(TokenPosition);
			}
		}
		int size() const { return(m_PostingsInMemory.size()); };
		PostingClass const& operator[](PostingListID Index) const
		{
			//kommer förändras iomed 
			return(m_PostingsInMemory[Index]);
		}
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
	class MBIndex
	{
		friend class BooleanQuerry;
	private:
		std::vector<std::string> m_DocumentIDs = {};
		TokenDictionary m_TokenDictionary;
		std::vector<PostingsList> m_PostingsLists;
		//std::unordered_map<SearchToken, PostingsList<TokenClass,PostingClass>> m_TokenMap = {};
		
		
		//MBError UpdatePostings(std::vector<SearchToken> const& TokensToUpdate, std::string const& DocumentName);
		MBError UpdatePostings(std::vector<TokenClass> const& DocumentTokens, DocID DocumentID)
		{
			MBError ReturnValue(true);
			for (size_t i = 0; i < DocumentTokens.size(); i++)
			{
				PostingListID PostinglistToUpdate = -1;
				std::string TokenData = DocumentTokens[i].GetTokenString();
				if (m_TokenDictionary.TokenInDictionary(TokenData))
				{
					PostinglistToUpdate = m_TokenDictionary.GetTokenPosting(TokenData);
				}
				else
				{
					m_TokenDictionary.AddNewToken(TokenData);
					m_PostingsLists.push_back(PostingsList());
					PostinglistToUpdate = m_PostingsLists.size() - 1;
				}
				m_PostingsLists[PostinglistToUpdate].AddPosting(DocumentID, i);
			}
			//sorterar den på slutet så kanske aningen offektivt för storra arrays
			return(ReturnValue);
		}
		PostingsList& GetPostinglist(PostingListID ID)
		{
			return(m_PostingsLists[ID]);
		}
	public:
		//MBError AddTextData(std::string const& TextData);
		std::vector<std::string> EvaluteBooleanQuerry(std::string const& BooleanQuerryToEvaluate)
		{
			std::vector<std::string> ReturnValue = {};
			BooleanQuerry ProcessedQuerry(BooleanQuerryToEvaluate);
			std::vector<DocID> Result = ProcessedQuerry.Evaluate(*this);
			return(ReturnValue);
		}
		MBError IndexTextDocument(std::string const& DocumentName)
		{
			MBError ReturnValue(true);
			std::ifstream FileToRead(DocumentName, std::ios::binary | std::ios::in);
			if (!FileToRead.is_open())
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "Failed to open file";
				return(ReturnValue);
			}
			size_t NumberOfBytes = std::filesystem::file_size(DocumentName);
			std::vector<TokenClass> DocumentTokens;
			{
				std::string DocumentData(NumberOfBytes, 0);
				FileToRead.read(&DocumentData[0], NumberOfBytes);
				DocumentTokens = TokenClass::TokenizeString(DocumentData);
			}
			m_DocumentIDs.push_back(DocumentName);
			ReturnValue = UpdatePostings(DocumentTokens, m_DocumentIDs.size()-1);
			return(ReturnValue);
		}
		void PrintIndex()
		{
			std::cout << "Indexed Documents: ";
			for (size_t i = 0; i < m_DocumentIDs.size(); i++)
			{
				std::cout << "DocID: " << i << " DocumentPath " << m_DocumentIDs[i] << std::endl;
			}
			for (auto DictionaryEntries : m_TokenDictionary)
			{
				std::cout << DictionaryEntries.TokenData << ":"<<std::endl;
				PostingsList const& TokenPostings = m_PostingsLists[DictionaryEntries.PostingIndex];
				for (size_t i = 0; i < TokenPostings.size(); i++)
				{
					TokenPostings[i].Print();
				}
			}
		}
	};
}