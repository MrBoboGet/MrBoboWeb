#include "MBDBObjectScript.h"
#include <filesystem>
#include <fstream>
#include <MBSearchEngine/MBUnicode.h>
#include <MBParsing/MBParsing.h>
#include <deque>
namespace MBDB
{
	MBDB_Object::MBDB_Object(MBDB_Object const& ObjectToCopy)
	{
		m_AtomicData = ObjectToCopy.p_CopyData();
		m_Type = ObjectToCopy.m_Type;
		//if (ObjectToCopy.IsAggregate())
		//{
		//	for (auto& Keys : ObjectToCopy.m_Attributes)
		//	{
		//		m_Attributes[Keys.first] = std::unique_ptr<MBDB_Object>(new MBDB_Object(*Keys.second));
		//	}
		//	for (auto& ArrayObject : ObjectToCopy.m_ArrayObjects)
		//	{
		//		m_ArrayObjects.push_back(std::unique_ptr<MBDB_Object>(new MBDB_Object(*ArrayObject)));
		//	}
		//}
	}
	MBDB_Object::MBDB_Object(MBDB_Object&& ObjectToMove) noexcept
	{
		swap(*this, ObjectToMove);
	}
	MBDB_Object& MBDB_Object::operator=(MBDB_Object RightObject) noexcept
	{
		swap(RightObject, *this);
		return(*this);
	}
	//MBDB_Object& MBDB_Object::operator=(MBDB_Object&& RightObject) noexcept
	//{
	//	swap(RightObject, *this);
	//	return(*this);
	//}
	void MBDB_Object::p_FreeData() const
	{
		if (m_AtomicData == nullptr)
		{
			return;
		}
		if (m_Type == MBDBO_Type::Boolean)
		{
			delete (bool*)m_AtomicData;
		}
		else if (m_Type == MBDBO_Type::Integer)
		{
			delete (uintmax_t*)m_AtomicData;
		}
		else if (m_Type == MBDBO_Type::String)
		{
			delete(std::string*)m_AtomicData;
		}
		else if (m_Type == MBDBO_Type::AggregateObject)
		{
			delete((std::map<std::string, std::unique_ptr<MBDB_Object>>*)m_AtomicData);
		}
		else if (m_Type == MBDBO_Type::Array)
		{
			delete((std::vector<std::unique_ptr<MBDB_Object>>*)m_AtomicData);
		}
		else
		{
			assert(false);
		}
	}
	MBDB_Object::~MBDB_Object()
	{
		p_FreeData();
	}
	void swap(MBDB_Object& LeftObject, MBDB_Object& RightObject)
	{
		std::swap(LeftObject.m_Type, RightObject.m_Type);
		std::swap(LeftObject.m_AtomicData, RightObject.m_AtomicData);
	}
	bool MBDB_Object::IsAggregate() const
	{
		return(MBDBO_Type::AggregateObject == m_Type);
	}
	bool MBDB_Object::HasAttribute(std::string const& AttributeName) const
	{
		if (m_Type != MBDBO_Type::AggregateObject)
		{
			return(false);
		}
		std::map<std::string, std::unique_ptr<MBDB_Object>> const& AttributeMap = *(std::map<std::string, std::unique_ptr<MBDB_Object>>*)m_AtomicData;
		return(AttributeMap.find(AttributeName) != AttributeMap.end());
	}
	MBDB_Object& MBDB_Object::GetAttribute(std::string const& AttributeName)
	{
		std::map<std::string, std::unique_ptr<MBDB_Object>>& AttributeMap = *(std::map<std::string, std::unique_ptr<MBDB_Object>>*)m_AtomicData;

		return(*AttributeMap[AttributeName]);
	}
	MBDBO_Type MBDB_Object::GetType() const
	{
		return(m_Type);
	}
	std::string MBDB_Object::GetStringData() const
	{
		if (m_Type != MBDBO_Type::String)
		{
			return("");
		}
		return(*(std::string*)m_AtomicData);
	}
	intmax_t MBDB_Object::GetIntegerData() const
	{
		return(*(intmax_t*)m_AtomicData);
	}
	std::vector<MBDB_Object>& MBDB_Object::GetArrayData()
	{
		return(*(std::vector<MBDB_Object>*)m_AtomicData);
	}
	MBDB_Object::MBDB_Object(std::string const& StringData)
	{
		m_Type = MBDBO_Type::String;
		m_AtomicData = new std::string(StringData);
	}
	MBDB_Object::MBDB_Object(intmax_t IntegerData)
	{
		m_Type = MBDBO_Type::Integer;
		m_AtomicData = new intmax_t(IntegerData);
	}
	//MBDB_Object::MBDB_Object(std::vector<MBDB_Object>&& ArrayToMove)
	//{
	//	m_Type = MBDBO_Type::Array;
	//	m_AtomicData = new std::vector<MBDB_Object>();
	//	std::swap(*(std::vector<MBDB_Object>*)m_AtomicData, ArrayToMove);
	//}
	//MBDB_Object::MBDB_Object(std::vector<MBDB_Object> const& ArrayToCopy)
	//{
	//	m_Type = MBDBO_Type::Array;
	//	m_AtomicData = new std::vector<MBDB_Object>();
	//	std::vector<MBDB_Object>& ObjectData = *(std::vector<MBDB_Object>*)m_AtomicData;
	//	for (size_t i = 0; i < ArrayToCopy.size(); i++)
	//	{
	//		ObjectData.push_back(ArrayToCopy[i]);
	//	}
	//}
	void* MBDB_Object::p_CopyData() const
	{
		if (m_AtomicData == nullptr)
		{
			return(nullptr);
		}
		if (m_Type == MBDBO_Type::Boolean)
		{
			bool& AtomicValue = *((bool*)m_AtomicData);
			return(new bool(AtomicValue));
		}
		else if (m_Type == MBDBO_Type::Integer)
		{
			intmax_t& AtomicValue = *((intmax_t*)m_AtomicData);
			return(new intmax_t(AtomicValue));
		}
		else if (m_Type == MBDBO_Type::String)
		{
			std::string& AtomicValue = *((std::string*)m_AtomicData);
			return(new std::string(AtomicValue));
		}
		else if (m_Type == MBDBO_Type::AggregateObject)
		{
			std::map<std::string, std::unique_ptr<MBDB_Object>>* ReturnValue = new std::map<std::string, std::unique_ptr<MBDB_Object>>();
			std::map<std::string, std::unique_ptr<MBDB_Object>> const& ObjectMap = *((const std::map<std::string, std::unique_ptr<MBDB_Object>>*)m_AtomicData);
			for (auto& Key : ObjectMap)
			{
				(*ReturnValue)[Key.first] = std::unique_ptr<MBDB_Object>(new MBDB_Object(*(Key.second)));
			}
			return(ReturnValue);
		}
		else if (m_Type == MBDBO_Type::Array)
		{
			std::vector<std::unique_ptr<MBDB_Object>>* ReturnValue = new std::vector<std::unique_ptr<MBDB_Object>>();
			std::vector<std::unique_ptr<MBDB_Object>> const& ObjectArray = *((const std::vector<std::unique_ptr<MBDB_Object>>*)m_AtomicData);
			for (auto& ArrayObject : ObjectArray)
			{
				ReturnValue->push_back(std::unique_ptr<MBDB_Object>(new MBDB_Object(*ArrayObject)));
			}
			return(ReturnValue);
		}
		assert(false);
	}
	std::string MBDB_Object::p_ExtractTag(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::string ReturnValue = "";
		MBError ParseError = "";
		size_t ParseOffset = InOffset;
		p_SkipWhitespace(DataToParse, ParseOffset, &ParseOffset);
		if (ParseOffset >= DataToParse.size())
		{
			ParseError = false;
			ParseError.ErrorMessage = "No quoted string present";
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return(ReturnValue);
		}
		ReturnValue = p_ParseQuotedString(DataToParse, ParseOffset, &ParseOffset, &ParseError);
		if (!ParseError)
		{
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		p_SkipWhitespace(DataToParse, ParseOffset, &ParseOffset);
		if (ParseOffset >= DataToParse.size())
		{
			ParseError = false;
			ParseError.ErrorMessage = "No attribute value after attribute tag";
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		if (DataToParse[ParseOffset] != ':')
		{
			ParseError = false;
			ParseError.ErrorMessage = "No attribute value after attribute tag";
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		ParseOffset += 1;
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
	std::string MBDB_Object::p_ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::string ReturnValue = "";
		size_t ParseOffset = InOffset;
		MBError ParseError(true);

		if (ObjectData[ParseOffset] != '\"')
		{
			ParseError = false;
			ParseError.ErrorMessage = "String doesnt begin with \"";
			if (OutError != nullptr)
			{
				*OutError = ParseError;
			}
			return("");
		}
		ParseOffset += 1;
		size_t StringBegin = ParseOffset;
		while (ParseOffset < ObjectData.size())
		{
			size_t PreviousParseOffset = ParseOffset;
			ParseOffset = ObjectData.find('\"', ParseOffset);
			if (ParseOffset >= ObjectData.size())
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
			if (NumberOfBackslashes & 1 != 0)
			{
				ParseOffset += 1;
				continue;
			}
			else
			{
				ReturnValue = ObjectData.substr(StringBegin, ParseOffset - StringBegin);
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
	intmax_t MBDB_Object::p_ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
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
	void MBDB_Object::p_SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset)
	{
		p_SkipWhitespace(DataToParse.data(), DataToParse.size(), InOffset, OutOffset);
	}
	void MBDB_Object::p_SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset)
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
	MBDB_Object MBDB_Object::p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& TableExpression)
	{
		assert(false);
		return(MBDB_Object());
	}
	MBDB_Object MBDB_Object::p_ParseArrayObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;

		ReturnValue.m_Type = MBDBO_Type::Array;
		p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
		ParseOffset += 1;//gör så vi är på värdet efter den försat [
		ReturnValue.m_AtomicData = new std::vector<std::unique_ptr<MBDB_Object>>();
		std::vector<std::unique_ptr<MBDB_Object>>& ReturnValue_Arrays = *(std::vector<std::unique_ptr<MBDB_Object>>*)ReturnValue.m_AtomicData;
		while (ParseOffset < ObjectData.size())
		{
			p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
			if (ParseOffset >= ObjectData.size())
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Array doesnt end";
				break;
			}
			if (ObjectData[ParseOffset] == ']')
			{
				break;
			}
			std::unique_ptr<MBDB_Object> NewObject = std::unique_ptr<MBDB_Object>(new MBDB_Object());
			*NewObject = p_EvaluateObject(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
			if (!EvaluationError)
			{
				break;
			}
			ReturnValue_Arrays.push_back(std::move(NewObject));

			p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
			if (ParseOffset >= ObjectData.size())
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Array doesnt end";
				break;
			}
			if (ObjectData[ParseOffset] == ',')
			{
				ParseOffset += 1;
			}
			else if (ObjectData[ParseOffset] == ']')
			{
				break;
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Invalid Array Delimiter";
				break;
			}
		}
		if (EvaluationError)
		{
			ParseOffset += 1;
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;

		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_EvaluateDBObjectExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
		std::string ObjectPath = "";
		//DB_LoadObject
		if (ParseOffset < ObjectData.size())
		{
			if (ObjectData.substr(ParseOffset, 13) == "DB_LoadObject")
			{
				ParseOffset += 14; //1 för parantesen
				ObjectPath = p_ParseQuotedString(ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
				ParseOffset += 1;
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Invalid DBObject Expression";
			}
		}
		else
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Invalid DBObject Expression";
		}
		//ReturnValue.LoadObject(

		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_ParseDatabaseExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;

		p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
		MBDB::MBDBObjectScript_ParsingContext ParsingContext;
		MBDBObjectScript_Statement DatabaseExpression = ParsingContext.ParseStatement(ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
		ParseOffset += 1;
		if (EvaluationError)
		{
			MBDB::MBDBObjectScript_ExecutionContext ExecutionContext;
			ReturnValue = ExecutionContext.EvaluateStatement(EvaluationInfo, DatabaseExpression, &EvaluationError);
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_ParseStaticAtomicObject(std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;

		char FirstCharacter = ObjectData[ParseOffset];
		if (FirstCharacter == 't' || FirstCharacter == 'T')
		{
			ParseOffset += 4;
			ReturnValue.m_Type = MBDBO_Type::Boolean;
			ReturnValue.m_AtomicData = new bool(true);
		}
		else if (FirstCharacter == 'f' || FirstCharacter == 'F')
		{
			ParseOffset += 5;
			ReturnValue.m_Type = MBDBO_Type::Boolean;
			ReturnValue.m_AtomicData = new bool(true);
		}
		else if (FirstCharacter == '\"')
		{
			ReturnValue.m_Type = MBDBO_Type::String;
			ReturnValue.m_AtomicData = new std::string(p_ParseQuotedString(ObjectData, ParseOffset, &ParseOffset, &EvaluationError));
		}
		else
		{
			//TODO lägg till support för floats
			ReturnValue.m_Type = MBDBO_Type::Integer;
			ReturnValue.m_AtomicData = new intmax_t(p_ParseJSONInteger(ObjectData, ParseOffset, &ParseOffset, &EvaluationError));
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_ParseAggregateObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		ReturnValue.m_Type = MBDBO_Type::AggregateObject;
		if (ObjectData[ParseOffset] == '{')
		{
			ParseOffset += 1;
		}
		ReturnValue.m_AtomicData = new std::map<std::string, std::unique_ptr<MBDB_Object>>();
		std::map<std::string, std::unique_ptr<MBDB_Object>>& ReturnValue_Map = *(std::map<std::string, std::unique_ptr<MBDB_Object>>*)ReturnValue.m_AtomicData;
		while (ParseOffset < ObjectData.size())
		{
			p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
			if (ParseOffset >= ObjectData.size())
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Invalid end of object";
				break;
			}
			if (ObjectData[ParseOffset] == '}')
			{
				break;
			}
			std::string NewAttributeTag = p_ExtractTag(ObjectData, ParseOffset, &ParseOffset);
			p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
			if (ParseOffset >= ObjectData.size())
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Tag has no attribute value";
				break;
			}
			std::unique_ptr<MBDB_Object> NewObject = std::unique_ptr<MBDB_Object>(new MBDB_Object());
			*NewObject = p_EvaluateObject(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
			if (!EvaluationError)
			{
				break;
			}
			ReturnValue_Map[NewAttributeTag] = std::move(NewObject);
			p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
			if (ParseOffset >= ObjectData.size())
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Invalid Delimiter";
				break;
			}
			if (ObjectData[ParseOffset] == ',')
			{
				ParseOffset += 1;
				continue;
			}
			if (ObjectData[ParseOffset] == '}')
			{
				break;
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Invalid delimiter";
				break;
			}
		}
		ParseOffset += 1;
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDB_Object::p_EvaluateObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		if (ObjectData.size() == 0)
		{
			if (OutError != nullptr)
			{
				*OutError = false;
				OutError->ErrorMessage = "Cant parse empty string as object";
			}
			return(ReturnValue);
		}
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
		if (ObjectData[ParseOffset] == '{')
		{
			ReturnValue = p_ParseAggregateObject(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
		}
		else if (ObjectData[ParseOffset] == '[')
		{
			ReturnValue = p_ParseArrayObject(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
		}
		else if (ObjectData[ParseOffset] == 'D')
		{
			ReturnValue = p_ParseDatabaseExpression(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
		}
		else
		{
			ReturnValue = p_ParseStaticAtomicObject(ObjectData, ParseOffset, &ParseOffset, &EvaluationError); //base caset i recursionen
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	std::string MBDB_Object::p_AggregateToJSON() const
	{
		std::string ReturnValue = "{";
		std::map<std::string, std::unique_ptr<MBDB_Object>> const& Attributes = *(std::map<std::string, std::unique_ptr<MBDB_Object>>*)m_AtomicData;
		for (auto& Key : Attributes)
		{
			ReturnValue += ::ToJason(Key.first);
			ReturnValue += ":";
			ReturnValue += Key.second->ToJason();
			ReturnValue += ",";
		}
		ReturnValue.resize(ReturnValue.size() - 1);
		ReturnValue += '}';
		return(ReturnValue);
	}
	std::string MBDB_Object::p_ArrayToJSON() const
	{
		std::string ReturnValue = "[";
		std::vector<std::unique_ptr<MBDB_Object>> const& Arrays = *(std::vector<std::unique_ptr<MBDB_Object>>*)m_AtomicData;
		for (size_t i = 0; i < Arrays.size(); i++)
		{
			ReturnValue += Arrays[i]->ToJason();
			if (i + 1 < Arrays.size())
			{
				ReturnValue += ",";
			}
		}
		ReturnValue += "]";
		return(ReturnValue);
	}
	std::string MBDB_Object::p_AtomicToJSON() const
	{
		std::string ReturnValue = "";
		if (m_Type == MBDBO_Type::Boolean)
		{
			bool BoolValue = *((bool*)m_AtomicData);
			if (BoolValue)
			{
				ReturnValue = "true";
			}
			else
			{
				ReturnValue = "false";
			}
		}
		else if (m_Type == MBDBO_Type::Integer)
		{
			uintmax_t IntegerValue = *((uintmax_t*)m_AtomicData);
			ReturnValue = std::to_string(IntegerValue);
		}
		else if (m_Type == MBDBO_Type::String)
		{
			std::string& StringReference = *((std::string*)m_AtomicData);
			ReturnValue = ::ToJason(StringReference);
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}
	std::string MBDB_Object::ToJason() const
	{
		std::string ReturnValue = "";
		if (m_Type == MBDBO_Type::Array)
		{
			ReturnValue = p_ArrayToJSON();
		}
		else if(m_Type == MBDBO_Type::AggregateObject)
		{
			ReturnValue = p_AggregateToJSON();
		}
		else
		{
			ReturnValue = p_AtomicToJSON();
		}
		return(ReturnValue);
	}
	MBError MBDB_Object::LoadObject(MBDBO_EvaluationInfo& CurrentEvaluationInfo, std::string const& ObjectFilepath)
	{
		std::ifstream InputFile = std::ifstream(ObjectFilepath, std::ios::in | std::ios::binary);
		size_t FileSize = std::filesystem::file_size(ObjectFilepath);
		std::string FileData = std::string(FileSize, 0);
		InputFile.read(FileData.data(), FileSize);
		MBError EvaluationError(true);
		MBDB_Object EvaluatedObject = p_EvaluateObject(CurrentEvaluationInfo, FileData, 0, nullptr, &EvaluationError);
		if (EvaluationError)
		{
			swap(*this, EvaluatedObject);
		}
		return(EvaluationError);
	}
	MBError MBDB_Object::LoadObject(std::string const& ObjectFilepath, std::string const& AssociatedUser, MBDB::MrBoboDatabase* AssociatedDatabase)
	{
		MBDBO_EvaluationInfo EvaluationInfo;
		EvaluationInfo.AssociatedDatabase = AssociatedDatabase;
		EvaluationInfo.EvaluatingUser = AssociatedUser;
		EvaluationInfo.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(ObjectFilepath).parent_path());
		EvaluationInfo.ObjectDirectory += '/';
		return(LoadObject(EvaluationInfo,ObjectFilepath));
	}
	//MBDBObjectScript_ParsingContext::
	std::string MBDBObjectScript_ParsingContext::p_ParseIdentifier(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::string ReturnValue = "";
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		const char* DataToParse = (const char*)Data;
		MBParsing::SkipWhitespace(DataToParse, DataSize, ParseOffset, &ParseOffset);
		if (ParseOffset < DataSize)
		{
			char FirstIdentifierCharacter = DataToParse[ParseOffset];
			if (FirstIdentifierCharacter == '\"')
			{
				ReturnValue = '"' + MBParsing::ParseQuotedString(DataToParse,DataSize,ParseOffset,&ParseOffset,&EvaluationError)+'"';
			}
			else
			{
				size_t IdentifierEnd = MBParsing::GetNextDelimiterPosition(m_IdentifierDelimiters, DataToParse, DataSize, ParseOffset, &EvaluationError);
				ReturnValue = std::string(DataToParse+ParseOffset, IdentifierEnd - ParseOffset);
				ParseOffset = IdentifierEnd;
			}
		}
		
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	std::string MBDBObjectScript_ParsingContext::p_ParseIdentifier(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		return(p_ParseIdentifier(DataToParse.data(), DataToParse.size(), InOffset, OutOffset, OutError));
	}
	bool MBDBObjectScript_ParsingContext::p_IdentifierIsLiteral(std::string const& StringToEvaluate)
	{
		bool ReturnValue = false;
		if (StringToEvaluate.size() == 0)
		{
			return(ReturnValue);
		}
		ReturnValue = ReturnValue || (StringToEvaluate[0] == '\"');
		ReturnValue = ReturnValue || (std::find(m_NumberChars.begin(), m_NumberChars.end(), StringToEvaluate[0]) != m_NumberChars.end());
		return(ReturnValue);
	}
	MBDBObjectScript_Statement MBDBObjectScript_ParsingContext::p_GetLiteralStatement(std::string const& StringToEvaluate)
	{
		MBDBObjectScript_Statement ReturnValue;
		ReturnValue.m_Type = MBDBObjectScript_StatementType::Literal;
		MBDBObjectScript_LiteralStatementData* NewStatementData = new MBDBObjectScript_LiteralStatementData();
		ReturnValue.m_StatementData = NewStatementData;
		if (StringToEvaluate.size() == 0)
		{
			return(ReturnValue);
		}
		else if (StringToEvaluate[0] == '\"')
		{
			if (StringToEvaluate.size() < 2)
			{
				return(ReturnValue);
			}
			NewStatementData->Type = MBDBObjectScript_LiteralType::String;
			NewStatementData->StringData = StringToEvaluate.substr(1, StringToEvaluate.size() - 2);
		}
		else if (std::find(m_NumberChars.begin(), m_NumberChars.end(), StringToEvaluate[0]) != m_NumberChars.end())
		{
			//TODO support för floats
			NewStatementData->Type = MBDBObjectScript_LiteralType::Integer;
			//TODO kanske ha lite mer error hantering här
			NewStatementData->IntegerData = std::stoi(StringToEvaluate);
		}
		return(ReturnValue);
	}
	MBDBObjectScript_Statement MBDBObjectScript_ParsingContext::ParseStatement(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		MBDBObjectScript_Statement ReturnValue;
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		const char* DataToParse = (const char*)Data;

		std::vector<char> ParsedBinaryOperators = {};
		std::deque<MBDBObjectScript_Statement> ParsedStatements = {};
		while (ParseOffset < DataSize)
		{
			if (!EvaluationError)
			{
				break;
			}
			MBParsing::SkipWhitespace(DataToParse, DataSize, ParseOffset, &ParseOffset);
			if (std::find(m_StatementDelimiters.begin(), m_StatementDelimiters.end(), DataToParse[ParseOffset]) != m_StatementDelimiters.end())
			{
				if (DataToParse[ParseOffset] == '(')
				{
					ParseOffset += 1;
					ParsedStatements.push_back(ParseStatement(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
					//ANTAGANDE ParseStatement slutar inte som vanliga grejer 1 efter den är klar, utan på själva delimiter
					ParseOffset += 1;
					continue;
				}
				else if (DataToParse[ParseOffset] == '.')
				{
					ParseOffset += 1;
					if (ParsedStatements.size() == 0)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Dot before object expression";
						break;
					}
					else
					{
						std::string ObjectFieldIdentifier = p_ParseIdentifier(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
						if (!EvaluationError)
						{
							break;
						}
						MBDBObjectScript_Statement FieldStatement;
						MBDBObjectScript_ObjectFieldStatementData* NewObjectData = new MBDBObjectScript_ObjectFieldStatementData();
						FieldStatement.m_Type = MBDBObjectScript_StatementType::ObjectField;
						FieldStatement.m_StatementData = NewObjectData;
						NewObjectData->FieldIdentifier = ObjectFieldIdentifier;
						NewObjectData->ObjectToEvaluate = std::move(ParsedStatements.back());
						ParsedStatements.pop_back();
						ParsedStatements.push_back(std::move(FieldStatement));
						continue;
					}
				}
				else if (DataToParse[ParseOffset] == '[')
				{
					if (ParsedStatements.size() == 0)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "[] without associated object";
						break;
					}
					MBDBObjectScript_Statement NewStatement;
					MBDBObjectScript_FunctionStatementData* NewStatementData = new MBDBObjectScript_FunctionStatementData();
					NewStatementData->FunctionIdentifier = "[]";
					NewStatementData->FunctionArguments.push_back(std::move(ParsedStatements.back()));
					ParsedStatements.pop_back();
					//NewStatementData->FunctionArguments = p_ParseFunctionArguments(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
					std::vector<MBDBObjectScript_Statement> Arguments = p_ParseFunctionArguments(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError); 
					for (size_t i = 0; i < Arguments.size(); i++)
					{
						NewStatementData->FunctionArguments.push_back(std::move(Arguments[i]));
					}
					NewStatement.m_StatementData = NewStatementData;
					NewStatement.m_Type = MBDBObjectScript_StatementType::FunctionCall;
					ParsedStatements.push_back(std::move(NewStatement));
				}
				else
				{
					//ParseOffset += 1;
					break;
				}
			}
			if (std::find(m_BinaryOperators.begin(), m_BinaryOperators.end(), DataToParse[ParseOffset]) != m_BinaryOperators.end())
			{
				ParsedBinaryOperators.push_back(DataToParse[ParseOffset]);
				ParseOffset += 1;
				if (ParseOffset >= DataSize)
				{
					EvaluationError = false;
					EvaluationError.ErrorMessage = "Binary operator without operand";
					break;
				}
			}
			std::string NextIdentifier = p_ParseIdentifier(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
			if (!EvaluationError)
			{
				break;
			}
			if (p_IdentifierIsLiteral(NextIdentifier))
			{
				ParsedStatements.push_back(p_GetLiteralStatement(NextIdentifier));
			}
			else if (m_Identifiers.find(NextIdentifier) != m_Identifiers.end())
			{
				MBDBObjectScript_Identifier const& IdentifierType = m_Identifiers[NextIdentifier];
				if (IdentifierType.Type == MBDBObjectScript_IdentifierType::Function)
				{
					MBDBObjectScript_Statement NewStatement;
					MBDBObjectScript_FunctionStatementData* NewStatementData = new MBDBObjectScript_FunctionStatementData();
					NewStatementData->FunctionIdentifier = NextIdentifier;
					NewStatementData->FunctionArguments = p_ParseFunctionArguments(DataToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
					NewStatement.m_StatementData = NewStatementData;
					NewStatement.m_Type = MBDBObjectScript_StatementType::FunctionCall;
					ParsedStatements.push_back(std::move(NewStatement));
				}
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Identifier Unknown";
				break;
			}
			if (ParsedBinaryOperators.size() > 0 && ParsedStatements.size() > 0)
			{
				std::string Operand = std::string(1,ParsedBinaryOperators.back());
				ParsedBinaryOperators.pop_back();
				//ANTAGANDE Kan aldrig komma flera än 1 operand här, annars är något fel
				MBDBObjectScript_Statement NewStatement;
				MBDBObjectScript_FunctionStatementData* NewStatementData = new MBDBObjectScript_FunctionStatementData();
				NewStatementData->FunctionIdentifier = Operand;
				NewStatementData->FunctionArguments.push_back(ParsedStatements.front());
				ParsedStatements.pop_front();
				NewStatementData->FunctionArguments.push_back(ParsedStatements.front());
				ParsedStatements.pop_front();
				NewStatement.m_StatementData = NewStatementData;
				NewStatement.m_Type = MBDBObjectScript_StatementType::FunctionCall;
				ParsedStatements.push_back(std::move(NewStatement));
			}
		}
		if (ParsedStatements.size() != 1)
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Unkown parsing error: more than one statement before return";
		}
		else
		{
			ReturnValue = std::move(ParsedStatements.front());
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDBObjectScript_Statement MBDBObjectScript_ParsingContext::ParseStatement(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		return(ParseStatement(DataToParse.data(), DataToParse.size(), InOffset, OutOffset, OutError));
	}
	std::vector<MBDBObjectScript_Statement> MBDBObjectScript_ParsingContext::p_ParseFunctionArguments(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::vector<MBDBObjectScript_Statement> ReturnValue = {};
		MBError EvaluationError(true);
		size_t ParseOffset = InOffset;
		const char* DataToParse = (const char*)Data;

		//ANTAGANDE, funktionen börjar på en (
		ParseOffset += 1;
		while (ParseOffset < DataSize)
		{
			MBParsing::SkipWhitespace(DataToParse, ParseOffset, &ParseOffset);
			if (DataToParse[ParseOffset] == ')')
			{
				ParseOffset += 1;
				break;
			}
			if (DataToParse[ParseOffset] == ',')
			{
				ParseOffset += 1;
			}
			ReturnValue.push_back(ParseStatement(Data, DataSize, ParseOffset, &ParseOffset, &EvaluationError));
			if (!EvaluationError)
			{
				ReturnValue = std::vector<MBDBObjectScript_Statement>();
				break;
			}
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	std::vector<MBDBObjectScript_Statement> MBDBObjectScript_ParsingContext::p_ParseFunctionArguments(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		return(p_ParseFunctionArguments(DataToParse.data(), DataToParse.size(), InOffset, OutOffset, OutError));
	}

	//MBDBObjectScript_Statement
	MBDBObjectScript_Statement::MBDBObjectScript_Statement(MBDBObjectScript_Statement const& ObjectToCopy)
	{
		m_Type = ObjectToCopy.m_Type;
		m_StatementData = ObjectToCopy.p_CopyData();
	}
	MBDBObjectScript_Statement::MBDBObjectScript_Statement(MBDBObjectScript_Statement&& ObjectToCopy) noexcept
	{
		swap(*this, ObjectToCopy);
	}
	MBDBObjectScript_Statement& MBDBObjectScript_Statement::operator=(MBDBObjectScript_Statement ObjectToCopy)
	{
		swap(*this, ObjectToCopy);
		return(*this);
	}
	MBDBObjectScript_Statement::~MBDBObjectScript_Statement()
	{
		p_FreeData();
	}
	void MBDBObjectScript_Statement::p_FreeData() const
	{
		if (m_StatementData == nullptr)
		{
			return;
		}
		if (m_Type == MBDBObjectScript_StatementType::FunctionCall)
		{
			delete ((MBDBObjectScript_FunctionStatementData*)m_StatementData);
			return;
		}
		else if (m_Type == MBDBObjectScript_StatementType::Literal)
		{
			delete((MBDBObjectScript_LiteralStatementData*)m_StatementData);
			return;
		}
		else if (m_Type == MBDBObjectScript_StatementType::ObjectField)
		{
			delete((MBDBObjectScript_ObjectFieldStatementData*)m_StatementData);
			return;
		}
		else if(m_Type == MBDBObjectScript_StatementType::Null)
		{
			return;
		}
		assert(false);
	}
	void* MBDBObjectScript_Statement::p_CopyData() const
	{
		void* ReturnValue = nullptr;
		if (m_Type == MBDBObjectScript_StatementType::FunctionCall)
		{
			MBDBObjectScript_FunctionStatementData* NewStatementData = new MBDBObjectScript_FunctionStatementData();
			MBDBObjectScript_FunctionStatementData& ObjectData = *(MBDBObjectScript_FunctionStatementData*)m_StatementData;
			NewStatementData->FunctionIdentifier = ObjectData.FunctionIdentifier;
			for (size_t i = 0; i < ObjectData.FunctionArguments.size(); i++)
			{
				NewStatementData->FunctionArguments.push_back(ObjectData.FunctionArguments[i]);
			}
			ReturnValue = NewStatementData;
		}
		else if (m_Type == MBDBObjectScript_StatementType::Literal)
		{
			MBDBObjectScript_LiteralStatementData* NewStatementData = new MBDBObjectScript_LiteralStatementData();
			MBDBObjectScript_LiteralStatementData& ObjectData = *(MBDBObjectScript_LiteralStatementData*)m_StatementData;
			*NewStatementData = ObjectData;
			ReturnValue = NewStatementData;
		}
		else if (m_Type == MBDBObjectScript_StatementType::ObjectField)
		{
			MBDBObjectScript_ObjectFieldStatementData* NewStatementData = new MBDBObjectScript_ObjectFieldStatementData();
			MBDBObjectScript_ObjectFieldStatementData& ObjectData = *(MBDBObjectScript_ObjectFieldStatementData*)m_StatementData;
			*NewStatementData = ObjectData;
			ReturnValue = NewStatementData;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDBObjectScript_Statement::p_GetLiteralObject() const
	{
		MBDB_Object ReturnValue;
		MBDBObjectScript_LiteralStatementData& ObjectData = *(MBDBObjectScript_LiteralStatementData*)m_StatementData;
		if (ObjectData.Type == MBDBObjectScript_LiteralType::Integer)
		{
			ReturnValue = MBDB_Object(ObjectData.IntegerData);
		}
		else if (ObjectData.Type == MBDBObjectScript_LiteralType::String)
		{
			ReturnValue = MBDB_Object(ObjectData.StringData);
		}
		return(ReturnValue);
	}
	void swap(MBDBObjectScript_Statement& LeftStatement, MBDBObjectScript_Statement& RightStatement) noexcept
	{
		std::swap(LeftStatement.m_Type, RightStatement.m_Type);
		std::swap(LeftStatement.m_StatementData, RightStatement.m_StatementData);
	}

	//MBDBObjectScript_ExecutionContext
	MBDB_Object MBDBObjectScript_ExecutionContext::p_DBLoadObject(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError)
	{
		MBError EvaluationError = false;
		MBDB_Object ReturnValue;
		if (Arguments.size() != 1)
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Invalid number of arguments";
		}
		if (!EvaluationError)
		{
			MBDB_Object ArgumentValue = EvaluateStatement(EvaluationInfo, Arguments[0], &EvaluationError);
			if (EvaluationError)
			{
				if (ArgumentValue.GetType() != MBDBO_Type::String)
				{
					EvaluationError = false;
					EvaluationError.ErrorMessage = "Function requires string as argument";
				}
				else
				{
					std::string ObjectPath = ArgumentValue.GetStringData();
					MBDBO_EvaluationInfo NewInfoToUse = EvaluationInfo;
					std::filesystem::path ObjectFilePath = std::filesystem::path(ObjectFilePath);
					NewInfoToUse.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(ObjectPath).parent_path());
					if (NewInfoToUse.ObjectDirectory == "")
					{
						NewInfoToUse.ObjectDirectory = "./";
					}
					if (NewInfoToUse.ObjectDirectory.back() != '/')
					{
						NewInfoToUse.ObjectDirectory += '/';
					}
					ReturnValue.LoadObject(NewInfoToUse, ArgumentValue.GetStringData());
				}
			}
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDBObjectScript_ExecutionContext::p_EvaluateObjectFunction(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError)
	{
		MBError EvaluationError = true;
		MBDB_Object ReturnValue;


		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}
	MBDB_Object MBDBObjectScript_ExecutionContext::p_AddObjects(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError)
	{
		MBDB_Object ReturnValue;
		MBError EvaluationError(true);
		if (Arguments.size() != 2)
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Invalid number of arguments for \"+\" binary operation";
			return(ReturnValue);
		}
		MBDB_Object LeftOperand = EvaluateStatement(EvaluationInfo, Arguments[0], &EvaluationError);
		if (!EvaluationError)
		{
			return(ReturnValue);
		}
		MBDB_Object RightOperand = EvaluateStatement(EvaluationInfo, Arguments[1], &EvaluationError);
		if (!EvaluationError)
		{
			return(ReturnValue);
		}
		if (LeftOperand.GetType() != RightOperand.GetType())
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Error when evaluating binary operator \"+\": Can only add objects of same type";
			return(ReturnValue);
		}
		MBDBO_Type AddType = RightOperand.GetType();
		if (AddType == MBDBO_Type::String)
		{
			ReturnValue = MBDB_Object(LeftOperand.GetStringData() + RightOperand.GetStringData());
		}
		else if (AddType == MBDBO_Type::Integer)
		{
			ReturnValue = MBDB_Object(LeftOperand.GetIntegerData() + RightOperand.GetIntegerData());
		}
		else if (AddType == MBDBO_Type::Array)
		{

		}
		return(ReturnValue);
	}
	MBDB_Object MBDBObjectScript_ExecutionContext::EvaluateStatement(MBDBO_EvaluationInfo& EvaluationInfo, MBDBObjectScript_Statement const& StatementToEvaluate, MBError* OutError)
	{
		MBError EvaluationError = true;
		MBDB_Object ReturnValue;
		if (StatementToEvaluate.m_Type == MBDBObjectScript_StatementType::FunctionCall)
		{
			//SEMANTIK en function call är något som enbart beror på argumenten och är som en builtin som evalueras här, medans [] definierar en funktion som ska evalueras av objektet
			//och definieras därmed o MBDB_Object klassen
			MBDBObjectScript_FunctionStatementData const& FunctionStatement = *(MBDBObjectScript_FunctionStatementData*)StatementToEvaluate.m_StatementData;
			if (m_BuiltinFunctions.find(FunctionStatement.FunctionIdentifier) != m_BuiltinFunctions.end())
			{
				MBDB_ECBuiltinFunction BuiltinFunction = m_BuiltinFunctions[FunctionStatement.FunctionIdentifier];
				ReturnValue = (this->*BuiltinFunction)(EvaluationInfo, FunctionStatement.FunctionArguments, &EvaluationError);
			}
			else
			{
				EvaluationError = false;
				EvaluationError.ErrorMessage = "Function identifier not defined";
			}
		}
		else if (StatementToEvaluate.m_Type == MBDBObjectScript_StatementType::Literal)
		{
			ReturnValue = StatementToEvaluate.p_GetLiteralObject();
		}
		else if (StatementToEvaluate.m_Type == MBDBObjectScript_StatementType::ObjectField)
		{
			MBDBObjectScript_ObjectFieldStatementData const& FieldStatementData = *(MBDBObjectScript_ObjectFieldStatementData*)StatementToEvaluate.m_StatementData;
			MBDB_Object ObjectWithField = EvaluateStatement(EvaluationInfo, FieldStatementData.ObjectToEvaluate, &EvaluationError);
			if (EvaluationError)
			{
				if (ObjectWithField.GetType() == MBDBO_Type::AggregateObject)
				{
					if (!ObjectWithField.HasAttribute(FieldStatementData.FieldIdentifier))
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Object has no attribute \"" + FieldStatementData.FieldIdentifier + "\"";
					}
					else
					{
						ReturnValue = std::move(ObjectWithField.GetAttribute(FieldStatementData.FieldIdentifier));
					}
				}
			}
		}
		if (OutError != nullptr)
		{
			*OutError = EvaluationError;
		}
		return(ReturnValue);
	}

}
std::string ToJason(MBDB::MBDB_Object const& ObjectToEncode)
{
	return(ObjectToEncode.ToJason());
}