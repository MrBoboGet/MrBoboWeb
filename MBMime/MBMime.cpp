#include "MBMime.h"
#include <cstring>
#include <algorithm>

namespace MBMIME
{
	MIMETypeTuple MIMETypeConnector::GetTupleFromExtension(std::string const& Extension)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			for (size_t j = 0; j < SuppportedTupples[i].FileExtensions.size(); j++)
			{
				if (SuppportedTupples[i].FileExtensions[j] == Extension)
				{
					return(SuppportedTupples[i]);
				}
			}
		}
		return(NullTupple);
	}
	MIMETypeTuple MIMETypeConnector::GetTupleFromDocumentType(MIMEType DocumentType)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			if (SuppportedTupples[i].FileMIMEType == DocumentType)
			{
				return(SuppportedTupples[i]);
			}
		}
		return(NullTupple);
	}

	std::unordered_map<std::string, std::string> ExtractMIMEHeaders(const void* Data, size_t DataSize, size_t InOffset,size_t* OutOffset)
	{
		std::unordered_map<std::string, std::string> ReturnValue = {};
		const char* DataToParse = (const char*)Data;
		size_t ParseOffset = InOffset;
		while(ParseOffset < DataSize)
		{
			if (std::memcmp(DataToParse + ParseOffset, "\r\n", 2) == 0)
			{
				ParseOffset += 2;
				break;
			}
			//ANTAGANDE börjar alltid på ett header namn
			size_t NextColon = std::find(DataToParse + ParseOffset, DataToParse + DataSize, ':')-DataToParse;
			if (NextColon == DataSize)
			{
				//Ska egentligen vara ett error här
				break;
			}
			std::string NewHeaderName = MBUnicode::UnicodeStringToLower(std::string(DataToParse + ParseOffset, NextColon - ParseOffset));
			ParseOffset = NextColon + 1;
			size_t BodyEnd = std::find(DataToParse + ParseOffset, DataToParse + DataSize, '\r')-DataToParse;
			ReturnValue[NewHeaderName] = std::string(DataToParse + ParseOffset, BodyEnd - ParseOffset);
			ParseOffset = BodyEnd + 2;
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	std::unordered_map<std::string, std::string> ExtractMIMEHeaders(std::string const& DataToParse, size_t InOffset,size_t* OutOffset)
	{
		return(ExtractMIMEHeaders(DataToParse.data(), DataToParse.size(), InOffset, OutOffset));
	}
	//MIMEMultipartDocumentExtractor

	MIMEMultipartDocumentExtractor::MIMEMultipartDocumentExtractor(const void* Data,size_t DataSize, size_t ParseOffset)
	{
		m_DataSize = DataSize;
		m_DocumentData = Data;
		m_ParseOffset = ParseOffset;
	}
	std::unordered_map<std::string,std::string> MIMEMultipartDocumentExtractor::ExtractHeaders()
	{
		std::unordered_map<std::string, std::string> ReturnValue = ExtractMIMEHeaders(m_DocumentData,m_DataSize,m_ParseOffset,&m_ParseOffset);
		if (m_ContentBoundary == "")
		{
			std::string ContentType = ReturnValue["content-type"];
			size_t BoundaryBegin = ContentType.find("boundary=") + 9;
			m_ContentBoundary = ContentType.substr(BoundaryBegin);
			m_PartDataIsAvailable = true;
			std::string StartBoundary = "--" + m_ContentBoundary;
			if (MBParsing::FindSubstring(m_DocumentData, m_DataSize, StartBoundary.data(), StartBoundary.size(), m_ParseOffset) == m_ParseOffset)
			{
				m_ParseOffset += StartBoundary.size() + 2; //\r\n
			}
		}
		return(ReturnValue);
	}
	std::string MIMEMultipartDocumentExtractor::ExtractPartData(size_t MaxNumberOfBytes)
	{
		std::string ReturnValue = "";
		//ANTAGANDE börjar antingen på en boundary, headers extractade, eller mitt i
		std::string StartBoundary = "--" + m_ContentBoundary;
		if (MBParsing::FindSubstring(m_DocumentData, m_DataSize, StartBoundary.data(), StartBoundary.size(), m_ParseOffset) == m_ParseOffset)
		{
			m_ParseOffset += StartBoundary.size() + 2; //\r\n
		}
		std::string Boundary = "\r\n--" + m_ContentBoundary;
		size_t NextEndBoundary = MBParsing::FindSubstring(m_DocumentData, m_DataSize, Boundary.data(), Boundary.size(), m_ParseOffset);

		size_t BytesToExtract = std::min(MaxNumberOfBytes, m_DataSize - m_ParseOffset);
		if (NextEndBoundary != -1)
		{
			BytesToExtract = std::min(BytesToExtract, NextEndBoundary-m_ParseOffset);
		}
		else
		{
			throw std::exception();
		}
		ReturnValue = std::string(((char*)m_DocumentData)+m_ParseOffset, BytesToExtract);
		m_ParseOffset += BytesToExtract;
		if (MBParsing::FindSubstring(m_DocumentData, m_DataSize, Boundary.data(), Boundary.size(), m_ParseOffset) == m_ParseOffset)
		{
			m_ParseOffset += Boundary.size() + 4;//-- -- \r\n
			if (MBParsing::FindSubstring(m_DocumentData, m_DataSize, "--\r\n", 4, m_ParseOffset) == m_ParseOffset)
			{
				m_ParseOffset += 4;
				m_Finished = true;
			}
			m_PartDataIsAvailable = false;
		}
		if (m_ParseOffset>= m_DataSize)
		{
			m_Finished = true;
		}
		return(ReturnValue);
	}
	bool MIMEMultipartDocumentExtractor::PartDataIsAvailable()
	{
		return(m_PartDataIsAvailable);
	}
	bool MIMEMultipartDocumentExtractor::Finished()
	{
		return(m_Finished);
	}
}