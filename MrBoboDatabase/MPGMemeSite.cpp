#define NOMINMAX
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MinaStringOperations.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <filesystem>
#include <MBSearchEngine/MBUnicode.h>
//username cookie = 
//password cookie = 
enum class DBPermissions
{
	Read,
	Edit,
	Upload,
	Null,
};
//std::map<std::string, std::unique_ptr<MBDB_Object>> m_Attributes = {};
//std::vector<std::unique_ptr<MBDB_Object>> m_ArrayObjects = {};
//MBDBO_Type m_Type = MBDBO_Type::Null;
//bool m_IsAtomic = false;
//std::string m_AtomicValue = "";
MBDB_Object::MBDB_Object(MBDB_Object const& ObjectToCopy)
{
	m_IsAtomic = ObjectToCopy.m_IsAtomic;
	m_AtomicData = ObjectToCopy.p_CopyAtomicData();
	m_Type = ObjectToCopy.m_Type;
	if (ObjectToCopy.IsAggregate())
	{
		for (auto& Keys : ObjectToCopy.m_Attributes)
		{
			m_Attributes[Keys.first] = std::unique_ptr<MBDB_Object>(new MBDB_Object(*Keys.second));
		}
		for (auto& ArrayObject : ObjectToCopy.m_ArrayObjects)
		{
			m_ArrayObjects.push_back(std::unique_ptr<MBDB_Object>(new MBDB_Object(*ArrayObject)));
		}
	}
}
MBDB_Object::MBDB_Object(MBDB_Object&& ObjectToMove) noexcept
{
	swap(*this, ObjectToMove);
}
MBDB_Object& MBDB_Object::operator=(MBDB_Object RightObject)
{
	swap(RightObject, *this);
	return(*this);
}
void MBDB_Object::p_FreeAtomicData()
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
	else
	{
		assert(false);
	}
}
MBDB_Object::~MBDB_Object()
{
	p_FreeAtomicData();
}
void swap(MBDB_Object& LeftObject, MBDB_Object& RightObject)
{
	std::swap(LeftObject.m_IsAtomic, RightObject.m_IsAtomic);
	std::swap(LeftObject.m_Type, RightObject.m_Type);
	std::swap(LeftObject.m_AtomicData, RightObject.m_AtomicData);
	std::swap(LeftObject.m_Attributes, RightObject.m_Attributes);
	std::swap(LeftObject.m_ArrayObjects, RightObject.m_ArrayObjects);
}
bool MBDB_Object::IsAggregate() const
{
	return(!m_IsAtomic);
}
bool MBDB_Object::HasAttribute(std::string const& AttributeName) const
{
	return(m_Attributes.find(AttributeName) != m_Attributes.end());
}
MBDB_Object& MBDB_Object::GetAttribute(std::string const& AttributeName)
{
	return(*m_Attributes[AttributeName]);
}
MBDBO_Type MBDB_Object::GetType() const
{
	return(m_Type);
}
void* MBDB_Object::p_CopyAtomicData() const
{
	if (!m_IsAtomic)
	{
		return(nullptr);
	}
	if (m_Type == MBDBO_Type::Boolean)
	{
		bool& AtomicValue = *((bool*)m_AtomicData);
		return(new bool(AtomicValue));
	}
	else if(m_Type == MBDBO_Type::Integer)
	{
		intmax_t& AtomicValue = *((intmax_t*)m_AtomicData);
		return(new intmax_t(AtomicValue));
	}
	else if (m_Type == MBDBO_Type::String)
	{
		std::string& AtomicValue = *((std::string*)m_AtomicData);
		return(new std::string(AtomicValue));
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
		ReturnValue = std::stoi(ObjectData.substr(IntBegin,IntEnd-IntBegin));
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
MBDB_Object MBDB_Object::p_EvaluateTableExpression(MBDBO_EvaluationInfo & EvaluationInfo,std::string const& TableExpression)
{
	assert(false);
	return(MBDB_Object());
}
MBDB_Object MBDB_Object::p_ParseArrayObject(MBDBO_EvaluationInfo & EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset, MBError* OutError)
{
	MBDB_Object ReturnValue;
	MBError EvaluationError(true);
	size_t ParseOffset = InOffset;
	
	ReturnValue.m_Type = MBDBO_Type::Array;
	p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
	ParseOffset += 1;//gör så vi är på värdet efter den försat [
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
		ReturnValue.m_ArrayObjects.push_back(std::move(NewObject));

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
	ReturnValue.LoadObject(

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

	std::vector<std::string> FunctionCases = { "DB_LoadObject","DB_LoadTable" };
	bool ExpressionEvaluated = false;
	p_SkipWhitespace(ObjectData, ParseOffset, &ParseOffset);
	for (size_t i = 0; i < FunctionCases.size(); i++)
	{
		if (ParseOffset + FunctionCases[i].size() - 1 < ObjectData.size())
		{
			if (ObjectData.substr(ParseOffset, FunctionCases[i].size()) == FunctionCases[i])
			{
				if (i == 0)
				{
					p_EvaluateDBObjectExpression(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
					ExpressionEvaluated = true;
				}
				if (i == 1)
				{
					p_EvaluateTableExpression(EvaluationInfo, ObjectData, ParseOffset, &ParseOffset, &EvaluationError);
					ExpressionEvaluated = true;
				}
			}
		}
	}
	if (!ExpressionEvaluated)
	{
		EvaluationError = false;
		EvaluationError.ErrorMessage = "Invalid Database expression";
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

	ReturnValue.m_IsAtomic = true;
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
		ReturnValue.m_Attributes[NewAttributeTag] = std::move(NewObject);
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
	else if(ObjectData[ParseOffset] == 'D')
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
	for (auto& Key : m_Attributes)
	{
		ReturnValue +=  ::ToJason(Key.first);
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
	for (size_t i = 0; i < m_ArrayObjects.size(); i++)
	{
		ReturnValue += m_ArrayObjects[i]->ToJason();
		if (i+1 < m_ArrayObjects.size())
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
	if (m_IsAtomic)
	{
		ReturnValue = p_AtomicToJSON();
	}
	else if (m_Type == MBDBO_Type::Array)
	{
		ReturnValue = p_ArrayToJSON();
	}
	else
	{
		ReturnValue = p_AggregateToJSON();
	}
	return(ReturnValue);
}
MBError MBDB_Object::LoadObject(std::string const& ObjectFilepath, std::string const& AssociatedUser, MBDB::MrBoboDatabase* AssociatedDatabase)
{
	std::ifstream InputFile = std::ifstream(ObjectFilepath, std::ios::in | std::ios::binary);
	size_t FileSize = std::filesystem::file_size(ObjectFilepath);
	std::string FileData = std::string(FileSize, 0);
	InputFile.read(FileData.data(), FileSize);
	MBError EvaluationError(true);
	MBDBO_EvaluationInfo EvaluationInfo;
	EvaluationInfo.AssociatedDatabase = AssociatedDatabase;
	EvaluationInfo.EvaluatingUser = AssociatedUser;
	EvaluationInfo.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(ObjectFilepath).parent_path());
	MBDB_Object EvaluatedObject = p_EvaluateObject(EvaluationInfo, FileData,0,nullptr, &EvaluationError);
	if (EvaluationError)
	{
		swap(*this, EvaluatedObject);
	}
	return(EvaluationError);
}

std::mutex DatabaseMutex;
//MBDB::MrBoboDatabase DBSite_Database("./TestDatabas",0);
MBDB::MrBoboDatabase* WebsiteDatabase = nullptr;
std::mutex WritableDatabaseMutex;
MBDB::MrBoboDatabase* WritableDatabase = nullptr;
std::mutex LoginDatabaseMutex;
MBDB::MrBoboDatabase* LoginDatabase = nullptr;
std::mutex DBIndexMapMutex;
std::unordered_map<std::string, MBSearchEngine::MBIndex>* __DBIndexMap = nullptr;

std::mutex __MBTopResourceFolderMutex;
std::string __MBTopResourceFolder = "./";
std::string MBDBGetResourceFolderPath()
{
	std::lock_guard<std::mutex> Lock(__MBTopResourceFolderMutex);
	return(__MBTopResourceFolder + "/MBDBResources/");
}
void InitDatabase()
{
	//utgår från den foldern som programmet körs i
	std::fstream MBDBConfigFile("./MBDBConfigFile");
	std::string CurrentFileLine = "";
	std::string ResourceFolderConfig = "MBDBTopFolder=";
	if (MBDBConfigFile.is_open())
	{
		while (std::getline(MBDBConfigFile, CurrentFileLine))
		{
			if (CurrentFileLine.substr(0, ResourceFolderConfig.size()) == ResourceFolderConfig)
			{
				__MBTopResourceFolder = CurrentFileLine.substr(ResourceFolderConfig.size());
			}
		}
	}
	std::cout << __MBTopResourceFolder << std::endl;
	if (WebsiteDatabase == nullptr)
	{
		WebsiteDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder+"/TestDatabas", 0);
	}
	if (WritableDatabase == nullptr)
	{
		WritableDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder+"/TestDatabas", 1);
	}
	if (LoginDatabase == nullptr)
	{
		LoginDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder+"/MBGLoginDatabase", 0);
	}
	//läser in mbdb config filen och initaliserar directoryn med rätt
	__DBIndexMap = new std::unordered_map<std::string, MBSearchEngine::MBIndex>();
	std::filesystem::directory_iterator IndexIterator(MBDBGetResourceFolderPath() + "Indexes");
	for (auto& DirectoryEntry : IndexIterator)
	{
		if (DirectoryEntry.is_regular_file())
		{
			(*__DBIndexMap)[DirectoryEntry.path().filename().generic_string()] = MBSearchEngine::MBIndex(DirectoryEntry.path().generic_string());
		}
	}
}
int MBGWebsiteMain()
{
#ifndef NDEBUG
	std::cout << "Is Debug" << std::endl;
#endif // DEBUG
	MBSockets::Init();
	InitDatabase();
	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
	TestServer.AddRequestHandler({ DBLogin_Predicate,DBLogin_ResponseGenerator });
	TestServer.AddRequestHandler({ DBSite_Predicate,DBSite_ResponseGenerator });
	TestServer.AddRequestHandler({ UploadFile_Predicate,UploadFile_ResponseGenerator });
	TestServer.AddRequestHandler({ DBGet_Predicate,DBGet_ResponseGenerator });
	TestServer.AddRequestHandler({ DBView_Predicate,DBView_ResponseGenerator });
	TestServer.AddRequestHandler({ DBViewEmbedd_Predicate,DBViewEmbedd_ResponseGenerator });
	TestServer.AddRequestHandler({ DBAdd_Predicate,DBAdd_ResponseGenerator });
	TestServer.AddRequestHandler({ DBGeneralAPI_Predicate,DBGeneralAPI_ResponseGenerator });
	TestServer.AddRequestHandler({ DBUpdate_Predicate,DBUpdate_ResponseGenerator });
	TestServer.StartListening();
	return(0);
}
struct Cookie
{
	std::string Name = "";
	std::string Value = "";
};
std::vector<Cookie> GetCookiesFromRequest(std::string const& RequestData)
{
	std::vector<Cookie> ReturnValue = {};
	std::vector<std::string> Cookies = Split(MBSockets::GetHeaderValue("Cookie", RequestData), "; ");
	for (size_t i = 0; i < Cookies.size(); i++)
	{
		size_t FirstEqualSignPos = Cookies[i].find_first_of("=");
		std::string CookieName = Cookies[i].substr(0, FirstEqualSignPos);
		std::string CookieValue = Cookies[i].substr(FirstEqualSignPos+1);
		ReturnValue.push_back({ CookieName, CookieValue });
	}
	return(ReturnValue);
}
std::vector<MBDB::MBDB_RowData> DB_GetUser(std::string const& UserName, std::string const& PasswordHash)
{
	std::string SQLStatement = "SELECT * FROM Users WHERE UserName=? AND PasswordHash=?;";
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	{
		std::lock_guard<std::mutex> Lock(LoginDatabaseMutex);
		MBDB::SQLStatement* NewStatement = LoginDatabase->GetSQLStatement(SQLStatement);
		NewStatement->BindString(UserName, 1);
		NewStatement->BindString(PasswordHash, 2);
		QuerryResult = LoginDatabase->GetAllRows(NewStatement);
		NewStatement->FreeData();
		LoginDatabase->FreeSQLStatement(NewStatement);
	}
	return(QuerryResult);
}
DBPermissionsList GetConnectionPermissions(std::string const& RequestData)
{
	DBPermissionsList ReturnValue;
	std::vector<Cookie> Cookies = GetCookiesFromRequest(RequestData);
	std::string UserName = "";
	std::string PasswordHash = "";
	for (size_t i = 0; i < Cookies.size(); i++)
	{
		if (Cookies[i].Name == "DBUsername")
		{
			UserName = Cookies[i].Value;
		}
		if (Cookies[i].Name == "DBPassword")
		{
			PasswordHash = Cookies[i].Value;
		}
	}
	std::string SQLStatement = "SELECT * FROM Users WHERE UserName=?;";
	std::vector<MBDB::MBDB_RowData> QuerryResult = DB_GetUser(UserName, PasswordHash);
	if (QuerryResult.size() != 0)
	{
		std::tuple<int, std::string, std::string, int, int, int> UserInfo = QuerryResult[0].GetTuple<int, std::string, std::string, int, int, int>();
		std::string RequestPassword = std::get<2>(UserInfo);
		if (RequestPassword == PasswordHash)
		{
			ReturnValue.IsNull = false;
			if (std::get<3>(UserInfo))
			{
				ReturnValue.Read = true;
			}
			if (std::get<4>(UserInfo))
			{
				ReturnValue.Edit = true;
			}
			if (std::get<5>(UserInfo))
			{
				ReturnValue.Upload = true;
			}
		}
	}
	return(ReturnValue);
}
std::string DBGetUsername(std::string const& RequestData)
{
	std::string ReturnValue = "";
	std::vector<Cookie> Cookies = GetCookiesFromRequest(RequestData);
	for (size_t i = 0; i < Cookies.size(); i++)
	{
		if (Cookies[i].Name == "DBUsername")
		{
			ReturnValue = Cookies[i].Value;
			break;
		}
	}
	return(ReturnValue);
}
std::string DBGetPassword(std::string const& RequestData)
{
	std::string ReturnValue = "";
	std::vector<Cookie> Cookies = GetCookiesFromRequest(RequestData);
	for (size_t i = 0; i < Cookies.size(); i++)
	{
		if (Cookies[i].Name == "DBPassword")
		{
			ReturnValue = Cookies[i].Value;
			break;
		}
	}
	return(ReturnValue);
}
bool DBLogin_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBLogin")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string ServerResources = AssociatedServer->GetResourcePath("mrboboget.se");
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::unordered_map<std::string, std::string> FileVariables = {};
	
	std::string RequestUsername = DBGetUsername(RequestData);
	std::string RequestPassword = DBGetPassword(RequestData); 
	FileVariables["LoginValue"] = RequestUsername;
	if (DB_GetUser(RequestUsername,RequestPassword).size() != 0)
	{
		FileVariables["LoginValue"] = "Currently logged in as: " + FileVariables["LoginValue"];
	}
	ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(ServerResources + "DBLogin.html", ServerResources),FileVariables);
	return(ReturnValue);
}

bool DBSite_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >=1)
	{
		if (Directorys[0] == "DBSite")
		{
			return(true);
		}
	}
	return(false);
}

