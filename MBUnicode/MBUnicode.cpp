#include "MBUnicode.h"
#include <string>
#include <MBUtility/MBStrings.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <codecvt>
#include <filesystem>

#include "CodepointProperties.h"

namespace MBUnicode
{
	//unicdocdeCodePoint
	bool IsPrintableAscii(char AsciiCharacter)
	{
		return(AsciiCharacter > 31 && AsciiCharacter != 127);
	}
	Codepoint CodepointFromString (std::string const& Data, int Offset,int* OutNumberOfBytesRead)
	{
		Codepoint ReturnValue = 0;
		int BytesRead = 0;
		if ((Data[Offset] & (1 << 7)) == 0)
		{
			BytesRead += 1;
			ReturnValue = Data[Offset];
		}
		else
		{
			int NumberOfBytes = 2;
			for (size_t i = 3; i < 7; i++)
			{
				uint8_t BitValue = (uint8_t(Data[Offset]) >> (8 - i)) & 1;
				if (BitValue == 0)
				{
					break;
				}
				else
				{
					NumberOfBytes += 1;
				}
			}
			for (size_t i = 0; i < NumberOfBytes; i++)
			{
				BytesRead += 1;
				uint32_t ValueToAdd = 0;
				if (i == 0)
				{
					ValueToAdd = uint32_t(Data[Offset+i])%(1<<(8-NumberOfBytes));
				}
				if (i > 0)
				{
					unsigned char DataHeader = uint8_t(Data[Offset + i]) >> 6;
					if (DataHeader != 2)
					{
						ReturnValue = -1;
						break;
					}
					ValueToAdd = uint8_t(Data[Offset+i]) & uint8_t(~(1 << 7));
				}
				ReturnValue += (ValueToAdd << ((NumberOfBytes - i - 1) * 6));
			}
		}
		*OutNumberOfBytesRead = BytesRead;
		return(ReturnValue);
	}
	//UnicodeString
	UnicodeString::UnicodeString(std::string const& StringToConvert)
	{
		int Offset = 0;
		while (Offset!=StringToConvert.size())
		{
			int NewBytesRead = 0;
			Codepoint NewCodepoint = CodepointFromString(StringToConvert, Offset, &NewBytesRead);
			if (NewCodepoint == -1)
			{
				//invalid formatering
				break;
			}
			Offset += NewBytesRead;
			m_Codepoints.push_back(NewCodepoint);
		}
	}
	std::string UnicodeString::GetHexRepresentation()
	{
		std::string ReturnValue = "";
		for (size_t i = 0; i < m_Codepoints.size(); i++)
		{
			ReturnValue += MBUtility::HexEncodeInt(m_Codepoints[i]) + " ";
		}
		return(ReturnValue);
	}
	std::string UnicodeStringToLower(std::string const& StringToParse)
	{
		std::string ReturnValue = "";
		for (size_t i = 0; i < StringToParse.size(); i++)
		{
			ReturnValue += std::tolower(StringToParse[i]);
		}
		return(ReturnValue);
	}
	//std::string Convert_U16_U8(void* Data, size_t DataLength)
	//{
	//	std::u16string source = std::u16string((char16_t*)Data,DataLength);
	//	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	//	std::string dest = convert.to_bytes(source);
	//	return(dest);
	//}
	//std::string Convert_U16_U8(const char16_t* StringToConvert)
	//{
	//	std::u16string source = std::u16string(StringToConvert);
	//	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	//	std::string dest = convert.to_bytes(source);
	//	return(dest);
	//}
	std::string PathToUTF8(std::filesystem::path const& PathToProcess)
	{
		std::string ReturnValue = "";
#if defined(_WIN32)
		std::u16string source = std::u16string((char16_t*)PathToProcess.c_str());
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		ReturnValue = convert.to_bytes(source);
#else
		ReturnValue = PathToProcess.u8string();
#endif // __WINDOWS__

		return(ReturnValue);
	}

