#include <MrBoboDatabase/MrBoboDatabase.h>
#include <SQLite/sqlite3.h>
#include <iostream>
namespace MBDB
{
	void* CopyColumnData(MBDB_ColumnValueTypes TypeToCopy, void* DataToCopy)
	{
		void* NewData = nullptr;
		if (TypeToCopy == MBDB_ColumnValueTypes::Float)
		{
			NewData = new float;
			*(float*)NewData = *((float*)DataToCopy);
		}
		if (TypeToCopy == MBDB_ColumnValueTypes::Double)
		{
			NewData = new double;
			*(double*)NewData = *((double*)DataToCopy);
		}
		if (TypeToCopy == MBDB_ColumnValueTypes::Int32)
		{
			NewData = new int;
			*(int*)NewData = *((int*)DataToCopy);
		}
		if (TypeToCopy == MBDB_ColumnValueTypes::Int64)
		{
			NewData = new long long;
			*(long long*)NewData = *((long long*)DataToCopy);
		}
		if (TypeToCopy == MBDB_ColumnValueTypes::Text)
		{
			NewData = new std::string();
			*(std::string*)NewData = *((std::string*)DataToCopy);
		}
		if (TypeToCopy == MBDB_ColumnValueTypes::Blob)
		{
			NewData = new std::string();
			*(std::string*)NewData = *((std::string*)DataToCopy);
		}
		return(NewData);
	}
	MBDB_RowData::MBDB_RowData(MBDB_RowData& RowToCopy)
	{
		RawColumnData.resize(RowToCopy.RawColumnData.size());
		ColumnValueTypes = RowToCopy.ColumnValueTypes;
		size_t NumberOfColumns = RawColumnData.size();
		for (size_t i = 0; i < NumberOfColumns; i++)
		{
			delete(RawColumnData[i]);
			RawColumnData[i] = CopyColumnData(ColumnValueTypes[i],RowToCopy.RawColumnData[i]);
		}
	}
	MBDB_RowData& MBDB_RowData::operator=(MBDB_RowData RowToCopy)
	{
		//Eftersom vi definierat en copy constructer kan vi ta detta by value
		swap(*this, RowToCopy);
		return(*this);
	}
	MBDB_RowData::MBDB_RowData(MBDB_RowData&& RowToCopy)
	{
		swap(*this, RowToCopy);
	}
	MBDB_RowData& MBDB_RowData::operator=(MBDB_RowData&& RowToCopy)
	{
		//Eftersom vi definierat en copy constructer kan vi ta detta by value
		swap(*this, RowToCopy);
		return(*this);
	}
	MBDB_RowData::~MBDB_RowData()
	{
		size_t NumberOfColumns = RawColumnData.size();
		for (size_t i = 0; i < NumberOfColumns; i++)
		{
			delete(RawColumnData[i]);
		}
	}
	std::string JsonEncodeValue(MBDB_ColumnValueTypes DataType, void* RawData)
	{
		std::string ReturnValue = "";
		if (DataType == MBDB_ColumnValueTypes::Float)
		{
			ReturnValue += std::to_string(*(float*)RawData);
		}
		else if (DataType == MBDB_ColumnValueTypes::Double)
		{
			ReturnValue += std::to_string(*(double*)RawData);
		}
		else if (DataType == MBDB_ColumnValueTypes::Int32)
		{
			ReturnValue += std::to_string(*(int*)RawData);
		}
		else if (DataType == MBDB_ColumnValueTypes::Int64)
		{
			ReturnValue += std::to_string(*(long long*)RawData);
		}
		else if (DataType == MBDB_ColumnValueTypes::Text)
		{
			ReturnValue += "\""+*(std::string*)RawData+"\"";
		}
		return(ReturnValue);
	}
	std::string MBDB_RowData::ToJason()
	{
		size_t NumberOfColumns = RawColumnData.size();
		std::string ReturnValue = "{";
		for (size_t i = 0; i < NumberOfColumns; i++)
		{
			ReturnValue += "\"" + std::to_string(i) + "\"" + ":";
			ReturnValue += JsonEncodeValue(ColumnValueTypes[i], RawColumnData[i]);
			if (i+1 < NumberOfColumns)
			{
				ReturnValue += ",";
			}
		}
		ReturnValue += "}";
		return(ReturnValue);
	}


