#pragma once
#include <vector>
#include <MBUtility/MBErrorHandling.h>
#include <map>
#include <cstring>
namespace MBParsing
{
	struct LinearParseState
	{
		size_t ParseOffset = 0;
		MBError ParseError = MBError(true);
	};


	//BASE64 grejer
	unsigned char Base64CharToBinary(unsigned char CharToDecode);
	char ByteToBASE64(uint8_t ByteToEncode);
	std::string Base64ToBinary(std::string const& Base64Data);
	std::string BASE64Decode(const void* CharactersToRead, size_t NumberOfCharacters);
	std::string BASE64Encode(const void* DataToEncode, size_t DataLength);
	std::string BASE64Decode(std::string const& DataToDecode);
	std::string BASE64Encode(std::string const& DataToEncode);
	//

	void UpdateParseState(size_t CurrentOffset, MBError& ErrorToMove, size_t* OutOffset, MBError* OutError);

	size_t FindSubstring(const void* Data, size_t DataSize, const void* Data2, size_t Data2Size,size_t ParseOffset);
	size_t FindSubstring(std::string const& StringToSearch, std::string const& Substring, size_t ParseOffset);

	void SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
	void SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset);
	std::string ParseQuotedString(void const* DataToParse,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	std::string ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	intmax_t ParseJSONInteger(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	bool ParseJSONBoolean(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, const void* DataToParse,size_t DataSize, size_t InOffset, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, std::string const& DataToParse, size_t InOffset, MBError* OutError = nullptr);

	void WriteBigEndianInteger(void* Buffer, uintmax_t IntegerToWrite, char IntegerSize);

	uintmax_t ParseBigEndianInteger(std::string const& DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset);
	uintmax_t ParseBigEndianInteger(const void* DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset);

	typedef uintmax_t JSONIntegerType;
	typedef double JSONFloatType;
	class JSONObject;

	std::string ToJason(std::string const& ValueToJason);

	//typedef std::vector<JSONObject> JSONVectorType;
	enum class JSONObjectType
	{
		Array,
		Aggregate,
		Integer,
		String,
		Bool,
		Null
	};
	class JSONObject
	{
	private:
		friend void swap(JSONObject& LeftObject, JSONObject& RightObject);

		void* p_CloneData() const;
		void p_FreeData();

		JSONObjectType m_Type = JSONObjectType::Null;
		void* m_ObjectData = nullptr;

		std::string p_ToString_Array() const;
		std::string p_ToString_Aggregate() const;
		std::string p_ToString_Atomic() const;
	public:
		JSONObject() {};
		//JSONObject(JSONObjectType InitialType);
		JSONObject(JSONObject&& ObjectToSteal) noexcept;
		JSONObject(JSONObject const& ObjectToCopy);

		JSONObject(std::string StringInitializer);
		JSONObject(intmax_t IntegerInitializer);
		JSONObject(bool BoolInitializer);
		JSONObject(const char* StringInitializer);
		JSONObject(std::vector<JSONObject> VectorInitializer);
		JSONObject(std::map<std::string,JSONObject> VectorInitializer);

		JSONObject(JSONObjectType InitialType);

		template<typename T>
		JSONObject(std::vector<T> const& Values)
		{
			m_Type = JSONObjectType::Array;
			m_ObjectData = new std::vector<JSONObject>();
			std::vector<JSONObject>* DataPointer = (std::vector<JSONObject>*)m_ObjectData;
			for (size_t i = 0; i < Values.size(); i++)
			{
				DataPointer->push_back(JSONObject(Values[i]));
			}
		}

		JSONObject& operator=(JSONObject ObjectToCopy);
		~JSONObject();

		JSONObjectType GetType() const { return(m_Type); };

		intmax_t GetIntegerData() const;
		std::string const& GetStringData() const;
		bool GetBooleanData() const;

		JSONObject& operator[](std::string const& AttributeName);

		std::map<std::string, JSONObject> const& GetMapData() const;
		std::map<std::string, JSONObject>& GetMapData();
		std::vector<JSONObject>& GetArrayData();
		std::vector<JSONObject>const& GetArrayData() const;
		
		bool HasAttribute(std::string const& AttributeName) const;
		JSONObject& GetAttribute(std::string const& AttributeName);
		JSONObject const& GetAttribute(std::string const& AttributeName) const;
		std::string ToString() const;
	};
	//en rule
	//RuleObject.Matches(void const* DataToparse,size_t DataSize);
	//RuleObject.ParseObject(void const* DataToParse,size_t DataSize,size_t ParseOffset,size_t* OutOffset,MBError* OutError);
	
	template<typename ObjectType>
	class EmptyJSONRule
	{
	public:
		bool Matches(void const* DataToparse, size_t DataSize,size_t ParseOffset) { return(false); };
		ObjectType ParseObject(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
		{
			MBError EvaluationError = false;
			EvaluationError.ErrorMessage = "Empty rule can't parse object";
			size_t ParseOffset = InOffset;
			UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
			return(ObjectType());
		};
	};

	template<typename ResultObjectType, typename RuleObject = EmptyJSONRule<ResultObjectType>>
	class JSONLikeParser
	{
	private:
		std::vector<RuleObject> m_ExtraRules = {};
		//static std::string sp_ParseString(void const* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset, MBError* OutError);
		//static JSONIntegerType sp_ParseInteger(void const* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset, MBError* OutError);
		//static bool sp_ParseBoolean(void const* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset, MBError* OutError);

		ResultObjectType m_ParseArray(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
		{
			MBError EvaluationError = true;
			const char* Data = (const char*)DataToParse;
			size_t ParseOffset = InOffset;
			ResultObjectType ReturnValue;
			SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
			if (ParseOffset<DataSize && Data[ParseOffset] == '[')
			{
				ParseOffset += 1;
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Error parsing array object: Array doesn't start with [";
			}
			if (EvaluationError)
			{
				std::vector<ResultObjectType> ArrayObjects = {};
				while (ParseOffset < DataSize)
				{
					SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
					if (Data[ParseOffset] == ']')
					{
						ParseOffset += 1;
						break;
					}
					ArrayObjects.push_back(ParseObject(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
					if (!EvaluationError)
					{
						break;
					}
					SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
					if (Data[ParseOffset] == ',')
					{
						ParseOffset += 1;
					}
				}
				if (EvaluationError)
				{
					ReturnValue = ResultObjectType(std::move(ArrayObjects));
				}
			}
			UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
			return(ReturnValue);
		}
		ResultObjectType m_ParseAggregate(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
		{
			MBError EvaluationError = true;
			const char* Data = (const char*)DataToParse;
			size_t ParseOffset = InOffset;
			ResultObjectType ReturnValue;
			std::map<std::string, ResultObjectType> ParsedObjects = {};
			SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
			if (ParseOffset < DataSize && Data[ParseOffset] == '{')
			{
				ParseOffset += 1;
			}
			if (EvaluationError)
			{
				while (ParseOffset < DataSize && Data[ParseOffset] != '}')
				{
					SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
					std::string NewAttributeName = ParseQuotedString(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
					SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
					if (ParseOffset < DataSize && Data[ParseOffset] == ':')
					{
						ParseOffset += 1;
					}
					else
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Error evaluating aggregate object: no value delimiter";
					}
					if (!EvaluationError)
					{
						break;
					}
					ParsedObjects[NewAttributeName] = ParseObject(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
					if (!EvaluationError)
					{
						break;
					}
					SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
					if (ParseOffset < DataSize)
					{
						if (Data[ParseOffset] == ',')
						{
							ParseOffset += 1;
						}
						else if (Data[ParseOffset] == '}')
						{
							break;
						}
					}
					else
					{
						EvaluationError = false;
						EvaluationError = "Error parsing aggregate object: no end delimiter found";
						break;
					}

				}
			}
			if (ParseOffset < DataSize && Data[ParseOffset] == '}')
			{
				ParseOffset += 1;
			}
			ReturnValue = ResultObjectType(std::move(ParsedObjects));
			UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
			return(ReturnValue);
		}
		//static JSONFloatType sp_ParseFloat(const void* DataToparse, size_t DataSize, size_t ParseOffset, size_t* OutOffset, MBError* OutError);
	public:
		ResultObjectType ParseObject(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
		{
			MBError EvaluationError = true;
			const char* Data = (const char*)DataToParse;
			size_t ParseOffset = InOffset;
			ResultObjectType ReturnValue;
			SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
			char FirstCharacter = 0;
			if (ParseOffset < DataSize)
			{
				FirstCharacter = Data[ParseOffset];
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "error parsing object: end of file";
			}
			if (EvaluationError)
			{
				if (FirstCharacter == '{')
				{
					ReturnValue = m_ParseAggregate(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
				}
				else if (FirstCharacter == '[')
				{
					ReturnValue = m_ParseArray(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
				}
				else if (FirstCharacter == 'T' || FirstCharacter == 't' || FirstCharacter == 'F' || FirstCharacter == 'f')
				{
					ReturnValue = ResultObjectType(ParseJSONBoolean(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
				}
				else if (FirstCharacter == '1' || FirstCharacter == '2' || FirstCharacter == '3' || FirstCharacter == '4' || FirstCharacter == '5' || FirstCharacter == '6'
					|| FirstCharacter == '7' || FirstCharacter == '8' || FirstCharacter == '9' || FirstCharacter == '0')
				{
					ReturnValue = ResultObjectType(ParseJSONInteger(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
				}
				else if (FirstCharacter == '\"')
				{
					ReturnValue = ResultObjectType(ParseQuotedString(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
				}
				else if (FirstCharacter == 'n')
				{
					if (DataSize - ParseOffset < 4)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Invalid character literal";
					}
					else if (std::memcmp(Data + ParseOffset, "null",4) != 0)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Invalid character literal";
					}
					else
					{
						//default constructor == null
						ReturnValue = ResultObjectType();
						ParseOffset += 4;
					}
				}
				else
				{
					bool Matched = false;
					for (RuleObject& Rule : m_ExtraRules)
					{
						if (Rule.Matches(Data, DataSize, ParseOffset))
						{
							Matched = true;
							ReturnValue = Rule.ParseObject(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
							break;
						}
					}
					if (!Matched)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Error parsing object: no rules matched";
					}
				}
			}
			UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
			return(ReturnValue);
		}
	};
	JSONObject ParseJSONObject(const void* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset,MBError* OutError);
	JSONObject ParseJSONObject(std::string const& Data, size_t ParseOffset, size_t* OutOffset, MBError* OutError);
};