#include <MBSearchEngine/MBSearchEngine.h>
#include <MinaStringOperations.h>
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
	std::vector<SearchToken> SearchToken::TokenizeString(std::string const& DataToTokenize)
	{
		std::vector<SearchToken> ReturnValue = {};
		size_t CurrentOffset = 0;
		while (CurrentOffset != DataToTokenize.npos)
		{
			constexpr int NumberOfDelimiters = 3;
			std::array<std::string, NumberOfDelimiters> Delimiter = { " ","\n","\r" };
			std::array<size_t, NumberOfDelimiters> DelimiterPositions = { 0,0,0 };
			DelimiterPositions[0] = DataToTokenize.find(Delimiter[0], CurrentOffset);
			DelimiterPositions[1] = DataToTokenize.find(Delimiter[1], CurrentOffset);
			DelimiterPositions[2] = DataToTokenize.find(Delimiter[2], CurrentOffset);
			size_t NewOffset = std::min_element(DelimiterPositions.begin(), DelimiterPositions.end())[0];
			//size_t NewOffset = StringToTokenize.find(" ", CurrentOffset);
			if (NewOffset - CurrentOffset > 1)
			{
				//undviker \r\n
				ReturnValue.push_back(SearchToken(DataToTokenize.substr(CurrentOffset, NewOffset - CurrentOffset)));
				if (NewOffset == DataToTokenize.npos)
				{
					break;
				}
			}
			CurrentOffset = NewOffset + 1;
		}
		return(ReturnValue);
	}
	//END Searchtoken

	//posting

	//END Posting

	//PostingsList
	//bool CompareDocumentReferences(std::string const& LeftDocumentReference, std::string const& RightDocumentReference)
	//{
	//	return(LeftDocumentReference < RightDocumentReference);
	//}
	//std::string GetPostingReference(std::vector<Posting> const& Postings,int IndexToGet)
	//{
	//	return(Postings[IndexToGet].DocumentReference);
	//}
	//bool ComparePostings(Posting const& LeftPosting, Posting const& RightPosting)
	//{
	//	return(LeftPosting.DocumentReference < RightPosting.DocumentReference);
	//}
	//void PostingsList::AddPosting(SearchToken const& TokenToAdd, std::string DocumentID, int TokenPosition)
	//{
	//	//förutsätter att listan är sorterad
	//	int DocumentPostingPosition = MBAlgorithms::BinarySearch(Postings, DocumentID, GetPostingReference, CompareDocumentReferences);
	//	if (DocumentPostingPosition == -1)
	//	{
	//		//helt pantad algorithm egentligen, vi lägger till en på slutet och sedan sorterar vi, men kräver nog egentligen att man har lite mer eftertanke med hur
	//		//man vill att datastrukturen ska se ut
	//		Postings.push_back(Posting(DocumentID));
	//		std::sort(Postings.begin(), Postings.end(), ComparePostings);
	//	}
	//	else
	//	{
	//		Postings[DocumentPostingPosition].NumberOfOccurances += 1;
	//	}
	//}
	//END PostingsList

	//std::vector<SearchToken> TokenizeString(std::string const& StringToTokenize)
	//{
	//	std::vector<SearchToken> ReturnValue = {};
	//	size_t CurrentOffset = 0;
	//	while (CurrentOffset != StringToTokenize.npos)
	//	{
	//		constexpr int NumberOfDelimiters = 3;
	//		std::array<std::string, NumberOfDelimiters> Delimiter = { " ","\n","\r" };
	//		std::array<size_t, NumberOfDelimiters> DelimiterPositions = {0,0,0};
	//		DelimiterPositions[0] = StringToTokenize.find(Delimiter[0], CurrentOffset);
	//		DelimiterPositions[1] = StringToTokenize.find(Delimiter[1], CurrentOffset);
	//		DelimiterPositions[2] = StringToTokenize.find(Delimiter[2], CurrentOffset);
	//		size_t NewOffset = std::min_element(DelimiterPositions.begin(),DelimiterPositions.end())[0];
	//		//size_t NewOffset = StringToTokenize.find(" ", CurrentOffset);
	//		if (NewOffset - CurrentOffset > 1)
	//		{
	//			//undviker \r\n
	//			ReturnValue.push_back(SearchToken(StringToTokenize.substr(CurrentOffset, NewOffset - CurrentOffset)));
	//			if (NewOffset == StringToTokenize.npos)
	//			{
	//				break;
	//			}
	//		}
	//		CurrentOffset = NewOffset + 1;
	//	}
	//	return(ReturnValue);
	//}
	//MBIndex
	//MBError MBIndex::IndexTextDocument(std::string const& DocumentName)
	//{
	//	MBError ReturnValue(true);
	//	std::ifstream FileToRead(DocumentName, std::ios::binary | std::ios::in);
	//	if (!FileToRead.is_open())
	//	{
	//		ReturnValue = false;
	//		ReturnValue.ErrorMessage = "Failed to open file";
	//		return(ReturnValue);
	//	}
	//	size_t NumberOfBytes = std::filesystem::file_size(DocumentName);
	//	std::vector<SearchToken> DocumentTokens;
	//	{
	//		std::string DocumentData(NumberOfBytes, 0);
	//		FileToRead.read(&DocumentData[0], NumberOfBytes);
	//		DocumentTokens = TokenizeString(DocumentData);
	//	}
	//	ReturnValue = UpdatePostings(DocumentTokens, DocumentName);
	//	return(ReturnValue);
	//}
	//MBError MBIndex::UpdatePostings(std::vector<SearchToken> const& DocumentTokens, std::string const& DocumentName)
	//{
	//	MBError ReturnValue(true);
	//	for (size_t i = 0; i < DocumentTokens.size(); i++)
	//	{
	//		if (m_TokenMap.find(DocumentTokens[i]) == m_TokenMap.end())
	//		{
	//			m_TokenMap[DocumentTokens[i]] = PostingsList();
	//		}
	//		m_TokenMap[DocumentTokens[i]].AddPosting(DocumentTokens[i], DocumentName, i);
	//	}
	//	//sorterar den på slutet så kanske aningen offektivt för storra arrays
	//	return(ReturnValue);
	//}
	//void MBIndex::PrintIndex()
	//{
	//	auto TokenMapIterator = m_TokenMap.begin();
	//	while (TokenMapIterator != m_TokenMap.end())
	//	{
	//		SearchToken const& CurrentWord = TokenMapIterator->first;
	//		PostingsList const& CurrentPostingList = TokenMapIterator->second;
	//		std::cout <<+"\""+ CurrentWord.TokenName +"\": "<<std::endl;
	//		for (size_t i = 0; i < CurrentPostingList.size(); i++)
	//		{
	//			std::cout << CurrentPostingList[i].DocumentReference + " " << CurrentPostingList[i].NumberOfOccurances << std::endl;
	//		}
	//		TokenMapIterator++;
	//	}
	//}
	//END MBIndex

	//BooleanQuerry
	TopQuerry  BooleanQuerry::GetSubquerries(std::string const& QuerryToEvaluate)
	{
		TopQuerry ReturnValue;
		std::string QuerryToParse = RemoveRedundatParenthesis(QuerryToEvaluate);
		std::vector<size_t> OperatorPositions = GetLogicalOperatorPositions(QuerryToParse);
		if (OperatorPositions.size() == 0)
		{
			ReturnValue.IsAtomic = true;
			return(ReturnValue);
		}
		std::vector<int> OperatorDepth = std::vector<int>(OperatorPositions.size());
		for (size_t i = 0; i < OperatorPositions.size(); i++)
		{
			OperatorDepth[i] = GetCharacterParenthesisDepth(QuerryToParse, OperatorPositions[i]);
			if (OperatorDepth[i] < 0)
			{
				m_ParseStatus = false;
				m_ParseStatus.ErrorMessage = "Invalid parenthesis";
				return(ReturnValue);
			}
		}
		//checkar validityn av paranteser, AND och OR kan bara vara på samma djup som sig själva
		for (size_t i = 0; i < OperatorPositions.size() - 1; i++)
		{
			if (OperatorDepth[i] == OperatorDepth[i + 1])
			{
				if (GetLogicOperator(QuerryToParse, OperatorDepth[i]) != GetLogicOperator(QuerryToParse, OperatorDepth[i + 1]))
				{
					m_ParseStatus = false;
					m_ParseStatus.ErrorMessage = "Missing parenthesis between operators";
					return(ReturnValue);
				}
			}
		}
		//nu är parsingen i ett valid state och vi kommer då att bryta upp den i sub querrys utifrån operatorn med lägst depth
		int LowestDetph = 1000000000;
		size_t LowestDepthOperatorPosition = -1;
		for (size_t i = 0; i < OperatorPositions.size(); i++)
		{
			if (OperatorDepth[i] < LowestDetph)
			{
				LowestDetph = OperatorDepth[i];
				LowestDepthOperatorPosition = OperatorPositions[i];
			}
		}
		int OperatorLength = 0;
		if (GetLogicOperator(QuerryToParse, LowestDepthOperatorPosition) == "AND")
		{
			ReturnValue.Operator = BoleanBinaryOperator::AND;
			OperatorLength = 5;
		}
		else if (GetLogicOperator(QuerryToParse, LowestDepthOperatorPosition) == "OR")
		{
			ReturnValue.Operator = BoleanBinaryOperator::OR;
			OperatorLength = 4;
		}
		ReturnValue.IsAtomic = false;
		ReturnValue.FirstOperand = RemoveRedundatParenthesis(QuerryToParse.substr(0, LowestDepthOperatorPosition));
		ReturnValue.SecondOperand = RemoveRedundatParenthesis(QuerryToParse.substr(LowestDepthOperatorPosition + OperatorLength));
		return(ReturnValue);
	}
	BooleanQuerry::BooleanQuerry(std::string const& QuerryToParse)
	{
		TopQuerry Querry = GetSubquerries(QuerryToParse);
		if (!m_ParseStatus)
		{
			//error hände, slut med resten
			return;
		}
		if (Querry.IsAtomic)
		{
			m_IsAtomic = true;
			m_QuerryString = QuerryToParse;
		}
		else
		{
			m_BinaryOperator = Querry.Operator;
			m_SubQuerries.push_back(BooleanQuerry(Querry.FirstOperand));
			m_SubQuerries.push_back(BooleanQuerry(Querry.SecondOperand));
			m_ParseStatus.ErrorMessage = "";
			if (!m_SubQuerries[0].IsValid())
			{
				m_ParseStatus = false;
				m_ParseStatus.ErrorMessage += "Error parcing subquerry 1: " + m_SubQuerries[0].GetErrorMessage() + "\n";
			}
			if (!m_SubQuerries[1].IsValid())
			{
				m_ParseStatus = false;
				m_ParseStatus.ErrorMessage += "Error parcing subquerrys 2: " + m_SubQuerries[1].GetErrorMessage();
			}
		}
	}
	std::vector<DocID> Intersect(std::vector<DocID> const& LeftOrderedSet, std::vector<DocID> const& RightOrderedSet)
	{
		std::vector<DocID> ReturnValue = {};
		int LeftIterator = 0;
		int RightIterator = 0;
		while (LeftIterator < LeftOrderedSet.size() && RightIterator < RightOrderedSet.size())
		{
			if (LeftOrderedSet[LeftIterator] == RightOrderedSet[RightIterator])
			{
				ReturnValue.push_back(LeftOrderedSet[LeftIterator]);
				LeftIterator++;
				RightIterator++;
			}
			else
			{
				if (LeftOrderedSet[LeftIterator] < RightOrderedSet[RightIterator])
				{
					LeftIterator++;
				}
				else
				{
					RightIterator++;
				}
			}
		}
		return(ReturnValue);
	}
	std::vector<DocID> Union(std::vector<DocID> const& LeftOrderedSet, std::vector<DocID> const& RightOrderedSet)
	{
		std::vector<DocID> ReturnValue = {};
		int LeftIterator = 0;
		int RightIterator = 0;
		while (LeftIterator < LeftOrderedSet.size() && RightIterator < RightOrderedSet.size())
		{
			if (LeftOrderedSet[LeftIterator] == RightOrderedSet[RightIterator])
			{
				ReturnValue.push_back(LeftOrderedSet[LeftIterator]);
				LeftIterator++;
				RightIterator++;
			}
			else if (LeftOrderedSet[LeftIterator] < RightOrderedSet[RightIterator])
			{
				ReturnValue.push_back(LeftOrderedSet[LeftIterator]);
				LeftIterator++;
			}
			else
			{
				ReturnValue.push_back(RightOrderedSet[RightIterator]);
				RightIterator++;
			}
		}
		while (LeftIterator < LeftOrderedSet.size())
		{
			ReturnValue.push_back(LeftOrderedSet[LeftIterator]);
			LeftIterator++;
		}
		while (RightIterator < RightOrderedSet.size())
		{
			ReturnValue.push_back(RightOrderedSet[RightIterator]);
			RightIterator++;
		}
		return(ReturnValue);
	}
	std::vector<DocID> BooleanQuerry::EvaluateWordQuerry(MBIndex& Index,std::string const& WordToEvaluate)
	{
		std::vector<DocID> ReturnValue = {};
		PostingListID WordPosting = Index.m_TokenDictionary.GetTokenPosting(m_QuerryString);
		PostingsList& ListReference = Index.m_PostingsLists[WordPosting];
		for (size_t i = 0; i < ListReference.size(); i++)
		{
			ReturnValue.push_back(ListReference[i].DocumentReference);
		}
		return(ReturnValue);
	}
	bool PostinglistIteratorsAreValid(std::vector<int>& PostinglistIterators, std::vector<PostingsList*>& Postinglists)
	{
		bool IsValid = true;
		for (size_t i = 0; i < PostinglistIterators.size(); i++)
		{
			if (PostinglistIterators[i] >= (*Postinglists[i]).size())
			{
				IsValid = false;
				break;
			}
		}
		return(IsValid);
	}
	DocID IterateToNextCommonDocument(std::vector<int>& PostinglistIterators,std::vector<PostingsList*>& Postinglists)
	{
		DocID ReturnValue = -1;
		int IteratorToCompareIndex = 0;
		//has to be a new document, if the current iterators are the function returns directly
		//PostinglistIterators[0]++;
		while (PostinglistIteratorsAreValid(PostinglistIterators,Postinglists))
		{
			if ((*Postinglists[IteratorToCompareIndex])[PostinglistIterators[IteratorToCompareIndex]].DocumentReference ==
				(*Postinglists[IteratorToCompareIndex + 1])[PostinglistIterators[IteratorToCompareIndex + 1]].DocumentReference)
			{
				IteratorToCompareIndex += 1;
				if (IteratorToCompareIndex == PostinglistIterators.size() - 1)
				{
					//alla är samma
					ReturnValue = (*Postinglists[0])[PostinglistIterators[0]].DocumentReference;
					break;
				}
				continue;
			}
			else
			{
				if ((*Postinglists[IteratorToCompareIndex])[PostinglistIterators[IteratorToCompareIndex]].DocumentReference <
					(*Postinglists[IteratorToCompareIndex + 1])[PostinglistIterators[IteratorToCompareIndex + 1]].DocumentReference)
				{
					//detta innebär att "dokumentförslaget" inte fungerar och vi går till toppen igen
					PostinglistIterators[0]++;
					IteratorToCompareIndex = 0;
				}
				else
				{
					PostinglistIterators[IteratorToCompareIndex + 1]++;
				}
			}
		}
		return(ReturnValue);
	}
	bool DocumentPositionIteratorsAreValid(std::vector<int> const& PostinglistIterators, std::vector<Posting*>& Postinglists)
	{
		bool IsValid = true;
		for (size_t i = 0; i < PostinglistIterators.size(); i++)
		{
			if (PostinglistIterators[i] >= (*Postinglists[i]).m_DocumentPositions.size())
			{
				IsValid = false;
				break;
			}
		}
		return(IsValid);
	}
	bool PhraseInDocument(std::vector<int> const& PostinglistIterators, std::vector<PostingsList*>& Postinglists)
	{
		bool ReturnValue = false;
		std::vector<int> DocumentPositionIterators = std::vector<int>(PostinglistIterators.size(), 0);
		std::vector<Posting*> Postings = {};
		for (size_t i = 0; i < PostinglistIterators.size(); i++)
		{
			Postings.push_back((Posting*)&((*Postinglists[i])[PostinglistIterators[i]]));
		}
		int IteratorToCompareIndex = 0;
		while (DocumentPositionIteratorsAreValid(DocumentPositionIterators,Postings))
		{
			if (Postings[IteratorToCompareIndex]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex]] + 1 ==
				Postings[IteratorToCompareIndex + 1]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex + 1]])
			{
				IteratorToCompareIndex++;
				if (IteratorToCompareIndex == PostinglistIterators.size() - 1)
				{
					ReturnValue = true;
					break;
				}
			}
			if (Postings[IteratorToCompareIndex]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex]] + 1 <
				Postings[IteratorToCompareIndex + 1]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex + 1]])
			{
				DocumentPositionIterators[0]++;
				IteratorToCompareIndex = 0;
			}
			if (Postings[IteratorToCompareIndex]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex]] + 1 >
				Postings[IteratorToCompareIndex + 1]->m_DocumentPositions[DocumentPositionIterators[IteratorToCompareIndex + 1]])
			{
				DocumentPositionIterators[IteratorToCompareIndex + 1]++;
			}
		}
		return(ReturnValue);
	}
	std::vector<DocID> BooleanQuerry::EvaluatePhraseQuerry(MBIndex& Index,std::string const& QuerryToEvaluate)
	{
		std::vector<DocID> ReturnValue = {};
		if (QuerryToEvaluate == "")
		{
			return ReturnValue;
		}
		std::vector<std::string> PhraseWords = Split(QuerryToEvaluate.substr(1,QuerryToEvaluate.size()-2), " ");
		std::vector<int> PostinglistIterators = std::vector<int>(PhraseWords.size(), 0);
		std::vector<PostingsList*> PostingLists = {};
		for (size_t i = 0; i < PhraseWords.size(); i++)
		{
			PostingListID NewPostingId = Index.m_TokenDictionary.GetTokenPosting(PhraseWords[i]);
			if (NewPostingId == -1)
			{
				return(ReturnValue);
			}
			PostingLists.push_back(&Index.GetPostinglist(NewPostingId));
		}
		DocID CurrentDocument = IterateToNextCommonDocument(PostinglistIterators,PostingLists);
		while(CurrentDocument != -1)
		{
			if (PhraseInDocument(PostinglistIterators, PostingLists))
			{
				ReturnValue.push_back(CurrentDocument);
			}
			PostinglistIterators[0]++;
			CurrentDocument = IterateToNextCommonDocument(PostinglistIterators, PostingLists);
		}
		return(ReturnValue);
	}
	std::vector<DocID> BooleanQuerry::Evaluate(MBIndex& Index)
	{
		std::vector<DocID> ReturnValue = {};
		if (!m_IsAtomic)
		{
			std::vector<DocID> FirstOperandResult = m_SubQuerries[0].Evaluate(Index);
			std::vector<DocID> SecondOperandResult = m_SubQuerries[1].Evaluate(Index);
			if (m_BinaryOperator == BoleanBinaryOperator::AND)
			{
				ReturnValue = Intersect(FirstOperandResult, SecondOperandResult);
			}
			else if (m_BinaryOperator == BoleanBinaryOperator::OR)
			{
				ReturnValue = Union(FirstOperandResult, SecondOperandResult);
			}
		}
		else
		{
			//nu kör vi på en basic som kollar om det är en phrase search eller om det är ett ord
			if (GetNextStringBeginning(m_QuerryString, 0) == -1)
			{
				//single word querry, vi slår upp den i dictionaryn, hittar postingen och konstruerar document listan
				PostingListID WordPosting = Index.m_TokenDictionary.GetTokenPosting(m_QuerryString);
				if (WordPosting == -1)
				{
					return(ReturnValue);
				}
				PostingsList& ListReference = Index.m_PostingsLists[WordPosting];
				for (size_t i = 0; i < ListReference.size(); i++)
				{
					ReturnValue.push_back(ListReference[i].DocumentReference);
				}
			}
			else
			{
				ReturnValue = EvaluatePhraseQuerry(Index,m_QuerryString);
				//j
			}
		}
		return(ReturnValue);
	}
	//END BooleanQuerry
}