MBSockets::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	MBSockets::HTTPDocument NewDocument;
	if (RequestType == "GET")
	{
		NewDocument.Type = MBSockets::HTTPDocumentType::HTML;
		NewDocument.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se")+ "/DBSite.html",AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	else if(RequestType == "POST")
	{
		DBPermissionsList ConnectionPermissions = GetConnectionPermissions(RequestData);
		if (!ConnectionPermissions.Read)
		{
			NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"Invalid permissions: Require permissions to read\",\"Rows\":[]}";
		}
		else
		{
			NewDocument.Type = MBSockets::HTTPDocumentType::json;
			//vi behöver parsa kroppen så vi får SQL koden som den enkoder i sin kropp
			std::string SQLCommand = MrPostOGet::GetRequestContent(RequestData);
			std::cout << SQLCommand << std::endl;
			bool CommandSuccesfull = true;
			MBError SQLError(true);
			std::vector<MBDB::MBDB_RowData> SQLResult = {};
			{
				std::lock_guard<std::mutex> Lock(DatabaseMutex);
				SQLResult = WebsiteDatabase->GetAllRows(SQLCommand, &SQLError);
				if (!SQLError)
				{
					CommandSuccesfull = false;
				}
			}
			//std::vector<std::string> ColumnNames = { "MemeID","MemeSource","MemeTags" };
			if (CommandSuccesfull)
			{
				std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\",\"Rows\":[";
				size_t NumberOfRows = SQLResult.size();
				for (size_t i = 0; i < NumberOfRows; i++)
				{
					JsonResponse += SQLResult[i].ToJason();
					if (i + 1 < NumberOfRows)
					{
						JsonResponse += ",";
					}
				}
				JsonResponse += "]}";
				NewDocument.DocumentData = JsonResponse;
			}
			else
			{
				NewDocument.DocumentData = "{\"MBDBAPI_Status\":" + ToJason(SQLError.ErrorMessage) + ",\"Rows\":[]}";
			}
			//std::cout << "JsonResponse sent: " << JsonResponse << std::endl;
		}
	}
	return(NewDocument);
}

