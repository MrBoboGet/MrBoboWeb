#include <string>
#include <vector>
#include <unordered_map>
#include <MBUtility/MBErrorHandling.h>
#include <MBUtility/MBInterfaces.h>
#include <utility>
namespace MBParsing
{
    class MemberVariable
    {
    public:
        std::string Name; 
        std::string DefaultValue;
    };
    class StructMemberVariable_List
    {
    public:
        std::string ListType;
    };
    class StructMemberVariable_Raw
    {
    public:
        std::string RawMemberName;
    };
    class StructMemberVariable_Struct
    {
    public:
        std::string StructType;
    };
    class MemberVariableVisitor;
    class StructMemberVariable
    {
    
    public:
        void Accept(MemberVariableVisitor& Visitor);
    };
    struct StructDefinition
    {
        std::string Name; 
        std::string ParentStruct;
        std::vector<StructMemberVariable> MemberVariables;
    };
    typedef int RuleIndex;
    //-1 reserved for the empty word / end of file
    typedef int TerminalIndex;
    typedef int ParseIndex;
    typedef int StructIndex;
    struct RuleComponent
    {
        bool IsTerminal = false;
        int Min = 1;
        //-1 means that Max is unbounded
        int Max = 1;
        std::string AssignedMember;
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
    struct Token
    {
        TerminalIndex Type;
        std::string Value;    
    };
    class MBCCDefinitions
    {
    private:
        void p_UpdateReferencesAndVerify();

        static std::string p_ParseIdentifier(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static Terminal p_ParseTerminal(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::pair<std::string,std::string> p_ParseDef(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static StructDefinition p_ParseStruct(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
        static std::vector<ParseRule> p_ParseParseRules(const char* Data,size_t DataSize,size_t ParseOffset,size_t* OutParseOffset);
    public:
        std::vector<Terminal> Terminals;
        std::vector<ParseRule> ParseRules;
        std::vector<StructDefinition> Structs;
        std::unordered_map<std::string,std::vector<ParseIndex>> NonTerminals;
        std::unordered_map<std::string,StructIndex> NonTerminalToStruct;
        std::unordered_map<std::string,TerminalIndex> NameToTerminal;
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
