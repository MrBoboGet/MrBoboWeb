#include <string>
#include <vector>
#include <unordered_map>
#include <MBUtility/MBErrorHandling.h>
#include <MBUtility/MBInterfaces.h>
#include <utility>
#include <variant>
namespace MBParsing
{
    class MemberVariable
    {
    public:
        std::string Name; 
        std::string DefaultValue;
    };
    class StructMemberVariable_List : public MemberVariable
    {
    public:
        std::string ListType;
    };
    class StructMemberVariable_Raw : public MemberVariable
    {
    public:
        std::string RawMemberType;
    };
    class StructMemberVariable_Struct : public MemberVariable
    {
    public:
        std::string StructType;
    };
    class StructMemberVariable_Int : public MemberVariable
    {
    public:
        int Value;
    };
    class StructMemberVariable_String : public MemberVariable
    {
    public:
        std::string Value;
    };
    //The more I use this class the more I realise that using the variant directly is most likely more than
    //enough
    class MemberVariableVisitor;
    class StructMemberVariable
    {
        std::variant<StructMemberVariable_Raw,StructMemberVariable_List,StructMemberVariable_Struct,StructMemberVariable_Int,StructMemberVariable_String> 
            m_Content;
    public:
        StructMemberVariable() = default;
        StructMemberVariable(StructMemberVariable_List ListMemberVariable);
        StructMemberVariable(StructMemberVariable_Raw RawMemberVariable);
        StructMemberVariable(StructMemberVariable_Struct StructMemberVariable);
        StructMemberVariable(StructMemberVariable_Int RawMemberVariable);
        StructMemberVariable(StructMemberVariable_String StructMemberVariable);
        std::string& GetName();
        std::string& GetDefaultValue();
        std::string const& GetName() const;
        std::string const& GetDefaultValue() const;
        template<typename T>
        bool IsType() const
        {
            return(std::holds_alternative<T>(m_Content));    
        }
        template<typename T>
        T& GetType()
        {
            return(std::get<T>(m_Content)); 
        }
        template<typename T>
        T const& GetType() const
        {
            return(std::get<T>(m_Content)); 
        }
        void Accept(MemberVariableVisitor& Visitor);
    };
    class MemberVariableVisitor
    {
    public:
        void Visit(StructMemberVariable_Raw const& Raw){};
        void Visit(StructMemberVariable_List const& List){};
        void Visit(StructMemberVariable_Struct const& Struct){};
        void Visit(StructMemberVariable_Int const& Int){};
        void Visit(StructMemberVariable_String const& String){};
    };
    struct StructDefinition
    {
        std::string Name; 
        std::string ParentStruct;
        std::vector<StructMemberVariable> MemberVariables;
        bool HasMember(std::string const& MemberToCheck) const;
        StructMemberVariable const& GetMember(std::string const& MemberName) const;
        StructMemberVariable& GetMember(std::string const& MemberName);
    };
    typedef int RuleIndex;
    //-1 reserved for the empty word / end of file
    typedef int TerminalIndex;
    typedef int NonTerminalIndex;
    typedef int ParseIndex;
    typedef int StructIndex;
    struct RuleComponent
    {
        bool IsTerminal = false;
        int Min = 1;
        //-1 means that Max is unbounded
        int Max = 1;
        std::string AssignedMember;
        std::string ReferencedRule;
        //can either be a RuleIndex, or a TerminalIndex depending
        ParseIndex ComponentIndex = 0;
    };
    struct ParseRule
    {
        std::vector<RuleComponent> Components;
    };
    struct Terminal
    {
        std::string Name;        
        std::string RegexDefinition;        
    };
    struct NonTerminal
    {
        std::string Name;    
        std::vector<ParseRule> Rules;
    };
    struct Token
    {
        TerminalIndex Type;
        std::string Value;    
    };
    class MBCCDefinitions
    {
    private:
        void p_VerifyStructs();
        void p_VerifyRules();
        void p_UpdateReferencesAndVerify();

        static std::string p_ParseIdentifier(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static Terminal p_ParseTerminal(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::pair<std::string,std::string> p_ParseDef(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static StructMemberVariable p_ParseMemberVariable(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static StructDefinition p_ParseStruct(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::vector<ParseRule> p_ParseParseRules(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
    public:
        std::vector<Terminal> Terminals;
        std::vector<NonTerminal> NonTerminals;
        std::vector<StructDefinition> Structs;
        std::unordered_map<std::string,TerminalIndex> NameToNonTerminal;
        std::unordered_map<std::string,TerminalIndex> NameToTerminal;
        std::unordered_map<std::string,StructIndex> NonTerminalToStruct;
        std::unordered_map<std::string,StructIndex> NameToStruct;

        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset);
        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,std::string& OutError);
    };
    
    class LLParserGenerator
    {
    public:
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
    };
      
}
