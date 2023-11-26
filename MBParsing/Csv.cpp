#include "Csv.h"
#include <array>

namespace MBParsing
{
       
    Column& CSVFile::operator[](size_t Index)
    {
        if(Index >= m_Columns.size())
        {
            throw std::runtime_error("Index out of range when accessing CSV column");   
        }
        return m_Columns[Index];
    }
    Column const& CSVFile::operator[](size_t Index) const
    {
        if(Index >= m_Columns.size())
        {
            throw std::runtime_error("Index out of range when accessing CSV column");   
        }
        return m_Columns[Index];
    }
    Column& CSVFile::operator[](std::string const&  ColumnName)
    {
        auto It = m_ColumnNames.find(ColumnName);
        if(It == m_ColumnNames.end())
        {
            throw std::runtime_error("Invalid column name when indexing CSV column");   
        }
        return m_Columns[It->second];
    }
    Column const& CSVFile::operator[](std::string const& ColumnName) const
    {
        auto It = m_ColumnNames.find(ColumnName);
        if(It == m_ColumnNames.end())
        {
            throw std::runtime_error("Invalid column name when indexing CSV column");   
        }
        return m_Columns[It->second];
    }
    std::vector<std::string> CSVFile::p_SplitString(std::string_view const& StringToSplit)
    {
        std::vector<std::string> ReturnValue;
        if(StringToSplit.size() == 0)
        {
            return ReturnValue;   
        }
        if(StringToSplit.back() == ',')
        {
            throw std::runtime_error("Comma may not appear at the end of a row");   
        }

        size_t ParseOffset = 0;
        while(ParseOffset < StringToSplit.size())
        {
            size_t NextDelimiter = StringToSplit.find_first_of(",\"",ParseOffset);
            if(NextDelimiter == StringToSplit.npos)
            {
                ReturnValue.push_back(std::string(StringToSplit.data()+ ParseOffset,StringToSplit.data()+StringToSplit.size()));
                break;
            }
            if(StringToSplit[NextDelimiter] == ',')
            {
                ReturnValue.push_back(std::string(StringToSplit.data()+ ParseOffset,StringToSplit.data()+NextDelimiter));
                ParseOffset = NextDelimiter+1;
            }
            else if(StringToSplit[NextDelimiter] == '"')
            {
                if(NextDelimiter >= 1)
                {
                    if(StringToSplit[NextDelimiter-1] != ',')
                    {
                        throw std::runtime_error("String delimited field cannot have whitespace before \"");
                    }   
                }
                std::string NewString;
                ParseOffset += 1;
                while(ParseOffset < StringToSplit.size())
                {
                    size_t NextQuotePosition = StringToSplit.find('"',ParseOffset);
                    if(NextQuotePosition == StringToSplit.size()-1)
                    {
                        NewString.append(StringToSplit.data()+ParseOffset,StringToSplit.data()+StringToSplit.size());
                        break;
                    }
                    if(StringToSplit[NextQuotePosition+1] == '"')
                    {
                        NewString.append(StringToSplit.data()+ParseOffset,StringToSplit.data()+NextQuotePosition);
                        ParseOffset = NextQuotePosition + 2;
                    }
                    else if(StringToSplit[NextQuotePosition+1] == ',')
                    {
                        NewString.append(StringToSplit.data()+ParseOffset,StringToSplit.data()+NextQuotePosition);
                        ParseOffset = NextQuotePosition + 2;   
                        break;
                    }
                    else
                    {
                        throw std::runtime_error("Quote delimited field cannot have trailing characters");   
                    }
                }
                ReturnValue.push_back(std::move(NewString));
            }
        }
        return ReturnValue;
    }
    CSVFile::Type CSVFile::p_InferType(std::string const& ColumnToInspect)
    {
        Type ReturnValue = Type::String;
        if(ColumnToInspect.size() == 0)
        {
            return ReturnValue;   
        }
        bool ContainsDot = false;
        bool IsNumber = true;
        for(auto Char : ColumnToInspect)
        {
            if(Char == '.')
            {
                ContainsDot = true;
            }   
            else 
            {
                if(!(Char >= '0' && Char <= '9'))
                {
                    IsNumber = false;
                    break;
                }   
            }
        }
        if(IsNumber)
        {
            if(ContainsDot)
            {
                ReturnValue = Type::Float;
            }
            else
            {
                ReturnValue = Type::Integer;   
            }
        }
        return ReturnValue;
    }
    std::vector<CSVFile::Type> CSVFile::p_InferTypes(std::vector<std::vector<std::string>> const& TotalData)
    {
        std::vector<Type> ReturnValue;
        std::vector<std::array<int,3>> InferedTypes = std::vector<std::array<int,3>>(TotalData[0].size(),std::array<int,3>());
        ReturnValue.reserve(InferedTypes.size());
        size_t RowCount = 0;
        for(auto const& Row : TotalData)
        {
            size_t ColumnCount = 0;
            for(auto const& Column : Row)
            {
                InferedTypes[ColumnCount][size_t(p_InferType(Column))] += 1;
                ColumnCount += 1;
            }
            RowCount += 1;
            if(RowCount >= SniffRowCount)
            {
                break;   
            }
        }
        for(auto const& ColumnType : InferedTypes)
        {
            auto MaxElemIt = std::max_element(ColumnType.begin(),ColumnType.end());
            Type NewType = Type(MaxElemIt-ColumnType.begin());
            ReturnValue.push_back(NewType);
        }
        return ReturnValue;
    }
    void CSVFile::p_AddRows(std::vector<Column>& Columns, std::vector<Type> const& Types,std::vector<std::vector<std::string>> const& TotalData)
    {
        size_t ColumnIndex = 0;
        for(auto const& ColType : Types)
        {
            if(ColType == Type::Integer)
            {
                Columns[ColumnIndex] = Column::VecType<CsvIntType>();
            }
            else if(ColType == Type::Float)
            {
                Columns[ColumnIndex] = Column::VecType<CsvFloatType>();
            }
            else if(ColType == Type::String)
            {
                Columns[ColumnIndex] = Column::VecType<CsvStringType>();
            }
            ColumnIndex += 1;
        }
        for(auto const& Row : TotalData)
        {
            ColumnIndex = 0;
            for(auto const& Column : Row)
            {
                Type ColType = Types[ColumnIndex];
                if(ColType == Type::Integer)
                {
                    Columns[ColumnIndex].GetType<CsvIntType>().push_back(std::stoll(Column));
                }
                else if(ColType == Type::Float)
                {
                    Columns[ColumnIndex].GetType<CsvFloatType>().push_back(std::stof(Column));
                }
                else if(ColType == Type::String)
                {
                    Columns[ColumnIndex].GetType<CsvStringType>().push_back(Column);
                }
                ColumnIndex++;   
            }
        }
    }
    CSVFile CSVFile::ParseCSVFile(MBUtility::StreamReader& Reader)
    {
        //read name row
        CSVFile ReturnValue;
        std::string NameRow;
        while(NameRow == "")
        {
            if(Reader.EOFReached())
            {
                throw std::runtime_error("Empty csv file, no non empty line");   
            }
            NameRow = MBUtility::ReadLine(Reader);
        }
        auto Names = p_SplitString(NameRow);
        size_t Index = 0;
        for(auto const& Name : Names)
        {
            ReturnValue.m_ColumnNames[Name] = Index;
            ReturnValue.m_Columns.push_back(Column());
            Index++;
        }
        //first data row, determines types
        std::vector<std::vector<std::string>> TotalData;
        while(!Reader.EOFReached())
        {
            std::string NewRowText = MBUtility::ReadLine(Reader);
            if(NewRowText == "" && Reader.EOFReached())
            {
                break;   
            }
            auto RowIntervalls = p_SplitString(NewRowText);
            if(RowIntervalls.size() != ReturnValue.m_Columns.size())
            {
                throw std::runtime_error("Inconsistent number of columns in rows");   
            }
            std::vector<std::string> NewRow;
            for(auto const& Column : RowIntervalls)
            {
                NewRow.push_back(Column);
            }
            TotalData.push_back(std::move(NewRow));
        }
        auto Types = p_InferTypes(TotalData);
        p_AddRows(ReturnValue.m_Columns,Types,TotalData);
        return ReturnValue;
    }
}
