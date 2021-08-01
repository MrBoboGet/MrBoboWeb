#pragma once
#include <vector>
#include <MBErrorHandling.h>
namespace MBParsing
{
	struct LinearParseState
	{
		size_t ParseOffset = 0;
		MBError ParseError = MBError(true);
	};

	void SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
	void SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset);
	std::string ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, const void* DataToParse,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);

	class MBDBObjectScript_Statement
	{

	};

	class MBDBObjectScript_ParsingContext
	{
	private:

	public:

	};
	class MBDBObjectScript_ExecutionContext
	{
	private:

	public:

	};
};