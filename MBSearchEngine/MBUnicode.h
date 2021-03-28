#pragma once
#include <string>
#include <vector>
#include <MBSearchEngine/MBUnicodeMacros.h>
namespace MBUnicode
{
	typedef unsigned int Codepoint;

	struct CodepointRange
	{
		Codepoint Lower = 0;
		Codepoint Higher = 0;
	};

	class UnicodeString
	{
	private:
		std::vector<Codepoint> m_Codepoints = {};
		int m_NumberOfCharacters = 0;
	public:
		UnicodeString(std::string const& StringToConvert);
		std::string GetHexRepresentation();
	};

	bool CompareCodepointRange(CodepointRange const& LeftElement, CodepointRange const& RightElement);
	int CreateCMacroCodepointArrayFromPropertySpec(std::string const& InputFilename, std::string const& OutputFilename);
};