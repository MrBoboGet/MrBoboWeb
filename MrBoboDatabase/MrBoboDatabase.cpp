#include <MrBoboDatabase/MrBoboDatabase.h>
#include <SQLite/sqlite3.h>
#include <iostream>
namespace MBDB
{
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
		else if (TypeToConvert == SQLITE_NULL)
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
			*(std::string*)ReturnValue = std::string((char*)sqlite3_column_blob(StatementToInterpret, ColumnIndex), StringSize);
		}
		else if (SQLite3Type == SQLITE3_TEXT)
		{
			ReturnValue = new std::string();
			size_t StringSize = sqlite3_column_bytes(StatementToInterpret, ColumnIndex);
			const char* StringData = (char*)sqlite3_column_text(StatementToInterpret, ColumnIndex);
			std::string NewString = std::string(StringData, StringSize);
			*((std::string*)ReturnValue) = NewString;
		}
		else if (SQLite3Type == SQLITE_NULL)
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
			NewRow.ColumnValueTypes.push_back(NewColumnType);
			if (NewColumnType != MBDB_ColumnValueTypes::Null)
			{
				NewRow.RawColumnData.push_back(CopySQLite3Data(StatementToInterpret, i, SQLite3_ColumnType));
			}
			else
			{
				NewRow.RawColumnData.push_back(nullptr);
			}
		}
		return(NewRow);
	}
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
			ReturnValue += ToJason(*(std::string*)RawData);
		}
		else if (DataType == MBDB_ColumnValueTypes::Null)
		{
			ReturnValue += "null";
		}
		return(ReturnValue);
	}
	MBDB_ColumnValueTypes MBDB_RowData::GetColumnValueType(int ColumnIndex)
	{
		return(ColumnValueTypes[ColumnIndex]);
	}
	bool MBDB_RowData::ColumnValueIsNull(int ColumnIndex)
	{
		return(RawColumnData[ColumnIndex] == nullptr);
	}
	std::string MBDB_RowData::JSONEncodeValue(size_t ColumnIndex) const
	{
		return(JsonEncodeValue(ColumnValueTypes[ColumnIndex], RawColumnData[ColumnIndex]));
	}
	std::string MBDB_RowData::ToJason() const
	{
		size_t NumberOfColumns = RawColumnData.size();
		std::string ReturnValue = "{\"ColumnCount\":"+std::to_string(RawColumnData.size())+",";
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

	//SQLStatement
	MBError SQLStatement::BindString(std::string const& ParameterData, int ParameterIndex)
	{
		MBError ReturnValue = MBError(true);
		int Error = sqlite3_bind_text(UnderlyingStatement, ParameterIndex, ParameterData.c_str(), ParameterData.size(), SQLITE_TRANSIENT);
		if (Error != SQLITE_OK)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = sqlite3_errstr(Error);
		}
		return(ReturnValue);
	}
	MBError SQLStatement::BindInt(MaxInt IntToBind, int ParameterIndex)
	{
		MBError ReturnValue = MBError(true);
		int Error = sqlite3_bind_int(UnderlyingStatement, ParameterIndex, IntToBind);
		if (Error != SQLITE_OK)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = sqlite3_errstr(Error);
		}
		return(ReturnValue);
	}
	MBError SQLStatement::FreeData()
	{
		MBError ReturnValue = MBError(true);
		int Error = sqlite3_finalize(UnderlyingStatement);
		if (!Error)
		{
			Error = false;
			ReturnValue.ErrorMessage = sqlite3_errstr(Error);
		}
		return(Error);
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
	MBError SQLStatement::BindNull(int ParameterIndex)
	{
		MBError ReturnValue(true);
		int Error = sqlite3_bind_null(UnderlyingStatement, ParameterIndex);
		if (Error != SQLITE_OK)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = sqlite3_errstr(Error);
		}
		return(ReturnValue);
	}
	MBError SQLStatement::BindValues(std::vector<std::string> const& ValuesToBind, std::vector<ColumnSQLType> const& ValueTypes, int Offset)
	{
		MBError ReturnValue(true);
		for (size_t i = 0; i < ValueTypes.size(); i++)
		{
			//if (ValuesToBind[i] == "null")
			//{
			//	ReturnValue = BindNull(i + 1 + Offset);
			//	continue;
			//}
			if (ValueTypes[i] == ColumnSQLType::Int)
			{
				MBDB::MaxInt NewInt = StringToInt(ValuesToBind[i], &ReturnValue);
				if (!ReturnValue)
				{
					break;
				}
				ReturnValue = BindInt(NewInt, i +1+Offset);
			}
			else
			{
				ReturnValue = BindString(ValuesToBind[i], i + 1+Offset);
				if (!ReturnValue)
				{
					break;
				}
			}
		}
		return(ReturnValue);
	}
	std::vector<MBDB_RowData> SQLStatement::GetAllRows(sqlite3* DBConnection, MBError* ErrorToReturn = nullptr)
	{
		//statementen är fungerande
		std::vector<MBDB_RowData> ReturnValue = {};
		int StatementValue = 0;
		if (IsValid())
		{
			while ((StatementValue = sqlite3_step(UnderlyingStatement)) != SQLITE_DONE)
			{
				if (StatementValue == SQLITE_ROW)
				{
					ReturnValue.push_back(CreateRowFromSQLiteStatement(UnderlyingStatement));
				}
				else if (StatementValue == SQLITE_ERROR)
				{
					std::string ErrorMessage = sqlite3_errmsg(DBConnection);
					std::cout << ErrorMessage << std::endl;
					if (ErrorToReturn != nullptr)
					{
						*ErrorToReturn = MBError(false);
						ErrorToReturn->ErrorMessage = ErrorMessage;
					}
					break;
				}
				else if (StatementValue == SQLITE_MISUSE)
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
		else
		{
			if (ErrorToReturn != nullptr)
			{
				*ErrorToReturn = MBError(false);
				ErrorToReturn->ErrorMessage = "Statement was invalid";
			}
		}
		return(ReturnValue);
	}
	SQLStatement::SQLStatement(std::string const& SQLQuerry, sqlite3* SQLiteConnection)
	{
		int ErrorCode;
		const char* UnusedPortion;
		ErrorCode = sqlite3_prepare_v2(SQLiteConnection, SQLQuerry.data(), SQLQuerry.size(), &UnderlyingStatement, &UnusedPortion);
		if (ErrorCode != SQLITE_OK)
		{
			IsInvalid = true;
			std::cout << sqlite3_errmsg(SQLiteConnection) << std::endl;
			//assert(false);
		}
	}
	//MrBoboDatabase
	SQLStatement* MrBoboDatabase::GetSQLStatement(std::string const& SQLCode)
	{
		return(new SQLStatement(SQLCode,UnderlyingConnection));
	}
	MBError MrBoboDatabase::FreeSQLStatement(SQLStatement* StatementToFree)
	{
		delete StatementToFree;
		return(MBError(true));
	}
	std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(SQLStatement* StatementToEvaluate, MBError* ErrorToReturn)
	{
		return(StatementToEvaluate->GetAllRows(UnderlyingConnection, ErrorToReturn));
	}
	MrBoboDatabase::MrBoboDatabase(std::string const& FilePath,unsigned int Options)
	{
		int DatabaseOptions = SQLITE_OPEN_FULLMUTEX;
		if ((Options & 1) == 0)
		{
			DatabaseOptions |= SQLITE_OPEN_READONLY;
		}
		else
		{
			DatabaseOptions |= SQLITE_OPEN_READWRITE;
		}
		int ErrorCode = sqlite3_open_v2(FilePath.c_str(), &UnderlyingConnection, DatabaseOptions, nullptr);
		if (ErrorCode != SQLITE_OK)
		{
			std::cout << sqlite3_errmsg(UnderlyingConnection) << std::endl;
			assert(false);
		}
	}
	std::vector<std::string> MrBoboDatabase::GetAllTableNames()
	{
		std::vector<std::string> ReturnValue = {};
		//sqlite dependant
		std::vector<MBDB_RowData> Names =  GetAllRows("SELECT name FROM sqlite_schema WHERE type = 'table' ORDER BY name;");
		for (size_t i = 0; i < Names.size(); i++)
		{
			ReturnValue.push_back(Names[i].GetColumnData<std::string>(0));
		}
		return(ReturnValue);
	}
	std::vector<ColumnInfo> MrBoboDatabase::GetColumnInfo(std::string const& TableName)
	{
		//just nu är det en SQLITE3 databas så vi använder dess
		std::vector<ColumnInfo> ReturnValue = {};
		std::vector<MBDB_RowData> TableInfo =  GetAllRows("SELECT * FROM pragma_table_info('" + TableName + "');");
		size_t TableInfoSize = TableInfo.size();
		for (size_t i = 0; i < TableInfoSize; i++)
		{
			ColumnInfo NewColumnInfo;
			NewColumnInfo.ColumnName = TableInfo[i].GetColumnData<std::string>(1);
			NewColumnInfo.ColumnType = ColumnTypeFromString(TableInfo[i].GetColumnData<std::string>(2));
			//Detta är prolly fel, relevant om vi komerm behöva använda det
			NewColumnInfo.Nullable = (TableInfo[i].GetColumnData<int>(3) == 0);
			//TODO detta kan enkelt ge undefined behavior om vi har olika datatyper som vi sparar i columntypen och som vi accessar.
			//borde göra en typedef för vilken integertype jag använder för att spara datan
			NewColumnInfo.PrimaryKeyIndex = TableInfo[i].GetColumnData<int>(5);
			ReturnValue.push_back(NewColumnInfo);
		}

		return(ReturnValue);
	}
	std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(std::string const& SQLQuerry,MBError* OutError)
	{
		std::vector<MBDB_RowData> ReturnValue = {};
		MBError ErrorToReturn(true);
		SQLStatement NewStatement(SQLQuerry, UnderlyingConnection);
		//sqlite3_stmt* NewStatement;
		//int ErrorCode;
		//const char* UnusedPortion;
		//ErrorCode = sqlite3_prepare_v2(UnderlyingConnection, SQLQuerry.data(), SQLQuerry.size(), &NewStatement,&UnusedPortion);
		if (!NewStatement.IsValid())
		{
			//assert(false);
			if (OutError != nullptr)
			{
				*OutError = false;
				OutError->ErrorMessage = sqlite3_errmsg(UnderlyingConnection);
				std::cout << OutError->ErrorMessage << std::endl;
			}
		}
		else
		{
			//statementen är fungerande
			ReturnValue = NewStatement.GetAllRows(UnderlyingConnection, OutError);
		}
		NewStatement.FreeData();
		return(ReturnValue);
	}
	void swap(MBDB::MBDB_RowData& Row1, MBDB::MBDB_RowData& Row2)
	{
		std::swap(Row1.RawColumnData, Row2.RawColumnData);
		std::swap(Row1.ColumnValueTypes, Row2.ColumnValueTypes);
	}
	std::string ColumnSQLTypeToString(ColumnSQLType ValueToConvert)
	{
		if (ValueToConvert == ColumnSQLType::Int)
		{
			return("int");
		}
		else if (ValueToConvert == ColumnSQLType::varchar)
		{
			return("varchar");
		}
		else if(ValueToConvert == ColumnSQLType::Null)
		{
			return("null");
		}
	}
}
std::string ToJason(bool ValueTojason)
{
	if (ValueTojason == true)
	{
		return("true");
	}
	else
	{
		return("false");
	}
}
std::string ToJason(MBDB::ColumnSQLType ValueToJason)
{
	return("\"" + ColumnSQLTypeToString(ValueToJason) + "\"");
}
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
std::string ToJason(long long ValueToJason)
{
	return(std::to_string(ValueToJason));
}
std::string ToJason(MBDB::ColumnInfo const& ValueToJason)
{
	std::string ReturnValue = "{";
	ReturnValue += "\"ColumnName\":" + ToJason(ValueToJason.ColumnName) + ",";
	ReturnValue += "\"ColumnType\":" + ToJason(ValueToJason.ColumnType) + ",";
	ReturnValue += "\"IsNullable\":" + ToJason(ValueToJason.Nullable) + ",";
	ReturnValue += "\"PrimaryKeyIndex\":" + ToJason((long long)ValueToJason.PrimaryKeyIndex) + "}";
	return(ReturnValue);
}
std::string ToJason(MBDB::MBDB_RowData const& ValueToJason)
{
	return(ValueToJason.ToJason());
}
std::string CombineJSONObjects(std::vector<std::string> const& ObjectNames, std::vector<std::string> const& ObjectsData)
{
	std::string ReturnValue = "{";
	for (size_t i = 0; i < ObjectNames.size(); i++)
	{
		ReturnValue += ToJason(ObjectNames[i])+":";
		ReturnValue += ObjectsData[i];
		if (i < ObjectNames.size())
		{
			ReturnValue += ",";
		}
	}
	ReturnValue += "}";
	return(ReturnValue);
}
