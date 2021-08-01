#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <assert.h>
#include <MBErrorHandling.h>
class sqlite3;
struct sqlite3_stmt;
namespace MBDB
{
	typedef int MaxInt;
	enum class MBDB_ColumnValueTypes
	{
		Float,
		Double,
		Int32,
		Int64,
		Text,
		Blob,
		Null
	};
	class MBDB_RowData
	{
	private:
		friend void swap(MBDB::MBDB_RowData& Row1, MBDB::MBDB_RowData& Row2);
		std::vector<MBDB_ColumnValueTypes> ColumnValueTypes = {};
		std::vector<void*> RawColumnData = {};
		template<typename T> bool IsCorrectType(int Index)
		{
			if (typeid(T) == typeid(float))
			{
				return(ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Float);
			}
			if (typeid(T) == typeid(double))
			{
				return(ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Double);
			}
			if (typeid(T) == typeid(int))
			{
				return(ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Int32);
			}
			if (typeid(T) == typeid(long long))
			{
				return(ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Int64);
			}
			if (typeid(T) == typeid(std::string))
			{
				return(ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Text || ColumnValueTypes[Index] == MBDB_ColumnValueTypes::Blob);
			}
		}


		template<typename T>
		std::tuple<T> CreateTuple(int* CurrentIndex) {
			assert(IsCorrectType<T>(*CurrentIndex));
			T NewValue = *((T*)RawColumnData[*CurrentIndex]);
			*CurrentIndex += 1;
			return std::make_tuple(NewValue);
		}

		template<typename T, typename Z,typename... Args>
		std::tuple<T, Z,Args...> CreateTuple(int* CurrentIndex)
		{
			assert(IsCorrectType<T>(*CurrentIndex));
			T NewValue = *((T*)RawColumnData[*CurrentIndex]);
			*CurrentIndex += 1;
			//assertar fel om vi gör en felaktig call
			return std::tuple_cat(std::make_tuple(NewValue), CreateTuple<Z,Args...>(CurrentIndex));
		}
	public:
		//constructors
		friend MBDB_RowData CreateRowFromSQLiteStatement(sqlite3_stmt* StatementToInterpret);
		MBDB_RowData(MBDB_RowData& RowToCopy);
		MBDB_RowData& operator=(MBDB_RowData RowToCopy);
		MBDB_RowData(MBDB_RowData&& RowToCopy);
		MBDB_RowData& operator=(MBDB_RowData&& RowToCopy);
		~MBDB_RowData();

		MBDB_RowData() {};

		std::string ToJason() const;

		template<typename... Args>
		std::tuple<Args...> GetTuple()
		{
			int CurrentIndex = 0;
			return(CreateTuple<Args...>(&CurrentIndex));
		}

		template<typename T> T GetColumnData(int ColumnIndex)
		{
			assert(IsCorrectType<T>(ColumnIndex));
			T ValueToReturn = *((T*)RawColumnData[ColumnIndex]);
			return(ValueToReturn);
		}
		MBDB_ColumnValueTypes GetColumnValueType(int ColumnIndex);
		bool ColumnValueIsNull(int ColumnIndex);
	};
	void swap(MBDB::MBDB_RowData& Row1, MBDB::MBDB_RowData& Row2);

	enum class ColumnSQLType
	{
		varchar,
		Int,
		Null
	};