	//MrBoboDatabase
	MrBoboDatabase::MrBoboDatabase(std::string const& FilePath,unsigned int Options)
	{
		int ErrorCode = sqlite3_open_v2(FilePath.c_str(), &UnderlyingConnection, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr);
		if (ErrorCode != SQLITE_OK)
		{
			std::cout << sqlite3_errmsg(UnderlyingConnection) << std::endl;
			assert(false);
		}
	}
	MBDB_ColumnValueTypes SQLite3TypeToMBType(int TypeToConvert)
	{
		MBDB_ColumnValueTypes ReturnValue = MBDB_ColumnValueTypes::Null;
		if (TypeToConvert == SQLITE_FLOAT)
		{
			ReturnValue = MBDB_ColumnValueTypes::Double;
		}
		else if (TypeToConvert == SQLITE_INTEGER)
		{
			ReturnValue = MBDB_ColumnValueTypes::Int32;
		}
		else if (TypeToConvert == SQLITE_BLOB)
		{
			ReturnValue = MBDB_ColumnValueTypes::Blob;
		}
		else if (TypeToConvert == SQLITE3_TEXT)
		{
			ReturnValue = MBDB_ColumnValueTypes::Text;
		}
		else if(TypeToConvert == SQLITE_NULL)
		{
			ReturnValue = MBDB_ColumnValueTypes::Null;
		}
		return(ReturnValue);
	}
	void* CopySQLite3Data(sqlite3_stmt* StatementToInterpret, int ColumnIndex, int SQLite3Type)
	{
		void* ReturnValue = nullptr;
		if (SQLite3Type == SQLITE_FLOAT)
		{
			ReturnValue = new double;
			*(double*)ReturnValue = sqlite3_column_double(StatementToInterpret, ColumnIndex);
		}
		else if (SQLite3Type == SQLITE_INTEGER)
		{
			ReturnValue = new int;
			*(int*)ReturnValue = sqlite3_column_int(StatementToInterpret, ColumnIndex);
		}
		else if (SQLite3Type == SQLITE_BLOB)
		{
			ReturnValue = new std::string();
			size_t StringSize = sqlite3_column_bytes(StatementToInterpret, ColumnIndex);
			*(std::string*)ReturnValue = std::string((char*)sqlite3_column_blob(StatementToInterpret, ColumnIndex),StringSize);
		}
		else if (SQLite3Type == SQLITE3_TEXT)
		{
			ReturnValue = new std::string();
			size_t StringSize = sqlite3_column_bytes(StatementToInterpret, ColumnIndex);
			const char* StringData = (char*)sqlite3_column_text(StatementToInterpret, ColumnIndex);
			std::string NewString = std::string(StringData, StringSize);
			*((std::string*)ReturnValue) = NewString;
		}
		else if(SQLite3Type == SQLITE_NULL)
		{
			assert(false);
		}
		return(ReturnValue);
	}
	MBDB_RowData CreateRowFromSQLiteStatement(sqlite3_stmt* StatementToInterpret)
	{
		int NumberOfRows = sqlite3_column_count(StatementToInterpret);
		MBDB_RowData NewRow;
		for (size_t i = 0; i < NumberOfRows; i++)
		{
			MBDB_ColumnValueTypes NewColumnType = MBDB_ColumnValueTypes::Null;
			int SQLite3_ColumnType = sqlite3_column_type(StatementToInterpret, i);
			NewColumnType = SQLite3TypeToMBType(SQLite3_ColumnType);
			if (NewColumnType != MBDB_ColumnValueTypes::Null)
			{
				NewRow.ColumnValueTypes.push_back(NewColumnType);
				NewRow.RawColumnData.push_back(CopySQLite3Data(StatementToInterpret, i, SQLite3_ColumnType));
			}
			else
			{
				continue;
				//assert(false);
			}
		}
		return(NewRow);
	}
	std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(std::string const& SQLQuerry,MBError* OutError)
	{
		std::vector<MBDB_RowData> ReturnValue = {};
		MBError ErrorToReturn(true);
		sqlite3_stmt* NewStatement;
		int ErrorCode;
		const char* UnusedPortion;
		ErrorCode = sqlite3_prepare_v2(UnderlyingConnection, SQLQuerry.data(), SQLQuerry.size(), &NewStatement,&UnusedPortion);
		if (ErrorCode != SQLITE_OK)
		{
			std::cout << sqlite3_errmsg(UnderlyingConnection) << std::endl;
			//assert(false);
		}
		else
		{
			//statementen �r fungerande
			int StatementValue = 0;
			while ((StatementValue = sqlite3_step(NewStatement)) != SQLITE_DONE)
			{
				if (StatementValue == SQLITE_ROW)
				{
					ReturnValue.push_back(CreateRowFromSQLiteStatement(NewStatement));
				}
				else if (StatementValue == SQLITE_ERROR)
				{
					std::cout << sqlite3_errmsg(UnderlyingConnection) << std::endl;
					break;
				}
				else if(StatementValue == SQLITE_MISUSE)
				{
					assert(false);
				}
				else if (StatementValue == SQLITE_BUSY)
				{
					assert(false);
				}
				else
				{
					break;
				}
			}
		}
		sqlite3_finalize(NewStatement);
		return(ReturnValue);
	}
	void swap(MBDB::MBDB_RowData& Row1, MBDB::MBDB_RowData& Row2)
	{
		std::swap(Row1.RawColumnData, Row2.RawColumnData);
		std::swap(Row1.ColumnValueTypes, Row2.ColumnValueTypes);
	}
}