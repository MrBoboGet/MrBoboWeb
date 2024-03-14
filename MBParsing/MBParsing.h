#pragma once
#include <stdint.h>
#include <vector>
#include <MBUtility/MBErrorHandling.h>
#include <map>
#include <cstring>

#include <MBUtility/MBInterfaces.h>
#include <unordered_map>

#include <regex>


#include <MBUtility/Meta.h>
#include <variant>
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

	std::string ParseQuotedString(char QuoteCharacter, void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

	
	intmax_t ParseJSONInteger(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	bool ParseJSONBoolean(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

    size_t GetNextDelimiterPosition(const char* BeginDelimiters,const char* EndDelimiters,const void* DataToParse,size_t DataSize,size_t InOffset);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, const void* DataToParse,size_t DataSize, size_t InOffset, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, std::string const& DataToParse, size_t InOffset, MBError* OutError = nullptr);

    void WriteBigEndianInteger(MBUtility::MBOctetOutputStream& OutStream,uint64_t IntegerToWrite,char IntegerSize);
	void WriteBigEndianInteger(void* Buffer, uint64_t IntegerToWrite, char IntegerSize);
	void WriteBigEndianInteger(void* Buffer, uint64_t IntegerToWrite, char IntegerSize,size_t WriteOffset,size_t* OutWriteOffset);


    uint64_t ParseBigEndianInteger(MBUtility::MBOctetInputStream& InStream,unsigned char IntegerSize);
	uint64_t ParseBigEndianInteger(std::string const& DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset);
	uint64_t ParseBigEndianInteger(const void* DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset);

	double ParseBigEndianIEEE754Float(const void* DataToParse, char FloatSize, size_t ParseOffset, size_t* OutParseOffset);
	void WriteBigEndianIEEE754Float(void* OutBuffer,double FloatToWrite , char FloatSize,size_t WriteOffset, size_t* OutWriteOffset);

	std::vector<std::string> TokenizeText(std::string const& TextInput);
	//std::vector<std::string> TokenizeText(MBUtility::MBOctetInputStream* InputStream);
	
	bool CharIsNumerical(char CharToCheck);
	bool CharIsAlphabetical(char CharTocheck);

	class ParseException : public std::exception
	{
	private:
		std::string m_ErrorString;
		size_t m_ParseOffset = 0;
	public:
		ParseException(size_t ParseOffset, std::string ErrorString)
		{
			m_ParseOffset = ParseOffset;
			m_ErrorString = std::move(ErrorString);
		}
		size_t GetParseOffset() const
		{
			return(m_ParseOffset);
		};
		virtual const char* what() const noexcept override
		{
			return(m_ErrorString.c_str()); 
		}
	};


	typedef uint32_t NameToken;
	class SyntaxTree
	{
		uint32_t m_Type = 0;//null typen
		std::vector<SyntaxTree> m_SubComponents = {};
		std::string m_LiteralData = "";
	public:
		SyntaxTree(std::string LiteralString,NameToken Type);
		SyntaxTree(NameToken Type);
		SyntaxTree() {};


		bool IsLiteral() const;
		std::string const& GetLiteralData() const;

		NameToken GetType() const;
		int GetChildCount() const;
		SyntaxTree& operator[](size_t Index);
		SyntaxTree const& operator[](size_t Index) const;

		void AddChild(SyntaxTree ChildToAdd);
	};

	enum class ParseSyntaxTypes
	{
		ARRAY,
		LITERAL
	};


	class BNFRule
	{
	public:
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount,size_t TokenOffset,size_t* OutTokenOffset,bool* Success) const = 0;
		virtual ~BNFRule()
		{

		}
	};

	//class BNFParser;
	//class BNFRuleReference
	//{
	//	std::unique_ptr<BNFRule> m_LiteralRule = nullptr;
	//	std::string m_IndirectRule;
	//	BNFParser* m_AssociatedParser;
	//public:
	//	BNFRuleReference(std::unique_ptr<BNFRule> AssociatedRule);
	//	BNFRuleReference(NameToken AssociatedRule,BNFParser* AssociatedParser);
	//	BNFRule const* operator->() const;
	//};

	class BNFRule_Literal : public BNFRule
	{
	private:
		std::string m_LiteralToParse = "";
	public:
		BNFRule_Literal(std::string AssociatedLiteral);
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
	};
	class BNFRule_Regex : public BNFRule
	{
	private:
		std::regex m_RegexToMatch;
		bool m_Invalid = false;
	public:
		BNFRule_Regex(std::string const& RegexLiteral);
		bool IsValid() const;
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
	};
	class BNFRule_OR : public BNFRule
	{
	private:
		std::vector<std::unique_ptr<BNFRule>> m_Alternatives;
	public:
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
		void AddAlternative(std::unique_ptr<BNFRule> AlternativeToAdd);
		size_t size() const { return(m_Alternatives.size()); }
	};
	class BNFRule_AND : public BNFRule
	{
	private:
		std::vector<std::unique_ptr<BNFRule>> m_RulesToCombine;
	public:
		BNFRule_AND(std::vector<std::unique_ptr<BNFRule>> InitialRules)
		{
			m_RulesToCombine = std::move(InitialRules);
		}
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const  override;
		void AddElement(std::unique_ptr<BNFRule> AlternativeToAdd);
		size_t size() const { return(m_RulesToCombine.size()); }
	};
	class BNFRule_Range : public BNFRule
	{
	private:
		int m_MinCount = -1;
		int m_MaxCount = -1;
		std::unique_ptr<BNFRule> m_UnderylingRule;
	public:
		BNFRule_Range(std::unique_ptr<BNFRule> UnderlyingRule, int MinCount, int MaxCount);
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
	};
	class BNFRule_NamedRule : public BNFRule
	{
	private:
		static std::vector<SyntaxTree> Flatten(SyntaxTree const& TreeToFlatten);
		NameToken m_RuleName = 0;
		std::unique_ptr<BNFRule> m_UnderlyingRule;
	public:
		BNFRule_NamedRule(NameToken RuleName, std::unique_ptr<BNFRule> UnderlyingRule);
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
	};
	
	class BNFParser;
	class BNFRule_RuleReference : public BNFRule
	{
	private:
		NameToken m_RuleName;
		BNFParser* m_AssociatedParser = nullptr;
	public:
		BNFRule_RuleReference(NameToken RuleName, BNFParser* AssociatedParser)
		{
			m_RuleName = RuleName;
			m_AssociatedParser = AssociatedParser;
		}
		virtual SyntaxTree Parse(std::string const* TokenData, size_t TokenCount, size_t TokenOffset, size_t* OutTokenOffset, bool* OutError) const override;
	};

	class BNFParser
	{
	private:
		NameToken m_CurrentTokenName = 100;

		std::unordered_map<NameToken, std::string> m_RuleToName;
		std::unordered_map<std::string, NameToken> m_NameToRule;

		std::unordered_map<NameToken, size_t> m_RuleIndexes;
		std::vector<std::unique_ptr<BNFRule>> m_Rules;

		struct RangeSpecification
		{
			int Min = -1;
			int Max = -1;
		};


		void ParseNamedRule(const void* Data,size_t DataSize,size_t ParseOffset,size_t* OutOffse);
		RangeSpecification ParseRange(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset);

		void p_AddRuleName(std::string const& RuleName);

		std::unique_ptr<BNFRule> ParseTerm(const void* Data, size_t DataSize, size_t ParseOffset, size_t* OutOffset);
		std::unique_ptr<BNFRule> ParseExpression(const void* Data,size_t DataSize,size_t ParseOffset,size_t* OutOffset);
		//std::unique_ptr<BNFRule> ParseRule(std::string const* TokenData, size_t TokenOffset, size_t TokenCount, size_t* OutTokenOffset, MBError* OutError);
		std::string p_GetName(NameToken TokenToConvert);
		void PrintTree(SyntaxTree const& TreeToPrint, int CurrentDepth);
	public:

		std::string GetRuleName(NameToken Rule);

		MBError InitializeRules(std::string const& RuleData);

		BNFRule const& operator[](std::string const& RuleName);
		BNFRule const& operator[](NameToken RuleValue);

		//debug
		void PrintTree(SyntaxTree const& TreeToPrint);


	};
	
	
	
	
	
	
	
	
	
	
	
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
        Float,
		Null
	};
	class JSONObject
	{
	private:
		friend void swap(JSONObject& LeftObject, JSONObject& RightObject);
		JSONObjectType m_Type = JSONObjectType::Null;
        std::variant<intmax_t,bool,double,std::string,std::vector<JSONObject>,std::map<std::string,JSONObject>> m_Data;

        static constexpr int m_IndentSpaceWidth = 4;

		std::string p_ToString_Array() const;
		std::string p_ToString_Aggregate() const;
		std::string p_ToString_Atomic() const;

		std::string p_ToPrettyString_Array(int IndentLevel) const;
		std::string p_ToPrettyString_Aggregate(int IndentLevel) const;
		std::string p_ToPrettyString(int IndentLevel) const;



	public:
		JSONObject() = default;
		//JSONObject(JSONObjectType InitialType);
		JSONObject(JSONObject&& ObjectToSteal) noexcept = default;
		JSONObject(JSONObject const& ObjectToCopy) = default;
		JSONObject& operator=(JSONObject const& ObjectToCopy) = default;

		JSONObject(std::string StringInitializer);
		JSONObject(intmax_t IntegerInitializer);
		JSONObject(int IntegerInitializer);
		JSONObject(double FloatInitializer);
		JSONObject(bool BoolInitializer);
		JSONObject(const char* StringInitializer);
		JSONObject(std::vector<JSONObject> VectorInitializer);
		JSONObject(std::map<std::string,JSONObject> VectorInitializer);

		JSONObject(JSONObjectType InitialType);

		template<typename T>
		JSONObject(std::vector<T> const& Values)
		{
			m_Type = JSONObjectType::Array;
		    m_Data = std::vector<JSONObject>();
			std::vector<JSONObject>* DataPointer = &std::get<std::vector<JSONObject>>(m_Data);
			for (size_t i = 0; i < Values.size(); i++)
			{
				DataPointer->push_back(JSONObject(Values[i]));
			}
		}
        //mainly in use by MBObjectSpec, implements the
        //"meta programming" neccessary in order to convert types consisting 
        //of standard library containers and GetJSON objects. Would in an ideal
        //world be implemented with Concepts instead
        template<typename T>
        static JSONObject ToJSON(T const& TypeToConvert)
        {
            JSONObject ReturnValue;
            if constexpr(std::is_same<T,std::string>::value ||
                    std::is_same<T,int>::value || 
                    std::is_same<T,bool>::value)
            {
                ReturnValue = JSONObject(TypeToConvert);
            }
            else if constexpr(MBUtility::IsInstantiation<std::vector,T>::value)
            {
                std::vector<JSONObject> Content;       
                for(auto const& Value : TypeToConvert)
                {
                    Content.push_back(ToJSON(Value));
                }
                ReturnValue = JSONObject(std::move(Content));
            }
            else if constexpr(MBUtility::IsInstantiation<std::unordered_map,T>::value)
            {
                std::map<std::string,JSONObject> Content;       
                for(auto const& Value : TypeToConvert)
                {
                    Content[Value.first] = ToJSON(Value.second);
                }
                ReturnValue = JSONObject(std::move(Content));
            }
            else if constexpr(MBUtility::IsInstantiation<std::map,T>::value)
            {
                std::map<std::string,JSONObject> Content;       
                for(auto const& Value : TypeToConvert)
                {
                    Content[Value.first] = ToJSON(Value.second);
                }
                ReturnValue = JSONObject(std::move(Content));
            }
            else if constexpr(MBUtility::IsInstantiation<std::variant,T>::value)
            {
                ReturnValue = std::visit([](auto const& Value) -> JSONObject
                        {
                            return(ToJSON(Value));
                        });
            }
            else
            {
                ReturnValue = TypeToConvert.GetJSON();   
            }
            return(ReturnValue);
        }
        //mainly used by MBObjectSpec and kinda hacky, ideally it would be
        //implemented using proper templates
        template<typename T>
        static T FromJSON(JSONObject const& JSONToConvert)
        {
            T ReturnValue;
            if constexpr( std::is_same<T,bool>::value)
            {
                ReturnValue = JSONToConvert.GetBooleanData();
            }
            else if constexpr(std::is_same<T,std::string>::value)
            {
                ReturnValue = JSONToConvert.GetStringData();
            }
            else if constexpr( std::is_integral<T>::value)
            {
                ReturnValue = JSONToConvert.GetIntegerData();
            }
            else if constexpr( std::is_floating_point<T>::value)
            {
                ReturnValue = JSONToConvert.GetFloatData();
            }
            else if constexpr(MBUtility::IsInstantiation<std::vector,T>::value)
            {
                for(auto const& Member : JSONToConvert.GetArrayData())
                {
                    ReturnValue.push_back(FromJSON<typename std::remove_reference<decltype(ReturnValue.front())>::type>(Member));
                }
            }
            else if constexpr(MBUtility::IsInstantiation<std::map,T>::value)
            {
                for(auto const& Member : JSONToConvert.GetMapData())
                {
                    ReturnValue[Member.first] = FromJSON<typename std::remove_reference<decltype(ReturnValue.front())>::type>(Member);
                }
            }
            else if constexpr(MBUtility::IsInstantiation<std::unordered_map,T>::value)
            {
                for(auto const& Member : JSONToConvert.GetMapData())
                {
                    ReturnValue[Member.first] = FromJSON<decltype(ReturnValue.end()->second)>(Member);
                }
            }
            else
            {
                ReturnValue.FillObject(JSONToConvert);
            }
            return(ReturnValue);
        }

		JSONObjectType GetType() const;

		intmax_t GetIntegerData() const;
		std::string const& GetStringData() const;
		bool GetBooleanData() const;
		double GetFloatData() const;

		JSONObject& operator[](std::string const& AttributeName);
		JSONObject const& operator[](std::string const& AttributeName) const;

		std::map<std::string, JSONObject> const& GetMapData() const;
		std::map<std::string, JSONObject>& GetMapData();
		std::vector<JSONObject>& GetArrayData();
		std::vector<JSONObject>const& GetArrayData() const;
		
		bool HasAttribute(std::string const& AttributeName) const;
		JSONObject& GetAttribute(std::string const& AttributeName);
		JSONObject const& GetAttribute(std::string const& AttributeName) const;
		std::string ToString() const;
		std::string ToPrettyString() const;
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
                SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
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
					        SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
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
	    ResultObjectType ParseJSONNumber(void const* DataToParse, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr)
        {
            ResultObjectType ReturnValue;
            bool IsInteger = true;

            size_t ParseOffset = InOffset;
            MBError ParseError(true);
            const char* ObjectData = (const char*)DataToParse;
            size_t IntBegin = ParseOffset;
            size_t IntEnd = ParseOffset;
            while (IntEnd < DataSize)
            {
                if (!(('0' <= ObjectData[IntEnd] && ObjectData[IntEnd] <= '9') || ObjectData[IntEnd] == '-' || ObjectData[IntEnd] == '.' || ObjectData[IntEnd] == 'e' || ObjectData[IntEnd] == 'E'))
                {
                    break;
                }
                if(ObjectData[IntEnd] == '.' || ObjectData[IntEnd] == 'e' || ObjectData[IntEnd] == 'E')
                {
                    IsInteger = false;
                }
                IntEnd += 1;
            }
            try
            {
                if(IsInteger)
                {
                    ReturnValue = ResultObjectType(std::stoll(std::string(ObjectData +IntBegin, ObjectData+ IntEnd)));
                }
                else
                {
                    ReturnValue = ResultObjectType(std::stod(std::string(ObjectData+IntBegin,ObjectData+IntEnd)));
                }
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
            return ReturnValue;
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
					|| FirstCharacter == '7' || FirstCharacter == '8' || FirstCharacter == '9' || FirstCharacter == '0' || FirstCharacter == '-')
				{
					ReturnValue = ParseJSONNumber(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
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
    JSONObject ParseJSONObject(std::filesystem::path const& FilePath,MBError* OutError);
	JSONObject ParseJSONObject(const void* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset,MBError* OutError);
	JSONObject ParseJSONObject(std::string const& Data, size_t ParseOffset, size_t* OutOffset, MBError* OutError);


	JSONObject ParseUBJSON(MBUtility::MBOctetInputStream* InputStream,MBError* OutError);
	void SerialiseUBJSON(MBUtility::MBOctetOutputStream& OutputStream, JSONObject const& ObjectToSerialise);
};
