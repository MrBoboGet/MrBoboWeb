#include "MBCC.h"
#include "MBParsing.h"
#include <assert.h>
#include <iostream>
namespace MBParsing
{
    //BEGIN StructMemberVariable
    
    StructMemberVariable::StructMemberVariable(StructMemberVariable_List ListMemberVariable)
        : m_Content(std::move(ListMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Raw RawMemberVariable)
        : m_Content(std::move(RawMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Struct StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_Int RawMemberVariable)
        : m_Content(std::move(RawMemberVariable))
    {
           
    }
    StructMemberVariable::StructMemberVariable(StructMemberVariable_String StructMemberVariable)
        : m_Content(std::move(StructMemberVariable))
    {
           
    }
    std::string& StructMemberVariable::GetName()
    {
        return(std::visit([&](MemberVariable& x) -> std::string& 
            {
                return(x.Name);      
            }, m_Content));
    }
    std::string& StructMemberVariable::GetDefaultValue()
    {
        return(std::visit([&](MemberVariable& x) -> std::string& 
            {
                return(x.DefaultValue);      
            }, m_Content));
    }
    std::string const& StructMemberVariable::GetName() const
    {
        return(std::visit([&](MemberVariable const& x) -> std::string const& 
            {
                return(x.Name);      
            }, m_Content));
    }
    std::string const& StructMemberVariable::GetDefaultValue() const
    {
        return(std::visit([&](MemberVariable const& x) -> std::string const& 
            {
                return(x.DefaultValue);      
            }, m_Content));
    }
    void StructMemberVariable::Accept(MemberVariableVisitor& Visitor)
    {
        
    }
    //END StructMemberVariable

    //BEGIN StructDefinition
    bool StructDefinition::HasMember(std::string const& MemberToCheck) const
    {
        for(auto const& Member : MemberVariables)
        {
            if(Member.GetName() == MemberToCheck)
            {
                return(true);  
            } 
        }       
        return(false);
    }
    StructMemberVariable const& StructDefinition::GetMember(std::string const& MemberName) const
    {
        for(auto const& Member : MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                return(Member);
            } 
        }       
        throw std::runtime_error("no member exists with name "+MemberName);
    }
    StructMemberVariable& StructDefinition::GetMember(std::string const& MemberName)
    {
        for(auto& Member : MemberVariables)
        {
            if(Member.GetName() == MemberName)
            {
                return(Member);  
            } 
        }       
        throw std::runtime_error("no member exists with name "+MemberName);
    }
    //END StructDefinition
    class MBCCParseError : public std::exception
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
            if(!((Data[ParseOffset] >= 65 && Data[ParseOffset] <= 90) || (Data[ParseOffset] >= 97 && Data[ParseOffset] <= 122)
                        || Data[ParseOffset] == '_' || (Data[ParseOffset] >= 48 && Data[ParseOffset] <= 57)))
            {
                break; 
            }   
            ParseOffset++;
        }
        ReturnValue = std::string(Data+StringBegin,Data+ParseOffset);
        if(ReturnValue == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: empty identifiers are not allowed",ParseOffset);
        }
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
        ParseOffset+=1;
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
        ParseOffset+=1;
        *OutParseOffset = ParseOffset;
        ReturnValue.Name = TerminalName;
        ReturnValue.RegexDefinition = RegexString;
        return(ReturnValue);
    }
    StructMemberVariable MBCCDefinitions::p_ParseMemberVariable(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        StructMemberVariable ReturnValue;
        size_t ParseOffset = InParseOffset;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: end of file before member variable definition or end of struct",ParseOffset);   
        }
        if(Data[ParseOffset] == '{')
        {
            ParseOffset+=1;    
            size_t TypeEnd = std::find(Data+ParseOffset,Data+DataSize,'}')-Data;
            if(TypeEnd >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: Raw member type requries } delimiting the end of the type name",ParseOffset);   
            }
            StructMemberVariable_Raw RawMemberVariable;
            RawMemberVariable.RawMemberType =  std::string(Data+ParseOffset,Data+TypeEnd);
            ParseOffset = TypeEnd+1;
            ReturnValue = StructMemberVariable(RawMemberVariable);
        }
        else
        {
            std::string StructType = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset); 
            SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);
            }
            if(StructType == "List")
            {
                if(Data[ParseOffset] != '<')
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: builtin type List requires tempalte argument",ParseOffset);
                }
                ParseOffset +=1;
                StructMemberVariable_List List;
                List.ListType = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
                SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize || Data[ParseOffset] != '>')
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: builtin type List requires delimiting > for template argument",ParseOffset);
                }
                ParseOffset += 1;
                ReturnValue = StructMemberVariable(List);
            }
            else
            {
                StructMemberVariable_Struct MemberType;
                MemberType.StructType = StructType;
                ReturnValue = StructMemberVariable(MemberType);
            }
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::string MemberVariableName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        ReturnValue.GetName() = MemberVariableName;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);   
        }
        if(Data[ParseOffset] == ';')
        {
            ParseOffset+=1;   
        }
        else if(Data[ParseOffset] == '=')
        {
            ParseOffset +=1;
            //A little bit of a hack, but doesnt require the parsing of any particular data
            size_t ValueEnd = std::find(Data+ParseOffset,Data+DataSize,';')-Data;
            if(ValueEnd >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ; for default value",ParseOffset);   
            }
            ReturnValue.GetDefaultValue() = std::string(Data+ParseOffset,Data+ValueEnd);
            ParseOffset = ValueEnd+1; 
        }
        else
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: member variable needs delimiting ;",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    /*
struct Hej1 : Hej2
{
    List<Hej2> Hejs;
    {RawValue} RawTest;
}
    */
    StructDefinition MBCCDefinitions::p_ParseStruct(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        StructDefinition ReturnValue;
        size_t ParseOffset = InParseOffset; 
        ReturnValue.Name = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset < DataSize && Data[ParseOffset] == ':')
        {
            ParseOffset+=1;
            ReturnValue.ParentStruct = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
            SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(ParseOffset >= DataSize || Data[ParseOffset] != '{')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: struct needs delimiting { for start of member variables",ParseOffset);   
        }
        ParseOffset+=1;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        bool EndDelimiterFound = false;
        while(ParseOffset < DataSize)
        {
            if(Data[ParseOffset] == '}')
            {
                EndDelimiterFound = true;
                ParseOffset+=1;
                break;
            }      
            ReturnValue.MemberVariables.push_back(p_ParseMemberVariable(Data,DataSize,ParseOffset,&ParseOffset));
            SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(!EndDelimiterFound)
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: struct needs delimiting } for end of member variables",ParseOffset);   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue); 
    }
    std::pair<std::string,std::string> MBCCDefinitions::p_ParseDef(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::pair<std::string,std::string> ReturnValue; 
        size_t ParseOffset = InParseOffset;
        std::string RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        if(RuleName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty rule name ",ParseOffset);
        } 
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs delimiting =",ParseOffset);
        }
        ParseOffset+=1;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        std::string StructName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
        if(StructName == "")
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: def needs non empty struct name ",ParseOffset);
        }
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset == DataSize || Data[ParseOffset] != ';')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: no delmiting ; for end of def",ParseOffset);
        }
        ParseOffset+=1;
        *OutParseOffset = ParseOffset;
        ReturnValue.first = RuleName;
        ReturnValue.second = StructName;
        return(ReturnValue);
    }
    std::vector<ParseRule> MBCCDefinitions::p_ParseParseRules(const char* Data,size_t DataSize,size_t InParseOffset,size_t* OutParseOffset)
    {
        std::vector<ParseRule> ReturnValue;
        size_t ParseOffset = InParseOffset;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        if(ParseOffset >= DataSize || Data[ParseOffset] != '=')
        {
            throw MBCCParseError("Syntactic error parsing MBCC definitions: rule needs delimiting = for name and content",ParseOffset);
        }
        ParseOffset +=1;
        SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        ParseRule CurrentRule;
        while(ParseOffset < DataSize)
        {
            //does allow for empty rules, s
            if(Data[ParseOffset] == '|')
            {
                if(CurrentRule.Components.size() == 0)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: emtpy rule is not allowed, | without corresponding components",ParseOffset);
                }
                ReturnValue.push_back(std::move(CurrentRule));          
                CurrentRule = ParseRule();
                ParseOffset+=1;
                SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                continue;
            }
            else if(Data[ParseOffset] == ';')
            {
                ParseOffset+=1;
                break;
            }
            RuleComponent NewComponent;
            std::string RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);    
            SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
            if(ParseOffset >= DataSize)
            {
                throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
            }
            if(Data[ParseOffset] == '=')
            {
                //member assignment    
                ParseOffset += 1;
                NewComponent.AssignedMember = RuleName;
                RuleName = p_ParseIdentifier(Data,DataSize,ParseOffset,&ParseOffset);
                SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
                if(ParseOffset >= DataSize)
                {
                    throw MBCCParseError("Syntactic error parsing MBCC definitions: missing ; in rule definition",ParseOffset);
                }
            }
            if(Data[ParseOffset] == '+')
            {
                NewComponent.Min = 1;    
                NewComponent.Max = -1;    
                ParseOffset++;
            }
            else if(Data[ParseOffset] == '?')
            {
                NewComponent.Min = 0;
                NewComponent.Max = 1;
                ParseOffset++;
            }
            else if(Data[ParseOffset] == '*')
            {
                NewComponent.Min = 0;
                NewComponent.Max = -1;
                ParseOffset++;
            }
            NewComponent.ReferencedRule = RuleName;
            CurrentRule.Components.push_back(NewComponent);
            SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        if(CurrentRule.Components.size() != 0)
        {
            ReturnValue.push_back(std::move(CurrentRule));   
        }
        *OutParseOffset = ParseOffset;
        return(ReturnValue);
    }
    bool h_TypeIsBuiltin(std::string const& TypeToVerify)
    {
        return(TypeToVerify == "String" || TypeToVerify == "Int");
    }
    void MBCCDefinitions::p_VerifyStructs()
    {
        for(auto& Struct : Structs)
        {
            //Verify that the parent struct actually exists    
            if(Struct.ParentStruct != "" && NameToStruct.find(Struct.Name) == NameToStruct.find(Struct.Name))
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: struct named \""+Struct.Name+"\" is the child of a non existing struct named \""+Struct.ParentStruct+"\"");
            } 
            for(auto& MemberVariable : Struct.MemberVariables)
            {
                if(MemberVariable.IsType<StructMemberVariable_List>())
                {
                    if(NameToStruct.find(MemberVariable.GetType<StructMemberVariable_List>().ListType) == NameToStruct.end())
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: List template value in struct \""+Struct.Name+ "\"references unknowns struct named \""+
                                MemberVariable.GetType<StructMemberVariable_List>().ListType+"\"");
                    }
                }
                else if(MemberVariable.IsType<StructMemberVariable_Struct>())
                {
                    StructMemberVariable_Struct& StructMember = MemberVariable.GetType<StructMemberVariable_Struct>();
                    if(h_TypeIsBuiltin(MemberVariable.GetType<StructMemberVariable_Struct>().StructType))
                    {
                        if(StructMember.StructType == "Int")
                        {
                            StructMemberVariable_Int NewMember;
                            NewMember.Name = MemberVariable.GetName();
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            try
                            {
                                NewMember.Value = std::stoi(NewMember.DefaultValue);
                            }
                            catch(std::exception const& e)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: Int member variable not a valid integer");
                            }
                            MemberVariable = StructMemberVariable(NewMember);
                        }
                        else if(StructMember.StructType == "String")
                        {
                            StructMemberVariable_String NewMember;
                            NewMember.Value = MemberVariable.GetDefaultValue();
                            NewMember.DefaultValue = MemberVariable.GetDefaultValue();
                            NewMember.Name = MemberVariable.GetName();
                            MemberVariable = StructMemberVariable(NewMember);
                               
                        }
                    }
                    else if(NameToStruct.find(MemberVariable.GetType<StructMemberVariable_Struct>().StructType) == NameToStruct.end())
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: member variable in struct \""+Struct.Name+"\" refernces unknowns struct named \""+
                                MemberVariable.GetType<StructMemberVariable_Struct>().StructType+"\"");
                    }
                }
            }
        }
    }
    void MBCCDefinitions::p_VerifyRules()
    {
        for(auto& NonTerminal : NonTerminals)
        {
            auto StructIt = NonTerminalToStruct.find(NonTerminal.Name);
            StructDefinition* AssociatedStruct = nullptr;
            if(StructIt != NonTerminalToStruct.end())
            {
                AssociatedStruct = &Structs[StructIt->second];
            }
            for(auto& Rule : NonTerminal.Rules)
            {
                for(auto& Component : Rule.Components)
                {
                    if(auto TermIt = NameToTerminal.find(Component.ReferencedRule); TermIt != NameToTerminal.end())
                    {
                        Component.IsTerminal = true;
                        Component.ComponentIndex = TermIt->second;
                    }   
                    else if(auto NonTermIt = NameToNonTerminal.find(Component.ReferencedRule); NonTermIt != NameToNonTerminal.end())
                    {
                        Component.IsTerminal = false;
                        Component.ComponentIndex = NonTermIt->second;              
                    }
                    else
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                "rule referencing unkown terminal/non-terminal named"
                                " \""+Component.ReferencedRule+"\"");
                    }
                    if(AssociatedStruct != nullptr)
                    {
                        if(Component.AssignedMember == "")
                        {
                            continue;   
                        }
                        if(!AssociatedStruct->HasMember(Component.AssignedMember))
                        {
                            throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                    "rule \""+NonTerminal.Name+"\" referencing non existing member \""+Component.AssignedMember+"\" "
                                    "of struct \""+AssociatedStruct->Name+"\"");
                        }
                        //Check type of assigned member
                        StructMemberVariable const& AssociatedMember = AssociatedStruct->GetMember(Component.AssignedMember); 
                        if(AssociatedMember.IsType<StructMemberVariable_Struct>())
                        {
                            StructMemberVariable_Struct const& StructMember = AssociatedMember.GetType<StructMemberVariable_Struct>();  
                            if(Component.IsTerminal)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning terminal \""+Component.ReferencedRule+"\" to member \""+StructMember.Name+
                                        "\": can only assign non-terminals to non builtin types");
                            }
                            //Struct have already been verified
                            //structmember uses 
                            if(NameToStruct[StructMember.StructType] != Component.ComponentIndex)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning non-terminal \""+Component.ReferencedRule+"\" to member \""+AssociatedMember.GetName()+"\": "+
                                        " member is of type "+Structs[NameToStruct[StructMember.StructType]].Name +" "
                                        "and non-terminal is of type "+Structs[Component.ComponentIndex].Name);
                            }
                        }
                        else if(AssociatedMember.IsType<StructMemberVariable_List>())
                        {
                            StructMemberVariable_List const& ListMember = AssociatedMember.GetType<StructMemberVariable_List>();  
                            if(Component.IsTerminal)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning terminal \""+Component.ReferencedRule+"\" to member \""+ListMember.Name+
                                        "\": can only assign non-terminals to non builtin types");
                            }
                            //Struct have already been verified
                            //structmember uses 
                            if(NameToStruct[ListMember.ListType] != Component.ComponentIndex)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error in member assignment for non-terminal \""+NonTerminal.Name+"\": "
                                        "error assigning non-terminal \""+Component.ReferencedRule+"\" to member \""+AssociatedMember.GetName()+"\": "+
                                        " member is of type "+Structs[NameToStruct[ListMember.ListType]].Name +" "
                                        "and non-terminal is of type "+Structs[Component.ComponentIndex].Name);
                            }
                        }
                        else if(AssociatedMember.IsType<StructMemberVariable_Int>())
                        {
                            if(Component.IsTerminal == false)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error with member assignment in non-terminal \""+NonTerminal.Name+"\": "
                                        "only a terminal can be assigned to builtin scalar types");
                            }     
                        }
                        else if(AssociatedMember.IsType<StructMemberVariable_String>())
                        {
                            if(Component.IsTerminal == false)
                            {
                                throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                        "error with member assignment in non-terminal \""+NonTerminal.Name+"\": "
                                        "only a terminal can be assigned to builtin scalar types");
                            }     
                        }
                    }
                    else
                    {
                        if(Component.AssignedMember != "")
                        {
                            throw std::runtime_error("Semantic error parsing MBCC definitions: "
                                    "non-terminal \""+NonTerminal.Name+"\" assigning to member \""+Component.AssignedMember+"\" "
                                    "but doesn't have any linked struct");
                        }
                    }
                } 
            }
        } 
    }
    //Parse def already verifies that all links between struct and non-terminal/terminal is true
    //here we only have to verify wheter or not the parse rules and structures abide by the semantics
    void MBCCDefinitions::p_UpdateReferencesAndVerify()
    {
        p_VerifyStructs();
        p_VerifyRules();
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
                if(ReturnValue.NameToTerminal.find(NewTerminal.Name) != ReturnValue.NameToTerminal.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: duplicate definition for terminal \""+NewTerminal.Name+"\"");
                }
                if(ReturnValue.NameToNonTerminal.find(NewTerminal.Name) != ReturnValue.NameToNonTerminal.end())
                {
                    throw std::runtime_error("Semantic error parsing MBCC definitions: attempting to define terminal with the same name as a nonterminal named \""+NewTerminal.Name+"\"");
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
                NonTerminal NewTerminal;
                NewTerminal.Rules = std::move(NewRules);
                NewTerminal.Name = CurrentIdentifier;
                auto NonTermIt = ReturnValue.NameToNonTerminal.find(NewTerminal.Name); 
                if(NonTermIt != ReturnValue.NameToNonTerminal.end())
                {
                    NonTerminal& AssociatedNonTerminal = ReturnValue.NonTerminals[NonTermIt->second];
                    AssociatedNonTerminal.Rules.insert(AssociatedNonTerminal.Rules.end(),std::make_move_iterator(NewTerminal.Rules.begin()),
                            std::make_move_iterator(NewTerminal.Rules.end()));
                }
                else
                {
                    if(ReturnValue.NameToTerminal.find(NewTerminal.Name) != ReturnValue.NameToTerminal.end())
                    {
                        throw std::runtime_error("Semantic error parsing MBCC definitions: attempting to define non-terminal with the same name as a terminal named \""+NewTerminal.Name+"\"");
                    }
                    size_t CurrentIndex = ReturnValue.NonTerminals.size();
                    ReturnValue.NameToNonTerminal[NewTerminal.Name] = CurrentIndex;
                    ReturnValue.NonTerminals.push_back(std::move(NewTerminal));
                }
            }
            MBParsing::SkipWhitespace(Data,DataSize,ParseOffset,&ParseOffset);
        }
        for(auto const& Def : UnresolvedDefs)
        {
            if(ReturnValue.NameToNonTerminal.find(Def.first) == ReturnValue.NameToNonTerminal.end())
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined rule \""+Def.first+"\"");    
            }
            if(ReturnValue.NameToStruct.find(Def.second) == ReturnValue.NameToStruct.end())
            {
                throw std::runtime_error("Semantic error parsing MBCC definitions: def referencing undefined struct \""+Def.second+"\"");    
            }
            ReturnValue.NonTerminalToStruct[Def.first] = ReturnValue.NameToStruct[Def.second];
        }
        ReturnValue.p_UpdateReferencesAndVerify();
        return(ReturnValue);
    }
    int h_OffsetToLine(const char* Data,size_t ParseOffset)
    {
        int ReturnValue = 0;    
        for(size_t i = 0; i < ParseOffset;i++)
        {
            if(Data[i] == '\n')
            {
                ReturnValue += 1;
            }   
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
        catch(MBCCParseError const& Exception)
        {
            OutError ="Error at line " +std::to_string(h_OffsetToLine(Data,Exception.ParseOffset))+" "+ Exception.ErrorMessage;
        }
        catch(std::exception const& e)
        {
            OutError = e.what(); 
        }
        return(ReturnValue);
    }
    //END MBCCDefinitions
    

    //BEGIN LLParserGenerator
    std::vector<bool> LLParserGenerator::p_RetrieveENonTerminals(MBCCDefinitions const& Grammar)
    {
        std::vector<bool> ReturnValue = std::vector<bool>(Grammar.NonTerminals.size(),false);
        //Transitive closure algorithm, naive implementation
        while(true)
        {
            bool HasChanged = false;    
            for(int i = 0; i < Grammar.NonTerminals.size();i++)
            {
                if(ReturnValue[i])
                {
                    continue;   
                }
                auto const& NonTerminal = Grammar.NonTerminals[i];   
                for(auto const& Rule : NonTerminal.Rules)
                {
                    bool IsENonTerminal = true;
                    for(auto const& Component : Rule.Components)
                    {
                        if(Component.IsTerminal)
                        {
                            break;   
                        }
                        if(Component.Min != 0 && !ReturnValue[Component.ComponentIndex])
                        {
                            IsENonTerminal = false;
                            break;
                        }
                    }  
                    if(IsENonTerminal)
                    {
                        HasChanged = true;
                        ReturnValue[i] = true;
                        break;
                    }
                }
            }
            if(!HasChanged)
            {
                break;
            }
        }
        return ReturnValue;         
    }
    //Maybe kinda slow, should do a proper ordo analysis of the algorithm
    void p_VerifyNonTerminalLeftRecursive(NonTerminalIndex CurrentIndex,std::vector<bool>& VisitedTerminals,std::vector<bool> const& ERules,MBCCDefinitions const& Grammar)
    {
        if(VisitedTerminals[CurrentIndex] == true)
        {
            throw std::runtime_error("Error creating LL parser for grammar: NonTerminal \""+Grammar.NonTerminals[CurrentIndex].Name+ "\" is left recursive");    
        }
        VisitedTerminals[CurrentIndex] = true;
        for(auto const& Rule : Grammar.NonTerminals[CurrentIndex].Rules)
        {
            for(auto const& Component : Rule.Components)
            {
                if(Component.IsTerminal)
                {
                    return;
                }
                p_VerifyNonTerminalLeftRecursive(CurrentIndex,VisitedTerminals,ERules,Grammar);
                if(!(Component.Min == 0 || ERules[Component.ComponentIndex]))
                {
                    break; 
                }
            }  
        }
    }
    void LLParserGenerator::p_VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules)
    {
        for(int i = 0; i <  Grammar.NonTerminals.size();i++)
        {
            std::vector<bool> VisitedTerminals = std::vector<bool>(i,false);
            p_VerifyNonTerminalLeftRecursive(i,VisitedTerminals,ERules,Grammar);
        }    
    }
    MBMath::MBDynamicMatrix<bool> LLParserGenerator::p_ConstructNonTermFollow(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules)
    {
        MBMath::MBDynamicMatrix<bool> ReturnValue(Grammar.NonTerminals.size(),Grammar.NonTerminals.size());
        //Transative closure algorithm: TODO optimize
        for(int i = 0; i < Grammar.NonTerminals.size();i++)
        {
            auto const& NonTerminal = Grammar.NonTerminals[i];    
            for(auto const& Rule : NonTerminal.Rules)
            {
                for(auto const& Component : NonTerminal.Rules)
                {
                     
                }   
            }
        }  

        return ReturnValue;
    }
    BoolTensor::BoolTensor(int i,int j,int k)
    {
        m_Data = std::vector<bool>(i * j * k,false);        
        m_J = j;
        m_K = k;
    }
    void BoolTensor::SetValue(int i,int j,int k)
    {
        m_Data[i*(m_J*m_K) + (j*m_K)+k] = true;
    }
    bool BoolTensor::GetValue(int i, int j, int k) const
    {
        return(m_Data[i*(m_J*m_K) + (j*m_K)+k]);
    }
    BoolTensor LLParserGenerator::p_ConstructFIRST(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules,int k)
    {
        BoolTensor ReturnValue(Grammar.NonTerminals.size(),k,Grammar.Terminals.size());
         
        return ReturnValue;      
    }
    void LLParserGenerator::p_WriteDefinitions(MBCCDefinitions const& Grammar,std::vector<TerminalStringMap> const& ParseTable,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut, int k)
    {
           
    }
    void LLParserGenerator::WriteLLParser(MBCCDefinitions const& InfoToWrite,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k)
    {
        
    } 
    //END LLParserGenerator
}
