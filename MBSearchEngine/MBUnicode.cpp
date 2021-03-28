#include <MBSearchEngine/MBUnicode.h>
#include <MBSearchEngine/MBUnicodeMacros.h>
#include <string>
#include <MinaStringOperations.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
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
			ReturnValue += HexEncodeInt(m_Codepoints[i]) + " ";
		}
		return(ReturnValue);
	}
	//helper function
	bool CompareCodepointRange(CodepointRange const& LeftElement, CodepointRange const& RightElement)
	{
		return(LeftElement.Lower < RightElement.Lower);
	}
	int CreateCMacroCodepointArrayFromPropertySpec(std::string const& InputFilename, std::string const& OutputFilename)
	{
		std::fstream InputFile(InputFilename);
		std::ofstream OutputFile(OutputFilename);
		if (!InputFile.is_open())
		{
			std::cout << "input file is not open" << std::endl;
			return(-1);
		}
		if (!OutputFile.is_open())
		{
			std::cout << "output file is not open" << std::endl;
			return(-1);
		}
		std::string CurrentLine = "";
		OutputFile << "{\\\n";
		std::vector<CodepointRange> CodePointRanges = {};
		while (std::getline(InputFile, CurrentLine))
		{
			if (CurrentLine[0] == '#' || CurrentLine[0] == '\n' || CurrentLine[0] == '\r' || CurrentLine == "")
			{
				continue;
			}
			std::string UnicodeRangeData = CurrentLine.substr(0, CurrentLine.find(" "));
			std::vector<std::string> Ranges = Split(UnicodeRangeData, "..");
			//std::string NewLine = "{0x" + Ranges[0] + ",";
			CodepointRange NewCodepointRange;
			NewCodepointRange.Lower = std::stoi(Ranges[0], nullptr, 16);
			if (Ranges.size() == 2)
			{
				if (Ranges[1] != "")
				{
					NewCodepointRange.Higher = std::stoi(Ranges[1], nullptr, 16);
					//NewLine += "0x" + Ranges[1] + "},\\";
				}
				else
				{
					NewCodepointRange.Higher = NewCodepointRange.Lower;
					//NewLine += "0x" + Ranges[0] + "},\\";
				}
			}
			else
			{
				NewCodepointRange.Higher = NewCodepointRange.Lower;
				//NewLine += "0x" + Ranges[0] + "},\\";
			}
			CodePointRanges.push_back(NewCodepointRange);
			//OutputFile << NewLine<<"\n";
		}
		std::sort(CodePointRanges.begin(), CodePointRanges.end(), CompareCodepointRange);
		for (size_t i = 0; i < CodePointRanges.size(); i++)
		{
			OutputFile << "{0x" + HexEncodeInt(CodePointRanges[i].Lower) + ",0x" + HexEncodeInt(CodePointRanges[i].Higher) + "},\\\n";
		}
		OutputFile << "}";
		return(0);
	}
}