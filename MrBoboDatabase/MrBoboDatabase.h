#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <assert.h>
#include <MBUtility/MBErrorHandling.h>
#include <stdexcept>
#include <variant>

#include <unordered_map>
class sqlite3;
struct sqlite3_stmt;
namespace MBDB
{
    typedef long long IntType;
    typedef double FloatType;
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
        typedef std::monostate Null;
        typedef std::variant<std::monostate,FloatType,IntType,std::string> VariantType;
        std::vector<MBDB_ColumnValueTypes> ColumnValueTypes = {};
        std::vector<VariantType> m_Data;
        std::unordered_map<std::string,int> m_ColumnNames;
        template<typename T> bool IsCorrectType(int Index) const
        {
            if(Index < 0 || Index >= m_Data.size())
            {
                throw std::runtime_error("Invalid index for MBDB_RowData");
            }
            return(std::holds_alternative<T>(m_Data[Index]));
        }


        template<typename T>
        std::tuple<T> CreateTuple(int* CurrentIndex) {
            T NewValue = std::get<T>(m_Data[*CurrentIndex]);
            *CurrentIndex += 1;
            return std::make_tuple(NewValue);
        }

        template<typename T, typename Z,typename... Args>
        std::tuple<T, Z,Args...> CreateTuple(int* CurrentIndex)
        {
            T NewValue = std::get<T>(m_Data[*CurrentIndex]);
            *CurrentIndex += 1;
            return std::tuple_cat(std::make_tuple(NewValue), CreateTuple<Z,Args...>(CurrentIndex));
        }
    public:
        //constructors
        friend MBDB_RowData CreateRowFromSQLiteStatement(sqlite3_stmt* StatementToInterpret);
        std::string ToJSON() const;
        std::string JSONEncodeValue(size_t ColumnIndex) const;
        std::string ColumnToString(size_t ColumnIndex) const;

        template<typename... Args>
        std::tuple<Args...> GetTuple()
        {
            int CurrentIndex = 0;
            return(CreateTuple<Args...>(&CurrentIndex));
        }
        template<typename T> T& GetColumnData(int ColumnIndex)
        {
            if(!IsCorrectType<T>(ColumnIndex))
            {
                throw std::runtime_error("Invalid type for column");
            }
            return(std::get<T>(m_Data[ColumnIndex]));
        }
        template<typename T> T const& GetColumnData(int ColumnIndex) const
        {
            if(!IsCorrectType<T>(ColumnIndex))
            {
                throw std::runtime_error("Invalid type for column");
            }
            return(std::get<T>(m_Data[ColumnIndex]));

        }
        template<typename T>
        bool IsType(int ColumnIndex) const
        {
            return(IsCorrectType<T>(ColumnIndex));   
        }

        VariantType& operator[](std::string const& View)
        {
            auto It = m_ColumnNames.find(View);
            if(It == m_ColumnNames.end())
            {
                throw std::runtime_error("Invalid column name '"+View+" when accessing SQL result row");
            }
            return m_Data[It->second];
        }
        VariantType const& operator[](std::string const& View) const
        {
            auto It = m_ColumnNames.find(View);
            if(It == m_ColumnNames.end())
            {
                throw std::runtime_error("Invalid column name '"+View+" when accessing SQL result row");
            }
            return m_Data.at(It->second);
        }

        MBDB_ColumnValueTypes GetColumnValueType(int ColumnIndex) const;
        bool ColumnValueIsNull(int ColumnIndex) const;
        size_t GetNumberOfColumns() const{ return(m_Data.size()); }
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
        SQLStatement(std::string const& SQLCode,sqlite3* DBConnection);

        std::unordered_map<std::string,int> m_ColumnNames;
        std::vector<MBDB_RowData> GetAllRows(sqlite3* DBConnection,MBError* OutError);
    public:
        SQLStatement() { }
        SQLStatement(SQLStatement const&) = delete;
        SQLStatement& operator=(SQLStatement&& Other)
        {
            std::swap(UnderlyingStatement,Other.UnderlyingStatement);
            return(*this);
        }
        SQLStatement& operator=(SQLStatement const&) = delete;
        SQLStatement(SQLStatement&&) = default;
        bool IsValid() { return(UnderlyingStatement != nullptr); };
        MBError BindString(std::string const& ParameterData, int ParameterIndex);
        MBError BindInt(IntType, int ParameterIndex);
        MBError BindNull(int ParameterIndex);
        MBError BindValues(std::vector<std::string> const& ValuesToBind, std::vector<ColumnSQLType> const& ValueTypes, int Offset);

