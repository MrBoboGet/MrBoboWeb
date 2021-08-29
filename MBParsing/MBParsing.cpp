#include "MBParsing.h"
#include <cstring>
#include <algorithm>
#include <assert.h>
namespace MBParsing
{
	unsigned char Base64CharToBinary(unsigned char CharToDecode)
	{
		if (CharToDecode >= 65 && CharToDecode <= 90)
		{
			return(CharToDecode - 65);
		}
		if (CharToDecode >= 97 && CharToDecode <= 122)
		{
			return(CharToDecode - 71);
		}
		if (CharToDecode >= 48 && CharToDecode <= 57)
		{
			return(CharToDecode + 4);
		}
		if (CharToDecode == '+')
		{
			return(62);
		}
		if (CharToDecode == '/')
		{
			return(63);
		}
		assert(false);
	}
	std::string Base64ToBinary(std::string const& Base64Data)
	{
		std::string ReturnValue = "";
		unsigned int Base64DataSize = Base64Data.size();
		unsigned int Offset = 0;
		while (Offset < Base64DataSize)
		{
			unsigned int NewBytes = 0;
			char CharactersProcessed = 0;
			for (size_t i = 0; i < 4; i++)
			{
				NewBytes += Base64CharToBinary(Base64Data[Offset]) << (18 - 6 * i);
				Offset += 1;
				CharactersProcessed += 1;
				if (Offset == Base64DataSize)
				{
					break;
				}
				if (Base64Data[Offset] == '=')
				{
					break;
				}
			}
			for (size_t i = 0; i < CharactersProcessed - 1; i++)
			{
				ReturnValue += char(NewBytes >> (16 - (i * 8)));
			}
			if (CharactersProcessed < 4)
			{
				break;
			}
		}
		return(ReturnValue);
	}
	char ByteToBASE64(uint8_t ByteToEncode)
	{
		if (ByteToEncode >= 0 && ByteToEncode <= 25)
		{
			return(ByteToEncode + 65);
		}
		else if (ByteToEncode >= 26 && ByteToEncode <= 51)
		{
			return(ByteToEncode + 71);
		}
		else if (ByteToEncode >= 52 && ByteToEncode <= 61)
		{
			return(ByteToEncode - 4);
		}
		else if (ByteToEncode == 62)
		{
			return('+');
		}
		else if (ByteToEncode == 63)
		{
			return('/');
		}
		assert(false);
	}
	std::string BASE64Decode(const void* CharactersToRead, size_t NumberOfCharacters)
	{
		return(Base64ToBinary(std::string((char*)CharactersToRead, NumberOfCharacters)));
	}
	std::string BASE64Encode(const void* DataToEncode, size_t DataLength)
	{
		std::string ReturnValue = "";
		uint32_t DataBuffer = 0;
		size_t BitsInBuffer = 0;
		size_t CurrentByteOffset = 0;
		uint8_t* DataPointer = (uint8_t*)DataToEncode;
		while (CurrentByteOffset < DataLength)
		{
			DataBuffer <<= 8;
			DataBuffer += DataPointer[CurrentByteOffset];
			BitsInBuffer += 8;
			while (BitsInBuffer >= 6)
			{
				uint8_t BitsBeforeData = (BitsInBuffer - 6);
				ReturnValue += ByteToBASE64((DataBuffer) >> BitsBeforeData);
				BitsInBuffer -= 6;
				DataBuffer = DataBuffer & (~(63 << BitsBeforeData));
			}
			CurrentByteOffset += 1;
		}
		while (BitsInBuffer != 0)
		{
			int8_t BitsBeforeData = (BitsInBuffer - 6);
			if (BitsBeforeData >= 0)
			{
				BitsInBuffer -= 6;
			}
			else
			{
				DataBuffer = DataBuffer << (-BitsBeforeData);
				BitsInBuffer = 0;
			}
			if (BitsBeforeData < 0)
			{
				BitsBeforeData = 0;
			}
			ReturnValue += ByteToBASE64((DataBuffer) >> BitsBeforeData);
			DataBuffer = DataBuffer & (~(63 << BitsBeforeData));
		}
		while (ReturnValue.size() % 4 != 0)
		{
			ReturnValue += "=";
		}
		return(ReturnValue);
	}
	std::string BASE64Decode(std::string const& DataToDecode)
	{
		return(BASE64Decode(DataToDecode.data(), DataToDecode.size()));
	}
	std::string BASE64Encode(std::string const& DataToEncode)
	{
		return(BASE64Encode(DataToEncode.data(), DataToEncode.size()));
	}


	void UpdateParseState(size_t CurrentOffset, MBError& ErrorToMove, size_t* OutOffset, MBError* OutError)
	{
		if (OutOffset != nullptr)
		{
			*OutOffset = CurrentOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = std::move(ErrorToMove);
		}
	}