bool UploadFile_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "UploadFile" && MBSockets::GetRequestType(RequestData) == "POST") 
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument UploadFile_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//Content-Type: multipart/form-data; boundary=---------------------------226143532618736951363968904467
	DBPermissionsList ConnectionPermissions = GetConnectionPermissions(RequestData);
	MBSockets::HTTPDocument NewDocument;
	NewDocument.Type = MBSockets::HTTPDocumentType::json;
	std::string Boundary = "";
	std::vector<std::string> ContentTypes = MBSockets::GetHeaderValues("Content-Type", RequestData);
	std::string FormType = "multipart/form-data";
	std::string BoundaryHeader = "; boundary=";
	if (!ConnectionPermissions.Upload)
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permission to upload\"}";
		NewDocument.RequestStatus = MBSockets::HTTPRequestStatus::Conflict;
		AssociatedConnection->Close();
		return(NewDocument);
	}
	for (size_t i = 0; i < ContentTypes.size(); i++)
	{
		if (ContentTypes[i].substr(0, FormType.size()) == FormType)
		{
			Boundary = ContentTypes[i].substr(FormType.size() + BoundaryHeader.size(),ContentTypes[i].size()-FormType.size()-BoundaryHeader.size()-1);
			break;
		}
	}
	if (Boundary == "")
	{
		assert(false);
	}
	int FirstBoundaryLocation = RequestData.find(Boundary);
	int FirstFormParameterLocation = RequestData.find(Boundary, FirstBoundaryLocation + Boundary.size())+Boundary.size()+2;
	int EndOfFirstParameters = RequestData.find("\r\n",FirstFormParameterLocation);
	std::string FieldParameters = RequestData.substr(FirstFormParameterLocation, EndOfFirstParameters - FirstFormParameterLocation);
	//hardcodat eftersom vi vet formtatet av formuläret
	std::vector<std::string> FirstFieldValues = Split(FieldParameters, "; ");
	std::string FileNameHeader = "filename=\"";
	std::string FileName =MBDBGetResourceFolderPath()+ FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
	int FilesWithSameName = 0;
	while(std::filesystem::exists(FileName))
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"FileAlreadyExists\"}";
		NewDocument.RequestStatus = MBSockets::HTTPRequestStatus::Conflict;
		//TODO close makar inte mycket sense för svaret kommer inte skickas, borde istället finnas sett extra options som är "close after send"
		AssociatedConnection->Close();
		return(NewDocument);
	}
	int FileDataLocation = RequestData.find("\r\n\r\n", FirstFormParameterLocation) + 4;
	std::ofstream NewFile(FileName, std::ios::out | std::ios::binary);
	int TotalDataWritten = RequestData.size()- FileDataLocation;
	NewFile.write(&RequestData[FileDataLocation], RequestData.size() - FileDataLocation);
	clock_t WriteTimer = clock();
	std::string NewData;
	while(AssociatedConnection->DataIsAvailable())
	{
		NewFile.seekp(0, std::ostream::end);
		NewData = AssociatedConnection->GetNextChunkData();
		TotalDataWritten += NewData.size();
		int BoundaryLocation = NewData.find(Boundary);
		if (BoundaryLocation != NewData.npos)
		{
			int TrailingCharacters = Boundary.size() + 2 + 2;
			NewData.resize(NewData.size() - TrailingCharacters);
		}
		NewFile.write(&NewData[0], NewData.size());
	}
	std::cout << "Total data written: " << TotalDataWritten << std::endl;
	std::cout << "Total time: " << (clock() - WriteTimer) / double(CLOCKS_PER_SEC) << std::endl;
	
	NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"ok\"}";
	return(NewDocument);
}

