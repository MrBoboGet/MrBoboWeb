#pragma once
#include <MrBoboSockets/MrBoboSockets.h>
#include <MBMime/MBMime.h>
#include <TextReader.h>
#include <cstring>

namespace MrPostOGet 
{
	class HTTPServer;

	class FileIntervallExtracter
	{
	private:
		std::ifstream FileToRead;
		std::vector<FiledataIntervall> IntervallsToRead = {};
		uint64_t FileSize = 0;
		uint64_t IntervallIndex = 0;
		uint64_t MaxDataInMemory = 10000000;
		uint64_t TotalDataRead = 0;
	public:
		FileIntervallExtracter(std::string const& FilePath, std::vector<FiledataIntervall> const& Intervalls, size_t MaxDataInMemory);
		std::string GetNextIntervall();
		bool IsDone();
	};

	enum class HTTPRequestStatus
	{
		OK = 200,
		PartialContent = 206,
		Authenticate = 401,
		NotFound = 404,
		Conflict = 409,
	};
	struct HTTPDocument
	{
		MBMIME::MIMEType Type = MBMIME::MIMEType::Null;
		HTTPRequestStatus RequestStatus = HTTPRequestStatus::OK;
		std::unordered_map<std::string, std::vector<std::string>> ExtraHeaders = {};
		std::vector<FiledataIntervall> IntervallsToRead = {};
		std::string DocumentData;
		std::string DocumentDataFileReference = "";
	};


	std::string GetHeaderValue(std::string Header, const std::string& HeaderContent);
	std::vector<std::string> GetHeaderValues(std::string const& HeaderTag, std::string const& HeaderContent);
	std::string GetRequestType(const std::string& RequestData);
	std::string GetRequestResource(const std::string& RequestData);


	//MBMIME::MIMEType DocumentTypeFromFileExtension(std::string const& FileExtension);
	//MBMIME::MediaType GetMediaTypeFromExtension(std::string const& FileExtension);
	
	///std::string GetMIMEFromDocumentType(MBMIME::MIMEType TypeToConvert);

	//std::string GenerateRequest(HTTPDocument const& DocumentToSend);
	//std::string GenerateRequest(const std::string& HTMLBody);

	class HTTPServerSocket : public MBSockets::ServerSocket
	{
	private:
		bool ChunksRemaining = false;
		bool RequestIsChunked = false;
		int CurrentChunkSize = 0;
		int CurrentChunkParsed = 0;
		int CurrentContentLength = 0;
		int ParsedContentData = 0;
		int p_GetNextChunkSize(int ChunkHeaderPosition, std::string const& Data, int& OutChunkDataBeginning);
		std::string p_UpdateAndDeChunkData(std::string const& ChunkedData);

		std::string p_GenerateRequest(HTTPDocument const& DocumentToSend);
		std::string p_GenerateRequest(const std::string& HTMLBody);
		std::string p_HTTPRequestStatusToString(HTTPRequestStatus StatusToConvert);
	public:
		bool DataIsAvailable();
		HTTPServerSocket(std::string const& Port);
		std::string GetHTTPRequest();
		void SendDataAsHTTPBody(const std::string& Data);
		void SendHTTPBody(const std::string& Data);
		void SendHTTPDocument(HTTPDocument const& DocumentToSend);
		std::string GetNextChunkData();
	};


	struct RequestHandler
	{
		bool (*RequestPredicate)(const std::string& RequestData);
		HTTPDocument(*RequestResponse)(const std::string& ResoureData, HTTPServer* AssociatedServer,HTTPServerSocket* AssociatedSocket);
	};	
	
	MBMIME::MIMEType MimeSniffDocument(std::string const& FilePath);
	//convinience funktioner
	std::string GetFileExtension(std::string const& StringData);
	std::string GetRequestContent(std::string const& RequestData);
	std::string LoadWholeFile(std::string const& FilePath);


