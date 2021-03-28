#include <MBSearchEngine/MBSearchEngine.h>
#include <MBAlgorithms.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <array>
namespace MBSearchEngine
{
	//searchToken
	bool SearchToken::operator==(const SearchToken& other) const
	{
		return(TokenName == other.TokenName);
	}
	SearchToken::SearchToken(std::string const& StringToNormalize)
	{
		//här vill vi egentligen converat allt till säg lowercase
		TokenName = StringToNormalize;
	}
	//END Searchtoken

	//posting

	//END Posting

	//PostingsList
	bool CompareDocumentReferences(std::string const& LeftDocumentReference, std::string const& RightDocumentReference)
	{
		return(LeftDocumentReference < RightDocumentReference);
	}
	std::string GetPostingReference(std::vector<Posting> const& Postings,int IndexToGet)
	{
		return(Postings[IndexToGet].DocumentReference);
	}
	bool ComparePostings(Posting const& LeftPosting, Posting const& RightPosting)
	{
		return(LeftPosting.DocumentReference < RightPosting.DocumentReference);
	}
	void PostingsList::AddPosting(SearchToken const& TokenToAdd, std::string DocumentID, int TokenPosition)
	{
		//förutsätter att listan är sorterad
		int DocumentPostingPosition = MBAlgorithms::BinarySearch(Postings, DocumentID, GetPostingReference, CompareDocumentReferences);
		if (DocumentPostingPosition == -1)
		{
			//helt pantad algorithm egentligen, vi lägger till en på slutet och sedan sorterar vi, men kräver nog egentligen att man har lite mer eftertanke med hur
			//man vill att datastrukturen ska se ut
			Postings.push_back(Posting(DocumentID));
			std::sort(Postings.begin(), Postings.end(), ComparePostings);
		}
		else
		{
			Postings[DocumentPostingPosition].NumberOfOccurances += 1;
		}
	}
	//END PostingsList

	std::vector<SearchToken> TokenizeString(std::string const& StringToTokenize)
	{
		std::vector<SearchToken> ReturnValue = {};
		size_t CurrentOffset = 0;
		while (CurrentOffset != StringToTokenize.npos)
		{
			constexpr int NumberOfDelimiters = 3;
			std::array<std::string, NumberOfDelimiters> Delimiter = { " ","\n","\r" };
			std::array<size_t, NumberOfDelimiters> DelimiterPositions = {0,0,0};
			DelimiterPositions[0] = StringToTokenize.find(Delimiter[0], CurrentOffset);
			DelimiterPositions[1] = StringToTokenize.find(Delimiter[1], CurrentOffset);
			DelimiterPositions[2] = StringToTokenize.find(Delimiter[2], CurrentOffset);
			size_t NewOffset = std::min_element(DelimiterPositions.begin(),DelimiterPositions.end())[0];
			//size_t NewOffset = StringToTokenize.find(" ", CurrentOffset);
			if (NewOffset - CurrentOffset > 1)
			{
				//undviker \r\n
				ReturnValue.push_back(SearchToken(StringToTokenize.substr(CurrentOffset, NewOffset - CurrentOffset)));
				if (NewOffset == StringToTokenize.npos)
				{
					break;
				}
			}
			CurrentOffset = NewOffset + 1;
		}
		return(ReturnValue);
	}
	//MBIndex
	MBError MBIndex::IndexTextDocument(std::string const& DocumentName)
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
		std::vector<SearchToken> DocumentTokens;
		{
			std::string DocumentData(NumberOfBytes, 0);
			FileToRead.read(&DocumentData[0], NumberOfBytes);
			DocumentTokens = TokenizeString(DocumentData);
		}
		ReturnValue = UpdatePostings(DocumentTokens, DocumentName);
		return(ReturnValue);
	}
	MBError MBIndex::UpdatePostings(std::vector<SearchToken> const& DocumentTokens, std::string const& DocumentName)
	{
		MBError ReturnValue(true);
		for (size_t i = 0; i < DocumentTokens.size(); i++)
		{
			if (m_TokenMap.find(DocumentTokens[i]) == m_TokenMap.end())
			{
				m_TokenMap[DocumentTokens[i]] = PostingsList();
			}
			m_TokenMap[DocumentTokens[i]].AddPosting(DocumentTokens[i], DocumentName, i);
		}
		//sorterar den på slutet så kanske aningen offektivt för storra arrays
		return(ReturnValue);
	}
	void MBIndex::PrintIndex()
	{
		auto TokenMapIterator = m_TokenMap.begin();
		while (TokenMapIterator != m_TokenMap.end())
		{
			SearchToken const& CurrentWord = TokenMapIterator->first;
			PostingsList const& CurrentPostingList = TokenMapIterator->second;
			std::cout <<+"\""+ CurrentWord.TokenName +"\": "<<std::endl;
			for (size_t i = 0; i < CurrentPostingList.size(); i++)
			{
				std::cout << CurrentPostingList[i].DocumentReference + " " << CurrentPostingList[i].NumberOfOccurances << std::endl;
			}
			TokenMapIterator++;
		}
	}
	//END MBIndex
}