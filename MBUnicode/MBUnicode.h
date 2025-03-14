
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <deque>

#include "MBUnicodeDefinitions.h"

#include <MBUtility/MBFIFOBuffer.h>
#include <MBUtility/MBVector.h>

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
		std::string m_LastError = "";

		static uint8_t p_GetNeededExtraBytes(uint8_t ByteToCheck);
	public:
		bool IsValid();
		std::string GetLastError();

		void InsertData(const void* DataToInsert, size_t DataSize);
		size_t AvailableCodepoints();
		Codepoint ExtractCodepoint();
		void Reset();


        //Newer, less allocaty interface
        static const unsigned char* ParseUTF8Codepoint(const unsigned char* Begin, const unsigned char* End,Codepoint& OutValue);
	};

	class GraphemeCluster
	{
	private:
        MBUtility::MBVector<char,4> m_InternalBuffer = {};

        bool p_CompareUTF8String(const char* UTF8String,size_t Size) const;
	public:
		bool operator==(GraphemeCluster const& OtherCluster)
		{
			return(m_InternalBuffer == OtherCluster.m_InternalBuffer);
		}
		bool operator!=(GraphemeCluster const& OtherCluster)
		{
			return(!(*this == OtherCluster));
		}
		
		bool operator==(char CharToCompare) const;
		bool operator!=(char CharToCompare) const;

		bool operator==(GraphemeCluster const& OtherCluster) const;
		bool operator!=(GraphemeCluster const& OtherCluster) const;

		bool operator==(std::string const& StringToCompare) const;
		bool operator!=(std::string const& StringToCompare) const;

		bool operator==(const char* StringToCompare) const;
		bool operator!=(const char* StringToCompare) const;

		bool operator==(std::string_view StringToCompare) const;
		bool operator!=(std::string_view StringToCompare) const;

		bool operator<(GraphemeCluster const& rhs) const
        {
            return m_InternalBuffer < rhs.m_InternalBuffer;
        }

        //TODO sus, assumes the string is a valid grapheme cluster
		GraphemeCluster& operator=(std::string const& StringToConvert);
		GraphemeCluster& operator=(char CharToConvert);
		GraphemeCluster& operator=(std::string_view String);
		GraphemeCluster& operator=(const char* String);

		GraphemeCluster() {};
		explicit GraphemeCluster(std::string const& StringToConvert);

		static bool ParseGraphemeCluster(GraphemeCluster& OutCluster, const void* InputData, size_t InputDataSize, size_t InputDataOffset, size_t* OutOffset);
		static bool ParseGraphemeClusters(std::vector<GraphemeCluster>& OutCluster, const void* InputData, size_t InputDataSize,size_t InputOffset);
		static bool ParseGraphemeClusters(std::vector<GraphemeCluster>& OutCluster, std::string const& InputString);
        
        bool IsASCIIControl() const;
        bool IsEmpty() const;


        std::string_view GetView() const
        {
            return std::string_view(m_InternalBuffer.data(),m_InternalBuffer.size());
        }
        
		std::string ToString() const
		{
			std::string ReturnValue = "";
            ReturnValue.insert(ReturnValue.begin(),m_InternalBuffer.data(),m_InternalBuffer.data()+m_InternalBuffer.size());
			return(ReturnValue);
		}
		void AddCodepoint(Codepoint CodepointToAdd)
		{
            std::string NewString = ToUTF8String(CodepointToAdd);
            for(size_t i = 0; i < NewString.size();i++)
            {
                m_InternalBuffer.push_back(NewString[i]);   
            }
		}
	};
	class GraphemeClusterSegmenter
	{
	private:
		std::deque<GraphemeCluster> m_DecodedCluster = {};
		GraphemeCluster m_CurrentCluster;
		GraphemeBreakProperty m_LastProperty = GraphemeBreakProperty::SOF;
	public:
		bool CodepointWouldBreak(Codepoint CodepointToTest) const;
		void InsertCodepoints(const Codepoint* CodepointsToInsert, size_t NumberOfCodepoints);
		void InsertCodepoint(Codepoint CodepointsToInsert);
		void Finalize();
		size_t AvailableClusters();
		GraphemeCluster ExtractCluster();
		void Reset();

        //Newer, less allocaty interface
        static const unsigned char* ParseGraphemeCluster(const unsigned char* Begin, const unsigned char* End);
	};

	std::string PathToUTF8(std::filesystem::path const& PathToProcess);
	//std::string Convert_U16_U8(void* Data, size_t DataLength);
	//std::string Convert_U16_U8(const char16_t* StringToConvert);
	std::string UnicodeStringToLower(std::string const& StringToParse);
	//int CreateCMacroCodepointArrayFromPropertySpec(std::string const& InputFilename, std::string const& OutputFilename);

	void CreateCodepointPropertiesHeader(std::string const& DataDirectory);
};

