
#include "MBParsing.h"
#include <assert.h>
namespace MBParsing
{
	void Test()
	{

		//MBParsing Test
		//std::string BNFTestData = "test = \"hej\";\ntest2 = test|\"hej2\";";
		std::string BNFSpecification = MBUtility::ReadWholeFile("./MBParsing/BNFSpecification.txt");
		MBParsing::BNFParser TestParser;
		MBError Result = TestParser.InitializeRules(BNFSpecification);
		if (!Result)
		{
			std::cout << Result.ErrorMessage << std::endl;
			exit(0);
		}
		std::string BNFExamples = MBUtility::ReadWholeFile("./MBParsing/BNFTests.txt");
		std::vector<std::string> Tokens = MBParsing::TokenizeText(BNFExamples);
		//MBParsing::SyntaxTree SynataxResult = TestParser["StatementList"].Parse(Tokens.data(), Tokens.size(), 0, nullptr, nullptr);
		bool SyntaxParseResult = true;
		MBParsing::SyntaxTree TermTest = TestParser["term"].Parse(Tokens.data(), Tokens.size(), 0, nullptr, &SyntaxParseResult);
		assert(SyntaxParseResult);
		TestParser.PrintTree(TermTest);
		MBParsing::SyntaxTree ExpressionTest = TestParser["expression"].Parse(Tokens.data(), Tokens.size(), 0, nullptr, &SyntaxParseResult);
		assert(SyntaxParseResult);
		TestParser.PrintTree(ExpressionTest);
		MBParsing::SyntaxTree StatementTest = TestParser["statement"].Parse(Tokens.data(), Tokens.size(), 0, nullptr, &SyntaxParseResult);
		assert(SyntaxParseResult);
		TestParser.PrintTree(StatementTest);
		MBParsing::SyntaxTree StatementListTest = TestParser["StatementList"].Parse(Tokens.data(), Tokens.size(), 0, nullptr, &SyntaxParseResult);
		assert(SyntaxParseResult);
		TestParser.PrintTree(StatementListTest);
		exit(0);
	}
 }