	void SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset)
	{
		SkipWhitespace(DataToParse.data(), DataToParse.size(), InOffset, OutOffset);
	}
	void SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset)
	{
		const char* Data = (const char*)DataToParse;
		size_t ParseOffset = InOffset;
		while (ParseOffset < DataLength)
		{
			bool IsWhitespace = false;
			IsWhitespace = IsWhitespace || (Data[ParseOffset] == ' ');
			IsWhitespace = IsWhitespace || (Data[ParseOffset] == '\t');
			IsWhitespace = IsWhitespace || (Data[ParseOffset] == '\n');
			IsWhitespace = IsWhitespace || (Data[ParseOffset] == '\r');
			if (IsWhitespace)
			{
				ParseOffset += 1;
			}
			else
			{
				break;
			}
		}
		*OutOffset = ParseOffset;
	}
	std::string ParseQuotedString(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::string ReturnValue = "";
		size_t ParseOffset = InOffset;
		MBError ParseError(true);
		const char* ObjectData = (const char*)DataToParse;

		if (ObjectData[ParseOffset] != '\"' && ObjectData[ParseOffset] != '\'')
		{
			ParseError = false;
			ParseError.ErrorMessage = "String doesnt begin with \' or \"";
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		char StringDelimiter = ObjectData[ParseOffset];
		ParseOffset += 1;
		//size_t StringBegin = ParseOffset;
		while (ParseOffset < DataSize)
		{
			size_t PreviousParseOffset = ParseOffset;
			ParseOffset = std::find(ObjectData+ParseOffset,ObjectData+DataSize, StringDelimiter)-ObjectData;
			if (ParseOffset >= DataSize)
			{
				ParseError = false;
				ParseError.ErrorMessage = "End of quoted string missing";
				break;
			}
			size_t NumberOfBackslashes = 0;
			size_t ReverseParseOffset = ParseOffset - 1;
			while (ReverseParseOffset > PreviousParseOffset)
			{
				if (ObjectData[ReverseParseOffset] == '\\')
				{
					NumberOfBackslashes += 1;
					ReverseParseOffset -= 1;
				}
				else
				{
					break;
				}
			}
			if ((NumberOfBackslashes & 1) != 0)
			{
				//ett ojämnt antal \\, vilket innebär att det ska decodas som en "
				ReturnValue += std::string(ObjectData + PreviousParseOffset, ParseOffset - PreviousParseOffset - 1) + '"';
				ParseOffset += 1;
				continue;
			}
			else
			{
				ReturnValue += std::string(ObjectData+ PreviousParseOffset, ParseOffset - PreviousParseOffset);
				ParseOffset += 1;
				break;
			}
		}
		if (OutError != nullptr)
		{
			*OutError = ParseError;
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	std::string ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError )
	{
		return(ParseQuotedString(ObjectData.data(), ObjectData.size(), InOffset, OutOffset, OutError));
	}
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset , MBError* OutError)
	{
		intmax_t ReturnValue = -1;
		size_t ParseOffset = InOffset;
		MBError ParseError(true);

		size_t IntBegin = ParseOffset;
		size_t IntEnd = ParseOffset;
		while (IntEnd < ObjectData.size())
		{
			if (!('0' <= ObjectData[IntEnd] && ObjectData[IntEnd] <= '9'))
			{
				break;
			}
			IntEnd += 1;
		}
		try
		{
			ReturnValue = std::stoi(ObjectData.substr(IntBegin, IntEnd - IntBegin));
			ParseOffset = IntEnd;
		}
		catch (const std::exception&)
		{
			ParseError = false;
			ParseError.ErrorMessage = "Error parsing JSON integer";
		}

		if (OutError != nullptr)
		{
			*OutError = ParseError;
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, const void* DataToParse, size_t DataSize, size_t InOffset, MBError* OutError)
	{
		size_t ReturnValue = -1;
		size_t ParseOffset = InOffset;
		MBError ParseError(true);
		const char* Data = (const char*)DataToParse;

		while (ParseOffset < DataSize)
		{
			bool IsDelimiter = false;
			for (size_t i = 0; i < Delimiters.size(); i++)
			{
				if (Data[ParseOffset] == Delimiters[i])
				{
					IsDelimiter = true;
					break;
				}
			}
			if (IsDelimiter)
			{
				break;
			}
			else
			{
				ParseOffset += 1;
			}
		}
		//ANTAGANDE här blir det lite implicit att EOF så att säga är en delimiter
		ReturnValue = ParseOffset;
		if (OutError != nullptr)
		{
			*OutError = ParseError;
		}
		return(ReturnValue);
	}
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, std::string const& DataToParse, size_t InOffset, MBError* OutError)
	{
		return(GetNextDelimiterPosition(Delimiters, DataToParse.data(), DataToParse.size(), InOffset, OutError));
	}
	size_t FindSubstring(const void* Data, size_t DataSize, const void* Data2, size_t Data2Size, size_t InOffset)
	{
		size_t ReturnValue = -1;
		const char* StringToSearch = (char*)Data;
		const char* SubstringToFind = (char*)Data2;
		size_t ParseOffset = InOffset;
		if (Data2Size == 0)
		{
			return(-1);
		}
		while (ParseOffset <DataSize && (ParseOffset+Data2Size <= DataSize)) 
		{
			if (std::memcmp(StringToSearch+ParseOffset, SubstringToFind, Data2Size) == 0)
			{
				ReturnValue = ParseOffset;
				break;
			}
			ParseOffset += 1;
		}
		return(ReturnValue);
	}
	size_t FindSubstring(std::string const& StringToSearch, std::string const& Substring, size_t ParseOffset)
	{
		return(FindSubstring(StringToSearch.data(), StringToSearch.size(), Substring.data(), Substring.size(), ParseOffset));
	}
};