	//BEGIN UnicodeCodepointSegmenter::
	uint8_t UnicodeCodepointSegmenter::p_GetNeededExtraBytes(uint8_t ByteToCheck)
	{
		if (ByteToCheck & (1 << 7) == 0)
		{
			return(0);
		}
		if (ByteToCheck >= 0b11111000)
		{
			m_LastError = "To many followup bytes in utf-8 codepoint header";
			return(-1);
		}
		else if (ByteToCheck >= 11110000)
		{
			return(3);
		}
		else if (ByteToCheck >= 11100000)
		{
			return(2);
		}
		else if (ByteToCheck >= 11000000)
		{
			return(1);
		}
	}
	void UnicodeCodepointSegmenter::InsertData(const void* DataToInsert, size_t DataSize)
	{
		if (!IsValid())
		{
			return;
		}
		uint8_t const* ByteData = (uint8_t const*)DataToInsert;
		for (size_t i = 0; i < DataSize; i++)
		{
			if (m_CurrentCodepointNeededBytes == 0)
			{
				uint8_t ContinuationBytes = p_GetNeededExtraBytes(ByteData[i]);
				if (ContinuationBytes == -1)
				{
					return;
				}
				if (ContinuationBytes == 0)
				{
					m_DecodedCodepoints.push_back(ByteData[i]);
					m_CurrentCodepoint = 0;
					m_CurrentCodepointNeededBytes = 0;
				}
				else
				{
					m_CurrentCodepoint = ByteData[i] & (0xff >> (ContinuationBytes + 2));
					m_CurrentCodepointNeededBytes = ContinuationBytes;
				}
			}
			else
			{
				if (ByteData[i] >= 0b11000000)
				{
					m_LastError = "Invalid utf-8 continuation byte";
					return;
				}
				m_CurrentCodepoint <<= 6;
				m_CurrentCodepoint += ByteData[i] & (0xff >> 2);
				m_CurrentCodepointNeededBytes -= 1;
				if (m_CurrentCodepointNeededBytes == 0)
				{
					m_DecodedCodepoints.push_back(m_CurrentCodepoint);
				}
			}
		}
	}
	size_t UnicodeCodepointSegmenter::AvailableCodepoints()
	{
		return(m_DecodedCodepoints.size());
	}
	Codepoint UnicodeCodepointSegmenter::ExtractCodepoint()
	{
		Codepoint ReturnValue = 0;
		if (m_DecodedCodepoints.size() > 0)
		{
			ReturnValue = m_DecodedCodepoints.front();
			m_DecodedCodepoints.pop_front();
		}
		return(ReturnValue);
	}	
	bool UnicodeCodepointSegmenter::IsValid()
	{
		return(m_LastError == "");
	}
	std::string UnicodeCodepointSegmenter::GetLastError()
	{
		return(m_LastError);
	}
	//END UnicodeCodepointSegmenter::

