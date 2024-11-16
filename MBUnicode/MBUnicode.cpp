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
#include <assert.h>

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
	std::string ToUTF8String(Codepoint CodepointToConvert)
	{
		uint8_t ExtraPoints = 0;
		if (CodepointToConvert < 0x80)
		{
			std::string ReturnValue = "";
			ReturnValue += char(CodepointToConvert);
			return(ReturnValue);
		}
		else if (CodepointToConvert >= 0x80 && CodepointToConvert <= 0x07ff)
		{
			ExtraPoints = 1;
		}
		else if (CodepointToConvert >= 0x800 && CodepointToConvert <= 0xffff)
		{
			ExtraPoints = 2;
		}
		else if (CodepointToConvert >= 0x1000 && CodepointToConvert <= 0x10ffff)
		{
			ExtraPoints = 3;
		}
		else
		{
			assert(false);
		}
		std::string ReturnValue = std::string(ExtraPoints + 1, 0);
		ReturnValue[0] = uint8_t(~0) << (7 - ExtraPoints);
		ReturnValue[0] += uint8_t(CodepointToConvert >> (6 * ExtraPoints)) & uint8_t((~0) >> (ExtraPoints + 2));
		uint8_t ExtractedBits = 8-(ExtraPoints + 2);
		uint8_t TotalBits = ExtractedBits + 6 * ExtraPoints;
		for (size_t i = 0; i < ExtraPoints; i++)
		{
			ReturnValue[1 + i] = 0b10000000;
			ReturnValue[1 + i] += uint8_t(CodepointToConvert >> (TotalBits - ExtractedBits-6))&uint8_t(uint8_t(~0)>>2);
			ExtractedBits += 6;
		}
		return(ReturnValue);
	}
	//BEGIN UnicodeCodepointSegmenter::
	uint8_t UnicodeCodepointSegmenter::p_GetNeededExtraBytes(uint8_t ByteToCheck)
	{
        uint8_t ReturnValue = 0;
		if ((ByteToCheck & (1 << 7)) == 0)
		{
			return 0;
		}
		if (ByteToCheck >= 0b11111000)
		{
			return -1;
		}
		else if (ByteToCheck >= 0b11110000)
		{
			return 3;
		}
		else if (ByteToCheck >= 0b11100000)
		{
			return 2;
		}
		else if (ByteToCheck >= 0b11000000)
		{
			return 1;
		}
        return ReturnValue;
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
    const unsigned char* UnicodeCodepointSegmenter::ParseUTF8Codepoint(const unsigned char* Begin, const unsigned char* End,Codepoint& OutValue)
    {
        static_assert(sizeof(char) == 1, "ParseUTF8 codepoint requires POSIX assumption that sizeof(char) == 1");
        const unsigned char* CurrentElement = Begin;
        size_t NeededExtraBytes = 0;
        while(CurrentElement < reinterpret_cast<const unsigned char*>(End))
        {
			if (NeededExtraBytes == 0)
			{
				uint8_t ContinuationBytes = p_GetNeededExtraBytes(*CurrentElement);
				if (ContinuationBytes == 0)
				{
					OutValue = *CurrentElement;
                    return CurrentElement+1;
				}
				else
				{
					OutValue = *CurrentElement & (0xff >> (ContinuationBytes + 2));
					NeededExtraBytes = ContinuationBytes;
				}
			}
			else
			{
				if (*CurrentElement >= 0b11000000)
				{
					throw std::runtime_error("Invalid UTF-8 continuation byte");
				}
				OutValue <<= 6;
				OutValue += *CurrentElement & (0xff >> 2);
				NeededExtraBytes -= 1;
				if (NeededExtraBytes == 0)
				{
                    return CurrentElement+1;
				}
			}
            ++CurrentElement;
        }
        return CurrentElement;
    }
	bool UnicodeCodepointSegmenter::IsValid()
	{
		return(m_LastError == "");
	}
	std::string UnicodeCodepointSegmenter::GetLastError()
	{
		return(m_LastError);
	}
	void UnicodeCodepointSegmenter::Reset()
	{
		*this = UnicodeCodepointSegmenter();
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
		else if (StringToConvert == "Regional_Indicator")
		{
			ReturnValue = GraphemeBreakProperty::RI;
		}
		else if (StringToConvert == "T")
		{
			ReturnValue = GraphemeBreakProperty::T;
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
		else if (PropertyToConvert == GraphemeBreakProperty::T)
		{
			ReturnValue = "T";
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
		ReturnValue = "static GraphemeBreakPropertySpecifier GraphemeBreakSpecifiers[] = ";
		ReturnValue += "{\n";
		std::string CurrentLine;
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
		ReturnValue += "};\n";
		ReturnValue += "constexpr size_t GraphemeBreakSpecifiersCount = sizeof(GraphemeBreakSpecifiers) / sizeof(MBUnicode::GraphemeBreakPropertySpecifier);\n";
		return(ReturnValue);
	}
	void CreateCodepointPropertiesHeader(std::string const& DataDirectory)
	{
		std::string DataToWrite = "#include \"MBUnicodeDefinitions.h\"\nnamespace MBUnicode\n{";
		DataToWrite += CreateGraphemeBreakSpecifiersArray(DataDirectory+"/GraphemeBreakProperty.txt");
		DataToWrite += "\n};";
		std::ofstream OutFile = std::ofstream(DataDirectory+"/CodepointProperties.h");
		OutFile << DataToWrite;
	}
	//implementeras inte r�tt f�r flaggor
	bool h_ShouldBreak(GraphemeBreakProperty LeftProperty, GraphemeBreakProperty RightProperty)
	{
		bool ReturnValue = true;
		//OBS bryter mot grapheme standarden, men tycker personligen det är inte speciellt intuitivt
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
        auto It = std::lower_bound(GraphemeBreakSpecifiers,GraphemeBreakSpecifiers+GraphemeBreakSpecifiersCount,
                CodepointToExamine,
                [](GraphemeBreakPropertySpecifier Lhs,Codepoint Rhs)
                {
                    return Lhs.Higher < Rhs;
                });
		//size_t LowerLimit = 0;
		//size_t HigherLimit = GraphemeBreakSpecifiersCount;
		//size_t PreviousGuess = -1;
		//while (LowerLimit != HigherLimit)
		//{
		//	size_t CurrentGuess = (LowerLimit + HigherLimit)/2;
		//	if (PreviousGuess == CurrentGuess)
		//	{
		//		break;
		//	}
		//	if (GraphemeBreakSpecifiers[CurrentGuess].Lower >= CodepointToExamine && GraphemeBreakSpecifiers[CurrentGuess].Higher <= CodepointToExamine)
		//	{
		//		ReturnValue = GraphemeBreakSpecifiers[CurrentGuess].Type;
		//	}
		//	else if (GraphemeBreakSpecifiers[CurrentGuess].Higher < CodepointToExamine)
		//	{
		//		LowerLimit = CurrentGuess;
		//	}
		//	else if (GraphemeBreakSpecifiers[CurrentGuess].Lower > CodepointToExamine)
		//	{
		//		HigherLimit = CurrentGuess;
		//	}
		//	PreviousGuess = CurrentGuess;
		//}
        if(It < GraphemeBreakSpecifiers+GraphemeBreakSpecifiersCount && (It->Lower <= CodepointToExamine && It->Higher <= CodepointToExamine))
        {
            return It->Type;
        }
		return(ReturnValue);
	}

	//BEGIN GraphemeCluster
	bool GraphemeCluster::operator==(char CharToCompare) const
	{
		if (m_InternalBuffer.size() != 1)
		{
			return(false);
		}
		bool ReturnValue = false;
		ReturnValue = m_InternalBuffer[0] == CharToCompare;
		return(ReturnValue);
	}
	bool GraphemeCluster::operator!=(char CharToCompare) const
	{
		return(!(*this == CharToCompare));
	}
	bool GraphemeCluster::operator==(GraphemeCluster const& OtherCluster) const
	{
		return(m_InternalBuffer == OtherCluster.m_InternalBuffer);
	}
	bool GraphemeCluster::operator!=(GraphemeCluster const& OtherCluster) const
	{
		return(!(*this == OtherCluster));
	}
	bool GraphemeCluster::operator==(std::string const& StringToCompare) const
	{
        return(p_CompareUTF8String(StringToCompare.data(),StringToCompare.size()));
	}
    bool GraphemeCluster::operator==(std::string_view StringToCompare) const
    {
        return p_CompareUTF8String(&StringToCompare[0],StringToCompare.size());
    }
    bool GraphemeCluster::operator!=(std::string_view StringToCompare) const
    {
        return !(*this == StringToCompare);
    }
	bool GraphemeCluster::operator!=(std::string const& StringToCompare) const
	{
		return(!(*this == StringToCompare));
	}
    bool GraphemeCluster::operator==(const char* StringToCompare) const
    {
        size_t StringSize = std::strlen(StringToCompare); 
        return(p_CompareUTF8String(StringToCompare,StringSize));
    }
    bool GraphemeCluster::operator!=(const char* StringToCompare) const
    {
        return(!(*this == StringToCompare));    
    }
    bool GraphemeCluster::p_CompareUTF8String(const char* UTF8String,size_t Size) const
    {
        if(Size != m_InternalBuffer.size())
        {
            return(false);   
        }
        bool ReturnValue = true; 
        ReturnValue = std::memcmp(UTF8String,m_InternalBuffer.data(),m_InternalBuffer.size()) == 0;
        return(ReturnValue);
    }
    GraphemeCluster& GraphemeCluster::operator=(std::string const& StringToConvert)
	{
		bool Result = GraphemeCluster::ParseGraphemeCluster(*this, StringToConvert.data(), StringToConvert.size(), 0, nullptr);
		if (!Result)
		{
			*this = GraphemeCluster();
		}
		return(*this);
	}
	GraphemeCluster& GraphemeCluster::operator=(char CharToConvert)
	{
		m_InternalBuffer.push_back(CharToConvert);
		return(*this);
	}
    GraphemeCluster& GraphemeCluster::operator=(std::string_view String)
    {
        m_InternalBuffer.clear();
        //TODO inefficient
        for(size_t i = 0; i < String.size();i++)
        {
            m_InternalBuffer.push_back(String[i]);   
        }
        return *this;
    }
    GraphemeCluster& GraphemeCluster::operator=(const char* String)
    {
        (*this) = std::string_view(String,strlen(String));
        return *this;
    }
	GraphemeCluster::GraphemeCluster(std::string const& StringToConvert)
	{
		bool Result = ParseGraphemeCluster(*this, StringToConvert.data(), StringToConvert.size(), 0, nullptr);
		if (!Result)
		{
			*this = GraphemeCluster();
		}
	}
    bool GraphemeCluster::IsASCIIControl() const
    {
        return(m_InternalBuffer.size() > 0 && m_InternalBuffer[0] < 32);    
    }
    bool GraphemeCluster::IsEmpty() const
    {
        return(m_InternalBuffer.size() == 0);
    }
    bool GraphemeCluster::ParseGraphemeCluster(GraphemeCluster& OutCluster, const void* InputData, size_t InputDataSize, size_t InputDataOffset, size_t* OutOffset)
	{
		bool ReturnVale = true;
		UnicodeCodepointSegmenter CodepointSegmenter;
		GraphemeClusterSegmenter GraphemeSegmenter;
		size_t CurrentOffset = InputDataOffset;
		size_t LastCodepointPosition = InputDataOffset;
		//TODO innefektiv algorithm som läser alla code points byte för byte, men även det ända sättet att grantera att det blir byte exakt
		while (GraphemeSegmenter.AvailableClusters() == 0 && CurrentOffset < InputDataSize  && CodepointSegmenter.IsValid())
		{
			CodepointSegmenter.InsertData(((const uint8_t*)InputData) + CurrentOffset, 1);
			if (CodepointSegmenter.AvailableCodepoints() > 0)
			{
				Codepoint NewCodepoint = CodepointSegmenter.ExtractCodepoint();
				if (GraphemeSegmenter.CodepointWouldBreak(NewCodepoint))
				{
					GraphemeSegmenter.Finalize();
					CurrentOffset = LastCodepointPosition;
					break;
				}
				else
				{
					GraphemeSegmenter.InsertCodepoint(NewCodepoint);
				}
				LastCodepointPosition = CurrentOffset+1;
			}
			CurrentOffset += 1;
		}
		GraphemeSegmenter.Finalize();
		if (GraphemeSegmenter.AvailableClusters() == 0 || !CodepointSegmenter.IsValid())
		{
			ReturnVale = false;
		}
		else
		{
			OutCluster = GraphemeSegmenter.ExtractCluster();
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = CurrentOffset;
		}
		return(ReturnVale);
	}
	bool GraphemeCluster::ParseGraphemeClusters(std::vector<GraphemeCluster>& OutCluster, const void* InputData, size_t InputDataSize,size_t InputOffset)
	{
		bool ReturnValue = true;
		size_t CurrentParseOffset = InputOffset;
		OutCluster = std::vector<GraphemeCluster>();
		while (CurrentParseOffset < InputDataSize)
		{
			GraphemeCluster NewCluster;
			ReturnValue = ParseGraphemeCluster(NewCluster, InputData, InputDataSize, CurrentParseOffset, &CurrentParseOffset);
			if (!ReturnValue)
			{
				break;
			}
			OutCluster.push_back(std::move(NewCluster));
		}
		return(ReturnValue);
	}
    bool GraphemeCluster::ParseGraphemeClusters(std::vector<GraphemeCluster>& OutCluster, std::string const& InputString)
    {
        bool ReturnValue = ParseGraphemeClusters(OutCluster,InputString.data(),InputString.size(),0);
        return ReturnValue;
    }

	//END GraphemeCluster

	//BEGIN UnicodeGraphemeClusterSegmenter
	void GraphemeClusterSegmenter::InsertCodepoint(Codepoint CodepointsToInsert)
	{
		InsertCodepoints(&CodepointsToInsert, 1);
	}
	bool GraphemeClusterSegmenter::CodepointWouldBreak(Codepoint CodepointToTest) const
	{
		if (m_CurrentCluster.IsEmpty())
		{
			return(false);
		}
		GraphemeBreakProperty NewProperty = h_GetCodepointProperty(CodepointToTest);
		bool ShouldBreak = h_ShouldBreak(m_LastProperty, NewProperty);
		return(ShouldBreak);
	}
	void GraphemeClusterSegmenter::InsertCodepoints(const Codepoint* CodepointsToInsert, size_t NumberOfCodepoints)
	{
		for (size_t i = 0; i < NumberOfCodepoints; i++)
		{
			GraphemeBreakProperty NewProperty = h_GetCodepointProperty(CodepointsToInsert[i]);
			bool ShouldBreak = h_ShouldBreak(m_LastProperty, NewProperty);
			if (ShouldBreak && !m_CurrentCluster.IsEmpty())
			{
				m_DecodedCluster.push_back(std::move(m_CurrentCluster));
				m_CurrentCluster = GraphemeCluster();
				m_CurrentCluster.AddCodepoint(CodepointsToInsert[i]);
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
		if (!m_CurrentCluster.IsEmpty())
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
	void GraphemeClusterSegmenter::Reset()
	{
		*this = GraphemeClusterSegmenter();
	}
    const unsigned char* GraphemeClusterSegmenter::ParseGraphemeCluster(const unsigned char* Begin, const unsigned char* End)
    {
        const unsigned char* CurrentElement = Begin;
        GraphemeBreakProperty LastProperty = GraphemeBreakProperty::SOF;
        while(CurrentElement < End)
        {
            Codepoint NewCodepoint = 0;
            auto NextCodepoint = UnicodeCodepointSegmenter::ParseUTF8Codepoint(CurrentElement,End,NewCodepoint);
            auto CodepointProperty = h_GetCodepointProperty(NewCodepoint);
            if(h_ShouldBreak(LastProperty,CodepointProperty))
            {
                return NextCodepoint;
            }
            LastProperty = CodepointProperty;
            CurrentElement = NextCodepoint;
        }
        return CurrentElement;
    }
	//END UnicodeGraphemeClusterSegmenter
}

