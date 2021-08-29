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

	//BASE64 grejer
	unsigned char Base64CharToBinary(unsigned char CharToDecode);
	char ByteToBASE64(uint8_t ByteToEncode);
	std::string Base64ToBinary(std::string const& Base64Data);
	std::string BASE64Decode(const void* CharactersToRead, size_t NumberOfCharacters);
	std::string BASE64Encode(const void* DataToEncode, size_t DataLength);
	std::string BASE64Decode(std::string const& DataToDecode);
	std::string BASE64Encode(std::string const& DataToEncode);
	//

	void UpdateParseState(size_t CurrentOffset, MBError& ErrorToMove, size_t* OutOffset, MBError* OutError);

	size_t FindSubstring(const void* Data, size_t DataSize, const void* Data2, size_t Data2Size,size_t ParseOffset);
	size_t FindSubstring(std::string const& StringToSearch, std::string const& Substring, size_t ParseOffset);

	void SkipWhitespace(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
	void SkipWhitespace(const void* DataToParse, size_t DataLength, size_t InOffset, size_t* OutOffset);
	std::string ParseQuotedString(void const* DataToParse,size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	std::string ParseQuotedString(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	intmax_t ParseJSONInteger(std::string const& ObjectData, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, const void* DataToParse,size_t DataSize, size_t InOffset, MBError* OutError = nullptr);
	size_t GetNextDelimiterPosition(std::vector<char> const& Delimiters, std::string const& DataToParse, size_t InOffset, MBError* OutError = nullptr);
};