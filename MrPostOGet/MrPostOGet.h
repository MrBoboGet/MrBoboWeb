#pragma once
#include <MrBoboSockets/MrBoboSockets.h>
#include <TextReader.h>
#include <cstring>

namespace MrPostOGet 
{
	class HTTPServer;
	struct RequestHandler
	{
		bool (*RequestPredicate)(const std::string& RequestData);
		MBSockets::HTTPDocument(*RequestResponse)(const std::string& ResoureData, HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedSocket);
	};	
	
	MBSockets::HTTPDocumentType MimeSniffDocument(std::string const& FilePath);
	//convinience funktioner
	std::string GetRequestContent(std::string const& RequestData);
	std::string LoadWholeFile(std::string const& FilePath);
	std::string ReplaceMPGVariables(std::string const& FileData, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithVariables(std::string const& Filepath, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithPreprocessing(std::string const& Filepath, std::string const& ResourcesPath);
	std::string GetFileExtension(std::string const& StringData);
	
	enum class HTTPRequestType
	{
		POST,
		GET,
		PUT,
		Null
	};
	struct HTTPClientRequest
	{
		HTTPRequestType Type = HTTPRequestType::Null;
		std::string RequestResource = "";
		std::unordered_map<std::string, std::string> SearchParameters = {};
		std::unordered_map<std::string, std::string> Headers = {};
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
		virtual MBSockets::HTTPDocument GenerateResponse(HTTPClientRequest const&, HTTPClientConnectionState const&, MBSockets::HTTPServerSocket*, HTTPServer*) 
		{
			return(MBSockets::HTTPDocument()); 
		};
	};
	class StaticRequestHandler : public HTTPRequestHandler
	{
	private:
		RequestHandler m_InternalHandler = { nullptr,nullptr };
	public:
		StaticRequestHandler(RequestHandler HandlerToConvert);
		bool HandlesRequest(HTTPClientRequest const& RequestToHandle, HTTPClientConnectionState const& ConnectionState, HTTPServer* AssociatedServer) override;
		MBSockets::HTTPDocument GenerateResponse(HTTPClientRequest const&, HTTPClientConnectionState const&, MBSockets::HTTPServerSocket*, HTTPServer*) override;
	};
	struct Cookie
	{
		std::string Name = "";
		std::string Value = "";
	};
	std::vector<Cookie> GetCookiesFromRequest(std::string const& RequestData);
	//std::vector<Cookie> GetCookiesFromRequest(HTTPClientRequest const& RequestData);
	class HTTPServer
	{
	private:
		uint16_t Port = -1;
		size_t MaxDocumentInMemorySize = 100000;
		MBSockets::HTTPServerSocket* ServerSocketen;
		
		std::string m_DefaultResourcePath = "";
		std::unordered_map<std::string, std::string> m_DomainResourcePaths = {};

		std::atomic<bool> m_UseSecureConnection{ true };

		std::mutex m_InternalsMutex;
		std::vector<std::unique_ptr<HTTPRequestHandler>> m_RequestHandlers = std::vector<std::unique_ptr<HTTPRequestHandler>>(0);

		void m_HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient);
		static MBSockets::HTTPDocument m_DefaultHandler(HTTPClientRequest const& Request, std::string const& ResourcePath,HTTPServer* AssociatedServer);
		static void p_ParseHTTPClientRequest(HTTPClientRequest& ClientRequest, std::string& RawData);
		static std::unordered_map<std::string, std::string> p_ParseSearchParameters(std::string const& URL);
		//std::mutex HTTPServerResourcesMutex;
	public:
		HTTPServer(std::string PathToResources, int PortToListenTo);
		std::string GenerateResponse(MBSockets::HTTPDocument const& Document);
		void LoadDomainResourcePaths(std::string const& ConfigFile);
		std::string GetResourcePath(std::string const& DomainName);
		std::string LoadWholeFile(std::string const& FilePath);
		void UseTLS(bool ShouldUse);
		//std::string LoadFileWithIntervalls(std::string const& FilePath, std::vector<FiledataIntervall> const& ByteRanges);
		MBSockets::HTTPDocument GetResource(std::string const& ResourcePath, std::vector<FiledataIntervall> const& Byteranges = {});
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
//std::string ToString();