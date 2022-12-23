#include <string>
#include <vector>
#include <unordered_map>
#include <MBUtility/MBErrorHandling.h>
#include <MBUtility/MBInterfaces.h>
#include <utility>
#include <variant>
#include <regex>
#include <deque>

#include <MBUtility/MBMatrix.h>
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
        TerminalIndex Type = -1;
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
        std::unordered_map<std::string,NonTerminalIndex> NameToNonTerminal;
        std::unordered_map<std::string,TerminalIndex> NameToTerminal;
        std::unordered_map<std::string,StructIndex> NonTerminalToStruct;
        std::unordered_map<std::string,StructIndex> NameToStruct;

        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset);
        static MBCCDefinitions ParseDefinitions(const char* Data,size_t DataSize,size_t ParseOffset,std::string& OutError);
    };
    //Missing entry can be deduced by the bool being false
    class TerminalStringMap
    {
    private:
    public:
        TerminalStringMap(int k);
        bool& operator[](std::vector<TerminalIndex> const& Key);
        bool const& operator[](std::vector<TerminalIndex> const& Key) const;
    };
    class BoolTensor
    {
    private:
        std::vector<bool> m_Data;
        int m_J = 0;
        int m_K = 0;
    public:
        BoolTensor(int i,int j,int k);
        void SetValue(int i,int j,int k );
        bool GetValue(int i, int j, int k) const;
    };
    class GLA
    {
    private:
        typedef int NodeIndex;
        struct GLAEdge
        {
            GLAEdge() = default;
            GLAEdge(TerminalIndex TerminalIndex,NodeIndex ConnectionIndex)
            {
                ConnectionTerminal = TerminalIndex;   
                Connection = ConnectionIndex;
            }
            //-1 means that it's and E connection
            TerminalIndex ConnectionTerminal = -1;
            NodeIndex Connection = -1;
        };
        struct GLANode
        {
            GLANode(int k)
            {
                Visiting = std::vector<bool>(k);    
            }
            std::vector<bool> Visiting;
            std::vector<GLAEdge> Edges;
        };
        //The first size(NonTerm) is the Initial nodes, the following size(NonTerm) are the accepting nodes, and then 
        //the following are internal connections
        //Maybe replace with MBVector if the size of the GLA edges i relativly bounded?
        std::vector<GLANode> m_Nodes;
        NonTerminalIndex m_NonTerminalCount = 0;
        TerminalIndex m_TerminalCount = 0;
        //std::vector<NodeIndex> m_ProductionBegin;

        std::vector<bool> p_LOOK(GLANode& Edge,int k);
    public:
        GLA(MBCCDefinitions const& Grammar,int k);
        //TODO optimize, currently exponential time algorithm
        MBMath::MBDynamicMatrix<bool> LOOK(NonTerminalIndex NonTerminal,int ProductionIndex,int k);
        //Used for A* components
        //MBMath::MBDynamicMatrix<bool> FIRST(NonTerminalIndex NonTerminal,int k);
        //BoolTensor CalculateFIRST();
        //BoolTensor CalculateFOLLOW();
    };
    class Tokenizer
    {
    private:      
        //Easy interfac, memeory map everything   
        size_t m_ParseOffset = 0;
        std::string m_TextData;
        std::vector<std::regex> m_TerminalRegexes;
        std::deque<Token> m_StoredTokens;
        Token p_ExtractToken();
    public:
        Tokenizer(std::string Text,std::vector<Terminal> const& Terminals);
        void ConsumeToken();
        Token const& Peek(int Depth = 0);
    };
    class LLParserGenerator
    {
        static std::vector<bool> p_RetrieveENonTerminals(MBCCDefinitions const& Grammar);
        static void p_VerifyNotLeftRecursive(MBCCDefinitions const& Grammar,std::vector<bool> const& ERules);
        //Non Terminal X Non Terminal Size
        //Based on "LL LK requries k > 1, and in turn on the Linear-approx-LL(k) algorithm. If I understand the 
        //algoritm correctly however, so might it be a bit of an pessimisation, as NonTermFollow of a NonTerminal 
        //Is actually dependant on the specific rule being processed
        //Non Terminal x K x Terminal 
        //TerminalIndex = -1 sentinel for empty rule, +1 for empty, +2 for EOF
        void p_WriteDefinitions(MBCCDefinitions const& Grammar,std::vector<TerminalStringMap> const& ParseTable,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut, int k);
        void p_WriteParser(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
        void p_WriteSource(MBCCDefinitions const& Grammar,std::vector<std::vector<MBMath::MBDynamicMatrix<bool>>> const& ProductionsLOOk,
                MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut);
    public:
        void WriteLLParser(MBCCDefinitions const& InfoToWrite,MBUtility::MBOctetOutputStream& HeaderOut,MBUtility::MBOctetOutputStream& SourceOut,int k = 2);
    };
}