bool DBGet_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DB")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string DatabaseResourcePath = MBDBGetResourceFolderPath();
	std::string URLResource = MBSockets::GetReqestResource(RequestData);
	std::string DatabaseResourceToGet = URLResource.substr(URLResource.find_first_of("DB/") + 3);
	if (!std::filesystem::exists(DatabaseResourcePath+DatabaseResourceToGet))
	{
		MBSockets::HTTPDocument Invalid;
		Invalid.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
		Invalid.Type = MBSockets::HTTPDocumentType::HTML;
		Invalid.DocumentData = "File not found";
		return(Invalid);
	}
	std::string RangeData = MBSockets::GetHeaderValue("Range", RequestData);
	std::string IntervallsData = RangeData.substr(RangeData.find_first_of("=") + 1);
	ReplaceAll(&IntervallsData, "\r", "");
	ReplaceAll(&IntervallsData, "\n", "");
	std::vector<FiledataIntervall> ByteIntervalls = {};
	if (RangeData != "")
	{
		std::vector<std::string> Intervalls = Split(ReplaceAll(IntervallsData," ",""), ",");
		for (int i = 0; i < Intervalls.size(); i++)
		{
			FiledataIntervall NewIntervall = { size_t(-1),size_t(-1) };
			std::vector<std::string> IntervallNumbers = Split(Intervalls[i], "-");
			if (IntervallNumbers[0] != "")
			{
				NewIntervall.FirstByte = std::stoll(IntervallNumbers[0]);
			}
			if (IntervallNumbers[1] != "")
			{
				NewIntervall.FirstByte = std::stoll(IntervallNumbers[1]);
			}
			ByteIntervalls.push_back(NewIntervall);
		}
	}


	MBSockets::HTTPDocument ReturnValue = AssociatedServer->GetResource(DatabaseResourcePath+DatabaseResourceToGet,ByteIntervalls);
	return(ReturnValue);
}
std::string GetEmbeddedVideo(std::string const& VideoPath, std::string const& WebsiteResourcePath)
{
	std::string ReturnValue = "";
	std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
	std::unordered_map<std::string, bool> BrowserSupported = { {"mp4",true},{"webm",true},{"ogg", true} };
	if (BrowserSupported.find(FileExtension) != BrowserSupported.end())
	{
		std::unordered_map<std::string, std::string> VariableValues = {};
		VariableValues["ElementID"] = VideoPath;
		VariableValues["MediaType"] = "video";
		VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
		VariableValues["FileType"] = FileExtension;
		ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
	}
	else
	{
		ReturnValue = "<p>Native broswer streaming not Supported</p><br><a href=\"/DB/" + VideoPath + "\">/DB/" + VideoPath + "</a>";
	}
	return(ReturnValue);
}
std::string GetEmbeddedAudio(std::string const& VideoPath, std::string const& WebsiteResourcePath)
{
	std::unordered_map<std::string, std::string> VariableValues = {};
	std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
	VariableValues["ElementID"] = VideoPath;
	VariableValues["MediaType"] = "audio";
	VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
	VariableValues["FileType"] = FileExtension;
	std::string ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
	return(ReturnValue);
}
std::string GetEmbeddedImage(std::string const& ImagePath)
{
	std::string ReturnValue = "<image src=\"/DB/" + ImagePath + "\" style=\"max-width:100%\"></image>";
	return(ReturnValue);
}
std::string GetEmbeddedPDF(std::string const& ImagePath)
{
	return("<iframe src=\"/DB/" + ImagePath + "\" style=\"width: 100%; height: 100%;max-height: 100%; max-width: 100%;\"></iframe>");
}
bool DBView_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBView")
		{
			return(true);
		}
	}
	return(false);
}
std::string GetEmbeddedResource(std::string const& MBDBResource,std::string const& ResourceFolder)
{
	if (!std::filesystem::exists(MBDBGetResourceFolderPath()+MBDBResource))
	{
		return("<p>File does not exist<p>");
	}
	std::string ReturnValue = "";
	std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	if (ResourceMedia == MBSockets::MediaType::Image)
	{
		ReturnValue = GetEmbeddedImage(MBDBResource);
	}
	else if (ResourceMedia == MBSockets::MediaType::Video)
	{
		ReturnValue = GetEmbeddedVideo(MBDBResource, ResourceFolder);
	}
	else if (ResourceMedia == MBSockets::MediaType::Audio)
	{
		ReturnValue = GetEmbeddedAudio(MBDBResource, ResourceFolder);
	}
	else if (ResourceMedia == MBSockets::MediaType::PDF)
	{
		ReturnValue = GetEmbeddedPDF(MBDBResource);
	}
	else
	{
		ReturnValue = "<p>File in unrecognized format</p><br><a href=\"/DB/" + MBDBResource + "\">/DB/" + MBDBResource + "</a>";
	}
	return(ReturnValue);
}
MBSockets::HTTPDocument DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string EmbeddedElement = "";

	std::string HandlerName = "DBView";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = MBDBGetResourceFolderPath();
	std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
	std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
	if (!std::filesystem::exists(DBResourcesPath+DBResource))
	{
		MBSockets::HTTPDocument Invalid;
		Invalid.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
		Invalid.Type = MBSockets::HTTPDocumentType::HTML;
		Invalid.DocumentData = "File not found";
		return(Invalid);
	}
	if (!std::filesystem::is_directory(DBResourcesPath + DBResource) && DBResource != "")
	{
		MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
		EmbeddedElement = GetEmbeddedResource(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		std::unordered_map<std::string, std::string> MapData = {};
		MapData["EmbeddedMedia"] = EmbeddedElement;
		std::string HTMLResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
		ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "DBViewTemplate.html", HTMLResourcePath), MapData);
	}
	else
	{
		ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
		ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se")+"DBViewFolder.html", AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	//= AssociatedServer->GetResource(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBViewTemplate.html");
	return(ReturnValue);
}

