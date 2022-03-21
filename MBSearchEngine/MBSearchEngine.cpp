#include <MBSearchEngine/MBSearchEngine.h>
#include <MBUtility/MBAlgorithms.h>
#include <MBUtility/MBStrings.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <array>
#include <MBUnicode/MBUnicode.h>
#include <map>
#include <cmath>
#include <MBSearchEngine/MBISaveIndexSpec.h>
namespace MBSearchEngine
{
	//Posting
	void swap(Posting& LeftPosting, Posting& RightPosting)
	{
		std::swap(LeftPosting.DocumentReference, RightPosting.DocumentReference);
		std::swap(LeftPosting.NumberOfOccurances, RightPosting.NumberOfOccurances);
		std::swap(LeftPosting.m_DocumentPositions, RightPosting.m_DocumentPositions);
	}
	//searchToken
	bool SearchToken::operator==(const SearchToken& other) const
	{
		return(TokenName == other.TokenName);
	}
	SearchToken::SearchToken(std::string const& StringToNormalize)
	{
		//h�r vill vi egentligen converat allt till s�g lowercase
		TokenName = MBUnicode::UnicodeStringToLower(StringToNormalize);
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

	//BEGIN BooleanQuerryIterator
		//bool m_IsFinished = false;
		//bool m_IsAtomic = false;
		//BoleanBinaryOperator m_BinaryOperator = BoleanBinaryOperator::Null;
		//BooleanQuerryIterator* m_LeftOperand = nullptr;
		//BooleanQuerryIterator* m_RightOperand = nullptr;
		//std::vector<std::shared_ptr<const PostingsList>> m_AssociatedPostings = {};
		//std::vector<PostingsListIterator> m_QuerryWordIterator = {};
	void swap(BooleanQuerryIterator& LeftIterator, BooleanQuerryIterator& RightIterator)
	{
		std::swap(LeftIterator.m_IsNegated, RightIterator.m_IsNegated);
		std::swap(LeftIterator.m_NegatedDocIDIterator, RightIterator.m_NegatedDocIDIterator);
		std::swap(LeftIterator.CurrentValue, RightIterator.CurrentValue);
		std::swap(LeftIterator.m_IsFinished, RightIterator.m_IsFinished);
		std::swap(LeftIterator.m_IsAtomic, RightIterator.m_IsAtomic);
		std::swap(LeftIterator.m_BinaryOperator, RightIterator.m_BinaryOperator);
		std::swap(LeftIterator.m_LeftOperand, RightIterator.m_LeftOperand);
		std::swap(LeftIterator.m_RightOperand, RightIterator.m_RightOperand);
		std::swap(LeftIterator.m_AssociatedPostings, RightIterator.m_AssociatedPostings);
		std::swap(LeftIterator.m_QuerryWordIterator, RightIterator.m_QuerryWordIterator);
		std::swap(LeftIterator.m_MaxDocID, RightIterator.m_MaxDocID);
	}
	BooleanQuerryIterator::BooleanQuerryIterator() 
	{
		m_IsFinished = true;
	}
	BooleanQuerryIterator::BooleanQuerryIterator(BooleanQuerry const& InitialQuerry,MBIndex& AssociatedIndex)
	{
		m_MaxDocID = AssociatedIndex.m_DocumentIDs.size();
		m_BinaryOperator = InitialQuerry.m_BinaryOperator;
		m_IsNegated = InitialQuerry.m_Negated;
		if (InitialQuerry.m_IsAtomic)
		{
			m_IsAtomic = true;
			//mer grejer h�r
			if (InitialQuerry.m_QuerryString[0] == '\"')
			{
				//phrase querry
				std::vector<std::string> WordsInPhrase = MBUtility::Split(InitialQuerry.m_QuerryString.substr(1, InitialQuerry.m_QuerryString.size() - 2)," ");
				for (size_t i = 0; i < WordsInPhrase.size(); i++)
				{
					PostingListID CurrentID = AssociatedIndex.p_GetPostingListID(WordsInPhrase[i]);
					std::shared_ptr<const PostingsList> NewPostinglist;
					if (CurrentID != -1)
					{
						NewPostinglist = AssociatedIndex.GetPostinglist(CurrentID);
					}
					else
					{
						NewPostinglist = std::make_shared<const PostingsList>();
					}
					m_QuerryWordIterator.push_back(NewPostinglist->begin());
					m_AssociatedPostings.push_back(NewPostinglist);
				}
			}
			else
			{
				PostingListID CurrentID = AssociatedIndex.p_GetPostingListID(InitialQuerry.m_QuerryString);
				std::shared_ptr<const PostingsList> NewPostinglist;
				if (CurrentID != -1)
				{
					NewPostinglist = AssociatedIndex.GetPostinglist(CurrentID);
				}
				else
				{
					NewPostinglist = std::make_shared<const PostingsList>();
				}
				m_QuerryWordIterator.push_back(NewPostinglist->begin());
				m_AssociatedPostings.push_back(NewPostinglist);
			}
		}
		else
		{
			m_IsAtomic = false;
			m_LeftOperand = new BooleanQuerryIterator(InitialQuerry.m_SubQuerries[0],AssociatedIndex);
			m_RightOperand = new BooleanQuerryIterator(InitialQuerry.m_SubQuerries[1],AssociatedIndex);
		}
		Increment();
	}
	BooleanQuerryIterator::BooleanQuerryIterator(BooleanQuerryIterator&& ObjectToSteal)
	{
		swap(*this, ObjectToSteal);
	}
	BooleanQuerryIterator::BooleanQuerryIterator(BooleanQuerryIterator const& ObjectToCopy)
	{
		m_IsFinished = ObjectToCopy.m_IsFinished;
		m_IsAtomic = ObjectToCopy.m_IsAtomic;
		m_NegatedDocIDIterator = ObjectToCopy.m_NegatedDocIDIterator;
		m_IsNegated = ObjectToCopy.m_IsNegated;
		m_BinaryOperator = ObjectToCopy.m_BinaryOperator;
		CurrentValue = ObjectToCopy.CurrentValue;
		m_AssociatedPostings = ObjectToCopy.m_AssociatedPostings;
		m_QuerryWordIterator = ObjectToCopy.m_QuerryWordIterator;
		m_MaxDocID = ObjectToCopy.m_MaxDocID;
		if (ObjectToCopy.m_LeftOperand != nullptr)
		{
			//om en �r det �r b�da det
			m_LeftOperand = new BooleanQuerryIterator(*ObjectToCopy.m_LeftOperand);
			m_RightOperand = new BooleanQuerryIterator(*ObjectToCopy.m_RightOperand);
		}
	}
	BooleanQuerryIterator::~BooleanQuerryIterator()
	{
		delete m_LeftOperand;
		delete m_RightOperand;
	}
	BooleanQuerryIterator& BooleanQuerryIterator::operator=(BooleanQuerryIterator NewObject)
	{
		swap(*this, NewObject);
		return(*this);
	}
	bool BooleanQuerryIterator::operator==(BooleanQuerryIterator const& RightIterator) const
	{
		if (m_IsFinished == true && RightIterator.m_IsFinished == true)
		{
			return(true);
		}
		bool ReturnValue = true;
		if ((m_LeftOperand == nullptr && RightIterator.m_LeftOperand != nullptr) || (m_LeftOperand != nullptr && RightIterator.m_LeftOperand == nullptr))
		{
			return(false);
		}
		if ((m_RightOperand == nullptr && RightIterator.m_RightOperand != nullptr) || (m_RightOperand != nullptr && RightIterator.m_RightOperand == nullptr))
		{
			return(false);
		}
		ReturnValue = ReturnValue && (m_IsFinished == RightIterator.m_IsFinished);
		ReturnValue = ReturnValue && (m_IsAtomic == RightIterator.m_IsAtomic);
		ReturnValue = ReturnValue && (m_NegatedDocIDIterator == RightIterator.m_NegatedDocIDIterator);
		ReturnValue = ReturnValue && (m_IsNegated == RightIterator.m_IsNegated);
		ReturnValue = ReturnValue && (m_BinaryOperator == RightIterator.m_BinaryOperator);
		ReturnValue = ReturnValue && (CurrentValue == RightIterator.CurrentValue);
		ReturnValue = ReturnValue && (m_AssociatedPostings == RightIterator.m_AssociatedPostings);
		ReturnValue = ReturnValue && (m_QuerryWordIterator == RightIterator.m_QuerryWordIterator);
		ReturnValue = ReturnValue && (m_MaxDocID == RightIterator.m_MaxDocID);
		if (m_LeftOperand != nullptr)
		{
			//om en ��r det �r b�da det
			ReturnValue = ReturnValue && (*m_LeftOperand == *RightIterator.m_LeftOperand);
			ReturnValue = ReturnValue && (*m_RightOperand == *RightIterator.m_RightOperand);
		}
		return(ReturnValue);
	}
	bool BooleanQuerryIterator::operator!=(BooleanQuerryIterator const& RightIterator) const
	{
		return(!(*this == RightIterator));
	}
	//�ndrar current value
	void BooleanQuerryIterator::p_IterateToNextCommonDocument(std::vector<PostingsListIterator>& IteratorToProcess)
	{
		size_t IteratorToCompareIndex = 0;
		while (p_IteratorsAreValid(IteratorToProcess))
		{
			DocID LeftValue = (*IteratorToProcess[IteratorToCompareIndex]).DocumentReference;
			DocID RightValue = (*IteratorToProcess[IteratorToCompareIndex + 1]).DocumentReference;
			if (LeftValue == RightValue)
			{
				IteratorToCompareIndex += 1;
				if (IteratorToCompareIndex == IteratorToProcess.size()-1)
				{
					return;
				}
			}
			else if (LeftValue > RightValue)
			{
				IteratorToProcess[IteratorToCompareIndex + 1]++;
			}
			else
			{
				IteratorToCompareIndex = 0;
				IteratorToProcess[0]++;
			}
		}
	}
	void BooleanQuerryIterator::p_IncrementPostingListIterators(std::vector<PostingsListIterator>& IteratorsToIncrement)
	{
		for (size_t i = 0; i < IteratorsToIncrement.size(); i++)
		{
			IteratorsToIncrement[i]++;
		}
	}
	bool hf_IteratorsAreValid(std::vector<std::vector<int>::const_iterator>& IteratorsToCheck, std::vector<std::vector<int>::const_iterator>& EndIterators)
	{
		bool ReturnValue = true;
		for (size_t i = 0; i < IteratorsToCheck.size(); i++)
		{
			if (IteratorsToCheck[i] == EndIterators[i])
			{
				ReturnValue = false;
				break;
			}
		}
		return(ReturnValue);
	}
	bool BooleanQuerryIterator::p_PhraseIsInDocument(std::vector<PostingsListIterator>& IteratorsToCheck)
	{
		if (!p_IteratorsAreValid(IteratorsToCheck))
		{
			return(false);
		}
		std::vector<std::vector<int>::const_iterator> PositionIterators = {};
		std::vector<std::vector<int>::const_iterator> EndIterators = {};
		for (size_t i = 0; i < IteratorsToCheck.size(); i++)
		{
			PositionIterators.push_back((*IteratorsToCheck[i]).m_DocumentPositions.begin());
			EndIterators.push_back((*IteratorsToCheck[i]).m_DocumentPositions.end());
		}
		size_t IteratorToCompareIndex = 0;
		while (hf_IteratorsAreValid(PositionIterators,EndIterators))
		{
			int LeftValue = (*PositionIterators[IteratorToCompareIndex]);
			int RightValue = *PositionIterators[IteratorToCompareIndex + 1];
			if (LeftValue == RightValue-1)
			{
				IteratorToCompareIndex += 1;
				if (IteratorToCompareIndex == PositionIterators.size()-1)
				{
					return(true);
				}
			}
			else if (LeftValue > RightValue-1)
			{
				PositionIterators[IteratorToCompareIndex + 1]++;
			}
			else
			{
				IteratorToCompareIndex = 0;
				PositionIterators[0]++;
			}
		}
		return(false);
	}
	bool BooleanQuerryIterator::p_IteratorsAreValid(std::vector<PostingsListIterator>& IteratorsToCheck)
	{
		bool ReturnValue = true;
		for (size_t i = 0; i < IteratorsToCheck.size(); i++)
		{
			if (IteratorsToCheck[i].HasEnded())
			{
				ReturnValue = false;
				break;
			}
		}
		return(ReturnValue);
	}
	void BooleanQuerryIterator::p_AtomicIncrement()
	{
		if (HasEnded())
		{
			return;
		}
		if (m_AssociatedPostings.size() == 1)
		{
			if (m_QuerryWordIterator[0].HasEnded() && !m_IsNegated)
			{
				m_IsFinished = true;
				CurrentValue = -1;
				return;
			}
			DocID NewValue = -1;
			if (!m_QuerryWordIterator[0].HasEnded())
			{
				NewValue = (*m_QuerryWordIterator[0]).DocumentReference;
			}
			CurrentValue = NewValue;
			(m_QuerryWordIterator[0])++;
			return;
		}
		else
		{
			DocID NewValue = -1;
			bool NewValueSet = false;
			while (p_IteratorsAreValid(m_QuerryWordIterator))
			{
				p_IterateToNextCommonDocument(m_QuerryWordIterator);
				if (p_PhraseIsInDocument(m_QuerryWordIterator))
				{
					NewValueSet = true;
					NewValue = (*m_QuerryWordIterator[0]).DocumentReference;
					p_IncrementPostingListIterators(m_QuerryWordIterator);
					break;
				}
				p_IncrementPostingListIterators(m_QuerryWordIterator);
			}
			if (!p_IteratorsAreValid(m_QuerryWordIterator) && NewValueSet == false)
			{
				CurrentValue = -1;
				m_IsFinished = true;
				return;
			}
			CurrentValue = NewValue;
		}
	}
	void BooleanQuerryIterator::p_Union_IterateToNext(BooleanQuerryIterator* LeftIterator, BooleanQuerryIterator* RightIterator)
	{
		while (!LeftIterator->HasEnded() && !RightIterator->HasEnded())
		{
			DocID LeftIteratorValue = **LeftIterator;
			DocID RightITeratorValue = **RightIterator;
			if (LeftIteratorValue == RightITeratorValue)
			{
				return;
			}
			else if(LeftIteratorValue < RightITeratorValue)
			{
				(*LeftIterator)++;
			}
			else
			{
				(*RightIterator)++;
			}
		}
	}
	void BooleanQuerryIterator::p_Disjunction_IterateToNext(BooleanQuerryIterator* LeftIterator, BooleanQuerryIterator* RightIterator)
	{
		return;
	}
	void BooleanQuerryIterator::p_Union_Increment(BooleanQuerryIterator* LeftIterator, BooleanQuerryIterator* RightIterator)
	{
		(*LeftIterator)++;
		(*RightIterator)++;
	}
	void BooleanQuerryIterator::p_Disjuntion_Increment(BooleanQuerryIterator* LeftIterator, BooleanQuerryIterator* RightIterator)
	{
		if (**LeftIterator < **RightIterator)
		{
			(*LeftIterator)++;
		}
		else if(**LeftIterator > **RightIterator)
		{
			(*RightIterator)++;
		}
		else
		{
			(*LeftIterator)++;
			(*RightIterator)++;
		}
	}
	void BooleanQuerryIterator::p_IncrementOperands()
	{
		if (m_BinaryOperator == BoleanBinaryOperator::AND)
		{
			p_Union_Increment(m_LeftOperand, m_RightOperand);
		}
		else
		{
			p_Disjuntion_Increment(m_LeftOperand, m_RightOperand);
		}
	}
	void BooleanQuerryIterator::p_NonAtomicIncrement()
	{
		if (HasEnded())
		{
			return;
		}
		DocID NewValue = -1;
		if (m_BinaryOperator == BoleanBinaryOperator::AND)
		{
			p_Union_IterateToNext(m_LeftOperand, m_RightOperand);
		}
		DocID LeftCurrentValue = **m_LeftOperand;
		DocID RightCurrentValue = **m_RightOperand;
		NewValue = std::min(LeftCurrentValue, RightCurrentValue);
		if (m_LeftOperand->HasEnded() && m_RightOperand->HasEnded() || ((m_LeftOperand->HasEnded() || m_RightOperand->HasEnded()) && m_BinaryOperator == BoleanBinaryOperator::AND))
		{
			CurrentValue = -1;
			m_IsFinished = true;
			return;
		}
		CurrentValue = NewValue;
		p_IncrementOperands();
	}
	void BooleanQuerryIterator::Increment()
	{
		if (HasEnded())
		{
			return;
		}
		if (!m_IsNegated || CurrentValue == -1)
		{
			if (m_IsAtomic)
			{
				p_AtomicIncrement();
			}
			else
			{
				p_NonAtomicIncrement();
			}
		}
		if(m_IsNegated)
		{
			//bara f�r att vanlig iteration sluta ska vi inte det h�r
			if (m_IsFinished)
			{
				m_IsFinished = false;
			}
			if (m_NegatedDocIDIterator+1 != CurrentValue)
			{
				m_NegatedDocIDIterator += 1;
			}
			else
			{
				while (true)
				{
					m_NegatedDocIDIterator = CurrentValue + 1;
					if (m_IsAtomic)
					{
						p_AtomicIncrement();
					}
					else
					{
						p_NonAtomicIncrement();
					}
					if (m_NegatedDocIDIterator != CurrentValue)
					{
						m_IsFinished = false;
						break;
					}
				}
			}
			if (m_NegatedDocIDIterator + 1 >= m_MaxDocID || m_NegatedDocIDIterator == -1)
			{
				m_NegatedDocIDIterator = -1;
				m_IsFinished = true;
				return;
			}
		}
	}
	BooleanQuerryIterator& BooleanQuerryIterator::operator++()
	{
		Increment();
		return(*this);
	}
	BooleanQuerryIterator& BooleanQuerryIterator::operator++(int)
	{
		++(*this);
		return(*this);
	}
	DocID BooleanQuerryIterator::operator*()
	{
		if (!m_IsNegated)
		{
			return(CurrentValue);
		}
		else
		{
			return(m_NegatedDocIDIterator);
		}
	}
	DocID BooleanQuerryIterator::operator->()
	{
		return(**this);
	}
	//END BooleanQuerryIterator


	//BooleanQuerry
	TopQuerry BooleanQuerry::GetSubquerries(std::string const& QuerryToEvaluate)
	{
		TopQuerry ReturnValue;
		std::string QuerryToParse = RemoveRedundatParenthesis(QuerryToEvaluate);
		std::vector<size_t> OperatorPositions = GetLogicalOperatorPositions(QuerryToParse);
		if (OperatorPositions.size() == 0)
		{
			ReturnValue.IsAtomic = true;
			if (QuerryToEvaluate.substr(0, 4) == "NOT ")
			{
				ReturnValue.IsNegated = true;
			}
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
		//checkar validityn av paranteser, AND och OR kan bara vara p� samma djup som sig sj�lva
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
		//nu �r parsingen i ett valid state och vi kommer d� att bryta upp den i sub querrys utifr�n operatorn med l�gst depth
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
		if (LowestDetph != 0 && QuerryToParse.substr(0, 4) == "NOT ")
		{
			ReturnValue.IsNegated = true;
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
	BooleanQuerryIterator BooleanQuerry::begin(MBIndex& AssociatedIndex)
	{
		return(BooleanQuerryIterator(*this, AssociatedIndex));
	}
	BooleanQuerryIterator BooleanQuerry::end(MBIndex& AssociatedIndex)
	{
		return(BooleanQuerryIterator());
	}
	BooleanQuerry::BooleanQuerry(std::string const& QuerryToParse)
	{
		TopQuerry Querry = GetSubquerries(QuerryToParse);
		if (!m_ParseStatus)
		{
			//error h�nde, slut med resten
			return;
		}
		m_Negated = Querry.IsNegated;
		if (Querry.IsAtomic)
		{
			m_IsAtomic = true;
			if (!m_Negated)
			{
				m_QuerryString = QuerryToParse;
			}
			else
			{
				m_QuerryString = RemoveRedundatParenthesis(RemoveRedundatParenthesis(QuerryToParse).substr(4));
			}
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
		std::shared_ptr<const PostingsList> PostingPointer = Index.GetPostinglist(WordPosting);
		const PostingsList& ListReference = *PostingPointer;
		for (size_t i = 0; i < ListReference.size(); i++)
		{
			ReturnValue.push_back(ListReference[i].DocumentReference);
		}
		return(ReturnValue);
	}
	bool PostinglistIteratorsAreValid(std::vector<int>& PostinglistIterators, std::vector<std::shared_ptr<const PostingsList>>& Postinglists)
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
	DocID IterateToNextCommonDocument(std::vector<int>& PostinglistIterators, std::vector<std::shared_ptr<const PostingsList>>& Postinglists)
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
					//alla �r samma
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
					//detta inneb�r att "dokumentf�rslaget" inte fungerar och vi g�r till toppen igen
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
	bool PhraseInDocument(std::vector<int> const& PostinglistIterators, std::vector<std::shared_ptr<const PostingsList>>& Postinglists)
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
		std::vector<std::string> PhraseWords = MBUtility::Split(QuerryToEvaluate.substr(1,QuerryToEvaluate.size()-2), " ");
		std::vector<int> PostinglistIterators = std::vector<int>(PhraseWords.size(), 0);
		std::vector<std::shared_ptr<const PostingsList>> PostingLists = {};
		for (size_t i = 0; i < PhraseWords.size(); i++)
		{
			PostingListID NewPostingId = Index.m_TokenDictionary.GetTokenPosting(PhraseWords[i]);
			if (NewPostingId == -1)
			{
				return(ReturnValue);
			}
			PostingLists.push_back(Index.GetPostinglist(NewPostingId));
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
			//nu k�r vi p� en basic som kollar om det �r en phrase search eller om det �r ett ord
			if (GetNextStringBeginning(m_QuerryString, 0) == -1)
			{
				//single word querry, vi sl�r upp den i dictionaryn, hittar postingen och konstruerar document listan
				PostingListID WordPosting = Index.m_TokenDictionary.GetTokenPosting(m_QuerryString);
				if (WordPosting == -1)
				{
					return(ReturnValue);
				}
				std::shared_ptr<const PostingsList> PostingPointer = Index.GetPostinglist(WordPosting);
				const PostingsList& ListReference = *PostingPointer;
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

	//BEGIN TokenDictionary
	void TokenDictionary::Save(std::fstream& OutFile)
	{
		//MBI_SaveVectorToFile(OutFile, m_TokenMapInMemory);
		MBI_SaveObjectToFile<TokenDictionary>(OutFile, *this);
	}
	void TokenDictionary::Load(std::fstream& FileToReadFrom)
	{
		//m_TokenMapInMemory = MBI_ReadVectorFromFile<TokenDictionaryEntry>(FileToReadFrom);
		*this = MBI_ReadObjectFromFile<TokenDictionary>(FileToReadFrom);
	}
	PostingListID TokenDictionary::GetTokenPosting(std::string const& TokenData)
	{
		PostingListID ReturnValue = -1;
		std::string NormalizedData = TokenClass(TokenData).GetTokenString();
		if (this->TokenInDictionary(NormalizedData))
		{
			ReturnValue = m_TokenMapInMemory[NormalizedData].PostingIndex;
		}
		return(ReturnValue);
		//int MapIndex = MBAlgorithms::BinarySearch(m_TokenMapInMemory, TokenData);
		//if (MapIndex != -1)
		//{
		//	return(m_TokenMapInMemory[MapIndex].PostingIndex);
		//}
	}
	int TokenDictionary::GetNumberOfTokens()
	{
		return(m_TokenMapInMemory.size());
	}
	PostingListID TokenDictionary::AddNewToken(std::string const& TokenData)
	{
		PostingListID NewListID = GetNumberOfTokens();
		//m_TokenMapInMemory.push_back(TokenDictionaryEntry(TokenData, NewListID));
		m_TokenMapInMemory[TokenData] = TokenDictionaryEntry(TokenData, NewListID);
		//extremt l�ngsamt och ass egentligen, men f�r t�nka ut datastrukturen lite b�ttre sen
		//std::sort(m_TokenMapInMemory.begin(), m_TokenMapInMemory.end());
		return(NewListID);
	}
	TokenDictionaryIterator TokenDictionary::begin()
	{
		TokenDictionaryIterator ReturnValue(m_TokenMapInMemory.begin());
		//ReturnValue.m_Offset = 0;
		return(ReturnValue);
	}
	TokenDictionaryIterator TokenDictionary::end()
	{
		TokenDictionaryIterator ReturnValue(m_TokenMapInMemory.end());
		//ReturnValue.m_Offset = m_TokenMapInMemory.size();
		return(ReturnValue);
	}
	bool TokenDictionary::TokenInDictionary(std::string const& TokenData)
	{
		return(m_TokenMapInMemory.find(TokenData) != m_TokenMapInMemory.end());
	}
	//END TokenDictionary

	//BEGIN TokenDictionaryEntry::
	TokenDictionaryEntry::TokenDictionaryEntry(std::string const& NewTokenData, PostingListID NewPostingIndex)
	{
		TokenData = NewTokenData;
		PostingIndex = NewPostingIndex;
	}
	bool TokenDictionaryEntry::operator<(std::string const& StringToCompare) const
	{
		return(TokenData < StringToCompare);
	}
	bool TokenDictionaryEntry::operator==(std::string const& StringToCompare) const
	{
		return(TokenData == StringToCompare);
	}
	bool TokenDictionaryEntry::operator<(TokenDictionaryEntry const& DictionaryEntryToCompare) const
	{
		return(TokenData < DictionaryEntryToCompare.TokenData);
	}
	bool TokenDictionaryEntry::operator==(TokenDictionaryEntry const& DictionaryEntryToCompare) const
	{
		return(TokenData == DictionaryEntryToCompare.TokenData);
	}
	//END TokenDictionaryEntry

	//Begin Posting
	Posting::Posting(DocID NewDocumentReference)
	{
		DocumentReference = NewDocumentReference;
		NumberOfOccurances = 0;
	};
	Posting::Posting()
	{

	}
	void Posting::Update(int TokenPosition)
	{
		NumberOfOccurances += 1;
		m_DocumentPositions.push_back(TokenPosition);
	}
	void Posting::Print() const
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
	bool Posting::operator<(Posting const& RightPosting) const
	{
		return(this->DocumentReference < RightPosting.DocumentReference);
	}
	bool Posting::operator<(DocID IDToCompare) const
	{
		return(DocumentReference < IDToCompare);
	}
	bool Posting::operator==(DocID IDToCompare) const
	{
		return(DocumentReference == IDToCompare);
	}
	//End Posting

	//Begin TokenDictionaryIterator
	//TokenDictionaryIterator::TokenDictionaryIterator(std::vector<TokenDictionaryEntry> const& DictionaryData)
	//	:m_DictionaryData(&DictionaryData)
	//{
	//	m_MaxElements = DictionaryData.size();
	//}
	//TokenDictionaryIterator::TokenDictionaryIterator(TokenDictionaryIterator const& IteratorToCopy)
	//	: m_DictionaryData(IteratorToCopy.m_DictionaryData)
	//{	
	//	m_Offset = IteratorToCopy.m_Offset;
	//	m_MaxElements = IteratorToCopy.m_MaxElements;
	//}
	TokenDictionaryIterator::TokenDictionaryIterator(std::map<std::string,TokenDictionaryEntry>::iterator const& InitialIterator)
	{
		_InternalIterator = InitialIterator;
	}
	bool TokenDictionaryIterator::operator==(TokenDictionaryIterator const& RightIterator) const
	{
		//kanske borde j�mf�ra huruvida dem �r har samma container ocks�, men skitsamma s� l�nge
		//return(m_Offset == RightIterator.m_Offset);
		return(_InternalIterator == RightIterator._InternalIterator);
	}
	bool TokenDictionaryIterator::operator!=(TokenDictionaryIterator const& RightIterator)
	{
		return(!(*this == RightIterator));
	}
	TokenDictionaryIterator& TokenDictionaryIterator::operator++()
	{
		//m_Offset += 1;
		_InternalIterator++;
		return(*this);
	}
	TokenDictionaryIterator& TokenDictionaryIterator::operator++(int)
	{
		//m_Offset += 1;
		_InternalIterator++;
		return(*this);
	}
	TokenDictionaryEntry& TokenDictionaryIterator::operator*() const
	{
		return((*_InternalIterator).second);
	}
	TokenDictionaryEntry& TokenDictionaryIterator::operator->() const
	{
		return((*_InternalIterator).second);
	}
	//END TokenDictionaryIterator

	//BEGIN PostingsListIterator
	PostingsListIterator::PostingsListIterator(const PostingsList* AssociatedPostingslist)
	{
		m_ListReference = AssociatedPostingslist;
	}
	PostingsListIterator::PostingsListIterator(PostingsListIterator const& IteratorToCopy )
	{
		m_ListReference = IteratorToCopy.m_ListReference;
		m_PostingsListOffset = IteratorToCopy.m_PostingsListOffset;
	}
	bool PostingsListIterator::HasEnded()
	{
		return(m_PostingsListOffset >= m_ListReference->size());
	}
	bool PostingsListIterator::operator==(PostingsListIterator const& RightIterator) const
	{
		return(m_PostingsListOffset == RightIterator.m_PostingsListOffset);
	}
	bool PostingsListIterator::operator!=(PostingsListIterator const& RightIterator) const
	{
		return(!(*this == RightIterator));
	}
	PostingsListIterator& PostingsListIterator::operator++()
	{
		m_PostingsListOffset += 1;
		return(*this);
	}
	PostingsListIterator& PostingsListIterator::operator++(int)
	{
		m_PostingsListOffset += 1;
		return(*this);
	}
	const Posting& PostingsListIterator::operator*() const
	{
		return((*m_ListReference)[m_PostingsListOffset]);
	}
	const Posting& PostingsListIterator::operator->() const
	{
		return((*m_ListReference)[m_PostingsListOffset]);
	}
	//END PostingsListIterator
	
	//Begin PostingsList
	void PostingsList::AddPosting(DocID DocumentID, int TokenPosition)
	{
		//f�ruts�tter att listan �r sorterad
		//int DocumentPostingPosition = MBAlgorithms::BinarySearch(m_PostingsInMemory, DocumentID);
		assert(ASSERTION_LastAddedDocumentID <= DocumentID);
		ASSERTION_LastAddedDocumentID = DocumentID;
		if (m_PostingsInMemory.find(DocumentID) == m_PostingsInMemory.end())
		{
			//helt pantad algorithm egentligen, vi l�gger till en p� slutet och sedan sorterar vi, men kr�ver nog egentligen att man har lite mer eftertanke med hur
			//man vill att datastrukturen ska se ut
			Posting NewMember = Posting(DocumentID);
			NewMember.Update(TokenPosition);
			//m_PostingsInMemory.push_back(Posting(DocumentID));
			//m_PostingsInMemory.back().Update(TokenPosition);
			m_PostingsInMemory[DocumentID] = NewMember;
			m_SortedDocumentIDs.push_back(DocumentID);
			m_DocumentFrequency += 1;
			//std::sort(m_PostingsInMemory.begin(), m_PostingsInMemory.end());
		}
		else
		{
			m_PostingsInMemory[DocumentID].Update(TokenPosition);
		}
	}
	int PostingsList::size() const { return(m_PostingsInMemory.size()); };
	PostingClass const& PostingsList::operator[](PostingListID Index) const
	{
		//kommer f�r�ndras iomed
		return(m_PostingsInMemory.at(m_SortedDocumentIDs[Index]));
	}
	size_t PostingsList::GetDocumentFrequency() const
	{
		return(m_DocumentFrequency);
	}
	PostingsListIterator PostingsList::begin() const
	{
		return(PostingsListIterator(this));
	}
	PostingsListIterator PostingsList::end() const
	{
		PostingsListIterator ReturnValue(this);
		ReturnValue.m_PostingsListOffset = size();
		return(ReturnValue);
	}
	void swap(PostingsList& LeftList, PostingsList& RightList)
	{
		std::swap(LeftList.m_PostingsInMemory, RightList.m_PostingsInMemory);
		std::swap(LeftList.m_SortedDocumentIDs, RightList.m_SortedDocumentIDs);
		std::swap(LeftList.m_DocumentFrequency, RightList.m_DocumentFrequency);
		//DEBUG
		std::swap(LeftList.ASSERTION_LastAddedDocumentID, RightList.ASSERTION_LastAddedDocumentID);
	}
	//END PostingsList
	
	//BEGIN PostingsListHandler
		//friend void MBI_SaveObjectToFile<PostingslistHandler>(std::fstream&, PostingslistHandler&);
		//friend PostingslistHandler MBI_ReadObjectFromFile<PostingslistHandler>(std::fstream&);
	size_t PostingslistHandler::NumberOfPostings()
	{
		return(m_NumberOfPostings);
	}
	bool PostingslistHandler::p_PostingIsInMemory(PostingListID IDToCheck)
	{
		return(m_PostingsInMemory.find(IDToCheck) != m_PostingsInMemory.end());
	}
	//ANTAGANDE postinglistorna tar aldrig bort element utan l�gger enbart till, vilket inneb�r att den senaste filen som inneh�ller en posting
	//har den mest uppdaterade varianten
	std::shared_ptr<PostingsList> PostingslistHandler::p_GetPostinglist(PostingListID IDToGet)
	{
		if (p_PostingIsInMemory(IDToGet))
		{
			return(m_PostingsInMemory[IDToGet]);
		}
		else
		{
			if (IDToGet + 1 > m_NumberOfPostings)
			{
				//kanske borde throw en exception
				assert(false);
			}
			else
			{
				//Att id:n �r mindre �n antalet postings inneb�r att den finns p� filen
				std::shared_ptr<PostingsList> NewPointer;
				for (int i = m_PostingsOnDisk.size()-1; i >= 0; i--)
				{
					PostingDiskDataInfo& CurrentDiskDataInfo = m_PostingsOnDisk[i];
					if (CurrentDiskDataInfo.PostinglistFilePositions.find(IDToGet) != CurrentDiskDataInfo.PostinglistFilePositions.end())
					{
						PostingsList* NewMemory = new PostingsList();
						CurrentDiskDataInfo.FileDataStream->seekg(CurrentDiskDataInfo.PostinglistFilePositions[IDToGet]);
						*NewMemory = MBI_ReadObjectFromFile<PostingsList>(*CurrentDiskDataInfo.FileDataStream);
						NewPointer = std::shared_ptr<PostingsList>(NewMemory);
						break;
					}
				}
				if (NewPointer.use_count() == 0)
				{
					//fick ingen ny data
					assert(false);
				}
				return(NewPointer);
			}
		}
	}
	void PostingslistHandler::p_WriteCacheToDisk(std::string OutputFilepath)
	{
		m_PostingsOnDisk.push_back(PostingDiskDataInfo());
		PostingDiskDataInfo& NewDiskInfo = m_PostingsOnDisk.back();
		NewDiskInfo.DiskDataFilepath = OutputFilepath;
		NewDiskInfo.FileDataStream = new std::fstream(OutputFilepath);
		for (std::pair<PostingListID,std::shared_ptr<PostingsList>> const& CurrentPosting : m_PostingsInMemory)
		{
			NewDiskInfo.PostinglistFilePositions[CurrentPosting.first] = NewDiskInfo.FileDataStream->tellp();
			MBI_SaveObjectToFile<PostingsList>(*NewDiskInfo.FileDataStream, *CurrentPosting.second);
		}
		m_PostingsInMemory.clear();
	}
	void PostingslistHandler::SetBaseFile(std::string const& BaseFilepath)
	{
		if (m_PostingsOnDisk.size() == 1)
		{
			PostingDiskDataInfo& DiskDataToModify = m_PostingsOnDisk[0];
			if (DiskDataToModify.FileDataStream == nullptr)
			{
				DiskDataToModify.DiskDataFilepath = BaseFilepath;
				DiskDataToModify.IsTemporaryFile = false;
				DiskDataToModify.FileDataStream = new std::fstream(BaseFilepath, std::ios::binary | std::ios::in);
			}
		}
	}
	void PostingslistHandler::p_ClearFileData()
	{
		for (size_t i = 0; i < m_PostingsOnDisk.size(); i++)
		{
			m_PostingsOnDisk[i].FileDataStream->flush();
			m_PostingsOnDisk[i].FileDataStream->close();
			delete m_PostingsOnDisk[i].FileDataStream;
			if (m_PostingsOnDisk[i].IsTemporaryFile)
			{
				std::filesystem::remove(m_PostingsOnDisk[i].DiskDataFilepath);
			}
		}
		m_PostingsOnDisk = {};
	}
	void PostingslistHandler::p_MergeFilesAndCache(std::fstream& FileToWriteTo)
	{
		PostingDiskDataInfo NewDataInfo;
		NewDataInfo.DiskDataFilepath = "";
		NewDataInfo.IsTemporaryFile = false;
		size_t PostingsCount = NumberOfPostings();
		size_t PostingsPositionArryPosition = FileToWriteTo.tellp();
		for (size_t i = 0; i < PostingsCount; i++)
		{
			MBI_SaveFixedSizeInteger(FileToWriteTo, 0, 4);
		}
		for (size_t i = 0; i < m_NumberOfPostings; i++)
		{
			//ANTAGANDE filen i minne �r mest uppdaterad, sedan �r det senaste filen p� disk
			NewDataInfo.PostinglistFilePositions[i] = FileToWriteTo.tellp();
			bool PostingIsWritten = false;
			if (p_PostingIsInMemory(i))
			{
				MBI_SaveObjectToFile<PostingsList>(FileToWriteTo, *m_PostingsInMemory[i]);
				PostingIsWritten = true;
			}
			int j = m_PostingsOnDisk.size() - 1;
			while (!PostingIsWritten && j >= 0)
			{
				PostingDiskDataInfo& CurrentInfo = m_PostingsOnDisk[j];
				if (CurrentInfo.PostinglistFilePositions.find(i) != CurrentInfo.PostinglistFilePositions.end())
				{
					size_t PostingBegin = CurrentInfo.PostinglistFilePositions[i];
					CurrentInfo.FileDataStream->seekp(PostingBegin);
					size_t PostingEnd = (*(++CurrentInfo.PostinglistFilePositions.find(i))).second;
					size_t BytesToCopy = PostingEnd - PostingBegin;
					char* BytesToWrite =(char*) malloc(BytesToCopy);
					CurrentInfo.FileDataStream->read(BytesToWrite, BytesToCopy);
					FileToWriteTo.write(BytesToWrite, BytesToCopy);
					free(BytesToWrite);
					PostingIsWritten = true;
				}
			}
		}
		p_ClearFileData();
		m_PostingsOnDisk.push_back(NewDataInfo);
		//nu uppdaterar vi listan
		size_t LastWritePosition = FileToWriteTo.tellg();
		FileToWriteTo.seekp(PostingsPositionArryPosition);
		for (size_t i = 0; i < PostingsCount; i++)
		{
			MBI_SaveFixedSizeInteger(FileToWriteTo, NewDataInfo.PostinglistFilePositions[i], 4);
		}
		FileToWriteTo.seekp(LastWritePosition);
	}

	std::shared_ptr<const PostingsList> PostingslistHandler::GetPostinglist(PostingListID IDToGet)
	{
		return(p_GetPostinglist(IDToGet));
	}
	void PostingslistHandler::AddPostinglist()
	{
		PostingListID NewPostingID = m_NumberOfPostings;
		m_NumberOfPostings += 1;
		m_PostingsInMemory[NewPostingID] = std::make_shared<PostingsList>();
	}
	void PostingslistHandler::UpdatePosting(PostingListID PostingListToUpdate, DocID TokenDocument, size_t TokenPosition)
	{
		std::shared_ptr<PostingsList> PostingToUpdate = p_GetPostinglist(PostingListToUpdate);
		PostingToUpdate->AddPosting(TokenDocument, TokenPosition);
	}
	//END PostingsListHandler

	//BEGIN MBIndex
	//MBError UpdatePostings(std::vector<SearchToken> const& TokensToUpdate, std::string const& DocumentName);
	MBError MBIndex::UpdatePostings(std::vector<TokenClass> const& DocumentTokens, DocID DocumentID)
	{
		MBError ReturnValue(true);
		for (size_t i = 0; i < DocumentTokens.size(); i++)
		{
			PostingListID PostinglistToUpdate = -1;
			std::string TokenData = DocumentTokens[i].GetTokenString();
			//if (TokenData == "iji" || TokenData == "iji's" || TokenData == "iji." || TokenData == "iji,")
			//{
			//	std::cout << TokenData << " " << DocumentID << std::endl;
			//}
			if (m_TokenDictionary.TokenInDictionary(TokenData))
			{
				PostinglistToUpdate = m_TokenDictionary.GetTokenPosting(TokenData);
			}
			else
			{
				m_TokenDictionary.AddNewToken(TokenData);
				//m_PostingsLists.push_back(PostingsList());
				m_PostinglistHandler.AddPostinglist();
				PostinglistToUpdate = m_PostinglistHandler.NumberOfPostings() - 1;
			}
			//if (TokenData == "iji")
			//{
			//	PostingsList& AssociatedList = this->GetPostinglist(m_TokenDictionary.GetTokenPosting(TokenData));
			//	std::cout << "debug" << std::endl;
			//}
			m_PostinglistHandler.UpdatePosting(PostinglistToUpdate,DocumentID, i);
		}
		//sorterar den p� slutet s� kanske aningen offektivt f�r storra arrays
		return(ReturnValue);
	}
	PostingListID MBIndex::p_GetPostingListID(std::string const& TokenToEvaluate)
	{
		return(m_TokenDictionary.GetTokenPosting(TokenToEvaluate));
	}

	std::shared_ptr<const PostingsList> MBIndex::GetPostinglist(PostingListID ID)
	{
		return(m_PostinglistHandler.GetPostinglist(ID));
	}
	MBIndex::MBIndex(std::string const& IndexDataPath)
	{
		Load(IndexDataPath);
	}
	MBIndex::MBIndex()
	{
	}
	MBError  MBIndex::Save(std::string const& OutFilename)
	{
		MBError ReturnValue(true);
		std::fstream OutFile(OutFilename, std::ios::binary | std::ios::out);
		if (OutFile.is_open())
		{
			clock_t Timer = clock();
			MBI_SaveVectorToFile<std::string>(OutFile, m_DocumentIDs);
			//std::cout << "Saving Document time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			MBI_SaveVectorToFile<DocumentIndexData>(OutFile, _DocumentIndexDataList);
			//std::cout << "Saving index data time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			MBI_SaveObjectToFile<TokenDictionary>(OutFile, m_TokenDictionary);
			//std::cout << "Saving token dictionary time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			MBI_SaveObjectToFile<PostingslistHandler>(OutFile, m_PostinglistHandler);
			//std::cout << "Saving posting handler time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
		}
		else
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Cant open outfile";
		}
		return(ReturnValue);
	}
	MBError  MBIndex::Load(std::string const& SavedIndexFile)
	{
		MBError ReturnValue(true);
		std::fstream InFile(SavedIndexFile, std::ios::binary | std::ios::in);
		if (InFile.is_open())
		{
			clock_t Timer = clock();
			m_DocumentIDs = MBI_ReadVectorFromFile<std::string>(InFile);
			//std::cout << "Reading Document ID's from file: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			_DocumentIndexDataList = MBI_ReadVectorFromFile<DocumentIndexData>(InFile);
			//std::cout << "Reading DocumentIndexDataList from file: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			m_TokenDictionary = MBI_ReadObjectFromFile<TokenDictionary>(InFile);
			//std::cout << "Reading tokendictionary from file: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
			//std::cout << "Bytes before postingslist: " << InFile.tellg()<<std::endl;
			m_PostinglistHandler = MBI_ReadObjectFromFile<PostingslistHandler>(InFile);
			m_PostinglistHandler.SetBaseFile(SavedIndexFile);
			//std::cout << "Reading postingslist from file: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
			Timer = clock();
		}
		else
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Cant open file";
		}
		return(ReturnValue);
	}
	BooleanQuerryIterator MBIndex::GetBooleanQuerryIterator(std::string const& BooleanQuerryToEvaluate)
	{
		BooleanQuerry ProcessedQuerry(BooleanQuerryToEvaluate);
		return(BooleanQuerryIterator(ProcessedQuerry,*this));
	}
	std::vector<std::string>  MBIndex::EvaluteBooleanQuerry(std::string const& BooleanQuerryToEvaluate)
	{
		std::vector<std::string> ReturnValue = {};
		BooleanQuerry ProcessedQuerry(BooleanQuerryToEvaluate);
		std::vector<DocID> Result = ProcessedQuerry.Evaluate(*this);
		for (size_t i = 0; i < Result.size(); i++)
		{
			ReturnValue.push_back(GetDocumentIdentifier(Result[i]));
		}
		return(ReturnValue);
	}
	struct VectorScore
	{
		float Score = 0;
		DocID ID = -1;
		bool operator<(VectorScore const& RightScore)
		{
			return(Score < RightScore.Score);
		}
	};
	std::vector<std::string>  MBIndex::EvaluteVectorModelQuerry(std::string const& VectorModelQuerry)
	{
		std::vector<std::string> QuerryTokens = MBUtility::Split(VectorModelQuerry, " ");
		std::vector<VectorScore> Result = std::vector<VectorScore>(m_DocumentIDs.size());
		for (size_t i = 0; i < QuerryTokens.size(); i++)
		{
			QuerryTokens[i] = MBUnicode::UnicodeStringToLower(QuerryTokens[i]);
		}
		for (size_t i = 0; i < Result.size(); i++)
		{
			Result[i].ID = i;
		}
		std::vector<std::string> ReturnValue = {};
		for (size_t i = 0; i < QuerryTokens.size(); i++)
		{
			PostingListID AssociatedPostingID = m_TokenDictionary.GetTokenPosting(QuerryTokens[i]);
			std::shared_ptr<const PostingsList> PostList = GetPostinglist(AssociatedPostingID);
			float TermInverseDocumentFrequency = GetInverseDocumentFrequency(AssociatedPostingID);
			for (Posting const& Postings : *PostList)
			{
				Result[Postings.DocumentReference].Score += TermInverseDocumentFrequency * Postings.NumberOfOccurances;
			}
		}
		for (size_t i = 0; i < ReturnValue.size(); i++)
		{
			Result[i].Score /= m_GetDocumentIndexData(i).DocumentLength;
		}
		std::make_heap(Result.begin(), Result.end());
		int NumberOfResultsToReturn = std::min(Result.size(), (size_t)10);
		for (size_t i = 0; i < NumberOfResultsToReturn; i++)
		{
			VectorScore& NextResult = Result.front();
			ReturnValue.push_back(GetDocumentIdentifier(NextResult.ID));
			std::pop_heap(Result.begin(),Result.end());
			Result.pop_back();
		}
		return(ReturnValue);
	}
	DocumentIndexData MBIndex::m_GetDocumentIndexData(DocID ID)
	{
		return(_DocumentIndexDataList[ID]);
	}
	void MBIndex::m_CalculateDocumentLengths()
	{
		_DocumentIndexDataList = std::vector<DocumentIndexData>(m_DocumentIDs.size());
		for (TokenDictionaryEntry const& DictionaryEntry : m_TokenDictionary)
		{
			std::shared_ptr<const PostingsList> AssociatedPosting = GetPostinglist(DictionaryEntry.PostingIndex);
			float TokenInvertedFrequency = GetInverseDocumentFrequency(DictionaryEntry.PostingIndex);
			for (Posting const& Postings : *AssociatedPosting)
			{
				float Weight = TokenInvertedFrequency * Postings.NumberOfOccurances;
				_DocumentIndexDataList[Postings.DocumentReference].DocumentLength += Weight*Weight ;
			}
		}
		for (size_t i = 0; i < _DocumentIndexDataList.size(); i++)
		{
			_DocumentIndexDataList[i].DocumentLength = std::sqrt(_DocumentIndexDataList[i].DocumentLength);
		}
	}
	void MBIndex::Finalize()
	{
		m_CalculateDocumentLengths();
	}
	MBError MBIndex::IndextTextData(std::string const& TextData, std::string const& DocumentIdentifier)
	{
		MBError ReturnValue(true);
		std::vector<TokenClass> DocumentTokens;
		{
			DocumentTokens = TokenClass::TokenizeString(TextData);
		}
		m_DocumentIDs.push_back(DocumentIdentifier);
		ReturnValue = UpdatePostings(DocumentTokens, m_DocumentIDs.size() - 1);
		return(ReturnValue);
	}
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
		std::vector<TokenClass> DocumentTokens;
		std::string DocumentData(NumberOfBytes, 0);
		FileToRead.read(&DocumentData[0], NumberOfBytes);
		IndextTextData(DocumentData, DocumentName);
		return(ReturnValue);
	}
	MBError MBIndex::IndexHTMLData(std::string const& DocumentData, std::string const& DocumentIdentifier)
	{
		MBError ReturnValue(true);
		HTMLNode HTMLDocument(DocumentData,0);
		//MBError ReturnValue = HTMLDocument.GetParseError();
		//if (!ReturnValue)
		//{
		//	std::cout << "Fel i parsingen" << std::endl;
		//}
		if (ReturnValue)
		{
			std::string TextData = HTMLDocument.GetVisableText();
			IndextTextData(TextData, DocumentIdentifier);
		}
		return(ReturnValue);
	}
	void MBIndex::PrintIndex()
	{
		std::cout << "Indexed Documents: ";
		for (size_t i = 0; i < m_DocumentIDs.size(); i++)
		{
			std::cout << "DocID: " << i << " DocumentPath " << m_DocumentIDs[i] << std::endl;
		}
		for (auto DictionaryEntries : m_TokenDictionary)
		{
			std::cout << DictionaryEntries.TokenData << ":" << std::endl;
			//PostingsList const& TokenPostings = m_PostingsLists[DictionaryEntries.PostingIndex];
			//for (size_t i = 0; i < TokenPostings.size(); i++)
			//{
			//	TokenPostings[i].Print();
			//}
		}
	}
	std::string MBIndex::GetDocumentIdentifier(DocID DocumentID)
	{
		return(m_DocumentIDs[DocumentID]);
	}
	float MBIndex::GetInverseDocumentFrequency(PostingListID ID)
	{
		size_t TotalNumberOfDocuments = m_DocumentIDs.size();
		std::shared_ptr<const PostingsList> CurrentList = m_PostinglistHandler.GetPostinglist(ID);
		float Result = std::log(double(TotalNumberOfDocuments) / double(CurrentList->GetDocumentFrequency()));
		return(Result);
	}
	//END MBIndex
	void MBIndex::Clear()
	{
		*this = MBIndex();
	}
}