        void BindBlob(std::string const& Value,int Index);
        void BindBlob(std::vector<uint8_t> const& Value,int Index);

        void BindString(std::string const& ParameterName,std::string const& Value);
        void BindBlob(std::string const& ParameterName,std::string const& Value);
        void BindBlob(std::string const& ParameterName,std::vector<uint8_t> const& Value);
        void BindInt(std::string const& ParameterName,int64_t Value);
        void BindNull(std::string const& ParameterName);


        template<typename T>
        void BindValue(std::string const& ParameterName,T Value)
        {
            if constexpr(std::is_same_v<T,std::string>)
            {
                BindString(ParameterName,std::move(Value));
            }
            else if constexpr(std::is_integral_v<T> || std::is_enum_v<T>)
            {
                BindInt(ParameterName,int64_t(Value));
            }
            else
            {
                static_assert(std::is_same_v<T,T>,"Error binding value: type not supported");   
            }
        }
        void Reset();
        ~SQLStatement();
    };
    enum class DBOpenOptions : uint64_t
    {
        ReadOnly = 1 << 1,
        ReadWrite = 1 << 2,
    };
    class MrBoboDatabase
    {
    private:
        sqlite3* UnderlyingConnection = nullptr;
    public:
        MrBoboDatabase(std::string const& DatabaseFile,DBOpenOptions Options);
        std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry,MBError* ErrorToReturn);
        std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry);
        //std::vector<MBDB_RowData> GetAllRows(std::string const& SQLQuerry,MrBoboDatabase* StructPointer,MBError* ErrorToReturn = nullptr);
        std::vector<MBDB_RowData> GetAllRows(SQLStatement& ,MBError* ErrorToReturn);
        std::vector<MBDB_RowData> GetAllRows(SQLStatement& Stmt);
        std::vector<std::string> GetAllTableNames();
        std::vector<ColumnInfo> GetColumnInfo(std::string const& TableName);
        SQLStatement GetSQLStatement(std::string const& SQLCode);
        //MBError FreeSQLStatement(SQLStatement* StatementToFree);

        MBDB_ResultIterator GetResultIterator(std::string const& SQLQuerry, MBError* ErrorToReturn = nullptr);
        ~MrBoboDatabase();
    };
}

std::string ToJSON(bool ValueTojason);
std::string ToJSON(MBDB::ColumnSQLType ValueToJSON);
std::string ToJSON(MBDB::MBDB_RowData const& RowDataToEncode);
std::string ToJSON(std::string const& ValueToJSON);
std::string ToJSON(long long ValueToJSON);
std::string ToJSON(MBDB::ColumnInfo const& ValueToJSON);
std::string CombineJSONObjects(std::vector<std::string> const& ObjectNames, std::vector<std::string> const& ObjectsData);

template<typename T>
std::string ToJSONArray(std::vector<T> const& ValuesToConvert, std::string ArrayName)
{
    std::string JsonRespone = "{\"" + ArrayName + "\":[";
    size_t TableNamesSize = ValuesToConvert.size();
    for (size_t i = 0; i < TableNamesSize; i++)
    {
        JsonRespone += ToJSON(ValuesToConvert[i]);
        if (i + 1 < TableNamesSize)
        {
            JsonRespone += ",";
        }
    }
    JsonRespone += "]}";
    return(JsonRespone);
}
template<typename T>
std::string ToJSONArray(std::vector<T> const& ValuesToConvert)
{
    std::string JsonRespone = "[";
    size_t TableNamesSize = ValuesToConvert.size();
    for (size_t i = 0; i < TableNamesSize; i++)
    {
        JsonRespone += ToJSON(ValuesToConvert[i]);
        if (i + 1 < TableNamesSize)
        {
            JsonRespone += ",";
        }
    }
    JsonRespone += "]";
    return(JsonRespone);
}
