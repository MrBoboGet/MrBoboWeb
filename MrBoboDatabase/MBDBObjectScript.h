#pragma once
#include <vector>
#include <MBUtility/MBErrorHandling.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <map>
#include <memory>

namespace MBDB
{
	enum class MBDBO_Type
	{
		String,
		Integer,
		Float,
		Boolean,
		Array,
		AggregateObject,//inneb�r att listan av attributes
		UnevaluatedDatabaseExpression,
		Null
	};
	struct MBDBO_EvaluationInfo
	{
		bool ParseOnly = false;
		std::string ObjectDirectory = "";
		std::string EvaluatingUser = "";
		MBDB::MrBoboDatabase* AssociatedDatabase = nullptr;
	};
	class MBDB_Object;
	typedef std::vector<MBDB_Object> MBDB_Object_ArrayType;
	typedef std::map<std::string, MBDB_Object> MBDB_Object_MapType;
	//typedef MBDBS
	class MBDB_Object
	{
	private:
		friend void swap(MBDB_Object& LeftObject, MBDB_Object& RightObject);
		//std::map<std::string, std::unique_ptr<MBDB_Object>> m_Attributes = {};
		//std::vector<std::unique_ptr<MBDB_Object>> m_ArrayObjects = {};
		MBDBO_Type m_Type = MBDBO_Type::Null;
		void* m_AtomicData = nullptr;
		bool m_FullyEvaluated = true;

		void* p_CopyData() const;
		void p_FreeData() const;
		static std::string p_ExtractTag(const void* Data,size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError = nullptr);
		static std::string p_ExtractTag(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError = nullptr);

		//static std::string p_ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//static intmax_t p_ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//static MBDB_Object p_

		//static void p_SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
		//static void p_SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset);

		//static MBDB_Object p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& TableExpression);
		//static MBDB_Object p_EvaluateTableExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//static MBDB_Object p_EvaluateDBObjectExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//static MBDB_Object p_EvaluateDBObjectExpression(MBDBO_EvaluationInfo& EvaluationInfo, const void* Data,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		
		
		//static MBDB_Object p_ParseObject(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

