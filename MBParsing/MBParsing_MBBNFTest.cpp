#include "MBCC.h"
#include <iostream>
#include <MBUtility/MBFiles.h>

void PrintResult(MBParsing::MBCCDefinitions const& DataToPrint)
{
    for(auto const& Def : DataToPrint.NonTerminalToStruct)
    {
        std::cout<<"def "<<Def.first<<"="<<Def.second<<"\n"; 
    }       
    for(auto const& Term : DataToPrint.Terminals)
    {
        std::cout<<"term "<<Term.Name<<"="<<Term.RegexDefinition<<"\n";    
    }
    std::cout.flush();
}
int main(int argc, char** argv)
{
    std::string FileData = MBUtility::ReadWholeFile("MBBNFTest.mbnf"); 
    std::string Error;
    MBParsing::MBCCDefinitions Result = MBParsing::MBCCDefinitions::ParseDefinitions(FileData.data(),FileData.size(),0,Error);
    if(Error != "")
    {
        std::cout<<Error<<std::endl;   
    }
    PrintResult(Result);
}
