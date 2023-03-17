#include "MBParsing.h"
#include "MBUtility/MBInterfaces.h"
#include <cstring>
#include <algorithm>
#include <assert.h>
#include <filesystem>
#include <stdexcept>

#include <regex>

//DEBUG
#include <iostream>
#include <stdint.h>

namespace MBParsing
{
	void WriteBigEndianInteger(void* Buffer, uint64_t IntegerToWrite, char IntegerSize, size_t WriteOffset, size_t* OutWriteOffset)
	{
		WriteBigEndianInteger(((uint8_t*) Buffer) + WriteOffset, IntegerToWrite, IntegerSize);
		if (OutWriteOffset != nullptr)
		{
			*OutWriteOffset = WriteOffset + IntegerSize;
		}
	}
    void WriteBigEndianInteger(MBUtility::MBOctetOutputStream& OutStream,uint64_t IntegerToWrite,char IntegerSize)
    {
        unsigned char Buffer[32];
        assert(IntegerSize < 32 && "WriteBigEndianInteger supports only integers from size 0-8");
        WriteBigEndianInteger(Buffer,IntegerToWrite,IntegerSize,0,nullptr);
        size_t WrittenBytes = OutStream.Write(Buffer,IntegerSize);

    }
	void WriteBigEndianInteger(void* Buffer, uint64_t IntegerToWrite, char IntegerSize)
	{
		uint8_t* ByteBuffer = (uint8_t*)Buffer;
		size_t BufferOffset = 0;
		for (int i = IntegerSize - 1; i >= 0; i--)
		{
			ByteBuffer[BufferOffset] = (unsigned char)((IntegerToWrite >> (i * 8)) % 256);
			BufferOffset += 1;
		}
	}

