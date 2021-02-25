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

		std::string ToJason();

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
		bool Nullable = false;
		int PrimaryKeyIndex = 0;
	};
	inline ColumnSQLType ColumnTypeFromString(std::string const& StringToConvert)
	{
		//ColumnSQLType ReturnValue = ColumnSQLType::Null;
		std::string VarChar = "varchar";
		if (StringToConvert == "int")
		{
			return(ColumnSQLType::Int);
		}
		else if (StringToConvert.substr(0,VarChar.size()) == "varchar")
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
	class MrBoboDatabase
	{
	private:
		sqlite3* UnderlyingConnection = nullptr;
	public:
		MrBoboDatabase(std::string const& DatabaseFile,unsigned int Options);
		std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry,MBError* ErrorToReturn = nullptr);
		std::vector<std::string> GetAllTableNames();
		std::vector<ColumnInfo> GetColumnInfo(std::string const& TableName);
		MBDB_ResultIterator GetResultIterator(std::string const& SQLQuerry, MBError* ErrorToReturn = nullptr);
	};
	std::string ToJason(bool ValueTojason);
	std::string ToJason(ColumnSQLType ValueToJason);
	std::string ToJason(std::string const& ValueToJason);
	std::string ToJason(long long ValueToJason);
	std::string ToJason(MBDB::ColumnInfo const& ValueToJason);

	template<typename T>
	std::string MakeJasonArray(std::vector<T> ValuesToConvert,std::string ArrayName)
	{
		std::string JsonRespone = "{\""+ArrayName+"\":[";
		size_t TableNamesSize = ValuesToConvert.size();
		for (size_t i = 0; i < TableNamesSize; i++)
		{
			JsonRespone += MBDB::ToJason(ValuesToConvert[i]);
			if (i + 1 < TableNamesSize)
			{
				JsonRespone += ",";
			}
		}
		JsonRespone += "]}";
		return(JsonRespone);
	}
}
