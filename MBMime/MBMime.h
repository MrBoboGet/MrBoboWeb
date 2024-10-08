#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <MBParsing/MBParsing.h>
#include <MBUnicode/MBUnicode.h>
namespace MBMIME
{
	enum class MIMEType
	{
		OctetString,
		HTML,
		png,
		jpg,
		json,
		ts,
		m3u8,
		mkv,
		javascript,
		css,
		mp4,
		PDF,
		GIF,
		mp3,
		Text,
		Wav,
		WebP,
		WebM,
		Opus,
        XML,
		Null
	};
	enum class MediaType
	{
		Video,
		Audio,
		Image,
		Text,
		PDF,
		Null
	};
	struct MIMETypeTuple
	{
		MIMEType FileMIMEType = MIMEType::Null;
		MediaType FileMediaType = MediaType::Null;
		std::vector<std::string> FileExtensions = {};
		std::string MIMEMediaString = "";
	};
	class MIMETypeConnector
	{
	private:
		std::vector<MIMETypeTuple> SuppportedTupples =
		{
			{MIMEType::OctetString,MediaType::Null,{},"application/octet-stream"},
			{MIMEType::png,	MediaType::Image,{"png"},"image/png"},
			{MIMEType::jpg,MediaType::Image,{"jpg","jpeg"},"image/jpeg"},
			{MIMEType::json,MediaType::Text,{"json"},"application/json"},
			{MIMEType::ts,MediaType::Video,{"ts"},"video/MP2T"},
			{MIMEType::m3u8,MediaType::Text,{"m3u8"},"application/x-mpegURL"},
			{MIMEType::mkv,MediaType::Video,{"mkv"},"video/x-matroska"},
			{MIMEType::javascript,MediaType::Text,{"js"},"text/javascript"},
			{MIMEType::css,MediaType::Text,{"css"},"text/css"},
			{MIMEType::mp4,MediaType::Video,{"mp4"},"video/mp4"},
			{MIMEType::HTML,MediaType::Text,{"html","htm"},"text/html"},
			{MIMEType::PDF,MediaType::PDF,{"pdf"},"application/pdf"},
			{MIMEType::GIF,MediaType::Image,{"gif"},"image/gif"},
			{MIMEType::mp3,MediaType::Audio,{"mp3"},"audio/mpeg"},
			{MIMEType::Text,MediaType::Text,{"txt"},"text/plain"},
			{MIMEType::Wav,MediaType::Audio,{"wav"},"audio/wav"},
			{MIMEType::WebP,MediaType::Image,{"webp"},"image/webp"},
			{MIMEType::WebM,MediaType::Video,{"webm"},"video/webm"},
			{MIMEType::Opus,MediaType::Audio,{"opus"},"audio/opus"},
            //TODO mega hack for UPNP
			{MIMEType::XML,MediaType::Text,{"xml"},"text/xml; charset=\"utf-8\""},
		};
		MIMETypeTuple NullTupple = { MIMEType::Null,MediaType::Null,{},"" };
	public:
		MIMETypeTuple GetTupleFromExtension(std::string const& Extension);
		MIMETypeTuple GetTupleFromDocumentType(MIMEType DocumentType);
	};
	std::unordered_map<std::string, std::vector<std::string>> ExtractMIMEHeaders(const void* Data,size_t DataSize, size_t ParseOffset,size_t* OutOffset = nullptr);
	std::unordered_map<std::string, std::vector<std::string>> ExtractMIMEHeaders(std::string const& DataToParse,size_t InOffset, size_t* ParseOffset);

	class MIMEMultipartDocumentExtractor
	{
	private:
		const void* m_DocumentData = nullptr;
		size_t m_DataSize = 0;
		size_t m_ParseOffset = 0;

		std::string m_LastBodyDataEnd = "";
		std::string m_ContentBoundary = "";
		bool m_Finished = false;
		bool m_PartDataIsAvailable = false;
	public:
		MIMEMultipartDocumentExtractor(const void* Data,size_t DataSize, size_t ParseOffset);
		std::unordered_map<std::string, std::vector<std::string>> ExtractHeaders();
		std::string ExtractPartData(size_t MaxNumberOfBytes = -1);
		bool PartDataIsAvailable();
		bool Finished();
	};

	std::string GetMIMEStringFromType(MIMEType TypeToConvert);
	MBMIME::MIMEType DocumentTypeFromFileExtension(std::string const& FileExtension);
	MBMIME::MediaType GetMediaTypeFromExtension(std::string const& FileExtension);
};
