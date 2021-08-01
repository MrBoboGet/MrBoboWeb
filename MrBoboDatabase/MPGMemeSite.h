#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrPostOGet/MrPostOGet.h>

struct DBPermissionsList
{
	bool Read = false;
	bool Edit = false;
	bool Upload = false;
	bool IsNull = true;
};
enum class MBDBO_Type
{
	String,
	Integer,
	Float,
	Boolean,
	Array,
	AggregateObject,//innebär att listan av attributes
	Null
};
struct MBDBO_EvaluationInfo
{
	std::string ObjectDirectory = "";
	std::string EvaluatingUser = "";
	MBDB::MrBoboDatabase* AssociatedDatabase = nullptr;
};
class MBDB_Object
{
private:
	friend void swap(MBDB_Object& LeftObject, MBDB_Object& RightObject);
	std::map<std::string, std::unique_ptr<MBDB_Object>> m_Attributes = {};
	std::vector<std::unique_ptr<MBDB_Object>> m_ArrayObjects = {};
	MBDBO_Type m_Type = MBDBO_Type::Null;
	bool m_IsAtomic = false;
	void* m_AtomicData = nullptr;
	
	void* p_CopyAtomicData() const;
	void p_FreeAtomicData();
	static std::string p_ExtractTag(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError = nullptr);

	static std::string p_ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	static intmax_t p_ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	//static MBDB_Object p_

	static void p_SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
	static void p_SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset);

	static MBDB_Object p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo,std::string const& TableExpression);
	static MBDB_Object p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	static MBDB_Object p_EvaluateDBObjectExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	static MBDB_Object p_EvaluateObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData,size_t InOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);
	static MBDB_Object p_ParseAggregateObject(MBDBO_EvaluationInfo & EvaluationInfo,std::string const& ObjectData,size_t InOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);
	static MBDB_Object p_ParseArrayObject(MBDBO_EvaluationInfo & EvaluationInfo,std::string const& ObjectData,size_t InOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);
	static MBDB_Object p_ParseDatabaseExpression(MBDBO_EvaluationInfo & EvaluationInfo,std::string const& ObjectData,size_t InOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);
	static MBDB_Object p_ParseStaticAtomicObject(std::string const& ObjectData,size_t InOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);

	std::string p_AggregateToJSON() const;
	std::string p_ArrayToJSON() const;
	std::string p_AtomicToJSON() const;
public:
	MBDB_Object() {};
	MBDB_Object(MBDB_Object const& ObjectToCopy);
	MBDB_Object(MBDB_Object&& ObjectToMove) noexcept;
	~MBDB_Object();
	MBDB_Object& operator=(MBDB_Object RightObject);

	MBError LoadObject(std::string const& ObjectFilepath,std::string const& AssociatedUser,MBDB::MrBoboDatabase* AssociatedDatabase); //read är implicit
	MBError LoadObject(MBDBO_EvaluationInfo& CurrentEvaluationInfo,std::string const& ObjectFilepath); //read är implicit
	
	bool IsAggregate() const;
	bool HasAttribute(std::string const& AttributeName) const;
	MBDB_Object& GetAttribute(std::string const& AttributeName);
	MBDBO_Type GetType() const;
	std::string ToJason() const;
};
std::string ToJason(MBDB_Object const& ObjectToEncode);


void InitDatabase();

bool DBSite_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedSocket);

bool UploadFile_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument UploadFile_ResponseGenerator(std::string const& RequestData,MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBGet_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBView_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBViewEmbedd_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBAdd_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBGeneralAPI_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBUpdate_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBLogin_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

int MBGWebsiteMain();