#pragma once
#include <MBUtility/StreamReader.h>
#include <unordered_map>
#include <cstdint>
#include <variant>
#include <typeindex>
#include <vector>

namespace MBParsing
{
    typedef int_least64_t CsvIntType;
    typedef std::string CsvStringType;
    typedef float CsvFloatType;
    class Column
    {
        std::variant<std::monostate,std::vector<int_least64_t>,std::vector<std::string>,std::vector<float>> m_Content;
    public:

        template<typename T>
        using VecType = std::vector<T>;

        template<typename T>
        Column& operator=(VecType<T> Data)
        {
            m_Content = std::move(Data);   
            return *this;
        }
        template<typename T>
        bool IsType() const
        {
            return std::holds_alternative<VecType<T>>(m_Content);
        }
        template<typename T>
        VecType<T>& GetType()
        {
            return std::get<VecType<T>>(m_Content);
        }
        template<typename T>
        VecType<T> const& GetType() const
        {
            return std::get<VecType<T>>(m_Content);
        }
    };
    class CSVFile
    {
    private:
        enum class Type
        {
            Float = 0,
            Integer,
            String,
            Null
        };
        
        std::vector<Column> m_Columns;
        std::unordered_map<std::string,size_t> m_ColumnNames;
        static size_t constexpr SniffRowCount = 30;
        size_t m_RowCount = 0;
        
        struct StringIntervall
        {
            size_t Begin = 0;
            size_t End = 0;
        };
        static std::vector<std::string> p_SplitString(std::string_view const& StringToSplit);
        static Type p_InferType(std::string const& ColumnToInspect);
        static std::vector<Type> p_InferTypes(std::vector<std::vector<std::string>> const& TotalData);
        static void p_AddRows(std::vector<Column>& Columns, std::vector<Type> const& Types,std::vector<std::vector<std::string>> const& ColumnValues);
    public:
        Column& operator[](size_t Index);
        Column const& operator[](size_t Index) const;
        Column& operator[](std::string const& ColumnName);
        Column const& operator[](std::string const& ColumnName) const;

        bool HasColName(std::string const& ColName) const
        {
            return m_ColumnNames.find(ColName) != m_ColumnNames.end();   
        }
        
        static CSVFile ParseCSVFile(MBUtility::StreamReader& Reader);
        size_t RowCount() const
        {
            return m_RowCount;
        }
        size_t ColumnCount() const
        {
            return m_Columns.size();
        }
        void Write(MBUtility::MBOctetOutputStream& OutStream);
    };
}