	struct Cookie
	{
		std::string Name = "";
		std::string Value = "";
	};
	std::string ReplaceMPGVariables(std::string const& FileData, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithVariables(std::string const& Filepath, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithPreprocessing(std::string const& Filepath, std::string const& ResourcesPath);


	std::vector<Cookie> GetCookiesFromRequest(std::string const& RequestData);

	enum class HTTPRequestType
	{
		POST,
		GET,
		PUT,
		HEAD,
		Null
	};
	std::string HTTPRequestTypeToString(HTTPRequestType RequestToTranslate);
	struct HTTPClientRequest
	{
		HTTPRequestType Type = HTTPRequestType::Null;
		std::string RequestResource = "";
		std::unordered_map<std::string, std::string> SearchParameters = {};
		std::unordered_map<std::string, std::vector<std::string>> Headers = {};
		std::string BodyData = "";

		std::string RawRequestData = "";
	};

	struct HTTPClientConnectionState
	{
	private:
	public:
		//void* UnspecifiedData = nullptr;
		//void (*UnspecifiedDataDeallocator)(void* DataToDeallocate);
	};

	class HTTPRequestHandler
	{
	private:

	public:
		virtual bool HandlesRequest(HTTPClientRequest const& RequestToHandle, HTTPClientConnectionState const& ConnectionState, HTTPServer* AssociatedServer) { return false; };
		virtual HTTPDocument GenerateResponse(HTTPClientRequest const&, HTTPClientConnectionState const&, HTTPServerSocket*, HTTPServer*) 
		{
			return(HTTPDocument()); 
		};
	};
	class StaticRequestHandler : public HTTPRequestHandler
	{
	private:
		RequestHandler m_InternalHandler = { nullptr,nullptr };
	public:
		StaticRequestHandler(RequestHandler HandlerToConvert);
		bool HandlesRequest(HTTPClientRequest const& RequestToHandle, HTTPClientConnectionState const& ConnectionState, HTTPServer* AssociatedServer) override;
		HTTPDocument GenerateResponse(HTTPClientRequest const&, HTTPClientConnectionState const&, HTTPServerSocket*, HTTPServer*) override;
	};

	class HTTPServer
	{
	private:
		uint16_t Port = -1;
		size_t MaxDocumentInMemorySize = 100000;
		HTTPServerSocket* ServerSocketen;
		
		std::string m_DefaultResourcePath = "";
		std::unordered_map<std::string, std::string> m_DomainResourcePaths = {};

		std::atomic<bool> m_UseSecureConnection{ true };

		std::mutex m_InternalsMutex;
		std::vector<std::unique_ptr<HTTPRequestHandler>> m_RequestHandlers = std::vector<std::unique_ptr<HTTPRequestHandler>>(0);

		void m_HandleConnectedSocket(HTTPServerSocket* ConnectedClient);
		static HTTPDocument m_DefaultHandler(HTTPClientRequest const& Request, std::string const& ResourcePath,HTTPServer* AssociatedServer);
		static void p_ParseHTTPClientRequest(HTTPClientRequest& ClientRequest, std::string& RawData);
		static std::unordered_map<std::string, std::string> p_ParseSearchParameters(std::string const& URL);
		//std::mutex HTTPServerResourcesMutex;
	public:
		HTTPServer(std::string PathToResources, int PortToListenTo);
		//std::string GenerateResponse(HTTPDocument const& Document);
		void LoadDomainResourcePaths(std::string const& ConfigFile);
		std::string GetResourcePath(std::string const& DomainName);
		std::string LoadWholeFile(std::string const& FilePath);
		void UseTLS(bool ShouldUse);
		//std::string LoadFileWithIntervalls(std::string const& FilePath, std::vector<FiledataIntervall> const& ByteRanges);
		HTTPDocument GetResource(std::string const& ResourcePath, std::vector<FiledataIntervall> const& Byteranges = {});
		void AddRequestHandler(RequestHandler HandlerToAdd);
		void AddRequestHandler(HTTPRequestHandler* HandlerToAdd);
		void StartListening();
		~HTTPServer();
	};


	class HTMLNode
	{
	private:
		friend void swap(HTMLNode& LeftNode, HTMLNode& RightNode);
		std::vector<HTMLNode> m_Children = {};
		std::string m_RawText = "";
		std::unordered_map<std::string, std::string> m_Attributes = {};
		std::string m_NodeTag = "";
		HTMLNode* m_Parent = nullptr;
		bool m_IsRawtext = false;

		//size_t m_ElementEndOffset = 0;
		//MBError m_ParseError = MBError(true);

		static std::string p_ExtractTagName(const void* DataToParse,size_t DataSize, size_t ParseOffset,size_t* OutOffset = nullptr,MBError* OutError = nullptr);
		static std::pair<std::string,std::string> p_ExtractTag(const void* DataToParse, size_t DataSize, size_t ParseOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		void p_UpdateChildParents();
	public:

		static bool TagIsEmpty(std::string const& StringToCheck);
		static HTMLNode ParseNode(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static HTMLNode ParseNode(std::string const& DataToParse, size_t InOffset, size_t* OutOffset = nullptr, MBError* OutError = nullptr);
		static HTMLNode CreateElement(std::string const& ElementTag);
		HTMLNode(std::string const& RawText);

		HTMLNode(HTMLNode const& NodeToCopy);
		HTMLNode(HTMLNode&& NodeToSteal) noexcept;
		void AppendChild(HTMLNode NewChild);
		//void AppendChild(HTMLNode&& ChildToCopy);

		std::string GetVisableText() const;
		HTMLNode(std::string const& HTMLToParse, size_t ParseOffset);
		HTMLNode() {};

		std::string& operator[](std::string const& AttributeName);
		std::string const& operator[](std::string const& AttributeName) const;

		std::string ToString() const;
	};
}