	//helper function
	bool CompareCodepointRange(GraphemeBreakPropertySpecifier const& LeftElement, GraphemeBreakPropertySpecifier const& RightElement)
	{
		return(LeftElement.Lower < RightElement.Lower);
	}
	GraphemeBreakProperty h_StringToProperty(std::string const& StringToConvert)
	{
		GraphemeBreakProperty ReturnValue = GraphemeBreakProperty::Null;
		if (StringToConvert == "Control")
		{
			ReturnValue = GraphemeBreakProperty::Control;
		}
		else if (StringToConvert == "L")
		{
			ReturnValue = GraphemeBreakProperty::L;
		}
		else if (StringToConvert == "V")
		{
			ReturnValue = GraphemeBreakProperty::V;
		}
		else if (StringToConvert == "LV")
		{
			ReturnValue = GraphemeBreakProperty::LV;
		}
		else if (StringToConvert == "LVT")
		{
			ReturnValue = GraphemeBreakProperty::LVT;
		}
		else if (StringToConvert == "CR")
		{
			ReturnValue = GraphemeBreakProperty::CR;
		}
		else if (StringToConvert == "LF")
		{
			ReturnValue = GraphemeBreakProperty::LF;
		}
		else if (StringToConvert == "Extend")
		{
			ReturnValue = GraphemeBreakProperty::Extend;
		}
		else if (StringToConvert == "ZWJ")
		{
			ReturnValue = GraphemeBreakProperty::ZWJ;
		}
		else if (StringToConvert == "EndOfFile")
		{
			ReturnValue = GraphemeBreakProperty::EndOfFile;
		}
		else if (StringToConvert == "SpacingMark")
		{
			ReturnValue = GraphemeBreakProperty::SpacingMark;
		}
		else if (StringToConvert == "Prepend")
		{
			ReturnValue = GraphemeBreakProperty::Prepend;
		}
		else if (StringToConvert == "Extended_Pictographic")
		{
			ReturnValue = GraphemeBreakProperty::Extended_Pictographic;
		}
		else if (StringToConvert == "RI")
		{
			ReturnValue = GraphemeBreakProperty::RI;
		}
		return(ReturnValue);
	}
	std::string h_PropertyToString(GraphemeBreakProperty PropertyToConvert)
	{
		std::string ReturnValue = "";
		if (PropertyToConvert == GraphemeBreakProperty::Control)
		{
			ReturnValue ="Control";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::L)
		{
			ReturnValue = "L";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::V)
		{
			ReturnValue = "V";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::LV)
		{
			ReturnValue = "LV";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::LVT)
		{
			ReturnValue = "LVT";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::CR)
		{
			ReturnValue = "CR";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::LF)
		{
			ReturnValue = "LF";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::Extend)
		{
			ReturnValue = "Extend";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::ZWJ)
		{
			ReturnValue = "ZWJ";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::EndOfFile)
		{
			ReturnValue = "EndOfFile";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::SpacingMark)
		{
			ReturnValue = "SpacingMark";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::Prepend)
		{
			ReturnValue = "Prepend";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::Extended_Pictographic)
		{
			ReturnValue = "Extended_Pictographic";
		}
		else if (PropertyToConvert == GraphemeBreakProperty::RI)
		{
			ReturnValue = "RI";
		}
		return(ReturnValue);
	}
	std::string CreateGraphemeBreakSpecifiersArray(std::string const& InputFilename)
	{
		std::fstream InputFile(InputFilename);
		std::string ReturnValue = "";
		if (!InputFile.is_open())
		{
			std::cout << "input file is not open" << std::endl;
			return("");
		}
		std::string CurrentLine = "static GraphemeBreakPropertySpecifier GraphemeBreakSpecifiers[] = ";
		ReturnValue += "{\n";
		std::vector<GraphemeBreakPropertySpecifier> CodePointRanges = {};
		std::vector<std::string> PropertyNames = {};
		while (std::getline(InputFile, CurrentLine))
		{
			if (CurrentLine[0] == '#' || CurrentLine[0] == '\n' || CurrentLine[0] == '\r' || CurrentLine == "")
			{
				continue;
			}
			std::string UnicodeRangeData = CurrentLine.substr(0, CurrentLine.find(" "));
			std::vector<std::string> Ranges = MBUtility::Split(UnicodeRangeData, "..");
			//std::string NewLine = "{0x" + Ranges[0] + ",";
			GraphemeBreakPropertySpecifier NewSpecifier;
			NewSpecifier.Lower = std::stoi(Ranges[0], nullptr, 16);
			if (Ranges.size() == 2)
			{
				if (Ranges[1] != "")
				{
					NewSpecifier.Higher = std::stoi(Ranges[1], nullptr, 16);
					//NewLine += "0x" + Ranges[1] + "},\\";
				}
				else
				{
					NewSpecifier.Higher = NewSpecifier.Lower;
					//NewLine += "0x" + Ranges[0] + "},\\";
				}
			}
			else
			{
				NewSpecifier.Higher = NewSpecifier.Lower;
				//NewLine += "0x" + Ranges[0] + "},\\";
			}	


			//
			size_t PropertyNameBegin = CurrentLine.find("; ") + 2;
			size_t PropertyNameEnd = CurrentLine.find(" #", PropertyNameBegin);
			//PropertyNames.push_back(CurrentLine.substr(PropertyNameBegin, PropertyNameEnd - PropertyNameBegin));
			NewSpecifier.Type = h_StringToProperty(CurrentLine.substr(PropertyNameBegin, PropertyNameEnd - PropertyNameBegin));


			CodePointRanges.push_back(NewSpecifier);



			//OutputFile << NewLine<<"\n";
		}
		std::sort(CodePointRanges.begin(), CodePointRanges.end(), CompareCodepointRange);
		for (size_t i = 0; i < CodePointRanges.size(); i++)
		{
			ReturnValue += "{0x" + MBUtility::HexEncodeInt(CodePointRanges[i].Lower) + ",0x" + MBUtility::HexEncodeInt(CodePointRanges[i].Higher) +
				"," + "GraphemeBreakProperty::" + h_PropertyToString(CodePointRanges[i].Type);
			ReturnValue += "},\n";
		}
		ReturnValue += "}\n";
		ReturnValue += "constexpr size_t GraphemeBreakSpecifiersCount = sizeof(GraphemeBreakSpecifiers) / sizeof(MBUnicode::GraphemeBreakPropertySpecifier);\n";
		return(ReturnValue);
	}
	void CreateCodepointPropertiesHeader(std::string const& DataDirectory)
	{
		std::string DataToWrite = "#include \"MBUnicodeDefinitions.h\"\nnamespace MBUnicode\n{";
		DataToWrite += CreateGraphemeBreakSpecifiersArray(DataDirectory+"/GraphemeBreakProperty.txt");
		DataToWrite += "\n};";
		std::ofstream OutFile = std::ofstream(DataDirectory+"/CodepointProperties.h");
	}
	//implementeras inte rätt för flaggor
	bool h_ShouldBreak(GraphemeBreakProperty LeftProperty, GraphemeBreakProperty RightProperty)
	{
		bool ReturnValue = true;
		if (LeftProperty == GraphemeBreakProperty::CR && RightProperty == GraphemeBreakProperty::LF)
		{
			ReturnValue = false;
		}
		else if (RightProperty == GraphemeBreakProperty::Extend || RightProperty == GraphemeBreakProperty::ZWJ)
		{
			ReturnValue = false;
		}
		else if (RightProperty == GraphemeBreakProperty::SpacingMark)
		{
			ReturnValue = false;
		}
		else if (LeftProperty == GraphemeBreakProperty::Prepend)
		{
			ReturnValue = false;
		}
		//TODO implementera hela extended grapheme break grejen med emojis och flaggor
		return(ReturnValue);
	}
	//grapheme grejer
	GraphemeBreakProperty h_GetCodepointProperty(Codepoint CodepointToExamine)
	{
		GraphemeBreakProperty ReturnValue = GraphemeBreakProperty::Null;
		size_t LowerLimit = 0;
		size_t HigherLimit = GraphemeBreakSpecifiersCount;
		while (LowerLimit != HigherLimit)
		{
			size_t CurrentGuess = LowerLimit + HigherLimit;
			if (GraphemeBreakSpecifiers[CurrentGuess].Lower >= CodepointToExamine && GraphemeBreakSpecifiers[CurrentGuess].Higher <= CodepointToExamine)
			{
				ReturnValue = GraphemeBreakSpecifiers[CurrentGuess].Type;
			}
			else if (GraphemeBreakSpecifiers[CurrentGuess].Higher < CodepointToExamine)
			{
				LowerLimit = CurrentGuess;
			}
			else if (GraphemeBreakSpecifiers[CurrentGuess].Lower > CodepointToExamine)
			{
				HigherLimit = CurrentGuess;
			}
		}
		return(ReturnValue);
	}
	//BEGIN UnicodeGraphemeClusterSegmenter
	void GraphemeClusterSegmenter::InsertCodepoints(const Codepoint* CodepointsToInsert, size_t NumberOfCodepoints)
	{
		for (size_t i = 0; i < NumberOfCodepoints; i++)
		{
			GraphemeBreakProperty NewProperty = h_GetCodepointProperty(CodepointsToInsert[i]);
			bool ShouldBreak = h_ShouldBreak(m_LastProperty, NewProperty);
			if (ShouldBreak && m_CurrentCluster != GraphemeCluster())
			{
				m_DecodedCluster.push_back(std::move(m_CurrentCluster));
				m_CurrentCluster = GraphemeCluster();
			}
			else
			{
				m_CurrentCluster.AddCodepoint(CodepointsToInsert[i]);
				m_LastProperty = NewProperty;
			}
		}
	}
	void GraphemeClusterSegmenter::Finalize()
	{
		if (m_CurrentCluster != GraphemeCluster())
		{
			m_DecodedCluster.push_back(m_CurrentCluster);
		}
	}
	size_t GraphemeClusterSegmenter::AvailableClusters()
	{
		return(m_DecodedCluster.size());
	}
	GraphemeCluster GraphemeClusterSegmenter::ExtractCluster()
	{
		GraphemeCluster ReturnValue;
		if(m_DecodedCluster.size() > 0)
		{
			ReturnValue = m_DecodedCluster.front();
			m_DecodedCluster.pop_front();
		}
		return(ReturnValue);
	}
	//END UnicodeGraphemeClusterSegmenter
}