    uint64_t ParseBigEndianInteger(MBUtility::MBOctetInputStream& InStream,unsigned char IntegerSize)
    {
        uint64_t ReturnValue = 0;
        unsigned char Buffer[32]; 
        assert(IntegerSize <= 8 && "ParseBigEndianInteger can only parse integers of size 0-8");
        InStream.Read(Buffer,IntegerSize);
        ReturnValue = ParseBigEndianInteger(Buffer,IntegerSize,0,nullptr);
        return(ReturnValue); 
    }
	uint64_t ParseBigEndianInteger(std::string const& DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset)
	{
		return(ParseBigEndianInteger(DataToParse.data(), IntegerSize, ParseOffset, OutParseOffset));
	}
	uint64_t ParseBigEndianInteger(const void* Data, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset)
	{
		uint64_t ReturnValue = 0;
		const char* DataToParse = (const char*)Data;
		for (size_t i = 0; i < IntegerSize; i++)
		{
			ReturnValue <<= 8;
			ReturnValue += (unsigned char)DataToParse[ParseOffset + i];
		}
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset + IntegerSize;
		}
		return(ReturnValue);
	}
	void WriteBigEndianIEEE754Float(void* OutBuffer, double FloatToWrite, char FloatSize, size_t WriteOffset, size_t* OutWriteOffset)
	{
		if (FloatSize != 4 && FloatSize != 8)
		{
			throw std::runtime_error("Invalid float size for IEEE754 float");
		}
		//
		if (FloatSize == 4)
		{
			//float NewFloat = FloatToWrite;
			//std::memcpy(((uint8_t*)OutBuffer) + WriteOffset, &NewFloat, 4);
			float NewFloat = FloatToWrite;
			uint32_t IntegerToWrite = *((uint32_t*)&NewFloat);
			WriteBigEndianInteger(OutBuffer, IntegerToWrite, 4, WriteOffset, nullptr);
		}
		else if (FloatSize == 8)
		{
			//std::memcpy(((uint8_t*)OutBuffer) + WriteOffset, &FloatToWrite, FloatSize);
			double NewFloat = FloatToWrite;
			uint64_t IntegerToWrite = *((uint64_t*)&NewFloat);
			WriteBigEndianInteger(OutBuffer, IntegerToWrite, 8, WriteOffset, nullptr);
		}
		if (OutWriteOffset != nullptr)
		{
			*OutWriteOffset = WriteOffset + FloatSize;
		}
	}
	double ParseBigEndianIEEE754Float(const void* DataToParse, char FloatSize, size_t ParseOffset, size_t* OutParseOffset)
	{
		//TODO obs obs obs, den här koden är absolut ass och inte portable på något sätt, men orkar inte fixa hur man faktisk ska parsa det nu
		double ReturnValue = 0;
		if (FloatSize != 4 && FloatSize != 8)
		{
			throw std::runtime_error("Invalid float size for IEEE754 float");
		}
		if (FloatSize == 4)
		{
			uint32_t RawFloatData = ParseBigEndianInteger(DataToParse, 4, ParseOffset, nullptr);
			float FloatToParse = *((float*)&RawFloatData);
			ReturnValue = FloatToParse;
		}
		if (FloatSize == 8)
		{
			uint64_t RawFloatData = ParseBigEndianInteger(DataToParse, 4, ParseOffset, nullptr);
			ReturnValue = *((double*)&RawFloatData);
		}
		ParseOffset += FloatSize;
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	unsigned char Base64CharToBinary(unsigned char CharToDecode)
	{
        unsigned char ReturnValue = 0;
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
        return(ReturnValue);
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
        char ReturnValue = 0;
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
        return(ReturnValue);
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
	std::string ParseQuotedString(char QuoteCharacter, void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::string ReturnValue = "";
		size_t ParseOffset = InOffset;
		MBError ParseError(true);
		const char* ObjectData = (const char*)DataToParse;

		if (ObjectData[ParseOffset] != QuoteCharacter)
		{
			ParseError = false;
			ParseError.ErrorMessage = "String doesnt begin with";
			ParseError.ErrorMessage += QuoteCharacter;
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		char StringDelimiter = QuoteCharacter;
		ParseOffset += 1;
		//size_t StringBegin = ParseOffset;
		while (ParseOffset < DataSize)
		{
			size_t NextEndDelimiter = std::find(ObjectData + ParseOffset, ObjectData + DataSize, StringDelimiter) - ObjectData;
			size_t NextBackslash = std::find(ObjectData + ParseOffset, ObjectData + DataSize, '\\') - ObjectData;
			if (NextBackslash == DataSize || NextEndDelimiter < NextBackslash)
			{
				if (NextEndDelimiter == DataSize)
				{
					ParseError = false;
					ParseError.ErrorMessage = "No end delimiter for string";
					break;
				}
				ReturnValue += std::string(ObjectData + ParseOffset, ObjectData + NextEndDelimiter);
				ParseOffset = NextEndDelimiter + 1;
				break;
			}
			else
			{
				if (NextBackslash + 2 >= DataSize)
				{
					ParseError = false;
					ParseError.ErrorMessage = "No end delimiter for string";
					break;
				}
				ReturnValue += std::string(ObjectData + ParseOffset, ObjectData + NextBackslash);
				ReturnValue += ObjectData[NextBackslash + 1];
				ParseOffset = NextBackslash + 2;
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
			size_t NextEndDelimiter = std::find(ObjectData+ParseOffset,ObjectData+DataSize, StringDelimiter)-ObjectData;
			size_t NextBackslash = std::find(ObjectData + ParseOffset, ObjectData + DataSize, '\\') - ObjectData;
			if (NextBackslash == DataSize || NextEndDelimiter < NextBackslash)
			{
				if (NextEndDelimiter == DataSize)
				{
					ParseError = false;
					ParseError.ErrorMessage = "No end delimiter for string";
					break;
				}
				ReturnValue += std::string(ObjectData + ParseOffset, ObjectData + NextEndDelimiter);
				ParseOffset = NextEndDelimiter + 1;
				break;
			}
			else
			{
				if (NextBackslash+2 >= DataSize)
				{
					ParseError = false;
					ParseError.ErrorMessage = "No end delimiter for string";
					break;
				}
				ReturnValue += std::string(ObjectData + ParseOffset, ObjectData + NextBackslash);
				//ReturnValue += ObjectData[NextBackslash + 1];
                if(ObjectData[NextBackslash+1] == 'n')
                {
                    ReturnValue += '\n';
                }
                else if(ObjectData[NextBackslash+1] == 'b')
                {
                    ReturnValue += '\b';
                }
                else if(ObjectData[NextBackslash+1] == 'f')
                {
                    ReturnValue += '\f';
                }
                else if(ObjectData[NextBackslash+1] == 'r')
                {
                    ReturnValue += '\r';
                }
                else if(ObjectData[NextBackslash+1] == 't')
                {
                    ReturnValue += '\b';
                }
                else
                {
                    ReturnValue += ObjectData[NextBackslash+1];   
                }
				ParseOffset = NextBackslash + 2;
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
	intmax_t ParseJSONInteger(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		intmax_t ReturnValue = -1;
		size_t ParseOffset = InOffset;
		MBError ParseError(true);
		const char* ObjectData = (const char*)DataToParse;
		size_t IntBegin = ParseOffset;
		size_t IntEnd = ParseOffset;
		while (IntEnd < DataSize)
		{
			if (!(('0' <= ObjectData[IntEnd] && ObjectData[IntEnd] <= '9') || ObjectData[IntEnd] == '-'))
			{
				break;
			}
			IntEnd += 1;
		}
		try
		{
			ReturnValue = std::stoll(std::string(ObjectData +IntBegin, ObjectData+ IntEnd));
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
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset , MBError* OutError)
	{
        return(ParseJSONInteger(ObjectData.data(),ObjectData.size(),InOffset,OutOffset,OutError));
	}
	bool ParseJSONBoolean(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		const char* Data = (const char*)DataToParse;
		MBError ParseError = true;
		size_t ParseOffset = InOffset;
		bool ReturnValue = false;
		SkipWhitespace(DataToParse, DataSize, ParseOffset, &ParseOffset);
		if (ParseOffset >= DataSize)
		{
			ParseError = false;
			ParseError.ErrorMessage = "Error parsing JSON boolean: no boolean data";
		}
		else
		{
			char FirstCharacter = Data[ParseOffset];
			if (FirstCharacter == 't')
			{
				if (ParseOffset + 3 < DataSize && std::memcmp(Data+ParseOffset,"true",4) == 0)
				{
					ReturnValue = true;
					ParseOffset += 4;
				}
				else
				{
					ParseError = false;
					ParseError.ErrorMessage = "Error parsing JSON boolean: invalid boolean data";
				}
			}
			else if (FirstCharacter == 'f')
			{
				if (ParseOffset + 4 < DataSize && std::memcmp(Data + ParseOffset, "false", 5) == 0)
				{
					ReturnValue = false;
					ParseOffset += 5;
				}
				else
				{
					ParseError = false;
					ParseError.ErrorMessage = "Error parsing JSON boolean: invalid boolean data";
				}
			}
			else
			{
				ParseError = false;
				ParseError.ErrorMessage = "Error parsing JSON boolean: invalid first character";
			}
		}
		UpdateParseState(ParseOffset, ParseError, OutOffset, OutError);
		return(ReturnValue);
	}
    size_t GetNextDelimiterPosition(const char* BeginDelimiters,const char* EndDelimiters,const void* DataToParse,size_t DataSize,size_t InOffset)
    {
		size_t ParseOffset = InOffset;
		const char* Data = (const char*)DataToParse;
		while (ParseOffset < DataSize)
		{
			bool IsDelimiter = false;
			for (size_t i = 0; i < EndDelimiters - BeginDelimiters; i++)
			{
				if (Data[ParseOffset] == BeginDelimiters[i])
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
		//ASSUMPTION EOF is an implicit delimiter
		return(ParseOffset);
       
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
		//ANTAGANDE h�r blir det lite implicit att EOF s� att s�ga �r en delimiter
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


	bool CharIsNumerical(char CharToCheck)
	{
		return(CharToCheck >= 48 && CharToCheck <= 57);
	}
	bool CharIsAlphabetical(char CharTocheck)
	{
		return((CharTocheck >= 65 && CharTocheck <= 90) || (CharTocheck >= 97 && CharTocheck <= 122));
	}


	//BEGIN JSONObject
	void swap(JSONObject& LeftObject, JSONObject& RightObject)
	{
		std::swap(LeftObject.m_Type, RightObject.m_Type);
		std::swap(LeftObject.m_ObjectData, RightObject.m_ObjectData);
	}

	void* JSONObject::p_CloneData() const
	{
		void* ReturnValue = nullptr;
		if (m_Type == JSONObjectType::Aggregate)
		{
			ReturnValue = new std::map<std::string, JSONObject>(*(std::map<std::string, JSONObject> const*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Array)
		{
			ReturnValue = new std::vector<JSONObject>(*(std::vector<JSONObject> const*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Integer)
		{
			ReturnValue = new intmax_t(*(intmax_t const*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Bool)
		{
			ReturnValue = new bool(*(bool const*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::String)
		{
			ReturnValue = new std::string(*(const std::string*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Null)
		{

		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}
	JSONObject& JSONObject::operator=(JSONObject ObjectToCopy)
	{
		swap(*this, ObjectToCopy);
		return(*this);
	}
	void JSONObject::p_FreeData()
	{
		if (m_Type == JSONObjectType::Aggregate)
		{
			delete ((std::map<std::string,JSONObject>*) m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Array)
		{
			delete ((std::vector<JSONObject>*) m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Integer)
		{
			delete ((intmax_t*) m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Bool)
		{
			delete ((bool*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::String)
		{
			delete ((std::string*)m_ObjectData);
		}
		else if (m_Type == JSONObjectType::Null)
		{
		
		}
		else
		{
			assert(false);
		}
	}
    JSONObject::JSONObject(double FloatInitializer)
    {
        
    }
    JSONObject::JSONObject(int IntegerInitializer)
    {
        m_Type = JSONObjectType::Integer;
        m_ObjectData = new intmax_t(IntegerInitializer);
    }
    JSONObject::JSONObject(JSONObject const& ObjectToCopy)
	{
		m_Type = ObjectToCopy.m_Type;
		m_ObjectData = ObjectToCopy.p_CloneData();
	}
	JSONObject::JSONObject(JSONObject&& ObjectToSteal) noexcept
	{
		swap(*this, ObjectToSteal);
	}
	JSONObject::JSONObject(const char* StringInitializer)
	{
		*this = JSONObject(std::string(StringInitializer));
	}
	//JSONObject::JSONObject(JSONObjectType InitialType)
	//{
	//	if(initia)
	//}
	JSONObject::JSONObject(std::string StringInitializer)
	{
		m_Type = JSONObjectType::String;
		m_ObjectData = new std::string(std::move(StringInitializer));
	}
	JSONObject::JSONObject(intmax_t IntegerInitializer)
	{
		m_Type = JSONObjectType::Integer;
		m_ObjectData = new intmax_t(IntegerInitializer);
	}
	JSONObject::JSONObject(bool BoolInitializer)
	{
		m_Type = JSONObjectType::Bool;
		m_ObjectData = new bool(BoolInitializer);
	}
	JSONObject::JSONObject(std::vector<JSONObject> VectorInitializer)
	{
		m_Type = JSONObjectType::Array;
		m_ObjectData = new std::vector<JSONObject>(std::move(VectorInitializer));
	}
	JSONObject::JSONObject(std::map<std::string, JSONObject> VectorInitializer)
	{
		m_Type = JSONObjectType::Aggregate;
		m_ObjectData = new std::map<std::string,JSONObject>(std::move(VectorInitializer));
	}
	JSONObject::JSONObject(JSONObjectType InitialType)
	{
		JSONObject ObjectToBecome;
		if (InitialType == JSONObjectType::Aggregate)
		{
			ObjectToBecome = JSONObject(std::map<std::string, JSONObject>());
		}
		else if (InitialType == JSONObjectType::Array)
		{
			ObjectToBecome = JSONObject(std::vector<JSONObject>());
		}
		else if (InitialType == JSONObjectType::Bool)
		{
			ObjectToBecome = false;
		}
		else if (InitialType == JSONObjectType::Integer)
		{
			ObjectToBecome = intmax_t(0);
		}
		else if (InitialType == JSONObjectType::String)
		{
			ObjectToBecome = "";
		}
		else if (InitialType == JSONObjectType::Null)
		{

		}
		else
		{
			throw std::runtime_error("Invalid json object type");
		}
		*this = std::move(ObjectToBecome);

	}
	JSONObject::~JSONObject()
	{
		p_FreeData();
	}

	intmax_t JSONObject::GetIntegerData() const
	{
		if (m_Type != JSONObjectType::Integer)
		{
			throw std::domain_error("JSON object not of integer type");
		}
		return(*(const intmax_t*)m_ObjectData);
	}
	std::string const& JSONObject::GetStringData() const
	{
		if (m_Type != JSONObjectType::String)
		{
			throw std::domain_error("JSON object not of string type");
		}
		std::string const& ReturnValue = *(const std::string*)m_ObjectData;
		return(ReturnValue);
	}
	bool JSONObject::GetBooleanData() const
	{
		if (m_Type != JSONObjectType::Bool)
		{
			throw std::domain_error("JSON object not of bool type");
		}
		return(*(const bool*)m_ObjectData);
	}

	std::map<std::string, JSONObject> const& JSONObject::GetMapData() const
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of Aggregate type");
		}
		return(*(std::map<std::string, JSONObject>*)m_ObjectData);
	}
	std::map<std::string, JSONObject>& JSONObject::GetMapData()
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of Aggregate type");
		}
		return(*(std::map<std::string, JSONObject>*)m_ObjectData);
	}
	std::vector<JSONObject>& JSONObject::GetArrayData()
	{
		if (m_Type != JSONObjectType::Array)
		{
			throw std::domain_error("JSON object not of array type");
		}
		return(*(std::vector<JSONObject>*)m_ObjectData);
	}
	std::vector<JSONObject>const& JSONObject::GetArrayData() const
	{
		if (m_Type != JSONObjectType::Array)
		{
			throw std::domain_error("JSON object not of array type");
		}
		return(*(const std::vector<JSONObject>*)m_ObjectData);
	}

	bool JSONObject::HasAttribute(std::string const& AttributeName) const
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of aggregate type");
		}
		std::map<std::string, JSONObject>& ObjectMap = *(std::map<std::string, JSONObject>*)m_ObjectData;
		return(ObjectMap.find(AttributeName) != ObjectMap.end());
	}
	JSONObject& JSONObject::GetAttribute(std::string const& AttributeName)
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of Aggregate type");
		}
		std::map<std::string, JSONObject>& ObjectMap = *(std::map<std::string, JSONObject>*)m_ObjectData;
		return(ObjectMap[AttributeName]);
	}
	JSONObject& JSONObject::operator[](std::string const& AttributeName)
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of aggregate type");
		}
		return(GetAttribute(AttributeName));
	}
	JSONObject const& JSONObject::operator[](std::string const& AttributeName) const
	{
		return(GetAttribute(AttributeName));
	}
	JSONObject const& JSONObject::GetAttribute(std::string const& AttributeName) const
	{
		if (m_Type != JSONObjectType::Aggregate)
		{
			throw std::domain_error("JSON object not of aggregae type");
		}
		std::map<std::string, JSONObject>& ObjectMap = *(std::map<std::string, JSONObject>*)m_ObjectData;
		return(ObjectMap.at(AttributeName));
	}
	std::string JSONObject::ToString() const
	{
		std::string ReturnValue = "";
		if (m_Type == JSONObjectType::Array)
		{
			ReturnValue = p_ToString_Array();
		}
		else if (m_Type == JSONObjectType::Aggregate)
		{
			ReturnValue = p_ToString_Aggregate();
		}
		else
		{
			ReturnValue = p_ToString_Atomic();
		}
		return(ReturnValue);
	}
	std::string JSONObject::p_ToPrettyString_Array(int IndentLevel) const
	{
		std::string ReturnValue;
        ReturnValue += std::string(m_IndentSpaceWidth*IndentLevel,' ');
        ReturnValue += "[\n";
        auto const& ArrayData = GetArrayData();
        for(JSONObject const& Element : ArrayData)
        {
            ReturnValue += Element.p_ToPrettyString(IndentLevel+1);
            ReturnValue += ",\n";
        }
        if(ArrayData.size() > 0)
        {
            ReturnValue.resize(ReturnValue.size()-2);  
        } 
        ReturnValue += '\n';
        ReturnValue += std::string(m_IndentSpaceWidth*IndentLevel,' ');
        ReturnValue += ']'; 
		return(ReturnValue);
	}
    std::string JSONObject::p_ToPrettyString(int IndentLevel) const
    {
		std::string ReturnValue;
        if(m_Type == JSONObjectType::Aggregate)
        {
            ReturnValue = p_ToPrettyString_Aggregate(IndentLevel);
        }
        else if(m_Type == JSONObjectType::Array)
        {
            ReturnValue = p_ToPrettyString_Array(IndentLevel);   
        }
        else
        {
            //atomic
            ReturnValue = std::string(m_IndentSpaceWidth*IndentLevel,' ');
            ReturnValue += ToString();       
        }
        return(ReturnValue);
    }
	std::string JSONObject::p_ToPrettyString_Aggregate(int IndentLevel) const
	{
		std::string ReturnValue;
        ReturnValue += std::string(m_IndentSpaceWidth*IndentLevel,' ');
        ReturnValue += "{\n";
        auto const& MapData = GetMapData(); 
        for(auto const& Element : MapData)
        {
            ReturnValue += std::string(m_IndentSpaceWidth*(IndentLevel+1),' ');
            ReturnValue += '"';
            ReturnValue += Element.first;
            ReturnValue += "\": ";
			if (Element.second.GetType() == JSONObjectType::Aggregate || Element.second.GetType() == JSONObjectType::Array)
			{
				ReturnValue += "\n";
				ReturnValue += Element.second.p_ToPrettyString(IndentLevel+1);
			}
			else
			{
				ReturnValue += Element.second.ToString();
			}
            ReturnValue += ",\n";
        }
        if(MapData.size() > 0)
        {
            ReturnValue.resize(ReturnValue.size()-2);
        }
        ReturnValue += '\n';
        ReturnValue += std::string(m_IndentSpaceWidth*IndentLevel,' ');
        ReturnValue += '}';
		return(ReturnValue);
	}
	std::string JSONObject::ToPrettyString() const
	{
		std::string ReturnValue = p_ToPrettyString(0); 
		return(ReturnValue);
	}
	std::string JSONObject::p_ToString_Array() const
	{
		std::string ReturnValue = "";
		std::vector<JSONObject> const& ArrayToConvert = *((std::vector<JSONObject>*)m_ObjectData);
		ReturnValue += "[";
		for (size_t i = 0; i < ArrayToConvert.size(); i++)
		{
			ReturnValue += ArrayToConvert[i].ToString();
			if (i + 1 < ArrayToConvert.size())
			{
				ReturnValue += ",";
			}
		}
		ReturnValue += "]";
		return(ReturnValue);
	}
	std::string JSONObject::p_ToString_Aggregate() const
	{
		std::string ReturnValue = "";
		std::map<std::string, JSONObject> const& Data = *((std::map<std::string, JSONObject>*)m_ObjectData);
		ReturnValue += "{";
		for(auto const& Entry : Data)
		{
			ReturnValue += ToJason(Entry.first)+":"+Entry.second.ToString()+",";
		}
		if (Data.size() > 0)
		{
			ReturnValue.resize(ReturnValue.size() - 1);
		}
		ReturnValue += "}";
		return(ReturnValue);
	}
	std::string JSONObject::p_ToString_Atomic() const
	{
		std::string ReturnValue = "";
		if (m_Type == JSONObjectType::Bool)
		{
			bool Data = *((bool*)m_ObjectData);
			ReturnValue = Data ? "true" : "false";
		}
		else if (m_Type == JSONObjectType::String)
		{
			ReturnValue = ToJason(*((std::string*)m_ObjectData));
		}
		else if (m_Type == JSONObjectType::Integer)
		{
			ReturnValue = std::to_string(*((intmax_t*)m_ObjectData));
		}
		else if (m_Type == JSONObjectType::Null)
		{
			ReturnValue = "null";
		}

		return(ReturnValue);
	}
	//END JSONObject

	std::string ToJason(std::string const& ValueToJason)
	{
		std::string EscapedJasonString = "";
		for (size_t i = 0; i < ValueToJason.size(); i++)
		{
			if (ValueToJason[i] == '"')
			{
				EscapedJasonString += "\\\"";
			}
			else if (ValueToJason[i] == '\\')
			{
				EscapedJasonString += "\\\\";
			}
			else if (ValueToJason[i] == '\b')
			{
				EscapedJasonString += "\\b";
			}
			else if (ValueToJason[i] == '\f')
			{
				EscapedJasonString += "\\f";
			}
			else if (ValueToJason[i] == '\n')
			{
				EscapedJasonString += "\\n";
			}
			else if (ValueToJason[i] == '\r')
			{
				EscapedJasonString += "\\r";
			}
			else if (ValueToJason[i] == '\t')
			{
				EscapedJasonString += "\\t";
			}
			else
			{
				EscapedJasonString += ValueToJason[i];
			}
		}
		return("\"" + EscapedJasonString + "\"");
	}


    JSONObject ParseJSONObject(std::filesystem::path const& FilePath,MBError* OutError)
    {
        JSONObject ReturnValue;
        std::string TotalFileData;
        if(!std::filesystem::exists(FilePath))
        {
            *OutError = false;
            OutError->ErrorMessage = "filepath doesn't exist";
            return(ReturnValue);
        }
        if(!std::filesystem::is_regular_file(FilePath))
        {
            *OutError = false;
            OutError->ErrorMessage = "filepath isn't a file";
            return(ReturnValue);
        }
        std::ifstream FileInput(FilePath.c_str(),std::ios::in);
        if(FileInput.is_open() == false)
        {
            *OutError = false;
            OutError->ErrorMessage = "filepath isn't a file";
            return(ReturnValue);
        }
        while(true)
        {
            constexpr size_t ReadChunkSize = 4096;
            char Buffer[ReadChunkSize];
            size_t ReadBytes = FileInput.read(Buffer,ReadChunkSize).gcount();
            TotalFileData.insert(TotalFileData.begin(),Buffer,Buffer+ReadBytes);
            if(ReadBytes < ReadChunkSize)
            {
                break;   
            }
        }
        ReturnValue = ParseJSONObject(TotalFileData,0,nullptr,OutError);
        return(ReturnValue);
    }
	JSONObject ParseJSONObject(const void* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		JSONLikeParser<JSONObject> JsonParser;
		MBError ParseError = true;
		size_t ParseOffset = InOffset;
		JSONObject ReturnValue = JsonParser.ParseObject(DataToParse, DataSize, ParseOffset, &ParseOffset, &ParseError);

		UpdateParseState(ParseOffset, ParseError, OutOffset, OutError);
		return(ReturnValue);

	}
	JSONObject ParseJSONObject(std::string const& Data, size_t ParseOffset, size_t* OutOffset, MBError* OutError)
	{
		return(ParseJSONObject(Data.data(), Data.size(), ParseOffset, OutOffset, OutError));
	}



	std::vector<std::string> TokenizeText(std::string const& TextInput)
	{
		std::vector<std::string> ReturnValue;
		
		try
		{
			//std::regex SplitRegex = std::regex(R"(([:space:]+)(\(|\)|\[|\]|\+|-|\*|[:alpha:]+|"[^"]*"))", std::regex_constants::extended);
			//std::regex SplitRegex = std::regex(R"(([[:space:]]*)|([[:alpha:]][[:alpha:][:digit:]_]*|[[:digit:]]+|"([^"\\]|\\.)*"|'([^'\\]|\\.)*'|\+=|-=|\*=|/=|==|=|\.|,|;|\+|\*|-|/|\(|\)|\[|]|\{|}))", 
			//	std::regex_constants::extended);
			std::regex SplitRegex = std::regex(R"(([[:space:]]*)|([[:alpha:]][[:alpha:][:digit:]_]*|[[:digit:]]+|"([^"\\]|\\.)*"|'([^'\\]|\\.)*'|>=|<=|\+=|-=|\*=|/=|==|[].^,<[>{}(;/)+*=-]))", 
				std::regex_constants::extended);
			std::sregex_iterator Iterator = std::sregex_iterator(TextInput.begin(), TextInput.end(), SplitRegex);
			std::sregex_iterator End;
			while (Iterator != End)
			{
				if ((*Iterator).size() >= 3)
				{
					std::string NewString = (*Iterator)[2].str();
					if (NewString != "")
					{
						ReturnValue.push_back(NewString);
					}
				}
				Iterator++;
			}
		}
		catch(std::exception const& e)
		{

		}
		return(ReturnValue);
	}


	//BEGIN SyntaxTree
	SyntaxTree::SyntaxTree(std::string LiteralString, NameToken Type)
	{
		m_Type = Type;
		m_LiteralData = std::move(LiteralString);
	}
	SyntaxTree::SyntaxTree(NameToken Type)
	{
		m_Type = Type;
	}
	bool SyntaxTree::IsLiteral() const
	{
		return(m_LiteralData != "");
	}
	std::string const& SyntaxTree::GetLiteralData() const
	{
		return(m_LiteralData);
	}
	NameToken SyntaxTree::GetType() const
	{
		return(m_Type);
	}
	int SyntaxTree::GetChildCount() const
	{
		return(m_SubComponents.size());
	}
	SyntaxTree& SyntaxTree::operator[](size_t Index)
	{
		if (Index >= m_SubComponents.size())
		{
			throw std::runtime_error("SyntaxTree child index out of range");
		}
		return(m_SubComponents[Index]);
	}
	SyntaxTree const& SyntaxTree::operator[](size_t Index) const
	{
		if (Index >= m_SubComponents.size())
		{
			throw std::runtime_error("SyntaxTree child index out of range");
		}
		return(m_SubComponents[Index]);
	}
	void SyntaxTree::AddChild(SyntaxTree ChildToAdd)
	{
		if (IsLiteral())
		{
			throw std::runtime_error("Literal node cant have children");
		}
		m_SubComponents.push_back(std::move(ChildToAdd));
	}
	//END SyntaxTree


	//BEGIN BNFRule_Literal
	BNFRule_Literal::BNFRule_Literal(std::string AssociatedLiteral)
	{
		m_LiteralToParse = std::move(AssociatedLiteral);
	}
	SyntaxTree BNFRule_Literal::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue;
		size_t CurrentOffset = TokenOffset;
		bool Result = true;
		if (TokenOffset >= TokenCount)
		{
			Result = false;
		}
		if (Result)
		{
			if (TokenData[CurrentOffset] == m_LiteralToParse)
			{
				ReturnValue = SyntaxTree(TokenData[CurrentOffset], NameToken(ParseSyntaxTypes::LITERAL));
				CurrentOffset += 1;
			}
			else
			{
				Result = false;
			}
		}
		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr)
		{
			*OutTokenOffset = CurrentOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_Literal

	//BEGIN BNFRule_Regex
	BNFRule_Regex::BNFRule_Regex(std::string const& RegexLiteral)
	{
		bool ErrorOccured = false;
		try
		{
			m_RegexToMatch = std::regex(RegexLiteral, std::regex::extended);
		}
		catch (const std::exception& e)
		{
			m_Invalid = true;
		}
	}
	bool BNFRule_Regex::IsValid() const
	{
		return(!m_Invalid);
	}
	SyntaxTree BNFRule_Regex::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue;
		size_t CurrentOffset = TokenOffset;
		bool Result = true;
		if (TokenOffset >= TokenCount)
		{
			Result = false;
		}
		if (Result)
		{
			if (std::regex_match(TokenData[CurrentOffset],m_RegexToMatch))
			{
				ReturnValue = SyntaxTree(TokenData[CurrentOffset], NameToken(ParseSyntaxTypes::LITERAL));
				CurrentOffset += 1;
			}
			else
			{
				Result = false;
			}
		}
		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr)
		{
			*OutTokenOffset = CurrentOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_Regex

	//BEGIN BNFRule_OR
	void BNFRule_OR::AddAlternative(std::unique_ptr<BNFRule> AlternativeToAdd)
	{
		m_Alternatives.push_back(std::move(AlternativeToAdd));
	}
	SyntaxTree BNFRule_OR::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue;
		size_t OutOffset = TokenOffset;
		bool Result = false;
		for (size_t i = 0; i < m_Alternatives.size(); i++)
		{
			SyntaxTree NewResult = m_Alternatives[i]->Parse(TokenData, TokenCount, TokenOffset, &OutOffset, &Result);
			if (Result)
			{
				ReturnValue = std::move(NewResult);
				break;
			}
		}
		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr && Result)
		{
			*OutTokenOffset = OutOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_OR

	//BEGIN BNFRule_AND
	void BNFRule_AND::AddElement(std::unique_ptr<BNFRule> AlternativeToAdd)
	{
		m_RulesToCombine.push_back(std::move(AlternativeToAdd));
	}
	SyntaxTree BNFRule_AND::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue = SyntaxTree(NameToken(ParseSyntaxTypes::ARRAY));
		size_t CurrentOffset = TokenOffset;
		bool Result = false;
		for (size_t i = 0; i < m_RulesToCombine.size(); i++)
		{
			SyntaxTree NewResult = m_RulesToCombine[i]->Parse(TokenData, TokenCount, CurrentOffset, &CurrentOffset, &Result);
			if (!Result)
			{
				ReturnValue = SyntaxTree();
				break;
			}
			ReturnValue.AddChild(std::move(NewResult));
		}
		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr && Result)
		{
			*OutTokenOffset = CurrentOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_AND

	//BEGIN BNFRule_Range
	BNFRule_Range::BNFRule_Range(std::unique_ptr<BNFRule> UnderlyingRule, int MinCount, int MaxCount)
	{
		m_UnderylingRule = std::move(UnderlyingRule);
		m_MinCount = MinCount;
		m_MaxCount = MaxCount;
	}
	SyntaxTree BNFRule_Range::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue = SyntaxTree(NameToken(ParseSyntaxTypes::ARRAY));
		bool Result = false;
		size_t CurrentOffset = TokenOffset;

		if (m_MinCount == -1)
		{
			throw std::runtime_error("Minimum amount of elements has to be specified");
		}
		int ParsedElements = 0;
		bool ParseElementsResult = false;
		while (true)
		{
			if (m_MaxCount != -1 && ParsedElements >= m_MaxCount)
			{
				break;
			}
			SyntaxTree NewElement = m_UnderylingRule->Parse(TokenData, TokenCount, CurrentOffset, &CurrentOffset,&ParseElementsResult);
			if (!ParseElementsResult)
			{
				break;
			}
			ReturnValue.AddChild(std::move(NewElement));
			ParsedElements += 1;
		}
		if (ParsedElements < m_MinCount)
		{
			Result = false;
			ReturnValue = SyntaxTree();
		}
		else
		{
			//vi kan aldrig parsa för många
			Result = true;
		}
		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr && Result)
		{
			*OutTokenOffset = CurrentOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_Range

	//BEGIN BNFRule_NamedRule
	std::vector<SyntaxTree> BNFRule_NamedRule::Flatten(SyntaxTree const& TreeToFlatten)
	{
		std::vector<SyntaxTree> ReturnValue;
		if (TreeToFlatten.GetType() != NameToken(ParseSyntaxTypes::ARRAY))
		{
			ReturnValue.push_back(TreeToFlatten);
		}
		else
		{
			for (size_t i = 0; i < TreeToFlatten.GetChildCount(); i++)
			{
				std::vector<SyntaxTree> NewChilds = Flatten(TreeToFlatten[i]);
				for (size_t j = 0; j < NewChilds.size(); j++)
				{
					ReturnValue.push_back(std::move(NewChilds[j]));
				}
			}
		}
		return(ReturnValue);
	}
	BNFRule_NamedRule::BNFRule_NamedRule(NameToken RuleName, std::unique_ptr<BNFRule> UnderlyingRule)
	{
		m_RuleName = RuleName;
		m_UnderlyingRule = std::move(UnderlyingRule);
	}
	SyntaxTree BNFRule_NamedRule::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		SyntaxTree ReturnValue = SyntaxTree(m_RuleName);
		bool Result = false;
		size_t CurrentOffset = TokenOffset;

		SyntaxTree NewElement = m_UnderlyingRule->Parse(TokenData, TokenCount, CurrentOffset, &CurrentOffset, &Result);

		if (Result)
		{
			std::vector<SyntaxTree> Children = Flatten(NewElement);
			for (size_t i = 0; i < Children.size(); i++)
			{
				ReturnValue.AddChild(std::move(Children[i]));
			}
		}

		if (OutError != nullptr)
		{
			*OutError = Result;
		}
		if (OutTokenOffset != nullptr && Result)
		{
			*OutTokenOffset = CurrentOffset;
		}
		return(ReturnValue);
	}
	//END BNFRule_NamedRule

	//BEGIN BNFRule_RuleReference
	SyntaxTree BNFRule_RuleReference::Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const
	{
		if (m_AssociatedParser == nullptr)
		{
			throw std::runtime_error("Associated parser not initialised");
		}
		SyntaxTree ReturnValue = (*m_AssociatedParser)[m_RuleName].Parse(TokenData, TokenCount, TokenOffset, OutTokenOffset, OutError);
		return(ReturnValue);
	}
	//END BNFRule_RuleReference

	const std::vector<char> G_BNFIdentifierDelimiters = { '\n',' ','\t','*',' ','+','{','}','(',')',';'};
	//BEGIN BNFParser
	BNFParser::RangeSpecification BNFParser::ParseRange(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset)
	{
		RangeSpecification ReturnValue;
		const char* CharData = (const char*)Data;
		size_t CurrentParseOffset = ParseOffset;

		if (CurrentParseOffset >= DataSize)
		{
			throw ParseException(CurrentParseOffset, "unexpected end of file when parsing range");
		}
		if (CharData[CurrentParseOffset] != '{')
		{
			throw ParseException(CurrentParseOffset, "invalid begin of range");
		}
		size_t RangeBegin = CurrentParseOffset;
		size_t RangeEnd = CurrentParseOffset;
		while (RangeEnd < DataSize)
		{
			if (CharData[RangeEnd] != '}')
			{
				RangeEnd += 1;
			}
			else
			{
				break;
			}
		}
		if (RangeEnd == DataSize)
		{
			throw ParseException(CurrentParseOffset, "end of range not found");
		}
		size_t RangeMiddle = RangeBegin + 1;
		while (RangeMiddle < RangeEnd)
		{
			if (CharData[RangeMiddle] != ',')
			{
				RangeMiddle += 1;
			}
			else
			{
				break;
			}
		}
		if (RangeMiddle == RangeEnd)
		{
			throw ParseException(CurrentParseOffset, "begin and and of range has to be specified");
		}
		std::string FirstRangeSpecifier = std::string(CharData + RangeBegin + 1, RangeMiddle - (RangeBegin + 1));
		std::string SecondRangeSpecifier = std::string(CharData + RangeMiddle + 1, RangeEnd - (RangeMiddle + 1));

		bool ParseIntegerError = true;
		std::string ErrorMessage = "";
		try
		{
			ReturnValue.Min = std::stoi(FirstRangeSpecifier);
			ReturnValue.Max = std::stoi(SecondRangeSpecifier);
		}
		catch(std::exception const& e)
		{
			ParseIntegerError = false;
			ErrorMessage = e.what();
		}
		if (!ParseIntegerError)
		{
			throw ParseException(CurrentParseOffset, "Error parssing range integers: "+ErrorMessage);
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = RangeEnd+1;
		}
		return(ReturnValue);
	}
	std::unique_ptr<BNFRule> BNFParser::ParseTerm(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset)
	{
		std::unique_ptr<BNFRule> ReturnValue;
		const char* CharData = (const char*)Data;
		size_t CurrentParseOffset = ParseOffset;
		
		if(CurrentParseOffset >= DataSize)
		{
			throw ParseException(CurrentParseOffset, "unexpected end of file when parsing term");
		}

		if (CharData[CurrentParseOffset] == '\"')
		{
			MBError ParseResult = true;
			std::string LiteralData = ParseQuotedString(Data, DataSize, CurrentParseOffset, &CurrentParseOffset, &ParseResult);
			if (!ParseResult)
			{
				throw ParseException(CurrentParseOffset, "Error parsing term: " + ParseResult.ErrorMessage);
			}
			ReturnValue = std::unique_ptr<BNFRule>(new BNFRule_Regex(std::move(LiteralData)));
		}
		else if (CharData[CurrentParseOffset] == '\'')
		{
			MBError ParseResult = true;
			std::string LiteralData = ParseQuotedString('\'',Data, DataSize, CurrentParseOffset, &CurrentParseOffset, &ParseResult);
			if (!ParseResult)
			{
				throw ParseException(CurrentParseOffset, "Error parsing term: " + ParseResult.ErrorMessage);
			}
			ReturnValue = std::unique_ptr<BNFRule>(new BNFRule_Literal(std::move(LiteralData)));
		}
		else if (CharIsAlphabetical(CharData[CurrentParseOffset]))
		{
			std::string RuleName = "";
			while (CurrentParseOffset < DataSize && CharIsAlphabetical(CharData[CurrentParseOffset]))
			{
				RuleName += CharData[CurrentParseOffset];
				CurrentParseOffset += 1;
			}
			if (m_NameToRule.find(RuleName) == m_NameToRule.end())
			{
				m_CurrentTokenName += 1;
				m_NameToRule[RuleName] = m_CurrentTokenName;
				m_RuleToName[m_CurrentTokenName] = RuleName;
				//throw ParseException(CurrentParseOffset, "rule term not defined");
			}
			ReturnValue = std::unique_ptr<BNFRule>(new BNFRule_RuleReference(m_NameToRule[RuleName],this));
		}
		else
		{
			throw ParseException(CurrentParseOffset, "Expected literal string or identifier name");
		}

		if (OutOffset != nullptr)
		{
			*OutOffset = CurrentParseOffset;
		}
		return(ReturnValue);
	}
	std::unique_ptr<BNFRule> BNFParser::ParseExpression(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset)
	{
		std::unique_ptr<BNFRule> ReturnValue = nullptr;
		size_t CurrentParseOffset = ParseOffset;
		const char* CharData = (const char*)Data;
		if (CurrentParseOffset >= DataSize)
		{
			throw ParseException(CurrentParseOffset, "Expression needs atleast one element");
		}
		std::unique_ptr<BNFRule_OR> TopOrClause = std::unique_ptr<BNFRule_OR>(new BNFRule_OR());
		//std::unique_ptr<BNFRule_AND> CurrentAndClause = std::unique_ptr<BNFRule_AND>(new BNFRule_AND());
		std::vector<std::unique_ptr<BNFRule>> CurrentAndClauses;
		while (CurrentParseOffset < DataSize)
		{
			SkipWhitespace(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
			if (CharData[CurrentParseOffset] == ';' || CharData[CurrentParseOffset] == ')' || CharData[CurrentParseOffset] == ']' || CharData[CurrentParseOffset] == '}')
			{
				CurrentParseOffset += 1;
				break;
			}
			if (CharData[CurrentParseOffset] == '|')
			{
				CurrentParseOffset += 1;
				if (CurrentAndClauses.size() > 1)
				{
					TopOrClause->AddAlternative(std::unique_ptr<BNFRule>(new BNFRule_AND(std::move(CurrentAndClauses))));
				}
				else if(CurrentAndClauses.size() == 1)
				{
					TopOrClause->AddAlternative(std::move(CurrentAndClauses.front()));
				}
				else
				{
					throw ParseException(CurrentParseOffset, "Atleast one clause needed before |");
				}
				CurrentAndClauses.clear();
				continue;
			}
			std::unique_ptr<BNFRule> NewTerm = nullptr;
			if (CharData[CurrentParseOffset] == '(')
			{
				CurrentParseOffset += 1;
				NewTerm = ParseExpression(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
			}
			else if (CharData[CurrentParseOffset] == '[')
			{
				CurrentParseOffset += 1;
				NewTerm = ParseExpression(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
				NewTerm = std::unique_ptr<BNFRule_Range>(new BNFRule_Range(std::move(NewTerm), 0, 1));
			}
			else if (CharData[CurrentParseOffset] == '{')
			{
				CurrentParseOffset += 1;
				NewTerm = ParseExpression(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
				NewTerm = std::unique_ptr<BNFRule_Range>(new BNFRule_Range(std::move(NewTerm), 0, -1));
			}
			else
			{
				NewTerm = ParseTerm(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
			}
			if (CurrentParseOffset < DataSize)
			{
				if (CharData[CurrentParseOffset] == '+')
				{
					CurrentParseOffset += 1;
					NewTerm = std::unique_ptr<BNFRule>(new BNFRule_Range(std::move(NewTerm), 1, -1));
				}
				else if (CharData[CurrentParseOffset] == '*')
				{
					CurrentParseOffset += 1;
					NewTerm = std::unique_ptr<BNFRule>(new BNFRule_Range(std::move(NewTerm), 0, -1));
				}
				else if (CharData[CurrentParseOffset] == '{')
				{
					RangeSpecification NewRange = ParseRange(Data, DataSize, CurrentParseOffset, &CurrentParseOffset);
				}
			}
			CurrentAndClauses.push_back(std::move(NewTerm));
		}
		if (TopOrClause->size() == 0)
		{
			if (CurrentAndClauses.size() > 1)
			{
				ReturnValue = std::unique_ptr<BNFRule>(new BNFRule_AND(std::move(CurrentAndClauses)));
			}
			else if(CurrentAndClauses.size() == 1)
			{
				ReturnValue = std::move(CurrentAndClauses.front());
			}
			else
			{
				throw ParseException(CurrentParseOffset, "No valid expresssions parsed");
			}
		}
		else
		{
			if (CurrentAndClauses.size() > 1)
			{
				TopOrClause->AddAlternative(std::unique_ptr<BNFRule>(new BNFRule_AND(std::move(CurrentAndClauses))));
			}
			else if (CurrentAndClauses.size() == 1)
			{
				TopOrClause->AddAlternative(std::move(CurrentAndClauses.front()));
			}
			else
			{
				throw ParseException(CurrentParseOffset, "No valid expresssions parsed");
			}
			ReturnValue = std::move(TopOrClause);
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = CurrentParseOffset;
		}
		return(ReturnValue);
	}
	void BNFParser::ParseNamedRule(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset)
	{
		//börjar alltid på namnet 
		MBError Result = true;
		size_t CurrentParseOffset = ParseOffset;
		const char* CharData = (const char*)Data;
		size_t NameEnd = GetNextDelimiterPosition(G_BNFIdentifierDelimiters, Data, DataSize, CurrentParseOffset, &Result);
		if (!Result || NameEnd >= DataSize)
		{
			throw ParseException(CurrentParseOffset, "unexpected end of identifier");
		}
		std::string RuleName = std::string(((const char*)Data) + CurrentParseOffset, NameEnd - CurrentParseOffset);
		SkipWhitespace(Data, DataSize, NameEnd, &CurrentParseOffset);
		if (CurrentParseOffset >= DataSize || CharData[CurrentParseOffset] != '=')
		{
			throw ParseException(CurrentParseOffset, "expected = after new rule name");
		}
		CurrentParseOffset += 1;
		if (m_NameToRule.find(RuleName) == m_NameToRule.end())
		{
			m_CurrentTokenName += 1;
			m_NameToRule[RuleName] = m_CurrentTokenName;
			m_RuleToName[m_CurrentTokenName] = RuleName;
		}
	
		//std::unique_ptr<BNFRule_NamedRule> NewRule = std::unique_ptr<BNFRule_NamedRule>(new BNFRule_NamedRule())

		std::unique_ptr<BNFRule> RuleExpression = ParseExpression(Data,DataSize, CurrentParseOffset, &CurrentParseOffset);

		std::unique_ptr<BNFRule_NamedRule> NewRule = std::unique_ptr<BNFRule_NamedRule>(new BNFRule_NamedRule(m_NameToRule[RuleName], std::move(RuleExpression)));


		m_RuleIndexes[m_NameToRule[RuleName]] = m_Rules.size();
		m_Rules.push_back(std::move(NewRule));

		//std::vector<std::unique_ptr<BNFRule>> CurrentAndTerms;
		//while (CurrentParseOffset < DataSize)
		//{
		//	if (CharData[CurrentParseOffset] == ';')
		//	{
		//		break;
		//	}
		//	std::unique_ptr<BNFRule> NewRule = ParseExpression()
		//}
		if (OutOffset != nullptr)
		{
			*OutOffset = CurrentParseOffset;
		}
	}
	std::string p_GetName(NameToken TokenToConvert)
	{
		std::string ReturnValue;

		return(ReturnValue);
	}
	void BNFParser::PrintTree(SyntaxTree const& TreeToPrint,int CurrentDepth)
	{
		std::string Indentation = std::string(CurrentDepth*4, ' ');
		if (TreeToPrint.IsLiteral())
		{
			std::cout << Indentation << "Literal: " << TreeToPrint.GetLiteralData() << std::endl;
		}
		else
		{
			std::string Name = m_RuleToName[TreeToPrint.GetType()];
			std::cout<<Indentation << Name << ":" << std::endl;
			for (size_t i = 0; i < TreeToPrint.GetChildCount(); i++)
			{
				PrintTree(TreeToPrint[i], CurrentDepth + 1);
			}
		}
	}
	void BNFParser::PrintTree(SyntaxTree const& TreeToPrint)
	{
		PrintTree(TreeToPrint, 0);
	}
	std::string BNFParser::GetRuleName(NameToken Rule)
	{
		if (Rule == NameToken(ParseSyntaxTypes::LITERAL))
		{
			return("LITERAL");
		}
		return(m_RuleToName.at(Rule));
	}
	MBError BNFParser::InitializeRules(std::string const& RuleData)
	{
		MBError ReturnValue = true;
		size_t CurrentParseOffset = 0;
		void const* DataToParse = RuleData.data();
		size_t DataSize = RuleData.size();
		try
		{
			while (CurrentParseOffset < RuleData.size())
			{
				SkipWhitespace(DataToParse, DataSize, CurrentParseOffset, &CurrentParseOffset);
				ParseNamedRule(DataToParse, DataSize, CurrentParseOffset, &CurrentParseOffset);
				SkipWhitespace(DataToParse, DataSize, CurrentParseOffset, &CurrentParseOffset);
				if (!ReturnValue)
				{
					break;
				}
			}
		}
		catch (ParseException const& e)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Error at offset "+ std::to_string(e.GetParseOffset())+": "+ e.what();
		}


		return(ReturnValue);
	}
	BNFRule const& BNFParser::operator[](std::string const& RuleName)
	{
		return(*m_Rules.at(m_RuleIndexes.at(m_NameToRule.at(RuleName))));
	}
	BNFRule const& BNFParser::operator[](NameToken RuleValue)
	{
		return(*m_Rules.at(m_RuleIndexes.at(RuleValue)));
	}
	//END BNFParser








	//BEGIN UBJSON parsing
	enum class UBJSON_Type : uint8_t
	{
		Null = 'Z',
		NoOp = 'N',
		True = 'T',
		False = 'F',

		Int8 = 'i',
		UInt8 = 'U',
		Int16 = 'I',
		Int32 = 'l',
		Int64 = 'L',
		Float32 = 'd',
		Float64 = 'D',
		HighPrecision = 'H',

		TypeSpecifier = '$',
		CountSpecifier = '#',

		String = 'S',
		Char = 'C',
		ArrayBegin = '[',
		ArrayEnd = ']',
		ObjectBegin = '{',
		ObjectEnd = '}',
		INVALID,
	};
	JSONObject ParseUBJSON_Object(MBUtility::MBOctetInputStream* InputStream, UBJSON_Type* OutType);
	JSONObject ParseUBJSON_ObjectValue(UBJSON_Type ObjectType,MBUtility::MBOctetInputStream* InputStream);
	JSONObject ParseUBJSON_ArrayData(MBUtility::MBOctetInputStream* InputStream)
	{
		std::vector<JSONObject> ReturnValue;
		int Count = -1;
		UBJSON_Type ContainingType = UBJSON_Type::INVALID;
		intmax_t NumberOfElements = -1;
		while (true)
		{
			UBJSON_Type OutType = UBJSON_Type::INVALID;
			JSONObject NewValue = ParseUBJSON_Object(InputStream, &OutType);
			if (OutType == UBJSON_Type::TypeSpecifier)
			{
				InputStream->Read(&ContainingType, 1);
				//måste alltid finns en count om type är specificerad
				char buf;
				InputStream->Read(&buf, 1);
				NumberOfElements = ParseUBJSON_Object(InputStream, nullptr).GetIntegerData();

			}
			else if (OutType == UBJSON_Type::CountSpecifier)
			{
				NumberOfElements = ParseUBJSON_Object(InputStream, nullptr).GetIntegerData();
			}
			else if (OutType != UBJSON_Type::ArrayEnd && NumberOfElements == -1)
			{
				ReturnValue.push_back(std::move(NewValue));
			}
			else if(OutType == UBJSON_Type::ArrayEnd)
			{
				break;
			}
			if (NumberOfElements != -1)
			{
				for (size_t i = 0; i < NumberOfElements; i++)
				{
					if (ContainingType == UBJSON_Type::INVALID)
					{
						ReturnValue.push_back(ParseUBJSON_Object(InputStream, nullptr));
					}
					if (ContainingType == UBJSON_Type::Int8 || ContainingType == UBJSON_Type::Int16 || ContainingType == UBJSON_Type::Int32 || ContainingType == UBJSON_Type::Int64
						|| ContainingType == UBJSON_Type::UInt8)
					{
						size_t ElementSize = 0;
						if (ContainingType == UBJSON_Type::Int16)
						{
							ElementSize = 2;
						}
						else if (ContainingType == UBJSON_Type::Int64)
						{
							ElementSize = 8;
						}
						else if (ContainingType == UBJSON_Type::Int32)
						{
							ElementSize = 4;
						}
						else if (ContainingType == UBJSON_Type::Int8 || ContainingType == UBJSON_Type::UInt8)
						{
							ElementSize = 1;
						}
						else
						{
							throw std::runtime_error("Invalid integer type");
						}
						size_t TotalDataSize = ElementSize * NumberOfElements;
						std::string TotalData = std::string(TotalDataSize,0);
						size_t ReadBytes = InputStream->Read(TotalData.data(), TotalDataSize);
						if (ReadBytes != TotalDataSize)
						{
							throw std::runtime_error("Error parsing integer array, insufficent data");
						}
						return(TotalData);
					}
					else
					{
						ReturnValue.push_back(ParseUBJSON_ObjectValue(ContainingType, InputStream));
					}
				}
				break;
			}
		}
		return(ReturnValue);
	}
	std::map<std::string, JSONObject> ParseUBJSON_AggregateData(MBUtility::MBOctetInputStream* InputStream)
	{
		std::map<std::string, JSONObject> ReturnValue;
		UBJSON_Type ContainingType = UBJSON_Type::INVALID;
		intmax_t NumberOfElements = -1;
		while (true)
		{
			UBJSON_Type OutType = UBJSON_Type::INVALID;
			JSONObject NewValue = ParseUBJSON_Object(InputStream, &OutType);
			if (OutType == UBJSON_Type::TypeSpecifier)
			{
				InputStream->Read(&ContainingType, 1);
				//måste alltid finns en count om type är specificerad
				char buf;
				InputStream->Read(&buf, 1);
				NumberOfElements = ParseUBJSON_Object(InputStream, nullptr).GetIntegerData();

			}
			else if (OutType == UBJSON_Type::CountSpecifier)
			{
				NumberOfElements = ParseUBJSON_Object(InputStream, nullptr).GetIntegerData();
			}
			else if (OutType != UBJSON_Type::ObjectEnd && NumberOfElements == -1)
			{
				intmax_t NameSize = NewValue.GetIntegerData();
				std::string StringData = std::string(NameSize, 0);
				size_t ReadData = InputStream->Read(StringData.data(), NameSize);
				if (ReadData != NameSize)
				{
					throw std::runtime_error("String data missing");
				}
				ReturnValue[StringData] = ParseUBJSON_Object(InputStream, nullptr);
			}
			else if (OutType == UBJSON_Type::ObjectEnd)
			{
				break;
			}
			if (NumberOfElements != -1)
			{
				for (size_t i = 0; i < NumberOfElements; i++)
				{
					std::string StringData = ParseUBJSON_ObjectValue(UBJSON_Type::String, InputStream).GetStringData();
					if (ContainingType == UBJSON_Type::INVALID)
					{
						ReturnValue[StringData] = ParseUBJSON_Object(InputStream, nullptr);
					}
					else
					{
						ReturnValue[StringData] = ParseUBJSON_ObjectValue(ContainingType, InputStream);
					}
				}
				break;
			}
		}
		return(ReturnValue);
	}
	int64_t ParseUBJSON_Integer(MBUtility::MBOctetInputStream* InputStream, uint8_t IntegerSize)
	{
		int64_t ReturnValue;
		std::string IntegerBuffer = std::string(IntegerSize, 0);
		size_t ReadBytes = InputStream->Read(IntegerBuffer.data(), IntegerSize);
		if (ReadBytes != IntegerSize)
		{
			throw std::runtime_error("Insufficient bytes left in stream to read integer data");
		}
		//TODO supporta negativa värden.....
		ReturnValue = (int64_t) ParseBigEndianInteger(IntegerBuffer.data(), IntegerSize, 0, nullptr);
		return(ReturnValue);
	}
	JSONObject ParseUBJSON_ObjectValue(UBJSON_Type ObjectType, MBUtility::MBOctetInputStream* InputStream)
	{
		JSONObject ReturnValue;
		if (ObjectType == UBJSON_Type::Null)
		{
		}
		else if (ObjectType == UBJSON_Type::NoOp)
		{
			//kinda whack vet inte hur man ska tolka det
		}
		else if (ObjectType == UBJSON_Type::True)
		{
			ReturnValue = true;
		}
		else if (ObjectType == UBJSON_Type::False)
		{
			ReturnValue = false;
		}
		else if (ObjectType == UBJSON_Type::Int8)
		{
			ReturnValue = ParseUBJSON_Integer(InputStream, 1);
		}
		else if (ObjectType == UBJSON_Type::UInt8)
		{
			ReturnValue = ParseUBJSON_Integer(InputStream, 1);
		}
		else if (ObjectType == UBJSON_Type::Int16)
		{
			ReturnValue = ParseUBJSON_Integer(InputStream, 2);
		}
		else if (ObjectType == UBJSON_Type::Int32)
		{
			ReturnValue = ParseUBJSON_Integer(InputStream, 4);
		}
		else if (ObjectType == UBJSON_Type::Char)
		{
			uint8_t CharValue = 0;
			size_t ReadBytes = InputStream->Read(&CharValue, 1);
			if (ReadBytes < 1 || CharValue > 127)
			{
				throw std::runtime_error("Error parsing char character, invalid value or end of data");
			}
			ReturnValue = (intmax_t)CharValue;
		}
		else if (ObjectType == UBJSON_Type::String)
		{
			//TODO inneffektiv metod, men jajaj
			intmax_t StringLength = ParseUBJSON_Object(InputStream, nullptr).GetIntegerData();
			std::string StringData = std::string(StringLength, 0);
			size_t ReadBytes = InputStream->Read(StringData.data(), StringLength);
			if (ReadBytes != StringLength)
			{
				throw std::runtime_error("Missing data when parsing string");
			}
			ReturnValue = std::move(StringData);
		}
		else if (ObjectType == UBJSON_Type::ArrayBegin)
		{
			ReturnValue = ParseUBJSON_ArrayData(InputStream);
		}
		else if (ObjectType == UBJSON_Type::ObjectBegin)
		{
			ReturnValue = ParseUBJSON_AggregateData(InputStream);
		}
		else if (ObjectType == UBJSON_Type::Int64)
		{
			ReturnValue = ParseUBJSON_Integer(InputStream, 8);
		}
		else if (ObjectType == UBJSON_Type::Float32)
		{
			throw std::runtime_error("Parsing of binary float not yet supported");
		}
		else if (ObjectType == UBJSON_Type::Float64)
		{
			throw std::runtime_error("Parsing of binary float not yet supported");
		}
		return(ReturnValue);
	}
	JSONObject ParseUBJSON_Object(MBUtility::MBOctetInputStream* InputStream,UBJSON_Type* OutType)
	{
		JSONObject ReturnValue;
		UBJSON_Type ObjectType = UBJSON_Type::INVALID;
		size_t ReadBytes = InputStream->Read(&ObjectType, 1);
		if (ReadBytes != 1)
		{
			throw std::runtime_error("end of stream reached before end of object");
		}
		ReturnValue = ParseUBJSON_ObjectValue(ObjectType, InputStream);
		if (OutType != nullptr)
		{
			*OutType = ObjectType;
		}
		return(ReturnValue);
	}

	JSONObject ParseUBJSON(MBUtility::MBOctetInputStream* InputStream, MBError* OutError)
	{
		JSONObject ReturnValue;
		try
		{
			ReturnValue = ParseUBJSON_Object(InputStream,nullptr);
		}
		catch (std::exception const& e)
		{
			ReturnValue = JSONObject();
			*OutError = false;
			OutError->ErrorMessage = e.what();
		}
		return(ReturnValue);
	}
	void SerialiseUBJSON_Integer(MBUtility::MBOctetOutputStream& OutputStream, uint64_t IntegerToSerialise)
	{
		UBJSON_Type TypeToWrite;
		uint8_t IntegerSize;
		if (IntegerToSerialise < (1 << 8))
		{
			TypeToWrite = UBJSON_Type::UInt8;
			IntegerSize = 1;
		}
		else if (IntegerToSerialise < (1 << 16))
		{
			TypeToWrite = UBJSON_Type::Int16;
			IntegerSize = 2;
		}
		else if (IntegerToSerialise < (1ull << 32))
		{
			TypeToWrite = UBJSON_Type::Int32;
			IntegerSize = 4;
		}
		else
		{
			TypeToWrite = UBJSON_Type::Int64;
			IntegerSize = 8;
		}
		OutputStream.Write(&TypeToWrite, 1);
		uint8_t DataToWrite[8];
		WriteBigEndianInteger(DataToWrite, IntegerToSerialise, IntegerSize);
		OutputStream.Write(DataToWrite, IntegerSize);
	}
	void SerialiseUBJSON_Bool(MBUtility::MBOctetOutputStream& OutputStream, bool BooltoSerialise)
	{
		UBJSON_Type TypeToWrite;
		if (BooltoSerialise)
		{
			TypeToWrite = UBJSON_Type::True;
		}
		else
		{
			TypeToWrite = UBJSON_Type::False;
		}
		OutputStream.Write(&TypeToWrite, 1);
	}
	void SerialiseUBJSON_String(MBUtility::MBOctetOutputStream& OutputStream, std::string const& DataToSerialise)
	{
		//TODO obs detta är inte egentligen korrekt och rätt hacky, eftersom jag bara än så länge är intresserad av att läsa slippi filer, där jag implicit parsar in byte arrayer som strings
		//så kommer jag även serialisera strings till byte arrayer
		
		//heuristik för binär data: 0 finns
		if (DataToSerialise.find(char(0)) != DataToSerialise.npos)
		{
			UBJSON_Type BeginType = UBJSON_Type::ArrayBegin;
			OutputStream.Write(&BeginType, 1);
			UBJSON_Type TypeSpecifier = UBJSON_Type::TypeSpecifier;
			UBJSON_Type SpecifiedType = UBJSON_Type::UInt8;
			OutputStream.Write(&TypeSpecifier, 1);
			OutputStream.Write(&SpecifiedType, 1);
			UBJSON_Type CountSpecifier = UBJSON_Type::CountSpecifier;
			OutputStream.Write(&CountSpecifier, 1);
			SerialiseUBJSON_Integer(OutputStream, DataToSerialise.size());
			OutputStream.Write(DataToSerialise.data(), DataToSerialise.size());
		}
		else
		{
			UBJSON_Type BeginType = UBJSON_Type::String;
			OutputStream.Write(&BeginType, 1);
			SerialiseUBJSON_Integer(OutputStream, DataToSerialise.size());
			OutputStream.Write(DataToSerialise.data(), DataToSerialise.size());
		}

	}
	void SerialiseUBJSON_Array(MBUtility::MBOctetOutputStream& OutputStream, std::vector<JSONObject> const& ArrayToSerialise)
	{
		UBJSON_Type StartType = UBJSON_Type::ArrayBegin;
		OutputStream.Write(&StartType, 1);
		for (size_t i = 0; i < ArrayToSerialise.size(); i++)
		{
			SerialiseUBJSON(OutputStream,ArrayToSerialise[i]);
		}
		UBJSON_Type EndType = UBJSON_Type::ArrayEnd;
		OutputStream.Write(&EndType, 1);
	}
	void SerialiseUBJSON_Aggregate(MBUtility::MBOctetOutputStream& OutputStream, std::map<std::string,JSONObject> const& AgreggateToSerialise)
	{
		UBJSON_Type StartType = UBJSON_Type::ObjectBegin;
		OutputStream.Write(&StartType, 1);
		//TODO OBS turbo hack för slippi
		if (AgreggateToSerialise.find("raw") != AgreggateToSerialise.end()) 
		{
			std::string StringToWrite = "raw";
			SerialiseUBJSON_Integer(OutputStream, StringToWrite.size());
			OutputStream.Write(StringToWrite.data(), StringToWrite.size());
			SerialiseUBJSON(OutputStream, AgreggateToSerialise.at("raw"));
		}
		for (auto const& Element : AgreggateToSerialise)
		{
			std::string const& StringToWrite = Element.first;
			if (StringToWrite == "raw")
			{
				continue;
			}
			SerialiseUBJSON_Integer(OutputStream,StringToWrite.size());
			OutputStream.Write(StringToWrite.data(), StringToWrite.size());
			SerialiseUBJSON(OutputStream, Element.second);
		}
		UBJSON_Type EndType = UBJSON_Type::ObjectEnd;
		OutputStream.Write(&EndType, 1);
	}
	void SerialiseUBJSON_Null(MBUtility::MBOctetOutputStream& OutputStream)
	{
		UBJSON_Type TypeToWrite = UBJSON_Type::Null;
		OutputStream.Write(&TypeToWrite, 1);
	}
	void SerialiseUBJSON(MBUtility::MBOctetOutputStream& OutputStream, JSONObject const& ObjectToSerialise)
	{
		JSONObjectType CurrentType = ObjectToSerialise.GetType();
		if (CurrentType == JSONObjectType::Aggregate)
		{
			SerialiseUBJSON_Aggregate(OutputStream, ObjectToSerialise.GetMapData());
		}
		else if (CurrentType == JSONObjectType::Bool)
		{
			SerialiseUBJSON_Bool(OutputStream, ObjectToSerialise.GetBooleanData());
		}
		else if (CurrentType == JSONObjectType::Integer)
		{
			SerialiseUBJSON_Integer(OutputStream, ObjectToSerialise.GetIntegerData());
		}
		else if(CurrentType == JSONObjectType::String)
		{
			SerialiseUBJSON_String(OutputStream, ObjectToSerialise.GetStringData());
		}
		else if (CurrentType == JSONObjectType::Array)
		{
			SerialiseUBJSON_Array(OutputStream, ObjectToSerialise.GetArrayData());
		}
		else if (CurrentType == JSONObjectType::Null)
		{
			SerialiseUBJSON_Null(OutputStream);
		}
		OutputStream.Flush();
	}
	//END UBJSON parsing
};
