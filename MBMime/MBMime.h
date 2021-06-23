#pragma once
#include <vector>
#include <string>
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
		};
		MIMETypeTuple NullTupple = { MIMEType::Null,MediaType::Null,{},"" };
	public:
		MIMETypeTuple GetTupleFromExtension(std::string const& Extension);
		MIMETypeTuple GetTupleFromDocumentType(MIMEType DocumentType);
	};
};