bool DBViewEmbedd_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBViewEmbedd")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue = MBSockets::HTTPDocument();
	std::string HandlerName = "DBViewEmbedd/";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = MBDBGetResourceFolderPath();
	std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
	std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	ReturnValue.Type = MBSockets::DocumentTypeFromFileExtension(ResourceExtension);
	if (ResourceMedia == MBSockets::MediaType::Image)
	{
		ReturnValue.DocumentData = GetEmbeddedImage(DBResource);
	}
	else if (ResourceMedia == MBSockets::MediaType::Video)
	{
		ReturnValue.DocumentData = GetEmbeddedVideo(DBResource,AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	else if (ResourceMedia == MBSockets::MediaType::Audio)
	{
		ReturnValue.DocumentData = GetEmbeddedAudio(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	return(ReturnValue);
}

bool DBAdd_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBAdd")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	//std::string TableName = RequestData.substr(RequestResource.find("DBAdd/") + 6);
	//std::vector < std::string> ExistingTableNames = {};
	//låter all denna kod köras i javascript, blir det enklaste
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath+"DBAdd.html", ResourcePath);
	return(ReturnValue);
}

bool DBGeneralAPI_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBGeneralAPI")
		{
			return(true);
		}
	}
	return(false);
}

std::string DBGeneralAPIGetDirective(std::string const& RequestBody)
{
	size_t FirstSpace = RequestBody.find(" ");
	std::string APIDirective = RequestBody.substr(0, FirstSpace);
	return(APIDirective);
}
std::vector<std::string> DBGeneralAPIGetArguments(std::string const& RequestBody,MBError* OutError = nullptr)
{
	size_t FirstSpace = RequestBody.find(" ");
	std::vector<std::string> ReturnValue = {};
	size_t ParsePosition = FirstSpace + 1;
	while (ParsePosition != std::string::npos && ParsePosition < RequestBody.size())
	{
		size_t SizeEnd = RequestBody.find(" ", ParsePosition);
		size_t ArgumentSize = 0;
		if (SizeEnd == std::string::npos)
		{
			break;
		}
		try
		{
			ArgumentSize = std::stoi(RequestBody.substr(ParsePosition, SizeEnd - ParsePosition));
		}
		catch (const std::exception&)
		{
			*OutError = MBError(false);
			OutError->ErrorMessage = "Failed to parse argument size";
		}
		ReturnValue.push_back(RequestBody.substr(SizeEnd + 1, ArgumentSize));
		ParsePosition = SizeEnd + 1 + ArgumentSize + 1;
	}
	//std::vector<std::string> ReturnValue = Split(RequestBody.substr(FirstSpace + 1),",");
	return(ReturnValue);
}


