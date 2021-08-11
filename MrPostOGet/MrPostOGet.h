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
	void HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient, std::vector<RequestHandler> RequestHandlers, std::string ResourcesPath, HTTPServer* AssociatedServer);
	
	
	MBSockets::HTTPDocumentType MimeSniffDocument(std::string const& FilePath);
	//convinience funktioner
	std::string GetRequestContent(std::string const& RequestData);
	std::string LoadWholeFile(std::string const& FilePath);
	std::string ReplaceMPGVariables(std::string const& FileData, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithVariables(std::string const& Filepath, std::unordered_map<std::string, std::string> const& VariablesMap);
	std::string LoadFileWithPreprocessing(std::string const& Filepath, std::string const& ResourcesPath);
	std::string GetFileExtension(std::string const& StringData);
	

	class HTTP
	{

	};

	class HTTPServer
	{
	private:
		std::string ContentPath = "";
		int Port = -1;
		size_t MaxDocumentInMemorySize = 100000;
		MBSockets::HTTPServerSocket* ServerSocketen;
		MBSockets::TraversalProtocol TraversalProtocolet = MBSockets::TraversalProtocol::TCP;
		std::vector<RequestHandler> ServerRequestHandlers = std::vector<RequestHandler>(0);
		//std::mutex HTTPServerResourcesMutex;
	public:
		HTTPServer(std::string PathToResources, int PortToListenTo);
		std::string GenerateResponse(MBSockets::HTTPDocument const& Document);
		std::string GetResourcePath(std::string const& DomainName);
		std::string LoadWholeFile(std::string const& FilePath);
		std::string LoadFileWithIntervalls(std::string const& FilePath, std::vector<FiledataIntervall> const& ByteRanges);
		MBSockets::HTTPDocument GetResource(std::string const& ResourcePath, std::vector<FiledataIntervall> const& Byteranges = {});
		void AddRequestHandler(RequestHandler HandlerToAdd);
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