	struct ColumnInfo
	{
		std::string ColumnName = "";
		ColumnSQLType ColumnType = ColumnSQLType::Null;
		MBDB_ColumnValueTypes ColumnDataType = MBDB_ColumnValueTypes::Null;
		bool Nullable = false;
		int PrimaryKeyIndex = 0;
	};
	inline ColumnSQLType ColumnTypeFromString(std::string const& StringToConvert)
	{
		//ColumnSQLType ReturnValue = ColumnSQLType::Null;
		std::string VarChar = "varchar";
		std::string LowerString = "";
		for (size_t i = 0; i < StringToConvert.size(); i++)
		{
			LowerString += std::tolower(StringToConvert[i]);
		}
		if (LowerString.substr(0,3) == "int")
		{
			return(ColumnSQLType::Int);
		}
		else if (LowerString.substr(0,VarChar.size()) == "varchar")
		{
			return(ColumnSQLType::varchar);
		}
		return(ColumnSQLType::Null);
	}
	class MBDB_ResultIterator
	{
	private:
		std::vector<std::string> ColumnNames = {};
	public:
		bool IsFinished();
		operator bool() { return(!IsFinished()); };
		MBDB_RowData GetNextResult();
	};
	class MrBoboDatabase;
	class SQLStatement
	{
	private:
		friend MrBoboDatabase;
		sqlite3_stmt* UnderlyingStatement = nullptr;
		bool IsInvalid = false;
		SQLStatement(std::string const& SQLCode,sqlite3* DBConnection);
		std::vector<MBDB_RowData> GetAllRows(sqlite3* DBConnection,MBError* OutError);
	public:
		bool IsValid() { return(!IsInvalid); };
		MBError BindString(std::string const& ParameterData, int ParameterIndex);
		MBError BindInt(MaxInt, int ParameterIndex);
		MBError BindNull(int ParameterIndex);
		MBError BindValues(std::vector<std::string> const& ValuesToBind, std::vector<ColumnSQLType> const& ValueTypes, int Offset);
		MBError FreeData();
	};
	class MrBoboDatabase
	{
	private:
		sqlite3* UnderlyingConnection = nullptr;
	public:
		MrBoboDatabase(std::string const& DatabaseFile,unsigned int Options);
		std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry,MBError* ErrorToReturn = nullptr);
		std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry,MrBoboDatabase* StructPointer,MBError* ErrorToReturn = nullptr);
		std::vector<MBDB_RowData> GetAllRows(SQLStatement*,MBError* ErrorToReturn = nullptr);
		std::vector<std::string> GetAllTableNames();
		std::vector<ColumnInfo> GetColumnInfo(std::string const& TableName);
		SQLStatement* GetSQLStatement(std::string const& SQLCode);
		MBError FreeSQLStatement(SQLStatement* StatementToFree);

		MBDB_ResultIterator GetResultIterator(std::string const& SQLQuerry, MBError* ErrorToReturn = nullptr);
	};
}

std::string ToJason(bool ValueTojason);
std::string ToJason(MBDB::ColumnSQLType ValueToJason);
std::string ToJason(MBDB::MBDB_RowData const& RowDataToEncode);
std::string ToJason(std::string const& ValueToJason);
std::string ToJason(long long ValueToJason);
std::string ToJason(MBDB::ColumnInfo const& ValueToJason);
std::string CombineJSONObjects(std::vector<std::string> const& ObjectNames, std::vector<std::string> const& ObjectsData);

template<typename T>
std::string MakeJasonArray(std::vector<T> const& ValuesToConvert, std::string ArrayName)
{
	std::string JsonRespone = "{\"" + ArrayName + "\":[";
	size_t TableNamesSize = ValuesToConvert.size();
	for (size_t i = 0; i < TableNamesSize; i++)
	{
		JsonRespone += ToJason(ValuesToConvert[i]);
		if (i + 1 < TableNamesSize)
		{
			JsonRespone += ",";
		}
	}
	JsonRespone += "]}";
	return(JsonRespone);
}
template<typename T>
std::string MakeJasonArray(std::vector<T> const& ValuesToConvert)
{
	std::string JsonRespone = "[";
	size_t TableNamesSize = ValuesToConvert.size();
	for (size_t i = 0; i < TableNamesSize; i++)
	{
		JsonRespone += ToJason(ValuesToConvert[i]);
		if (i + 1 < TableNamesSize)
		{
			JsonRespone += ",";
		}
	}
	JsonRespone += "]";
	return(JsonRespone);
}