std::string GetTableNamesBody(std::vector<std::string> const& Arguments)
{
	std::vector<std::string> TableNames = {};
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		TableNames = WebsiteDatabase->GetAllTableNames();
	}
	std::string JSONTableNames = "\"TableNames\":"+MakeJasonArray(TableNames);
	//std::string JsonRespone = "{[";
	//size_t TableNamesSize = TableNames.size();
	//for (size_t i = 0; i < TableNamesSize; i++)
	//{
	//	JsonRespone += MBDB::ToJason(TableNames[i]);
	//	if (i + 1 < TableNamesSize)
	//	{
	//		JsonRespone += ",";
	//	}
	//}
	//JsonRespone += "]}";
	std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\"," + JSONTableNames + "}";
	return(JsonResponse);
}
std::string GetTableInfoBody(std::vector<std::string> const& Arguments)
{
	//första argumentet är tablen vi vill ha
	if (Arguments.size() == 0)
	{
		return("");
	}
	std::vector<MBDB::ColumnInfo> TableInfo = {};
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		TableInfo = WebsiteDatabase->GetColumnInfo(Arguments[0]);
	}
	std::string JSONResponse = "{\"MBAPI_Status\":\"ok\",\"TableInfo\":" + MakeJasonArray(TableInfo) + "}";
	return(JSONResponse);
}
inline long long StringToInt(std::string const& IntData, MBError* OutError = nullptr)
{
	long long ReturnValue = 0;
	try
	{
		ReturnValue = std::stoi(IntData);
	}
	catch (const std::exception&)
	{
		*OutError = MBError(false);
		OutError->ErrorMessage = "Failed to parse int";
	}
	return(ReturnValue);
}
std::vector<MBDB::MBDB_RowData> EvaluateBoundSQLStatement(std::string SQLCommand,std::vector<std::string> const& ColumnValues,
	std::vector<int> ColumnIndex,std::string TableName,MBError* OutError)
{
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
	MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);

	std::vector<MBDB::ColumnInfo> TableColumnInfo = WritableDatabase->GetColumnInfo(TableName);
	for (size_t i = 0; i < ColumnValues.size(); i++)
	{
		if (TableColumnInfo[ColumnIndex[i]].ColumnType == MBDB::ColumnSQLType::Int)
		{
			MBDB::MaxInt NewInt = StringToInt(ColumnValues[i], OutError);
			if (!OutError)
			{
				break;
			}
			*OutError = NewStatement->BindInt(NewInt, i + 1);
		}
		else
		{
			*OutError = NewStatement->BindString(ColumnValues[i], i + 1);
			if (!OutError)
			{
				break;
			}
		}
	}
	if (OutError)
	{
		QuerryResult = WritableDatabase->GetAllRows(NewStatement, OutError);
	}
	NewStatement->FreeData();
	WritableDatabase->FreeSQLStatement(NewStatement);
	return(QuerryResult);
}
std::string DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	std::string SQLCommand = "INSERT INTO " + Arguments[0] + "("; //VALUES (";
	std::vector<std::string> ColumnNames = {};
	std::vector<std::string> ColumnValues = {};
	std::vector<int> ColumnIndex = {};
	MBError DataBaseError(true);


	for (size_t i = 1; i < Arguments.size(); i++)
	{
		int FirstColon = Arguments[i].find_first_of(":");
		int SecondColon = Arguments[i].find(":", FirstColon + 1);
		int NewColumnIndex = -1;
		NewColumnIndex = StringToInt(Arguments[i].substr(FirstColon + 1, SecondColon - FirstColon),&DataBaseError);
		if (!DataBaseError)
		{
			ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
			return(ReturnValue);
		}
		ColumnNames.push_back(Arguments[i].substr(0, FirstColon));
		ColumnValues.push_back(Arguments[i].substr(SecondColon + 1));
		ColumnIndex.push_back(NewColumnIndex);
		SQLCommand += ColumnNames[i - 1];
		if (i + 1 < Arguments.size())
		{
			SQLCommand += ",";
		}
	}
	SQLCommand += ") VALUES(";
	for (size_t i = 1; i < Arguments.size(); i++)
	{
		SQLCommand += "?";
		if (i + 1 < Arguments.size())
		{
			SQLCommand += ",";
		}
	}
	SQLCommand += ");";
	std::vector<MBDB::MBDB_RowData> QuerryResult = EvaluateBoundSQLStatement(SQLCommand,ColumnValues,ColumnIndex,Arguments[0],&DataBaseError);
	if (DataBaseError)
	{
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	}
	else
	{
		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	}
	return(ReturnValue);
}
std::string DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments)
{
	//UPDATE table_name
	//SET column1 = value1, column2 = value2, ...
	//	WHERE condition;
	std::string ReturnValue = "";
	std::string SQLCommand = "UPDATE " + Arguments[0] + " SET "; //VALUES (";
	std::vector<std::string> ColumnNames = {};
	std::vector<std::string> OldColumnValues = {};
	std::vector<std::string> NewColumnValues = {};
	MBError DataBaseError(true);


	for (size_t i = 1; i < Arguments.size(); i++)
	{
		if ((i % 3) == 1)
		{
			ColumnNames.push_back(Arguments[i]);
		}
		if ((i % 3) == 2)
		{
			OldColumnValues.push_back(Arguments[i]);
		}
		if ((i % 3) == 0)
		{
			NewColumnValues.push_back(Arguments[i]);
		}
	}
	for (size_t i = 0; i < OldColumnValues.size(); i++)
	{
		SQLCommand += ColumnNames[i] + "=?";
		if (i + 1 < OldColumnValues.size())
		{
			SQLCommand += ", ";
		}
	}
	SQLCommand += " WHERE ";
	for (size_t i = 0; i < ColumnNames.size(); i++)
	{
		SQLCommand += "("+ColumnNames[i] + "=?";
		if (OldColumnValues[i] == "null")
		{
			SQLCommand += " OR "+ColumnNames[i]+ " IS NULL";
		}
		SQLCommand += ")";
		if (i + 1 < ColumnNames.size())
		{
			SQLCommand += " AND ";
		}
	}
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	{
		std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
		MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);
	
		//DEBUG GREJER
		//MBDB::SQLStatement* DebugStatement = WritableDatabase->GetSQLStatement("SELECT * FROM Music WHERE RowNumber=82");
		//std::vector<MBDB::MBDB_RowData> DebugResult = WritableDatabase->GetAllRows(DebugStatement);
		//auto DebugTuple = DebugResult[0].GetTuple<int, std::string, std::string, std::string, std::string, std::string, std::string>();
		//std::ofstream DebugFile("./DebugDatabaseData.txt", std::ios::out|std::ios::binary);
		//std::ofstream DebugInput("./DebugInputData.txt", std::ios::out | std::ios::binary);
		//DebugFile << std::get<6>(DebugTuple);
		//DebugInput << Arguments[21];

		std::vector<MBDB::ColumnInfo> ColumnInfo = WritableDatabase->GetColumnInfo(Arguments[0]);
		std::vector<MBDB::ColumnSQLType> ColumnTypes = {};
		for (size_t i = 0; i < ColumnInfo.size(); i++)
		{
			ColumnTypes.push_back(ColumnInfo[i].ColumnType);
		}
		NewStatement->BindValues(NewColumnValues, ColumnTypes, 0);
		NewStatement->BindValues(OldColumnValues, ColumnTypes, NewColumnValues.size());
		QuerryResult = WritableDatabase->GetAllRows(NewStatement,&DataBaseError);
		NewStatement->FreeData();
		WritableDatabase->FreeSQLStatement(NewStatement);
	}
	if (DataBaseError)
	{
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	}
	else
	{
		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	}
	return(ReturnValue);
}
//DBAPI_GetFolderContents
enum class MBDirectoryEntryType
{
	Directory,
	RegularFile,
	Null
};
struct MBDirectoryEntry
{
	std::filesystem::path Path = "";
	MBDirectoryEntryType Type = MBDirectoryEntryType::Null;
};
std::vector<MBDirectoryEntry> GetDirectoryEntries(std::string const& DirectoryPath,MBError* OutError = nullptr)
{
	std::vector<MBDirectoryEntry> ReturnValue = {};
	if (!std::filesystem::exists(DirectoryPath))
	{
		if (OutError != nullptr)
		{
			*OutError = false;
			OutError->ErrorMessage = "Directory does not exists";
		}
		return(ReturnValue);
	}
	std::filesystem::directory_iterator DirectoryIterator(DirectoryPath);
	while (DirectoryIterator != std::filesystem::end(DirectoryIterator))
	{
		MBDirectoryEntry NewDirectoryEntry;
		NewDirectoryEntry.Path = DirectoryIterator->path();
		if (DirectoryIterator->is_directory())
		{
			NewDirectoryEntry.Type = MBDirectoryEntryType::Directory;
		}
		else if(DirectoryIterator->is_regular_file())
		{
			NewDirectoryEntry.Type = MBDirectoryEntryType::RegularFile;
		}
		DirectoryIterator++;
		ReturnValue.push_back(NewDirectoryEntry);
	}
	if (OutError != nullptr)
	{
		*OutError = true;
	}
	return(ReturnValue);
}
std::string MakePathRelative(std::filesystem::path const& PathToProcess, std::string DirectoryToMakeRelative)
{
	bool RelativeFolderTraversed = false;
	std::filesystem::path::iterator PathIterator = PathToProcess.begin();
	std::string ReturnValue = "";
	while (PathIterator != PathToProcess.end())
	{
		if (RelativeFolderTraversed)
		{
			ReturnValue += "/"+PathIterator->generic_string();
		}
		if (PathIterator->generic_string() == DirectoryToMakeRelative)
		{
			RelativeFolderTraversed = true;
		}
		PathIterator++;
	}
	return(ReturnValue);
}
std::string MBDirectoryEntryTypeToString(MBDirectoryEntryType TypeToConvert)
{
	if (TypeToConvert == MBDirectoryEntryType::Directory)
	{
		return("Directory");
	}
	else if (TypeToConvert == MBDirectoryEntryType::RegularFile)
	{
		return("RegularFile");
	}
}
std::string ToJason(MBDirectoryEntry const& EntryToEncode)
{
	std::string ReturnValue = "{\"Path\":" + ToJason(EntryToEncode.Path.generic_string())+",";
	ReturnValue += "\"Type\":"+ToJason(MBDirectoryEntryTypeToString(EntryToEncode.Type))+"}";
	return(ReturnValue);
}
//First argument is relative path in MBDBResources folder
std::string DBAPI_GetFolderContents(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	MBError ErrorResult(true);
	std::vector<MBDirectoryEntry> DirectoryEntries = GetDirectoryEntries(MBDBGetResourceFolderPath()+Arguments[0],&ErrorResult);
	if (!ErrorResult)
	{
		return("{\"MBDBAPI_Status\":\""+ErrorResult.ErrorMessage+"\"}");
	}
	for (size_t i = 0; i < DirectoryEntries.size(); i++)
	{
		DirectoryEntries[i].Path = MakePathRelative(DirectoryEntries[i].Path, "MBDBResources");
	}
	std::string ErrorPart = "\"MBDBAPI_Status\":\"ok\"";
	std::string DirectoryPart = "\"DirectoryEntries\":"+MakeJasonArray<MBDirectoryEntry>(DirectoryEntries);
	ReturnValue = "{" + ErrorPart + "," + DirectoryPart + "}";
	return(ReturnValue);
}
std::string DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments)
{
	//ett arguments som är WhereStringen, ghetto aff egetntligen men men,måste vara på en immutable table så vi inte fuckar grejer
	std::string ReturnValue = "";
	if (Arguments.size() < 2)
	{
		return("{\"MBDBAPI_Status\":\"Invalid number of arguments\"}");
	}
	std::vector<MBDB::MBDB_RowData> RowResponse = {};
	MBError DatabaseError(true);
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		std::string SQlQuerry = "SELECT * FROM " + Arguments[0] + " " + Arguments[1];
		RowResponse = WritableDatabase->GetAllRows(SQlQuerry, &DatabaseError);
	}
	if (!DatabaseError)
	{
		return("{\"MBDBAPI_Status\":"+ToJason(DatabaseError.ErrorMessage)+"}");
	}
	ReturnValue = "{\"MBDBAPI_Status\":\"ok\",\"Rows\":" + MakeJasonArray(RowResponse)+"}";
	return(ReturnValue);
}
std::string DBAPI_Login(std::vector<std::string> const& Arguments)
{
	if (Arguments.size() != 2)
	{
		return("{\"MBDBAPI_Status\":\"Invalid Function call\"}");
	}
	std::vector<MBDB::MBDB_RowData> UserResult = DB_GetUser(Arguments[0], Arguments[1]);
	if (UserResult.size() == 0)
	{
		return("{\"MBDBAPI_Status\":\"Invalid UserName or Password\"}");
	}
	return("{\"MBDBAPI_Status\":\"ok\"}");
}
std::string DBAPI_GetAvailableIndexes(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "{\"MBDBAPI_Status\":\"ok\",";
	std::vector<std::string> AvailableIndexes = {};
	std::filesystem::directory_iterator DirectoryIterator(MBDBGetResourceFolderPath() + "Indexes");
	for (auto& DirectoryEntry : DirectoryIterator)
	{
		AvailableIndexes.push_back(DirectoryEntry.path().filename().generic_string());
	}
	ReturnValue += ToJason(std::string("AvailableIndexes"))+":"+MakeJasonArray(AvailableIndexes)+"}";
	return(ReturnValue);
}
bool DB_IndexExists(std::string const& IndexToCheck)
{
	return(std::filesystem::exists(MBDBGetResourceFolderPath() + "/Indexes/" + IndexToCheck));
}
std::string DBAPI_GetIndexSearchResult(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "{\"MBDBAPI_Status\":";
	if (Arguments.size() < 3)
	{
		ReturnValue += ToJason("Invalid function call")+"}";
	}
	if (!DB_IndexExists(Arguments[0]))
	{
		ReturnValue += ToJason("Index doesn't exists")+"}";
	}
	else
	{
		ReturnValue += "\"ok\",";
		std::lock_guard<std::mutex> Lock(DBIndexMapMutex);
		MBSearchEngine::MBIndex& IndexToSearch = (*__DBIndexMap)[Arguments[0]];
		std::vector<std::string> Result = {};
		if (Arguments[1] == "Boolean")
		{
			Result = IndexToSearch.EvaluteBooleanQuerry(Arguments[2]);
		}
		else
		{
			Result = IndexToSearch.EvaluteVectorModelQuerry(Arguments[2]);
		}
		ReturnValue += ToJason(std::string("IndexSearchResult"))+":"+MakeJasonArray(Result);
		ReturnValue += "}";
	}
	return(ReturnValue);
}
//
MBSockets::HTTPDocument DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	std::string RequestBody = MrPostOGet::GetRequestContent(RequestData);
	MBSockets::HTTPDocument ReturnValue;
	std::string Resourcepath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.Type = MBSockets::HTTPDocumentType::json;
	if (RequestType == "POST")
	{
		//tar fram api funktionen

		//eftersom det kan vara svårt att parsa argument med godtycklig text har varje argument först hur många bytes argumentent är
		size_t FirstSpace = RequestBody.find(" ");
		std::string APIDirective = DBGeneralAPIGetDirective(RequestBody);
		std::vector<std::string> APIDirectiveArguments = DBGeneralAPIGetArguments(RequestBody);
		DBPermissionsList ConnectionPermissions = GetConnectionPermissions(RequestData);
		if (APIDirective == "GetTableNames")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = GetTableNamesBody(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "GetTableInfo")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = GetTableInfoBody(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "AddEntryToTable")
		{
			if (ConnectionPermissions.Upload)
			{
				//Funktions prototyp TableNamn + ColumnName:stringcolumm data
				ReturnValue.DocumentData = DBAPI_AddEntryToTable(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require Upload permissions\"}";
			}
		}
		else if (APIDirective == "GetFolderContents")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = DBAPI_GetFolderContents(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "SearchTableWithWhere")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = DBAPI_SearchTableWithWhere(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "UpdateTableRow")
		{
			if (ConnectionPermissions.Upload)
			{
				ReturnValue.DocumentData = DBAPI_UpdateTableRow(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to Edit\"}";
			}
		}
		else if (APIDirective == "Login")
		{
			ReturnValue.DocumentData = DBAPI_Login(APIDirectiveArguments);
			std::string StringToCompare = "{\"MBDBAPI_Status\":\"ok\"}";
			if (ReturnValue.DocumentData.substr(0,StringToCompare.size()) == StringToCompare)
			{
				ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBUsername=" + APIDirectiveArguments[0] + "; Secure; " + "Max-Age=604800; Path=/");
				ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBPassword=" + APIDirectiveArguments[1] + "; Secure; " + "Max-Age=604800; Path=/");
			}
		}
		else if (APIDirective == "FileExists")
		{
			if (!ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to Read\"}";
			}
			else if (APIDirectiveArguments.size() != 1)
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid function call\"}";
			}
			else
			{
				std::string NewDocumentData = "{\"MBDBAPI_Status\":\"ok\",";
				NewDocumentData += "\"FileExists\":" + ToJason(std::filesystem::exists(MBDBGetResourceFolderPath() + APIDirectiveArguments[0]))+",";
				std::filesystem::path NewFilepath(APIDirectiveArguments[0]);
				NewFilepath = NewFilepath.parent_path();
				std::filesystem::path BaseFilepath(MBDBGetResourceFolderPath());
				bool FoldersExists = true;
				for (auto const& Directory : NewFilepath)
				{
					BaseFilepath += "/";
					BaseFilepath += Directory;
					if (!std::filesystem::exists(BaseFilepath))
					{
						FoldersExists = false;
						break;
					}
				}
				NewDocumentData += "\"DirectoriesExists\":" + ToJason(FoldersExists)+"}";
				ReturnValue.DocumentData = NewDocumentData;
			}
		}
		else if(APIDirective == "GetAvailableIndexes")
		{
			ReturnValue.DocumentData = DBAPI_GetAvailableIndexes(APIDirectiveArguments);
		}
		else if (APIDirective == "GetIndexSearchResult")
		{
			ReturnValue.DocumentData = DBAPI_GetIndexSearchResult(APIDirectiveArguments);
		}
		else
		{
			ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"UnknownCommand\"}";
			//ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(Resourcepath + "404.html", Resourcepath);
		}
	}
	else
	{
		ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
		ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(Resourcepath+"404.html", Resourcepath);
	}
	std::cout << ReturnValue.DocumentData << std::endl;
	return(ReturnValue);
}
bool DBUpdate_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBUpdate")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "DBUpdate.html", ResourcePath);
	return(ReturnValue);
}