#pragma once
#include <vector>
#include <string>
#include <iostream>
namespace MBSearchEngine
{
	inline int SlashesBeforeCharacter(std::string const& StringToParse, size_t CharacterOffset)
	{
		int ReturnValue = 0;
		if (CharacterOffset == 0)
		{
			return(0);
		}
		int ParseOffset = CharacterOffset - 1;
		while (ParseOffset > 0)
		{
			if (StringToParse[ParseOffset] == '/')
			{
				ReturnValue += 1;
				ParseOffset -= 1;
				continue;
			}
			break;
		}
		return(ReturnValue);
	}
	//deprecated, borde inte användas
	//inline std::vector<std::string> TokenizeQuerry(std::string QuerryToParse)
	//{
	//	std::vector<std::string> ReturnValue = {};
	//	size_t ParseOffset = 0;
	//	ParseOffset = QuerryToParse.find_first_not_of(' ');
	//	while (ParseOffset != QuerryToParse.npos && ParseOffset < QuerryToParse.size() && m_ParseStatus)
	//	{
	//		//space är en delimiter, men vi vill kunna skriva strings på vanligt sätt med "" som phrase och \ som escape character
	//		char CurrentCharacter = QuerryToParse[ParseOffset];
	//		if (CurrentCharacter != '\"')
	//		{
	//			size_t NextSpace = QuerryToParse.find(' ', ParseOffset);
	//			ReturnValue.push_back(QuerryToParse.substr(ParseOffset, NextSpace - ParseOffset));
	//			ParseOffset = NextSpace + 1;
	//		}
	//		else
	//		{
	//			//ska tolka detta som en string 
	//			size_t NextQuote = QuerryToParse.find('\"', ParseOffset + 1);
	//			while (true)
	//			{
	//				if (NextQuote == QuerryToParse.npos)
	//				{
	//					m_ParseStatus = false;
	//					m_ParseStatus.ErrorMessage = "Error parsing querry: Quote at position " + std::to_string(ParseOffset) + " is never closed";
	//					break;
	//				}
	//				int NumberOfSlashesBefore = SlashesBeforeCharacter(QuerryToParse, NextQuote);
	//				if ((NumberOfSlashesBefore & 1) != 0)
	//				{
	//					NextQuote = QuerryToParse.find('\"', NextQuote + 1);
	//				}
	//				else
	//				{
	//					break;
	//				}
	//			}
	//			//vi vill till skilnad från mellanslag ha med quoten i tokenen
	//			ReturnValue.push_back(QuerryToParse.substr(ParseOffset, NextQuote - ParseOffset + 1));
	//			ParseOffset = QuerryToParse.find_first_not_of(' ', NextQuote + 1);
	//		}
	//	}
	//	return(ReturnValue);
	//}
	inline int GetNextStringBeginning(std::string const& StringToParse, size_t Offset)
	{
		size_t ReturnValue = -1;
		size_t CurrentParseOffset = Offset;
		while (CurrentParseOffset < StringToParse.size())
		{
			size_t NextQuote = StringToParse.find('\"', CurrentParseOffset);
			if (NextQuote == StringToParse.npos)
			{
				ReturnValue = -1;
				break;
			}
			int NumberOfSlashes = SlashesBeforeCharacter(StringToParse, NextQuote);
			if ((NumberOfSlashes & 1) == 0)
			{
				ReturnValue = NextQuote;
				break;
			}
			CurrentParseOffset = NextQuote;
		}
		return(ReturnValue);
	}
	inline int NumberOfCharactersInString(std::string const& StringToParse, std::string const& StringToFind, size_t Offset, size_t End)
	{
		int ReturnValue = 0;
		size_t CurrentOffset = Offset;
		while (CurrentOffset < End)
		{
			size_t NextTargetString = StringToParse.find(StringToFind, CurrentOffset);
			if (NextTargetString == StringToParse.npos)
			{
				break;
			}
			if (NextTargetString + StringToFind.size() - 1 < End)
			{
				ReturnValue += 1;
			}
			CurrentOffset = NextTargetString + StringToFind.size();
		}
		return(ReturnValue);
	}
	inline size_t GetNextStringEnd(std::string const& StringToParse, size_t Offset)
	{
		size_t NextStringBeginning = GetNextStringBeginning(StringToParse, Offset);
		size_t ReturnValue = GetNextStringBeginning(StringToParse, NextStringBeginning + 1);
		return(ReturnValue);
	}
	inline int GetCharacterParenthesisDepth(std::string const& StringToParse, size_t CharacterOffset)
	{
		int CurrentDepth = 0;
		size_t CurrentParseOffset = 0;
		while (CurrentParseOffset < CharacterOffset)
		{
			size_t NextStringBeginning = GetNextStringBeginning(StringToParse, CurrentParseOffset);
			size_t FirstEnd = std::min(NextStringBeginning, CharacterOffset);
			CurrentDepth += NumberOfCharactersInString(StringToParse, "(", CurrentParseOffset, FirstEnd);
			CurrentDepth -= NumberOfCharactersInString(StringToParse, ")", CurrentParseOffset, FirstEnd);
			//either before the character offset or larger, meaning we break
			CurrentParseOffset = GetNextStringEnd(StringToParse, CurrentParseOffset);
		}
		return(CurrentDepth);
	}
	inline std::vector<size_t> GetLogicalOperatorPositions(std::string const& QuerryToParse)
	{
		std::vector<size_t> ReturnValue = {};
		size_t CurrentParseOffset = 0;
		while (CurrentParseOffset < QuerryToParse.size())
		{
			size_t NextStringBeginning = GetNextStringBeginning(QuerryToParse, CurrentParseOffset);
			size_t NextAND = QuerryToParse.find(" AND ", CurrentParseOffset);
			size_t NextOR = QuerryToParse.find(" OR ", CurrentParseOffset);
			size_t NextOperator = std::min(NextAND, NextOR);
			bool WasAnd = true;
			if (NextOperator < NextAND)
			{
				WasAnd = false;
			}
			if (NextOperator < NextStringBeginning && NextOperator < QuerryToParse.size())
			{
				ReturnValue.push_back(NextOperator);
			}
			if (NextOperator == QuerryToParse.npos)
			{
				break;
			}
			if (WasAnd)
			{
				CurrentParseOffset = NextOperator + 5;
			}
			else
			{
				CurrentParseOffset = NextOperator + 4;
			}
		}
		return(ReturnValue);
	}
	inline std::string GetLogicOperator(std::string const& QuerryToParse, size_t OperatorOffset)
	{
		if (QuerryToParse.substr(OperatorOffset, 4) == " OR ")
		{
			return("OR");
		}
		else if (QuerryToParse.substr(OperatorOffset, 5) == " AND ")
		{
			return("AND");
		}
		else
		{
			return("");
		}
	}
	inline bool ParenthesisAreValid(std::string const& StringToParse)
	{
		int ParenthesisDepth = 0;
		for (size_t i = 0; i < StringToParse.size(); i++)
		{
			if (StringToParse[i] == '(')
			{
				ParenthesisDepth += 1;
			}
			if (StringToParse[i] == ')')
			{
				ParenthesisDepth -= 1;
			}
			if (ParenthesisDepth < 0)
			{
				return(false);
			}
		}
		if (ParenthesisDepth != 0)
		{
			return(false);
		}
		else
		{
			return(true);
		}
	}
	inline std::string RemoveRedundatParenthesis(std::string const& StringToNormalize)
	{
		std::string ReturnValue = StringToNormalize;
		while (true)
		{
			if (StringToNormalize.size() < 2)
			{
				break;
			}
			if (ReturnValue[0] == '(' && ReturnValue.back() == ')')
			{
				std::string TrimmedString = ReturnValue.substr(1, ReturnValue.size() - 2);
				if (ParenthesisAreValid(TrimmedString))
				{
					ReturnValue = TrimmedString;
					continue;
				}
			}
			break;
		}
		return(ReturnValue);
	}
}