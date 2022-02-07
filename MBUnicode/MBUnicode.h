#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <deque>

#include "MBUnicodeDefinitions.h"

#include <MBUtility/MBFIFOBuffer.h>

namespace MBUnicode
{
	struct CodepointRange
	{
		Codepoint Lower = 0;
		Codepoint Higher = 0;
	};

	class UnicodeString
	{
	private:
		std::vector<Codepoint> m_Codepoints = {};
	public:
		UnicodeString(std::string const& StringToConvert);
		std::string GetHexRepresentation();
	};
	std::string ToUTF8String(Codepoint CodepointToConvert);
	class UnicodeCodepointSegmenter
	{
	private:
		std::deque<Codepoint> m_DecodedCodepoints = {};
		//MBUtility::FIFOBuffer<char> m_ByteFifoBuffer = {};
		Codepoint m_CurrentCodepoint = 0;
		uint8_t m_CurrentCodepointNeededBytes = 0;
		uint8_t p_GetNeededExtraBytes(uint8_t ByteToCheck);

		std::string m_LastError = "";
	public:
		bool IsValid();
		std::string GetLastError();

		void InsertData(const void* DataToInsert, size_t DataSize);
		size_t AvailableCodepoints();
		Codepoint ExtractCodepoint();
	};

	class GraphemeCluster
	{
	private:
		std::vector<Codepoint> m_InternalBuffer = {};
	public:
		bool operator==(GraphemeCluster const& OtherCluster)
		{
			return(m_InternalBuffer == OtherCluster.m_InternalBuffer);
		}
		bool operator!=(GraphemeCluster const& OtherCluster)
		{
			return(!(*this == OtherCluster));
		}
	

		Codepoint& operator[](size_t Index);
		Codepoint const& operator[](size_t Index) const;
		size_t size()
		{
			return(m_InternalBuffer.size());
		}
		std::string ToString()
		{
			std::string ReturnValue = "";
			for (size_t i = 0; i < m_InternalBuffer.size(); i++)
			{
				ReturnValue += ToUTF8String(m_InternalBuffer[i]);
			}
			return(ReturnValue);
		}
		void AddCodepoint(Codepoint CodepointToAdd)
		{
			m_InternalBuffer.push_back(CodepointToAdd);
		}
	};
	class GraphemeClusterSegmenter
	{
	private:
		std::deque<GraphemeCluster> m_DecodedCluster = {};
		GraphemeCluster m_CurrentCluster;
		GraphemeBreakProperty m_LastProperty = GraphemeBreakProperty::SOF;
	public:
		void InsertCodepoints(const Codepoint* CodepointsToInsert, size_t NumberOfCodepoints);
		void InsertCodepoint(Codepoint CodepointsToInsert);
		void Finalize();
		size_t AvailableClusters();
		GraphemeCluster ExtractCluster();
	};

	std::string PathToUTF8(std::filesystem::path const& PathToProcess);
	//std::string Convert_U16_U8(void* Data, size_t DataLength);
	//std::string Convert_U16_U8(const char16_t* StringToConvert);
	std::string UnicodeStringToLower(std::string const& StringToParse);
	//int CreateCMacroCodepointArrayFromPropertySpec(std::string const& InputFilename, std::string const& OutputFilename);

	void CreateCodepointPropertiesHeader(std::string const& DataDirectory);
};