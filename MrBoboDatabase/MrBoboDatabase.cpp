#include "MrBoboDatabase.h"
#include <sqlite3/sqlite3.h>
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
    std::variant<std::monostate,double,long long,std::string> CopySQLite3Data(sqlite3_stmt* StatementToInterpret, int ColumnIndex, int SQLite3Type)
    {
        std::variant<std::monostate,double,long long,std::string> ReturnValue;
        if (SQLite3Type == SQLITE_FLOAT)
        {
            ReturnValue = sqlite3_column_double(StatementToInterpret, ColumnIndex);
        }
        else if (SQLite3Type == SQLITE_INTEGER)
        {
            ReturnValue = (long long)sqlite3_column_int(StatementToInterpret, ColumnIndex);
        }
        else if (SQLite3Type == SQLITE_BLOB)
        {
            size_t StringSize = sqlite3_column_bytes(StatementToInterpret, ColumnIndex);
            ReturnValue = std::string((char*)sqlite3_column_blob(StatementToInterpret, ColumnIndex), StringSize);
        }
        else if (SQLite3Type == SQLITE3_TEXT)
        {
            size_t StringSize = sqlite3_column_bytes(StatementToInterpret, ColumnIndex);
            const char* StringData = (char*)sqlite3_column_text(StatementToInterpret, ColumnIndex);
            ReturnValue = std::string(StringData, StringSize);
        }
        else if (SQLite3Type == SQLITE_NULL)
        {

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
                NewRow.m_Data.push_back(CopySQLite3Data(StatementToInterpret, i, SQLite3_ColumnType));
            }
            else
            {
                NewRow.m_Data.emplace_back();
            }
            auto Name = sqlite3_column_name(StatementToInterpret,i);
            if(Name == nullptr)
            {
                throw std::runtime_error("Error creatign  SQL statement: out of memory");
            }
            NewRow.m_ColumnNames[std::string(Name)] = i;
        }
        
        return(NewRow);
    }
    MBDB_ColumnValueTypes MBDB_RowData::GetColumnValueType(int ColumnIndex) const
    {
        return(ColumnValueTypes[ColumnIndex]);
    }
    bool MBDB_RowData::ColumnValueIsNull(int ColumnIndex) const
    {
        if(ColumnIndex < 0 || ColumnIndex > m_Data.size())
        {
            throw std::runtime_error("Invalid column index for ColumnValueIsNull");
        }
        return(std::holds_alternative<std::monostate>(m_Data[ColumnIndex]));
    }
    std::string MBDB_RowData::JSONEncodeValue(size_t ColumnIndex) const
    {
        std::string ReturnValue;
        if(IsType<std::string>(ColumnIndex))
        {
            ReturnValue = ::ToJSON(std::get<std::string>(m_Data[ColumnIndex]));
        }
        else if(IsType<IntType>(ColumnIndex))
        {
            ReturnValue = std::to_string(std::get<IntType>(m_Data[ColumnIndex]));   
        }
        else if(IsType<FloatType>(ColumnIndex))
        {
            ReturnValue = std::to_string(std::get<FloatType>(m_Data[ColumnIndex]));   
        }
        else
        {
            //mono state, is not assigned       
            ReturnValue = "null";
        }
        return(ReturnValue);
    }
    std::string MBDB_RowData::ColumnToString(size_t ColumnIndex) const
    {
        std::string ReturnValue = "";
        MBDB_ColumnValueTypes DataType = GetColumnValueType(ColumnIndex);
        if(IsType<std::string>(ColumnIndex))
        {
            ReturnValue += std::get<std::string>(m_Data[ColumnIndex]);
        }
        else if(IsType<IntType>(ColumnIndex))
        {
            ReturnValue += std::to_string(std::get<IntType>(m_Data[ColumnIndex]));
        }
        else if(IsType<FloatType>(ColumnIndex))
        {
            ReturnValue += std::to_string(std::get<FloatType>(m_Data[ColumnIndex]));
        }
        else
        {
            ReturnValue += "null";
        }
        return(ReturnValue);
    }
    std::string MBDB_RowData::ToJSON() const
    {
        size_t NumberOfColumns = GetNumberOfColumns();
        std::string ReturnValue = "[";
        for (size_t i = 0; i < NumberOfColumns; i++)
        {
            ReturnValue += JSONEncodeValue(i);
            if (i+1 < NumberOfColumns)
            {
                ReturnValue += ",";
            }
        }
        ReturnValue += "]";
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
    MBError SQLStatement::BindInt(IntType IntToBind, int ParameterIndex)
    {
        MBError ReturnValue = MBError(true);
        int Error = sqlite3_bind_int64(UnderlyingStatement, ParameterIndex, IntToBind);
        if (Error != SQLITE_OK)
        {
            ReturnValue = false;
            ReturnValue.ErrorMessage = sqlite3_errstr(Error);
        }
        return(ReturnValue);
    }
    void  SQLStatement::BindBlob(std::string const& Value,int Index)
    {
        MBError ReturnValue = MBError(true);
        int Error = sqlite3_bind_blob64(UnderlyingStatement,Index,Value.data(),Value.size(),SQLITE_TRANSIENT);
        if (Error != SQLITE_OK)
        {
            std::string Message = sqlite3_errstr(Error);
            throw std::runtime_error("Error binding value to statement: " + Message);
        }
    }
    void  SQLStatement::BindBlob(std::vector<uint8_t> const& Value,int Index)
    {
        MBError ReturnValue = MBError(true);
        int Error = sqlite3_bind_blob64(UnderlyingStatement,Index,Value.data(),Value.size(),SQLITE_TRANSIENT);
        if (Error != SQLITE_OK)
        {
            std::string Message = sqlite3_errstr(Error);
            throw std::runtime_error("Error binding value to statement: " + Message);
        }
    }
    SQLStatement::~SQLStatement()
    {
        sqlite3_finalize(UnderlyingStatement);
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
            //    ReturnValue = BindNull(i + 1 + Offset);
            //    continue;
            //}
            if (ValueTypes[i] == ColumnSQLType::Int)
            {
                MBDB::IntType NewInt = StringToInt(ValuesToBind[i], &ReturnValue);
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
    void SQLStatement::BindBlob(std::string const& ParameterName,std::string const& Value)
    {
        auto Index = sqlite3_bind_parameter_index(UnderlyingStatement,ParameterName.data());
        if(Index == 0)
        {
            throw std::runtime_error("Error binding value in sql statement, invalid parameter name: "+ParameterName);
        }
        BindBlob(Value,Index);
    }
    void SQLStatement::BindBlob(std::string const& ParameterName,std::vector<uint8_t> const& Value)
    {
        auto Index = sqlite3_bind_parameter_index(UnderlyingStatement,ParameterName.data());
        if(Index == 0)
        {
            throw std::runtime_error("Error binding value in sql statement, invalid parameter name: "+ParameterName);
        }
        BindBlob(Value,Index);
    }
    void SQLStatement::BindString(std::string const& ParameterName,std::string const& Value)
    {
        auto Index = sqlite3_bind_parameter_index(UnderlyingStatement,ParameterName.data());
        if(Index == 0)
        {
            throw std::runtime_error("Error binding value in sql statement, invalid parameter name: "+ParameterName);
        }
        auto Result = BindString(Value,Index);
        if(!Result)
        {
            throw std::runtime_error("Error binding parameter: "+Result.ErrorMessage);
        }
    }
    void SQLStatement::BindInt(std::string const& ParameterName,int64_t Value)
    {
        auto Index = sqlite3_bind_parameter_index(UnderlyingStatement,ParameterName.data());
        if(Index == 0)
        {
            throw std::runtime_error("Error binding value in sql statement, invalid parameter name: "+ParameterName);
        }
        auto Result = BindInt(Value,Index);
        if(!Result)
        {
            throw std::runtime_error("Error binding parameter: "+Result.ErrorMessage);
        }
    }
    void SQLStatement::BindNull(std::string const& ParameterName)
    {
        auto Index = sqlite3_bind_parameter_index(UnderlyingStatement,ParameterName.data());
        if(Index == 0)
        {
            throw std::runtime_error("Error binding value in sql statement, invalid parameter name: "+ParameterName);
        }
        auto Result = BindNull(Index);
        if(!Result)
        {
            throw std::runtime_error("Error binding parameter: "+Result.ErrorMessage);
        }
    }
    void SQLStatement::Reset()
    {
        if(UnderlyingStatement != nullptr)
        {
            sqlite3_reset(UnderlyingStatement);
        }
    }
    std::vector<MBDB_RowData> SQLStatement::GetAllRows(sqlite3* DBConnection, MBError* ErrorToReturn = nullptr)
    {
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
                    throw std::runtime_error("Internal SQLITE_ERROR: Misuse");
                }
                else if (StatementValue == SQLITE_BUSY)
                {
                    throw std::runtime_error("Internal SQLITE_ERROR: Busy");
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
            throw std::runtime_error("Error preparing SQL statment");
        }
        else
        {
            int ColumnCount = sqlite3_column_count(UnderlyingStatement);
            for(int i = 0; i < ColumnCount;i++)
            {
                auto  Name = sqlite3_column_name(UnderlyingStatement,i);
                if(Name == nullptr)
                {
                    throw std::runtime_error("Error preparing SQL statement: unable to access name of column");   
                }
                m_ColumnNames[std::string(Name)]= i;
            }
        }
    }
    //MrBoboDatabase
    SQLStatement MrBoboDatabase::GetSQLStatement(std::string const& SQLCode)
    {
        return(SQLStatement(SQLCode,UnderlyingConnection));
    }
    MrBoboDatabase::~MrBoboDatabase()
    {
        sqlite3_close(UnderlyingConnection);
    }
    std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(SQLStatement& StatementToEvaluate, MBError* ErrorToReturn)
    {
        return(StatementToEvaluate.GetAllRows(UnderlyingConnection, ErrorToReturn));
    }
    std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(SQLStatement& Stmt)
    {
        MBError Result = true;
        auto ReturnValue = GetAllRows(Stmt,&Result);
        if (!Result)
        {
            throw std::runtime_error("Error evaluating statement: "+Result.ErrorMessage);
        }
        return ReturnValue;
    }
    MrBoboDatabase::MrBoboDatabase(std::string const& FilePath,DBOpenOptions Options)
    {
        uint64_t DatabaseOptions = SQLITE_OPEN_FULLMUTEX;
        if (uint64_t(Options) & uint64_t(DBOpenOptions::ReadOnly))
        {
            DatabaseOptions |= SQLITE_OPEN_READONLY;
        }
        else
        {
            DatabaseOptions |= SQLITE_OPEN_READWRITE;
            DatabaseOptions |= SQLITE_OPEN_CREATE;
        }
        int ErrorCode = sqlite3_open_v2(FilePath.c_str(), &UnderlyingConnection, DatabaseOptions, nullptr);
        if (ErrorCode != SQLITE_OK)
        {
            std::cout << sqlite3_errmsg(UnderlyingConnection) << std::endl;
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
        //just nu �r det en SQLITE3 databas s� vi anv�nder dess
        std::vector<ColumnInfo> ReturnValue = {};
        SQLStatement Statement = GetSQLStatement("SELECT * FROM pragma_table_info(?);");
        Statement.BindString(TableName,1);
        std::vector<MBDB_RowData> TableInfo =  GetAllRows(Statement);
        size_t TableInfoSize = TableInfo.size();
        for (size_t i = 0; i < TableInfoSize; i++)
        {
            ColumnInfo NewColumnInfo;
            NewColumnInfo.ColumnName = TableInfo[i].GetColumnData<std::string>(1);
            NewColumnInfo.ColumnType = ColumnTypeFromString(TableInfo[i].GetColumnData<std::string>(2));
            NewColumnInfo.Nullable = (TableInfo[i].GetColumnData<IntType>(3) == 0);
            NewColumnInfo.PrimaryKeyIndex = TableInfo[i].GetColumnData<IntType>(5);
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
            //statementen �r fungerande
            ReturnValue = NewStatement.GetAllRows(UnderlyingConnection, OutError);
        }
        return(ReturnValue);
    }
    std::vector<MBDB_RowData> MrBoboDatabase::GetAllRows(std::string const& SQLQuerry)
    {
        MBError Result = true;
        auto ReturnValue = GetAllRows(SQLQuerry,&Result);
        if(!Result)
        {
            throw std::runtime_error("Error executing sql statement: "+Result.ErrorMessage);   
        }
        return ReturnValue;
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
        throw std::runtime_error("Invalid ColumnSQLType");
    }
}
std::string ToJSON(bool ValueTojason)
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
std::string ToJSON(MBDB::ColumnSQLType ValueToJSON)
{
    return("\"" + ColumnSQLTypeToString(ValueToJSON) + "\"");
}
std::string ToJSON(std::string const& ValueToJSON)
{
    std::string EscapedJasonString = "";
    for (size_t i = 0; i < ValueToJSON.size(); i++)
    {
        if (ValueToJSON[i] == '"')
        {
            EscapedJasonString += "\\\"";
        }
        else if (ValueToJSON[i] == '\\')
        {
            EscapedJasonString += "\\\\";
        }
        else if (ValueToJSON[i] == '\b')
        {
            EscapedJasonString += "\\b";
        }
        else if (ValueToJSON[i] == '\f')
        {
            EscapedJasonString += "\\f";
        }
        else if (ValueToJSON[i] == '\n')
        {
            EscapedJasonString += "\\n";
        }
        else if (ValueToJSON[i] == '\r')
        {
            EscapedJasonString += "\\r";
        }
        else if (ValueToJSON[i] == '\t')
        {
            EscapedJasonString += "\\t";
        }
        else
        {
            EscapedJasonString += ValueToJSON[i];
        }
    }
    return("\"" + EscapedJasonString + "\"");
}
std::string ToJSON(long long ValueToJSON)
{
    return(std::to_string(ValueToJSON));
}
std::string ToJSON(MBDB::ColumnInfo const& ValueToJSON)
{
    std::string ReturnValue = "{";
    ReturnValue += "\"ColumnName\":" + ToJSON(ValueToJSON.ColumnName) + ",";
    ReturnValue += "\"ColumnType\":" + ToJSON(ValueToJSON.ColumnType) + ",";
    ReturnValue += "\"IsNullable\":" + ToJSON(ValueToJSON.Nullable) + ",";
    ReturnValue += "\"PrimaryKeyIndex\":" + ToJSON((long long)ValueToJSON.PrimaryKeyIndex) + "}";
    return(ReturnValue);
}
std::string ToJSON(MBDB::MBDB_RowData const& ValueToJSON)
{
    return(ValueToJSON.ToJSON());
}
std::string CombineJSONObjects(std::vector<std::string> const& ObjectNames, std::vector<std::string> const& ObjectsData)
{
    std::string ReturnValue = "{";
    for (size_t i = 0; i < ObjectNames.size(); i++)
    {
        ReturnValue += ToJSON(ObjectNames[i])+":";
        ReturnValue += ObjectsData[i];
        if (i < ObjectNames.size())
        {
            ReturnValue += ",";
        }
    }
    ReturnValue += "}";
    return(ReturnValue);
}