		static MBDB_Object p_EvaluateObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static MBDB_Object p_EvaluateObject(MBDBO_EvaluationInfo& EvaluationInfo, const void* Data,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		
		static MBDB_Object p_ParseAggregateObject(MBDBO_EvaluationInfo& EvaluationInfo, const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static MBDB_Object p_ParseAggregateObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		
		static MBDB_Object p_ParseArrayObject(MBDBO_EvaluationInfo& EvaluationInfo, const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static MBDB_Object p_ParseArrayObject(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		
		static MBDB_Object p_ParseDatabaseExpression(MBDBO_EvaluationInfo& EvaluationInfo, const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static MBDB_Object p_ParseDatabaseExpression(MBDBO_EvaluationInfo& EvaluationInfo, std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		
		static MBDB_Object p_ParseStaticAtomicObject(const void* Data,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static MBDB_Object p_ParseStaticAtomicObject(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

		std::string p_AggregateToJSON() const;
		std::string p_ArrayToJSON() const;
		std::string p_AtomicToJSON() const;

		std::string p_AggregateToFormattedJSON(size_t SpacesBefore) const;
		std::string p_ArrayToFormattedJSON(size_t SpacesBefore) const;
		std::string p_AtomicToFormattedJSON(size_t SpacesBefore) const;

	public:
		MBDB_Object() {};
		MBDB_Object(std::string const& StringData);
		MBDB_Object(intmax_t IntegerData);
		MBDB_Object(std::vector<MBDB_Object>&& ArrayToMove);
		MBDB_Object(std::vector<MBDB_Object> const& ArrayToCopy);

		MBDB_Object(MBDB_Object const& ObjectToCopy);
		MBDB_Object(MBDB_Object&& ObjectToMove) noexcept;
		~MBDB_Object();
		MBDB_Object& operator=(MBDB_Object RightObject) noexcept;
		//MBDB_Object& operator=(MBDB_Object&& RightObject) noexcept; //Inte helt s�ker p� om det h�r pessimiserar koden, men ville garantera att std::vector movar objekten

		MBError LoadObject(std::string const& ObjectFilepath, std::string const& AssociatedUser, MBDB::MrBoboDatabase* AssociatedDatabase); //read �r implicit
		MBError LoadObject(MBDBO_EvaluationInfo& CurrentEvaluationInfo, std::string const& ObjectFilepath); //read �r implicit

		static MBDB_Object ParseObject(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset,MBError* OutError);
		static MBDB_Object ParseObject(std::string const& DataToParse, size_t InOffset, size_t* OutOffset,MBError* OutError);
		void Evaluate(MBDBO_EvaluationInfo& EvaluationInfo,MBError* OutError);

		bool IsAggregate() const;
		bool HasAttribute(std::string const& AttributeName) const;
		bool IsEvaluated() const;
	
		MBDB_Object& GetAttribute(std::string const& AttributeName);
		MBDB_Object const& GetAttribute(std::string const& AttributeName) const;
		
		MBDBO_Type GetType() const;
		std::string GetStringData() const;
		intmax_t GetIntegerData() const;
		MBDB_Object_ArrayType& GetArrayData();
		MBDB_Object_ArrayType const& GetArrayData() const;
		
		std::string ToJason() const;
		std::string ToFormattedJason(size_t CurrentDepth = 0) const;
	};

	enum class MBDBObjectScript_StatementType
	{
		FunctionCall,
		ObjectField,
		Literal,
		UnevaluatedObject,
		Null
	};
	class MBDBObjectScript_Statement
	{
	private:
		friend class MBDBObjectScript_ParsingContext;
		friend class MBDBObjectScript_ExecutionContext;
		friend void swap(MBDBObjectScript_Statement& LeftStatement, MBDBObjectScript_Statement& RightStatement) noexcept;
		MBDBObjectScript_StatementType m_Type = MBDBObjectScript_StatementType::Null;
		void* m_StatementData = nullptr;
		void p_FreeData() const;
		void* p_CopyData() const;
		MBDB_Object p_GetLiteralObject() const;
	public:
		//MBDB_Object Evaluate();
		MBDBObjectScript_Statement() {};
		MBDBObjectScript_Statement(MBDBObjectScript_Statement const& ObjectToCopy);
		MBDBObjectScript_Statement(MBDBObjectScript_Statement&& ObjectToCopy) noexcept;
		MBDBObjectScript_Statement& operator=(MBDBObjectScript_Statement ObjectToCopy);
		~MBDBObjectScript_Statement();
	};
	typedef MBDBObjectScript_Statement MBDB_Object_UnevaluatedExpressionType;
	struct MBDBObjectScript_FunctionStatementData
	{
		std::string FunctionIdentifier = "";
		std::vector< MBDBObjectScript_Statement> FunctionArguments = {};
	};
	enum class MBDBObjectScript_LiteralType
	{
		Integer,
		Float,
		String,
		Null
	};
	//F�r lat f�r att faktiskt ha olika structs men walla liksom
	struct MBDBObjectScript_LiteralStatementData
	{
		MBDBObjectScript_LiteralType Type = MBDBObjectScript_LiteralType::Null;
		intmax_t IntegerData = 0;
		std::string StringData = "";
	};
	struct MBDBObjectScript_ObjectFieldStatementData
	{
		MBDBObjectScript_Statement ObjectToEvaluate;
		std::string FieldIdentifier = "";
	};
	struct MBDBObjectScript_UnEvaluatedObjectStatementData
	{
		MBDB_Object UnEvaluatedObject;
	};
	enum class MBDBObjectScript_IdentifierType
	{
		Function,
		Literal,
		ObjectField,
		Null
	};
	struct MBDBObjectScript_Identifier
	{
		MBDBObjectScript_IdentifierType Type = MBDBObjectScript_IdentifierType::Null;
	};
	class MBDBObjectScript_ParsingContext
	{
	private:
		std::vector<char> m_IdentifierDelimiters = { '(',')',',','.',' ','\n','\r','\t','+','-','*',';','{','}','[',']'};
		std::vector<char> m_StatementDelimiters = { '(',')',',',';','.','{','}','[',']'};
		std::vector<char> m_NumberChars = { '0','1','2','3','4','5','6','7','8','9' };
		std::vector<char> m_BinaryOperators = { '+','-','*','.'};
		std::map<std::string, MBDBObjectScript_Identifier> m_Identifiers = { 
		{"DBLoadObject",{MBDBObjectScript_IdentifierType::Function}},
		{"DBLoadTable",{MBDBObjectScript_IdentifierType::Function}},
		{"DBExecuteQuerry",{MBDBObjectScript_IdentifierType::Function}},
		{"DBGetColumns",{MBDBObjectScript_IdentifierType::Function}},
		};

		std::string p_ParseIdentifier(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		std::string p_ParseIdentifier(std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//MBDBObjectScript_Statement p_ParseStatement(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		//MBDBObjectScript_Statement p_ParseStatement(std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

		bool p_IdentifierIsLiteral(std::string const& StringToEvaluate);
		MBDBObjectScript_Statement p_GetLiteralStatement(std::string const& StringToEvaluate);

		std::vector<MBDBObjectScript_Statement> p_ParseFunctionArguments(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		std::vector<MBDBObjectScript_Statement> p_ParseFunctionArguments(std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

	public:
		MBDBObjectScript_Statement ParseStatement(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		MBDBObjectScript_Statement ParseStatement(std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	};
	class MBDBObjectScript_FunctionDefinition
	{
		
	};
	class MBDBObjectScript_ExecutionContext;
	typedef MBDB_Object (MBDBObjectScript_ExecutionContext::* MBDB_ECBuiltinFunction)(MBDBO_EvaluationInfo&, std::vector<MBDBObjectScript_Statement> const&, MBError*);
	class MBDBObjectScript_ExecutionContext
	{
	private:
		std::map<std::string, MBDB_ECBuiltinFunction> m_BuiltinFunctions = { 
			{"DBLoadObject",&MBDBObjectScript_ExecutionContext::p_DBLoadObject},
			{"LoadTable",&MBDBObjectScript_ExecutionContext::p_LoadTable},
			{"DBExecuteQuerry",&MBDBObjectScript_ExecutionContext::p_ExecuteQuerry},
			{"DBGetColumns",&MBDBObjectScript_ExecutionContext::p_GetColumns},
			{"[]",&MBDBObjectScript_ExecutionContext::p_EvaluateObjectFunction},
			{"+",&MBDBObjectScript_ExecutionContext::p_AddObjects}
		};
		MBDB_Object p_DBLoadObject(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
		MBDB_Object p_EvaluateObjectFunction(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
		MBDB_Object p_AddObjects(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
		MBDB_Object p_LoadTable(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
		MBDB_Object p_ExecuteQuerry(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
		MBDB_Object p_GetColumns(MBDBO_EvaluationInfo& EvaluationInfo, std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);

		MBDB_Object p_EvaluateArrayIndexing(MBDBO_EvaluationInfo& EvaluationInfo, MBDB_Object const& CallingObject,std::vector<MBDBObjectScript_Statement> const& Arguments, MBError* OutError);
	public:
		MBDB_Object EvaluateStatement(MBDBO_EvaluationInfo& EvaluationInfo,MBDBObjectScript_Statement const& StatementToEvaluate,MBError* OutError);
	};
}
std::string ToJason(MBDB::MBDB_Object const& ObjectToEncode);