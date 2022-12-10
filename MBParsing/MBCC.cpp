#include "MBCC.h"
#include "MBParsing.h"
namespace MBParsing
{
    class MBCCParseError : std::exception
    {
    public:
        std::string ErrorMessage;
        size_t ParseOffset = 0;
        MBCCParseError(std::string Error,size_t NewParseOffset)
        {
            ErrorMessage = Error;
            ParseOffset = NewParseOffset;
        }
    };
    std::string MBCCDefinitions::p_ParseIdentifier(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::string ReturnValue;
        size_t ParseOffset = InParseOffset;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        size_t StringBegin = ParseOffset;
        while(ParseOffset < DataSize)
        {
            if(Data[ParseOffset] == ' ' || Data[ParseOffset] == '\t' || Data[ParseOffset] == '\n')
            {
                ParseOffset++;
                break; 
            }   
            ParseOffset++;
        }
        ReturnValue = std::string(Data+StringBegin,Data+ParseOffset);
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    Terminal MBCCDefinitions::p_ParseTerminal(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        Terminal ReturnValue;  
        size_t ParseOffset = InParseOffset;
        std::string TerminalName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        if(TerminalName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs non empty terminal name ",ParseOffset);
        } 
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs delimiting =",ParseOffset);
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        MBError ParseStringResult = true;
        std::string RegexString = ParseQuotedString(Data,DataSize,ParseOffset,&ParseOffset,&ParseStringResult);
        if(!ParseStringResult)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: error parsing quoted string for terminal definition: "+
                    ParseStringResult.ErrorMessage,ParseOffset);
        }
        if(RegexString == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: term needs non empty regex definition",ParseOffset);
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset == DataSize || Data[ParseOffset] != ';')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delmiting ; for end of def",ParseOffset);
        }
        *OutParseOffset = ParseOffset;
        ReturnValue.Name = TerminalName;
        ReturnValue.RegexDefinition = RegexString;
        return(ReturnValue);
    }
    StructDefinition MBCCDefinitions::p_ParseStruct(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset)
    {
        StructDefinition ReturnValue;
        


        return(ReturnValue); 
    }
    std::pair<std::string,std::string> MBCCDefinitions::p_ParseDef(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::pair<std::string,std::string> ReturnValue; 
        size_t ParseOffset = InParseOffset;
        std::string RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        if(RuleName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty rule name \"",ParseOffset);
        } 
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs delimiting =\"",ParseOffset);
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::string StructName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        if(StructName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty struct name \"",ParseOffset);
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset == DataSize || Data[ParseOffset] != ';')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delmiting ; for end of def",ParseOffset);
        }
        *OutParseOffset = ParseOffset;
        ReturnValue.first = RuleName;
        ReturnValue.second = StructName;
        return(ReturnValue);
    }
    std::vector<ParseRule> MBCCDefinitions::p_ParseParseRules(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset)
    {
        std::vector<ParseRule> ReturnValue;
        
        return(ReturnValue);
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t InOffset)
    {
        MBCCDefinitions ReturnValue; 
        size_t ParseOffset = InOffset; 
        MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::vector<std::pair<std::string,std::string>> UnresolvedDefs;
        while(ParseOffset < DataSize)
        {
            std::string CurrentIdentifier = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            if(CurrentIdentifier == "def")
            {
                auto NewDef = p_ParseDef(Data,DataSize,ParseOffset,&ParseOffset); 
                UnresolvedDefs.push_back(NewDef);
            }
            else if(CurrentIdentifier == "term")
            {
                Terminal NewTerminal = p_ParseTerminal(Data,DataSize,ParseOffset,&ParseOffset);
                if(ReturnValue.NameToStruct.find(NewTerminal.Name) != ReturnValue.NameToStruct.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: duplicate definition for terminal \""+NewTerminal.Name+"\"");
                }
                size_t TerminalIndex = ReturnValue.Terminals.size();
                ReturnValue.NameToTerminal[NewTerminal.Name] = TerminalIndex;
                ReturnValue.Terminals.push_back(std::move(NewTerminal));

            }
            else if(CurrentIdentifier == "struct")
            {
                StructDefinition NewStruct = p_ParseStruct(Data,DataSize,ParseOffset,&ParseOffset);
                if(ReturnValue.NameToStruct.find(NewStruct.Name) != ReturnValue.NameToStruct.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: duplicate definition for struct \""+NewStruct.Name+"\"");
                }
                size_t StructIndex = ReturnValue.Structs.size();
                ReturnValue.NameToStruct[NewStruct.Name] = StructIndex;
                ReturnValue.Structs.push_back(std::move(NewStruct));
            }
            else
            {
                std::vector<ParseRule> NewRules = p_ParseParseRules(Data,DataSize,ParseOffset,&ParseOffset);
                size_t CurrentIndex = ReturnValue.ParseRules.size();
                auto& RuleVector = ReturnValue.ParseRules;
                RuleVector.insert(RuleVector.end(),std::make_move_iterator(NewRules.begin()),std::make_move_iterator(NewRules.end()));
                std::vector<RuleIndex>& RuleNameVector = ReturnValue.NonTerminals[CurrentIdentifier];
                for(int i = 0; i < NewRules.size();i++)
                {
                    RuleNameVector.push_back(CurrentIndex+i);    
                }
            }
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        for(auto const& Def : UnresolvedDefs)
        {
            //if(ReturnValue.NameToTerminal.find(Def.first) == ReturnValue.NameToTerminal.end())
            //{
            //    throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined rule \""+Def.first+"\"");    
            //}
            //if(ReturnValue.NameToStruct.find(Def.second) == ReturnValue.NameToStruct.end())
            //{
            //    throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined struct \""+Def.second+"\"");    
            //}
            ReturnValue.NonTerminalToStruct[Def.first] = ReturnValue.NameToStruct[Def.second];
        }
        return(ReturnValue);
    }
    MBCCDefinitions MBCCDefinitions::ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,std::string& OutError)
    {
        MBCCDefinitions ReturnValue; 
        OutError = "";
        try
        {
            ReturnValue = ParseDefinitions(Data,DataSize,ParseOffset); 
        }
        catch(std::exception const& e)
        {
            OutError = e.what(); 
        }
        return(ReturnValue);
    }
}
