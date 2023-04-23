#include "MBCC.h"
#include "MBUtility/MBInterfaces.h"
#include <iostream>
#include <MBUtility/MBFiles.h>

void PrintResult(MBParsing::MBCCDefinitions const& DataToPrint)
{
    for(auto const& Def : DataToPrint.NameToStruct)
    {
        std::cout<<"def "<<Def.first<<"="<<Def.second<<"\n"; 
    }       
    for(auto const& Term : DataToPrint.Terminals)
    {
        std::cout<<"term "<<Term.Name<<"="<<Term.RegexDefinition<<"\n";    
    }
    for(auto const& Struct : DataToPrint.Structs)
    {
        std::cout<<"struct "<<Struct.Name<<" parent "<<Struct.ParentStruct<<"\n";
        for(auto const& Member : Struct.MemberVariables)
        {
            std::cout<<"member "<<Member.GetName()<<" ";
            if(Member.IsType<MBParsing::StructMemberVariable_Raw>())
            {
                std::cout<<"Raw type: "<<Member.GetType<MBParsing::StructMemberVariable_Raw>().RawMemberType;
            }
            else if(Member.IsType<MBParsing::StructMemberVariable_Struct>())
            {
                std::cout<<"Regular type: "<<Member.GetType<MBParsing::StructMemberVariable_Struct>().StructType;
            }
            else if(Member.IsType<MBParsing::StructMemberVariable_List>())
            {
                std::cout<<"List type: "<<Member.GetType<MBParsing::StructMemberVariable_List>().ListType;
            }
            if(Member.GetDefaultValue() != "")
            {
                std::cout<<" DefaultValue="<<Member.GetDefaultValue();   
            }
            std::cout<<"\n";
            
        }
    }
    for(auto const& NonTerminal : DataToPrint.NonTerminals)
    {
        std::cout<<"NonTerminal "<<NonTerminal.Name<<"\n";
        for(auto const& Rule : NonTerminal.Rules)
        {
            std::cout<<"rule"<<"\n";
            for(auto const& Component : Rule.Components)
            {
                std::cout<<"comp referencing "<<Component.ReferencedRule;
                if(Component.AssignedMember != "")
                {
                    std::cout<<" with  assigned member "<<Component.AssignedMember;   
                }
                std::cout<<" "<<std::to_string(Component.Min);
                std::cout<<" "<<std::to_string(Component.Max);
                std::cout<<"\n";
            }
        }
    }
    std::cout.flush();
}
int main(int argc, char** argv)
{
    std::string FileToReadFrom = "ObjectSpec_BNF.mbnf";
    if(argc == 2)
    {
        FileToReadFrom = argv[1];    
    }
    std::string FileData = MBUtility::ReadWholeFile(FileToReadFrom); 
    std::cout<<FileData<<std::endl;
    std::string Error;
    MBParsing::MBCCDefinitions Result = MBParsing::MBCCDefinitions::ParseDefinitions(FileData.data(),FileData.size(),0,Error);
    if(Error != "")
    {
        std::cout<<Error<<std::endl;   
    }
    PrintResult(Result);
    MBUtility::MBFileOutputStream SourceFile("MBObjectSpec_Parser.cpp");
    MBUtility::MBFileOutputStream HeaderFile("MBObjectSpec_Parser.h");
    try
    {
        MBParsing::LLParserGenerator Generator;
        Generator.WriteLLParser(Result,SourceFile,HeaderFile,1);

    }
    catch(std::exception const& e)
    {
        std::cout<<e.what()<<std::endl;    
    }
}
