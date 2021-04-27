#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <MBErrorHandling.h>
#include <MBStrings.h>
#include <algorithm>
#include <MBStrings.h>
#include <assert.h>
inline std::string StringToLower(std::string const& StringToLowercase)
{
	std::string ReturnValue = "";
	for (size_t i = 0; i < StringToLowercase.size(); i++)
	{
		ReturnValue += std::tolower(StringToLowercase[i]);
	}
	return(ReturnValue);
}
inline bool TagIsEmpty(std::string const& StringToCheck)
{
	if (StringToCheck == "area" || StringToCheck == "area/")
	{
		return(true);
	}
	if (StringToCheck == "base" || StringToCheck == "base/")
	{
		return(true);
	}
	if (StringToCheck == "br" || StringToCheck == "br/")
	{
		return(true);
	}
	if (StringToCheck == "col" || StringToCheck == "col/")
	{
		return(true);
	}
	if (StringToCheck == "embed" || StringToCheck == "base/")
	{
		return(true);
	}
	if (StringToCheck == "hr" || StringToCheck == "hr/")
	{
		return(true);
	}
	if (StringToCheck == "img" || StringToCheck == "img/")
	{
		return(true);
	}
	if (StringToCheck == "input" || StringToCheck == "input/")
	{
		return(true);
	}
	if (StringToCheck == "keygen" || StringToCheck == "keygen/")
	{
		return(true);
	}
	if (StringToCheck == "link" || StringToCheck == "link/")
	{
		return(true);
	}
	if (StringToCheck == "meta" || StringToCheck == "meta/")
	{
		return(true);
	}
	if (StringToCheck == "param" || StringToCheck == "param/")
	{
		return(true);
	}
	if (StringToCheck == "source" || StringToCheck == "source/")
	{
		return(true);
	}
	if (StringToCheck == "track" || StringToCheck == "track/")
	{
		return(true);
	}
	if (StringToCheck == "wbr" || StringToCheck == "wbr/")
	{
		return(true);
	}
	return(false);
}
class HTMLNode
{
private:
	std::vector<HTMLNode> m_Children = {};
	std::string m_RawText = "";
	std::unordered_map<std::string, std::string> m_Attributes = {};
	std::string m_NodeTag = "";
	HTMLNode* m_Parent = nullptr;
	size_t m_ElementEndOffset = 0;
	bool m_IsRawtext = false;
	MBError m_ParseError = MBError(true);
	size_t m_GetEndtagOffset()
	{
		return(m_ElementEndOffset);
	}
	size_t ExtractTagName(std::string const& HTMLData, size_t ParseOffset)
	{
		size_t NameEnd = std::min(HTMLData.find(' ', ParseOffset + 1), HTMLData.find('>', ParseOffset + 1));
		m_NodeTag = StringToLower(HTMLData.substr(ParseOffset + 1, NameEnd - (ParseOffset + 1)));
		return(NameEnd);
	}
	size_t ExtractTag(std::string const& HTMLData, size_t Offset)
	{
		size_t NextAttribute = HTMLData.find_first_not_of(' ', Offset);
		if (NextAttribute == HTMLData.npos)
		{
			return(NextAttribute);
		}
		if (HTMLData[NextAttribute] == '>')
		{
			return(NextAttribute);
		}
		//vi har faktiskt en tag
		size_t NextAttributeNameEnd = std::min(HTMLData.find(' ', NextAttribute), HTMLData.find('=', NextAttribute));
		NextAttributeNameEnd = std::min(HTMLData.find('>', NextAttribute), NextAttributeNameEnd);
		if (NextAttributeNameEnd == HTMLData.npos)
		{
			m_ParseError = false;
			m_ParseError.ErrorMessage = "No end in tag detected";
			return(-1);
		}
		std::string NextAttributeName = HTMLData.substr(NextAttribute, NextAttributeNameEnd - NextAttribute);
		size_t NextAttributeValueBegin = HTMLData.find_first_not_of(' ', NextAttributeNameEnd);
		if (NextAttributeValueBegin == HTMLData.npos)
		{
			m_ParseError = false;
			m_ParseError.ErrorMessage = "No end in tag detected";
			return(-1);
		}
		if (HTMLData[NextAttributeValueBegin] == '>')
		{
			//den var tom
			return(NextAttributeValueBegin);
		}
		if (HTMLData[NextAttributeValueBegin] != '=')
		{
			m_ParseError = false;
			m_ParseError.ErrorMessage = "Invalid attribute syntax";
			return(-1);
		}
		size_t NextAttributeValueDataBegin = HTMLData.find_first_not_of(' ', NextAttributeValueBegin+1);
		char AttributeEndDelimiter = ' ';
		if (HTMLData[NextAttributeValueDataBegin] == '\'')
		{
			AttributeEndDelimiter = '\'';
		}
		if (HTMLData[NextAttributeValueDataBegin] == '\"')
		{
			AttributeEndDelimiter = '\"';
		}
		//size_t NextAttributeValueEnd = std::min(HTMLData.find(AttributeEndDelimiter, NextAttributeValueDataBegin+1), HTMLData.find('>', NextAttributeValueDataBegin+1));
		size_t NextAttributeValueEnd = 0;
		if (AttributeEndDelimiter == ' ')
		{
			NextAttributeValueEnd = std::min(HTMLData.find(AttributeEndDelimiter, NextAttributeValueDataBegin + 1), HTMLData.find('>', NextAttributeValueDataBegin + 1));
		}
		else
		{
			NextAttributeValueEnd = HTMLData.find(AttributeEndDelimiter, NextAttributeValueDataBegin + 1);
		}
		if (NextAttributeValueEnd == HTMLData.npos)
		{
			m_ParseError = false;
			m_ParseError.ErrorMessage = "Invalid attribute syntax";
			return(-1);
		}
		std::string NextAttributeValue = HTMLData.substr(NextAttributeValueDataBegin+1, NextAttributeValueEnd - NextAttributeValueDataBegin-1);
		m_Attributes[NextAttributeName] = NextAttributeValue;
		
		size_t NewParseOffset = NextAttributeValueEnd;
		if (AttributeEndDelimiter != ' ')
		{
			NewParseOffset += 1;
		}
		return(NewParseOffset);
	}
	HTMLNode(std::string const& RawText)
	{
		//lite parsing av den råa texten så vi tar borta massa newlines och gör den lite snyggare
		std::string TextToStore = RawText;
		TextToStore = MBUtility::ReplaceAll(TextToStore, "\r\n", " ");
		TextToStore = MBUtility::ReplaceAll(TextToStore, "\n", " ");
		TextToStore = MBUtility::ReplaceAll(TextToStore, "\r", " ");
		TextToStore = MBUtility::RemoveDuplicates(TextToStore, " ");
		TextToStore = MBUtility::RemoveLeadingString(TextToStore, " ");
		m_RawText = TextToStore;
		assert(TextToStore != "");
		m_IsRawtext = true;
	}
public:
	MBError GetParseError()
	{
		return(m_ParseError);
	}
	std::string GetVisableText()
	{
		std::string ReturnValue = "";
		for (size_t i = 0; i < m_Children.size(); i++)
		{
			ReturnValue += m_Children[i].GetVisableText();
			if (ReturnValue.size() > 0)
			{
				if (ReturnValue.back() != '\n')
				{
					ReturnValue += "\n";
				}
			}
		}
		if (m_IsRawtext)
		{
			ReturnValue += m_RawText;
		}
		return(ReturnValue);
	}
	HTMLNode(std::string const& HTMLToParse,size_t ParseOffset)
	{
		int CurrentTagDepth = 0;
		bool IsTagElement = false;
		if (HTMLToParse.substr(ParseOffset, 3) == "<!D" || HTMLToParse.substr(ParseOffset, 3) == "<!d")
		{
			ParseOffset = HTMLToParse.find("<", ParseOffset + 1);
		}
		if (HTMLToParse[ParseOffset] == '<')
		{
			//vi börjar på en tag
			IsTagElement = true;
			//extractar namnet
			ParseOffset = ExtractTagName(HTMLToParse,ParseOffset);
			//settar upp tag datanm
			if (TagIsEmpty(m_NodeTag))
			{
				m_ElementEndOffset = HTMLToParse.find('>', ParseOffset)+1;
				return;
			}
			while (HTMLToParse[ParseOffset] != '>' && m_ParseError)
			{
				ParseOffset = ExtractTag(HTMLToParse,ParseOffset);
			}
			ParseOffset += 1;
			if (m_NodeTag[0] == '/')
			{
				//fuck robustness principen och allt den står för, 99% säker att en sida som renderas korrekt egentligen inneåller felaktig html, med extra endtags
				//för att testa om det går att parsa ändå så helt enkelt skippar vi element som "börjar" med en endtag
				m_NodeTag = "";
				m_ElementEndOffset = ParseOffset;
				return;
			}
			//om det är är en tom tag vill vi returna nu
			if (ParseOffset == 0)
			{
				std::cout << "Konstig" << std::endl;
			}

			//nu har vi kommit till den innre htmlen
			while (m_ParseError)
			{
				size_t NextElementBegin = HTMLToParse.find("<", ParseOffset);
				if (ParseOffset == 0)
				{
					std::cout << "Konstig" << std::endl;
				}
				if (NextElementBegin == HTMLToParse.npos)
				{
					//ENBART HÄR FÖR ATT STÖDJA FELAKTIG DATA
					//m_ParseError = false;
					//m_ParseError.ErrorMessage = "No tag end detected";
					m_ElementEndOffset = NextElementBegin;
					break;
				}
				if (HTMLToParse.substr(NextElementBegin, 2) == "<!")
				{
					ParseOffset = HTMLToParse.find('>', NextElementBegin) + 1;
					continue;
				}
				if (NextElementBegin != ParseOffset)
				{
					//kanske finns rå text mellan
					std::string RawText = HTMLToParse.substr(ParseOffset, NextElementBegin - ParseOffset);
					std::string NormalizedText = RawText;
					std::vector<std::string> FillerCharacters = { "\t","\n"," ","\r" };
					size_t NextInterestingCharacter = NextElementBegin;
					for (size_t i = 0; i < FillerCharacters.size(); i++)
					{
						NormalizedText = MBUtility::ReplaceAll(NormalizedText, FillerCharacters[i], "");
					}
					if (NormalizedText.size() != 0)
					{
						//std::string RawText = HTMLToParse.substr(ParseOffset, NextElementBegin - ParseOffset);
						m_Children.push_back(HTMLNode(RawText));
						m_Children.back().m_Parent = this;
					}
				}
				if (HTMLToParse.substr(NextElementBegin, 2 + m_NodeTag.size()) == "</" + m_NodeTag)
				{
					m_ElementEndOffset = HTMLToParse.find('>', NextElementBegin) + 1;
					break;
				}
  				m_Children.push_back(HTMLNode(HTMLToParse, NextElementBegin));
				ParseOffset = m_Children.back().m_GetEndtagOffset();
				m_ParseError = (m_Children.back().m_ParseError == true);
				m_Children.back().m_Parent = this;
				if (!m_ParseError)
				{
					m_ParseError.ErrorMessage = "Error in parsing child node: "+ m_Children.back().m_ParseError.ErrorMessage;
				}
				if (ParseOffset >= HTMLToParse.size())
				{
					//ENBART HÄR FÖR ATT STÖDJA FELAKTIG DATA
					m_ElementEndOffset = ParseOffset;
					break;
				}
			}
		}
		else
		{
			//lägger in all data tills vi kommer till en ny tag
			assert(false);
		}
	}
};