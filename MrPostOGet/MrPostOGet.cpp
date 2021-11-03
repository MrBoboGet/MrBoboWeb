﻿#include "MrPostOGet.h"
#include <MBUtility/MBStrings.h>
#include <MBUnicode/MBUnicode.h>
#include <MBParsing/MBParsing.h>
#include <MBMime/MBMime.h>
namespace MrPostOGet
{

	//BEGIN Utility Functions
	std::string GetRequestType(const std::string& RequestData)
	{
		int FirstSpace = RequestData.find(" ");
		return(RequestData.substr(0, FirstSpace));
	}
	std::string GetRequestResource(const std::string& RequestData)
	{
		size_t FirstSlashPos = RequestData.find("/");
		size_t FirstSpaceAfterSlash = RequestData.find(" ", FirstSlashPos);
		std::string ReturnValue = RequestData.substr(FirstSlashPos + 1, FirstSpaceAfterSlash - FirstSlashPos - 1);
		bool Error = true; // borde göra något med denna med aja
		ReturnValue = MBUtility::URLDecodeData(ReturnValue, &Error);
		return(ReturnValue);
	}
	std::string GetHeaderValue(std::string Header, const std::string& HeaderContent)
	{
		std::string HeaderData = HeaderContent.substr(0, HeaderContent.find("\r\n\r\n") + 4);
		int HeaderPosition = HeaderData.find(Header + ": ");
		int FirstEndlineAfterContentPos = HeaderData.find("\r\n", HeaderPosition);
		if (HeaderPosition == HeaderContent.npos)
		{
			return("");
		}
		else
		{
			return(HeaderData.substr(HeaderPosition + Header.size() + 2, FirstEndlineAfterContentPos - (HeaderPosition + Header.size() + 2)));
		}
	}
	std::vector<std::string> GetHeaderValues(std::string const& HeaderTag, std::string const& HeaderContent)
	{
		std::string HeaderData = HeaderContent.substr(0, HeaderContent.find("\r\n\r\n") + 4);
		std::vector<std::string> ReturnValue = {};
		std::string StringToSearchFor = HeaderTag + ": ";
		size_t StringPosition = HeaderData.find(StringToSearchFor);
		int FirstEndlineAfterContentPos = HeaderData.find("\n", StringPosition);
		while (StringPosition != HeaderData.npos)
		{
			ReturnValue.push_back(HeaderData.substr(StringPosition + StringToSearchFor.size(), FirstEndlineAfterContentPos - (StringPosition + StringToSearchFor.size())));
			StringPosition = HeaderData.find(StringToSearchFor, StringPosition + StringToSearchFor.size());
			FirstEndlineAfterContentPos = HeaderData.find("\n", StringPosition);
		}
		return(ReturnValue);
	}

	std::vector<Cookie> GetCookiesFromRequest(HTTPClientRequest const& RequestData)
	{
		std::vector<Cookie> ReturnValue = {};
		std::vector<std::string> Cookies = MBUtility::Split(RequestData.Headers.at("cookie").front(),"; ");
		for (size_t i = 0; i < Cookies.size(); i++)
		{
			size_t FirstEqualSignPos = Cookies[i].find_first_of("=");
			std::string CookieName = Cookies[i].substr(0, FirstEqualSignPos);
			std::string CookieValue = Cookies[i].substr(FirstEqualSignPos + 1);
			ReturnValue.push_back({ CookieName, CookieValue });
		}
		return(ReturnValue);
	}
	std::vector<Cookie> GetCookiesFromRequest(std::string const& RequestData)
	{
		std::vector<Cookie> ReturnValue = {};
		std::vector<std::string> Cookies = MBUtility::Split(GetHeaderValue("Cookie", RequestData), "; ");
		for (size_t i = 0; i < Cookies.size(); i++)
		{
			size_t FirstEqualSignPos = Cookies[i].find_first_of("=");
			std::string CookieName = Cookies[i].substr(0, FirstEqualSignPos);
			std::string CookieValue = Cookies[i].substr(FirstEqualSignPos + 1);
			ReturnValue.push_back({ CookieName, CookieValue });
		}
		return(ReturnValue);
	}
	MBMIME::MIMEType MimeSniffDocument(std::string const& FilePath)
	{
		std::fstream DocumentToSniff = std::fstream(FilePath);
		std::string DocumentHeader(100, 0);
		size_t BytesRead = DocumentToSniff.read(&DocumentHeader[0], 100).gcount();
		if (DocumentHeader.substr(0, 6) == "<html>")
		{
			return(MBMIME::MIMEType::HTML);
		}
		//15
		if (BytesRead >= 14)
		{
			std::string LowerBeginning = "";
			for (size_t i = 0; i < 14; i++)
			{
				LowerBeginning += std::tolower(DocumentHeader[i]);
			}
			if (LowerBeginning == "<!doctype html")
			{
				return(MBMIME::MIMEType::HTML);
			}
		}
		return(MBMIME::MIMEType::Null);
	}
	std::string GetRequestContent(std::string const& RequestData)
	{
		size_t ContentStart = RequestData.find("\r\n\r\n") + 4;
		return(RequestData.substr(ContentStart));
	}
	std::string LoadWholeFile(std::string const& FilePath)
	{
		std::ifstream t(FilePath, std::ifstream::in | std::ifstream::binary);
		//size_t size = std::filesystem::file_size(FilePath);
		uint64_t size = MBGetFileSize(FilePath);
		std::string FileDataBuffer(size, ' ');
		t.read(&FileDataBuffer[0], size);
		uint64_t ReadCharacters = t.gcount();
		std::string FileData(FileDataBuffer.c_str(), ReadCharacters);
		return(FileData);
	}
	std::string ReplaceMPGVariables(std::string const& FileData, std::unordered_map<std::string, std::string> const& VariablesMap)
	{
		std::string ReturnValue = "";
		size_t LastParsePosition = 0;
		size_t ParsePosition = 0;
		std::string StartSequence = "${";
		std::string EndSequence = "}";
		while (ParsePosition != FileData.npos)
		{
			LastParsePosition = ParsePosition;
			ParsePosition = FileData.find(StartSequence, ParsePosition);
			size_t NextEndSequence = FileData.find(EndSequence, ParsePosition);
			//lägger till all data till vi komer till början
			ReturnValue += FileData.substr(LastParsePosition, ParsePosition - LastParsePosition);
			if (ParsePosition == FileData.npos)
			{
				break;
			}
			//nu kollar vi huruvida variabeln den beskriver finns i vår dictionary. Gör den inte det så tar vi och bara lägger in den råa strängen
			if (NextEndSequence == FileData.npos)
			{
				ReturnValue += FileData.substr(ParsePosition);
				break;
			}
			std::string VariableName = FileData.substr(ParsePosition + StartSequence.size(), NextEndSequence - (ParsePosition + StartSequence.size()));
			if (VariablesMap.find(VariableName) != VariablesMap.end())
			{
				ReturnValue += VariablesMap.at(VariableName);
				ParsePosition = NextEndSequence + 1;
			}
			else
			{
				ReturnValue += FileData.substr(ParsePosition, NextEndSequence - ParsePosition);
				ParsePosition = NextEndSequence + 1;
			}
		}
		return(ReturnValue);
	}
	std::string LoadFileWithVariables(std::string const& Filepath, std::unordered_map<std::string, std::string> const& VariablesMap)
	{
		if (!std::filesystem::exists(Filepath))
		{
			return("");
		}
		std::string FileData = LoadWholeFile(Filepath);
		return(ReplaceMPGVariables(FileData, VariablesMap));
	}
	std::string LoadFileWithPreprocessing(std::string const& Filepath, std::string const& ResourcesPath)
	{
		if (!std::filesystem::exists(Filepath))
		{
			return("");
		}
		std::string FileData = LoadWholeFile(Filepath);
		std::string ReturnValue = "";
		size_t LastParsePosition = 0;
		size_t ParsePosition = 0;
		std::string StartSequence = "#{";
		std::string EndSequence = "}";
		while (ParsePosition != FileData.npos)
		{
			LastParsePosition = ParsePosition;
			ParsePosition = FileData.find(StartSequence, ParsePosition);
			size_t NextEndSequence = FileData.find(EndSequence, ParsePosition);
			//lägger till all data till vi komer till början
			ReturnValue += FileData.substr(LastParsePosition, ParsePosition - LastParsePosition);
			if (ParsePosition == FileData.npos)
			{
				break;
			}
			//nu kollar vi huruvida variabeln den beskriver finns i vår dictionary. Gör den inte det så tar vi och bara lägger in den råa strängen
			if (NextEndSequence == FileData.npos)
			{
				ReturnValue += FileData.substr(ParsePosition);
				ParsePosition = NextEndSequence + 1;
				break;
			}
			size_t FirstParenthesesLocation = FileData.find("(", ParsePosition);
			size_t SecondParenthesesLocation = FileData.find(")", ParsePosition);
			std::string PreprocessingDirective = FileData.substr(ParsePosition + StartSequence.size(), FirstParenthesesLocation - (ParsePosition + StartSequence.size()));
			std::string DirectiveArguments = FileData.substr(FirstParenthesesLocation + 1, SecondParenthesesLocation - (FirstParenthesesLocation + 1));
			if (PreprocessingDirective == "include")
			{
				//vi copypastar filen med pre proccessing directives
				//om filen inte finns continuear vi helt enkelt
				if (!std::filesystem::exists(ResourcesPath + DirectiveArguments))
				{
					//filen finns inte, då tar vi bara och lägger till den parsade datan och contiunar
					ReturnValue += FileData.substr(ParsePosition, NextEndSequence - ParsePosition + 1);
					ParsePosition = NextEndSequence + 1;
					continue;
				}
				ReturnValue += LoadFileWithPreprocessing(ResourcesPath + DirectiveArguments, ResourcesPath);
			}
			ParsePosition = NextEndSequence + 1;
		}
		return(ReturnValue);
	}
	std::string GetFileExtension(std::string const& StringData)
	{
		std::string ReturnValue = "";
		size_t DotPosition = StringData.find_last_of(".");
		if (DotPosition != StringData.npos)
		{
			ReturnValue = StringData.substr(DotPosition + 1);
		}
		return(ReturnValue);
	}
	std::string HTTPRequestTypeToString(HTTPRequestType RequestToTranslate)
	{
		std::string ReturnValue = "";
		if (RequestToTranslate == HTTPRequestType::GET)
		{
			ReturnValue = "GET";
		}
		else if(RequestToTranslate == HTTPRequestType::POST)
		{
			ReturnValue = "POST";
		}
		else if (RequestToTranslate == HTTPRequestType::PUT)
		{
			ReturnValue = "PUT";
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}
	//END Utility Functions

	//BEGIN HTTPServer_ConnectionHandler
	HTTPServer_ConnectionHandler::HTTPServer_ConnectionHandler()
	{
		m_WorkerThread = std::move(std::thread(&HTTPServer_ConnectionHandler::p_ActiveConnectionWorker, this));
	}
	MPGConnectionHandle HTTPServer_ConnectionHandler::AddConnection(std::unique_ptr<std::thread> NewConnection)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		MPGConnectionHandle ReturnValue = m_CurrentHandleIndex;
		m_CurrentHandleIndex += 1;
		m_ActiveConnections[ReturnValue] = std::move(NewConnection);
		return(ReturnValue);
	}
	void HTTPServer_ConnectionHandler::RemoveConnection(MPGConnectionHandle HandleToRemove)
	{
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			m_HandlesToRemove.push(HandleToRemove);
		}
		m_WorkerConditional.notify_one();
		//if (m_ActiveConnections.find(HandleToRemove) != m_ActiveConnections.end())
		//{
		//	m_ActiveConnections.erase(HandleToRemove);
		//}

	}
	void HTTPServer_ConnectionHandler::p_ActiveConnectionWorker()
	{
		while (m_ShouldStop.load() == false)
		{
			std::unique_lock<std::mutex> Lock(m_InternalsMutex);
			while(m_HandlesToRemove.size() == 0)
			{
				m_WorkerConditional.wait(Lock);
			}
			while (m_HandlesToRemove.size() > 0)
			{
				MPGConnectionHandle HandleToRemove = m_HandlesToRemove.top();
				m_HandlesToRemove.pop();
				if (m_ActiveConnections.find(HandleToRemove) != m_ActiveConnections.end())
				{
					m_ActiveConnections[HandleToRemove]->join();
					m_ActiveConnections.erase(HandleToRemove);
				}
			}
			std::cout << "Number of active connections: " << m_ActiveConnections.size() << std::endl;
		}
	}
	HTTPServer_ConnectionHandler::~HTTPServer_ConnectionHandler()
	{
		m_ShouldStop = true;
		m_WorkerConditional.notify_one();
		m_WorkerThread.join();
	}

	//BEGIN HTTPServer
	bool HTTPServer::p_PathIsValid(std::string const& PathToCheck)
	{
		bool ReturnValue = true;
		if (PathToCheck.front() != '/')
		{
			ReturnValue = false;
		}
		if (PathToCheck.find("..") != PathToCheck.npos)
		{
			ReturnValue = false;
		}
		return(ReturnValue);
	}
	std::unordered_map<std::string, std::string> HTTPServer::p_ParseSearchParameters(std::string const& URL)
	{
		std::unordered_map<std::string, std::string> ReturnValue = {};
		size_t ParametersBegin = URL.find('?');
		if (ParametersBegin != URL.npos)
		{
			size_t ParseOffset = ParametersBegin + 1;
			while (ParseOffset < URL.size())
			{
				size_t NextEqualSign = URL.find('=', ParseOffset);
				std::string AttributeName = URL.substr(ParseOffset, NextEqualSign - ParseOffset);
				size_t AttributeBegin = NextEqualSign + 1;
				size_t AttributeEnd = std::min(URL.find('&', AttributeBegin), URL.size());
				ReturnValue[AttributeName] = URL.substr(AttributeBegin, AttributeEnd - AttributeBegin);

				ParseOffset = AttributeEnd;
				if (ParseOffset < URL.size())
				{
					ParseOffset += 1;
				}
			}
		}
		return(ReturnValue);
	}
	void HTTPServer::p_ParseHTTPClientRequest(HTTPClientRequest& ClientRequest, std::string& RawData)
	{
		size_t FirstSlash = RawData.find('/');
		size_t SpaceAfter = RawData.find(' ', FirstSlash);
		std::string RawURL = RawData.substr(FirstSlash, SpaceAfter - FirstSlash);
		bool Error = false;
		ClientRequest.RequestResource = MBUtility::URLDecodeData(RawURL,&Error);
		ClientRequest.SearchParameters = p_ParseSearchParameters(RawURL);
		size_t HeadersBegin = RawData.find("\r\n") + 2;
		ClientRequest.Headers = MBMIME::ExtractMIMEHeaders(RawData.data(), HeadersBegin, nullptr);

		std::string RequestType = GetRequestType(RawData);
		if (RequestType == "GET")
		{
			ClientRequest.Type = HTTPRequestType::GET;
		}
		else if(RequestType == "POST")
		{
			ClientRequest.Type = HTTPRequestType::POST;
		}
		else if (RequestType == "PUT")
		{
			ClientRequest.Type = HTTPRequestType::PUT;
		}
		else if (RequestType == "HEAD")
		{
			ClientRequest.Type = HTTPRequestType::HEAD;
		}
		else
		{
			assert(false);
		}
		//ANTAGANDE körs alltid på något som har dess headers skickade
		size_t BodyStart = RawData.find("\r\n\r\n") + 4;
		ClientRequest.BodyData = RawData.substr(BodyStart);


		ClientRequest.RawRequestData = std::move(RawData);
	}
	void HTTPServer::m_HandleConnectedSocket(std::unique_ptr<HTTPServerSocket> ConnectedClient,MPGConnectionHandle AssociatedHandle)
	{
		try
		{
			if (m_UseSecureConnection.load())
			{
				MBError ConnectError = ConnectedClient->EstablishTLSConnection();
				if (!ConnectError)
				{
					std::cout << ConnectError.ErrorMessage << std::endl;
				}
			}
			std::string RequestData;
			HTTPClientConnectionState ConnectionState;
			while ((RequestData = ConnectedClient->GetNextChunkData()) != "")
			{
				if (!ConnectedClient->IsConnected())
				{
					continue;
				}
				if (!ConnectedClient->IsValid())
				{
					break;
				}
				HTTPClientRequest CurrentRequest;
				p_ParseHTTPClientRequest(CurrentRequest, RequestData);
				if (!p_PathIsValid(CurrentRequest.RequestResource))
				{
					HTTPDocument DocumentToSend;
					DocumentToSend.RequestStatus = HTTPRequestStatus::NotFound;
					DocumentToSend.Type = MBMIME::MIMEType::Text;
					DocumentToSend.DocumentData = "Invalid request path";
					ConnectedClient->SendHTTPDocument(DocumentToSend);
					continue;
				}
				//ANTAGANDE när man väl lyssnar läggs inga handlers till, vilket gör att nedstående trick fungerar
				bool HandlerHasHandled = false;
				size_t NumberOfHandlers = 0;
				std::unique_ptr<HTTPRequestHandler>* RequestHandlers = nullptr;
				{
					std::lock_guard<std::mutex> Lock(m_InternalsMutex);
					NumberOfHandlers = m_RequestHandlers.size();
					RequestHandlers = m_RequestHandlers.data();
				}
				for (size_t i = 0; i < NumberOfHandlers; i++)
				{
					if (RequestHandlers[i]->HandlesRequest(CurrentRequest, ConnectionState, this))
					{
						//vi ska g�ra grejer med denna data, s� vi tar och skapar stringen som vi sen ska skicka
						MrPostOGet::HTTPDocument RequestResponse = RequestHandlers[i]->GenerateResponse(CurrentRequest, ConnectionState, ConnectedClient.get(), this);
						if (RequestResponse.DataSent == false)
						{
							ConnectedClient->SendHTTPDocument(RequestResponse);
						}
						HandlerHasHandled = true;
						break;
					}
				}
				if (HandlerHasHandled)
				{
					continue;
				}
				MrPostOGet::HTTPDocument DocumentToSend = m_DefaultHandler(CurrentRequest, this->GetResourcePath("mrboboget.se"), this);
				if (DocumentToSend.DataSent == false)
				{
					ConnectedClient->SendHTTPDocument(DocumentToSend);
				}
			}
		}
		catch (const std::exception& CaughtException)
		{
			std::cout << "Uncaught exception when handling connection: " << CaughtException.what() << std::endl;
		}
		m_ActiveConnectionsHandler.RemoveConnection(AssociatedHandle);
		//delete ConnectedClient;
	}
	MrPostOGet::HTTPDocument HTTPServer::m_DefaultHandler(HTTPClientRequest const& Request, std::string const& ResourcePath, HTTPServer* AssociatedServer)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		if (Request.Type != HTTPRequestType::GET)
		{
			ReturnValue.Type = MBMIME::MIMEType::HTML;
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
			ReturnValue.DocumentData = "<html><body>Bad Request</body><html>";
		}
		else
		{
			//std::cout << RequestData << std::endl;
			std::string ResourceToGet = ResourcePath + Request.RequestResource;
			std::string ResourceExtension = GetFileExtension(ResourceToGet);
			std::filesystem::path ActualResourcePath = std::filesystem::current_path().concat("/" + ResourceToGet);
			//std::cout << ResourceToGet << std::endl;
			if (std::filesystem::exists(ActualResourcePath))
			{
				if (Request.RequestResource == "/")
				{
					ReturnValue.Type = MBMIME::MIMEType::HTML;
					ReturnValue.DocumentData = LoadFileWithPreprocessing(ResourcePath + "index.htm", ResourcePath);
				}
				else
				{
					//väldigt ful undantagsfall för att få acme protokollet att fungera
					std::string ChallengeFolder = "./ServerResources/mrboboget.se/HTMLResources/.well-known/acme-challenge/";
					if (ResourceToGet.substr(0, ChallengeFolder.size()) == ChallengeFolder)
					{
						ReturnValue.Type = MBMIME::MIMEType::OctetString;
						std::string DocumentData = "";
						TextReader Data(ResourceToGet);
						for (int i = 0; i < Data.Size(); i++)
						{
							DocumentData += Data[i];
						}
						ReturnValue.DocumentData = DocumentData;
					}
					else
					{
						//undantagsfall utifall det är ett hmtl dokument, då vill vi ta och skicka den med includes
						if (ResourceExtension == "html" || ResourceExtension == "htm")
						{
							ReturnValue.Type = MBMIME::MIMEType::HTML;
							//TODO fixa så att hosten faktiskt är en del av detta, nu hardcodas det
							ReturnValue.DocumentData = LoadFileWithPreprocessing(ResourceToGet,ResourcePath);
						}
						else
						{
							ReturnValue = AssociatedServer->GetResource(ResourceToGet);
						}
					}
				}
			}
			else
			{
				ReturnValue.Type = MBMIME::MIMEType::HTML;
				ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
				ReturnValue.DocumentData = "<html><body>Bad Request</body><html>";
			}
		}
		return(ReturnValue);
	}
	//HTTPServer
	HTTPServer::HTTPServer(std::string PathToResources, int PortToListenTo)
	{
		ServerSocketen = new MBSockets::TCPServer(std::to_string(PortToListenTo));
		Port = PortToListenTo;
		m_DefaultResourcePath = PathToResources;
	}
	//std::string HTTPServer::GenerateResponse(MrPostOGet::HTTPDocument const& Document)
	//{
	//	return(MBSockets::GenerateRequest(Document));
	//}
	std::string HTTPServer::GetResourcePath(std::string const& DomainName)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		if (m_DomainResourcePaths.find(DomainName) != m_DomainResourcePaths.end())
		{
			return(m_DomainResourcePaths[DomainName]);
		}
		return(m_DefaultResourcePath);
	}
	void HTTPServer::LoadDomainResourcePaths(std::string const& ConfigFile)
	{
		std::string ConfigData = LoadWholeFile(ConfigFile);
		size_t ParseOffset = 0;
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		while (ParseOffset < ConfigData.size())
		{
			size_t NextEqualSign = ConfigData.find('=', ParseOffset);
			std::string NewDomain = ConfigData.substr(ParseOffset, NextEqualSign - ParseOffset);
			size_t LineEnd = std::min(ConfigData.find('\n', ParseOffset),ConfigData.size());
			std::string NewPath = ConfigData.substr(NextEqualSign + 1, LineEnd - NextEqualSign - 1);
			if (NewPath.back() == '\r')
			{
				NewPath.resize(NewPath.size() - 1);
			}
			m_DomainResourcePaths[NewDomain] = NewPath;
			ParseOffset = LineEnd + 1;
		}
	}
	std::string HTTPServer::LoadWholeFile(std::string const& FilePath)
	{
		std::ifstream t(FilePath, std::ifstream::in | std::ifstream::binary);
		//size_t size = std::filesystem::file_size(FilePath);
		uint64_t size = MBGetFileSize(FilePath);
		std::string FileDataBuffer(size, ' ');
		t.read(&FileDataBuffer[0], size);
		uint64_t ReadCharacters = t.gcount();
		std::string FileData(FileDataBuffer.c_str(), ReadCharacters);
		return(FileData);
	}
	void HTTPServer::UseTLS(bool ShouldUse)
	{
		m_UseSecureConnection.store(ShouldUse);
	}
	//std::string HTTPServer::LoadFileWithIntervalls(std::string const& FilePath, std::vector<FiledataIntervall> const& ByteRanges)
	//{
	//	std::string ReturnValue = "";
	//	std::ifstream FileToRead(FilePath, std::ifstream::in | std::ifstream::binary);
	//	//size_t LastBytePosition = std::filesystem::file_size(FilePath) - 1;
	//	size_t LastBytePosition = MBGetFileSize(FilePath) - 1;
	//	for (size_t i = 0; i < ByteRanges.size(); i++)
	//	{
	//		size_t NumberOfBytesToRead = ByteRanges[i].LastByte - ByteRanges[i].FirstByte;
	//		if (NumberOfBytesToRead < 0)
	//		{
	//			NumberOfBytesToRead = LastBytePosition - ByteRanges[i].FirstByte + 1;
	//		}
	//		size_t FirstByteToReadPosition = ByteRanges[i].FirstByte;
	//		if (FirstByteToReadPosition < 0)
	//		{
	//			NumberOfBytesToRead -= 1; //vi subtraherade med -1 över
	//
	//		}
	//		char* NewData = new char[500];
	//	}
	//	return(ReturnValue);
	//}
	MrPostOGet::HTTPDocument HTTPServer::GetResource(std::string const& ResourcePath, std::vector<FiledataIntervall> const& Byteranges)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		if (!std::filesystem::exists(ResourcePath))
		{
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
			ReturnValue.DocumentData = "";
			ReturnValue.DocumentDataFileReference = "";
			std::cout << "get file doesnt exist " + ResourcePath << std::endl;
		}

		if (Byteranges.size() != 0)
		{
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::PartialContent;
		}

		std::string ResourceExtension = ResourcePath.substr(ResourcePath.find_last_of(".") + 1);
		ReturnValue.Type = MBMIME::DocumentTypeFromFileExtension(ResourceExtension);
		if (ReturnValue.Type == MBMIME::MIMEType::Null)
		{
			std::cout << ResourceExtension << std::endl;
		}
		//if (std::filesystem::file_size(ResourcePath) > MaxDocumentInMemorySize || Byteranges.size() != 0)
		if (MBGetFileSize(ResourcePath) > MaxDocumentInMemorySize || Byteranges.size() != 0)
		{
			ReturnValue.DocumentDataFileReference = ResourcePath;
			ReturnValue.IntervallsToRead = Byteranges;
		}
		else
		{
			std::string FileData = LoadWholeFile(ResourcePath);
			ReturnValue.DocumentData = LoadWholeFile(ResourcePath);
		}
		return(ReturnValue);
	}
	void HTTPServer::AddRequestHandler(RequestHandler HandlerToAdd)
	{
		m_RequestHandlers.push_back(std::unique_ptr<HTTPRequestHandler>(new StaticRequestHandler(HandlerToAdd)));
	}
	void HTTPServer::AddRequestHandler(HTTPRequestHandler* HandlerToAdd)
	{
		m_RequestHandlers.push_back(std::unique_ptr<HTTPRequestHandler>(HandlerToAdd));
	}
	void HTTPServer::StartListening()
	{
		//huvud loopen som tar och faktiskt sk�ter hur allt funkar internt
		int NumberOfConnections = 0;
		//std::vector<std::thread*> CurrentActiveThreads = {};
		ServerSocketen->Bind();
		ServerSocketen->Listen();
		while (true)
		{
			ServerSocketen->Accept();
			//TODO detecta huruvida det är http eller https
			//MBError ConnectionError =  ServerSocketen->EstablishSecureConnection();
			//if (!ConnectionError)
			//{
			//	//error handla
			//	continue;
			//}
			if (ServerSocketen->IsValid())
			{
				NumberOfConnections += 1;
				MBSockets::TCPServer* NewTCPSocket = new MBSockets::TCPServer();
				ServerSocketen->TransferConnectedSocket(NewTCPSocket);
				std::thread* NewThread = new std::thread();
				//CurrentActiveThreads.push_back(NewThread);
				MPGConnectionHandle NewHandle = m_ActiveConnectionsHandler.AddConnection(std::unique_ptr<std::thread>(NewThread));
				*NewThread = std::move(std::thread(&HTTPServer::m_HandleConnectedSocket, this,std::unique_ptr<HTTPServerSocket>(new HTTPServerSocket(std::unique_ptr<MBSockets::ConnectSocket>(NewTCPSocket))),NewHandle));
				//std::cout << NumberOfConnections << std::endl;
			}
		}
	}
	HTTPServer::~HTTPServer()
	{
	}
	//BEGIN HTMLNode
	std::string HTMLNode::p_ExtractTagName(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError )
	{
		std::string ReturnValue = "";
		size_t ParseOffset = InOffset;
		MBError ParseError(true);
		const char* DataToParse = (char*)Data;
		//size_t NameEnd = std::min(HTMLData.find(' ', ParseOffset + 1), HTMLData.find('>', ParseOffset + 1));
		size_t NameEnd = std::min(std::find(DataToParse+ParseOffset+1,DataToParse+DataSize,' ')-DataToParse, std::find(DataToParse + ParseOffset + 1, DataToParse + DataSize, '>')-DataToParse);
		ReturnValue = MBUnicode::UnicodeStringToLower(std::string(DataToParse+(ParseOffset + 1), NameEnd - (ParseOffset + 1)));
		ParseOffset = NameEnd;
		MBParsing::UpdateParseState(ParseOffset, ParseError, OutOffset, OutError);
		return(ReturnValue);
	}

	std::pair<std::string, std::string> HTMLNode::p_ExtractTag(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		std::pair<std::string, std::string> ReturnValue = { "","" };
		size_t ParseOffset = InOffset;
		MBError EvalutionError(true);
		const char* HTMLData = (char*)Data;
		std::vector<char> NameDelimiters = { ' ','\t','\n','\r','\"','\'','='};
		MBParsing::SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
		size_t NameEnd = MBParsing::GetNextDelimiterPosition(NameDelimiters, Data, DataSize, ParseOffset);
		std::string AttributeName = std::string(HTMLData + ParseOffset, NameEnd - ParseOffset);
		ParseOffset = NameEnd;
		ParseOffset = std::find(HTMLData + ParseOffset, HTMLData + DataSize, '=') - HTMLData;
		ParseOffset += 1;
		MBParsing::SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
		std::string AttributeValue = MBParsing::ParseQuotedString(Data, DataSize, ParseOffset, &ParseOffset, &EvalutionError);
		ReturnValue = { AttributeName,AttributeValue };
		MBParsing::UpdateParseState(ParseOffset, EvalutionError, OutOffset, OutError);
		return(ReturnValue);
	}
	HTMLNode::HTMLNode(std::string const& RawText)
	{
		//lite parsing av den råa texten så vi tar borta massa newlines och gör den lite snyggare
		std::string TextToStore = RawText;
		//TextToStore = MBUtility::ReplaceAll(TextToStore, "\r\n", " ");
		//TextToStore = MBUtility::ReplaceAll(TextToStore, "\n", " ");
		//TextToStore = MBUtility::ReplaceAll(TextToStore, "\r", " ");
		//TextToStore = MBUtility::RemoveDuplicates(TextToStore, " ");
		//TextToStore = MBUtility::RemoveLeadingString(TextToStore, " ");
		m_RawText = std::move(TextToStore);
		//assert(TextToStore != "");
		m_IsRawtext = true;
	}
	std::string HTMLNode::GetVisableText() const
	{
		std::string ReturnValue = "";
		for (size_t i = 0; i < m_Children.size(); i++)
		{
			ReturnValue += m_Children[i].GetVisableText();
			if (ReturnValue.size() > 0)
			{
				if (ReturnValue.back() != '\n')
				{
					ReturnValue += "\n";
				}
			}
		}
		if (m_IsRawtext)
		{
			ReturnValue += m_RawText;
		}
		return(ReturnValue);
	}
	HTMLNode::HTMLNode(std::string const& HTMLToParse, size_t ParseOffset)
	{
		size_t OutOffset = 0;
		MBError EvaluationError(true);
		HTMLNode NewNode = ParseNode(HTMLToParse, ParseOffset,&OutOffset,&EvaluationError);
		if (!EvaluationError)
		{
			swap(*this, NewNode);
		}
	}
	std::string& HTMLNode::operator[](std::string const& AttributeName)
	{
		return(m_Attributes[AttributeName]);
	}
	std::string const& HTMLNode::operator[](std::string const& AttributeName) const
	{
		return(m_Attributes.at(AttributeName));
	}
	std::string HTMLNode::ToString() const
	{
		std::string ReturnValue = "";
		if (!m_IsRawtext)
		{
			ReturnValue = "<" + m_NodeTag;
			for (auto const& Attribute : m_Attributes)
			{
				ReturnValue += " ";
				ReturnValue += Attribute.first + "=" + "\"" + Attribute.second + "\"";
			}
			ReturnValue += ">\r\n";
			for (auto const& Child : m_Children)
			{
				ReturnValue += Child.ToString();
			}
			if (TagIsEmpty(m_NodeTag) == false)
			{
				ReturnValue += "\r\n</" + m_NodeTag + ">\r\n";
			}
		}
		else
		{
			ReturnValue = m_RawText;
		}
		return(ReturnValue);
	}
	bool HTMLNode::TagIsEmpty(std::string const& StringToCheck)
	{
		if (StringToCheck == "area" || StringToCheck == "area/")
		{
			return(true);
		}
		if (StringToCheck == "base" || StringToCheck == "base/")
		{
			return(true);
		}
		if (StringToCheck == "br" || StringToCheck == "br/")
		{
			return(true);
		}
		if (StringToCheck == "col" || StringToCheck == "col/")
		{
			return(true);
		}
		if (StringToCheck == "embed" || StringToCheck == "base/")
		{
			return(true);
		}
		if (StringToCheck == "hr" || StringToCheck == "hr/")
		{
			return(true);
		}
		if (StringToCheck == "img" || StringToCheck == "img/")
		{
			return(true);
		}
		if (StringToCheck == "input" || StringToCheck == "input/")
		{
			return(true);
		}
		if (StringToCheck == "keygen" || StringToCheck == "keygen/")
		{
			return(true);
		}
		if (StringToCheck == "link" || StringToCheck == "link/")
		{
			return(true);
		}
		if (StringToCheck == "meta" || StringToCheck == "meta/")
		{
			return(true);
		}
		if (StringToCheck == "param" || StringToCheck == "param/")
		{
			return(true);
		}
		if (StringToCheck == "source" || StringToCheck == "source/")
		{
			return(true);
		}
		if (StringToCheck == "track" || StringToCheck == "track/")
		{
			return(true);
		}
		if (StringToCheck == "wbr" || StringToCheck == "wbr/")
		{
			return(true);
		}
		return(false);
	}
	HTMLNode HTMLNode::ParseNode(const void* Data, size_t DataSize, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{

		
		HTMLNode ReturnValue;
		size_t ParseOffset = InOffset;
		MBError EvaluationError = OutOffset;
		
		const char* HTMLToParse = (char*)Data;
		
		MBParsing::SkipWhitespace(Data, DataSize, ParseOffset, &ParseOffset);
		//if (HTMLToParse.substr(ParseOffset, 3) == "<!D" || HTMLToParse.substr(ParseOffset, 3) == "<!d")
		if (std::string(HTMLToParse+ParseOffset,3) == "<!D" || std::string(HTMLToParse + ParseOffset, 3) == "<!d")
		{
			ParseOffset = std::find(HTMLToParse+ ParseOffset + 1,HTMLToParse+DataSize,'<')-HTMLToParse;
		}
		if (HTMLToParse[ParseOffset] == '<')
		{
			//vi börjar på en tag
			//extractar namnet
			
			//ParseOffset = ExtractTagName(HTMLToParse, ParseOffset);
			
			std::string NewTag = p_ExtractTagName(HTMLToParse, DataSize,ParseOffset,&ParseOffset,&EvaluationError);
			ReturnValue.m_NodeTag = NewTag;
			//settar upp tag datanm
			while (HTMLToParse[ParseOffset] != '>' && EvaluationError)
			{
				std::pair<std::string, std::string> NewAttribute = p_ExtractTag(HTMLToParse, DataSize, ParseOffset, &ParseOffset, &EvaluationError);
				ReturnValue.m_Attributes[NewAttribute.first] = NewAttribute.second;
			}
			ParseOffset += 1;
			if (TagIsEmpty(NewTag))
			{
				//ParseOffset = HTMLToParse.find('>', ParseOffset) + 1;
				MBParsing::UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
				return(ReturnValue);
			}
			if (ReturnValue.m_NodeTag[0] == '/')
			{
				//fuck robustness principen och allt den står för, 99% säker att en sida som renderas korrekt egentligen inneåller felaktig html, med extra endtags
				//för att testa om det går att parsa ändå så helt enkelt skippar vi element som "börjar" med en endtag
				ReturnValue.m_NodeTag = "";
				MBParsing::UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
				return(ReturnValue);
			}
			//om det är är en tom tag vill vi returna nu
			if (ParseOffset == 0)
			{
				std::cout << "Konstig" << std::endl;
			}

			//nu har vi kommit till den innre htmlen
			while (EvaluationError)
			{
				//size_t NextElementBegin = HTMLToParse.find("<", ParseOffset);
				size_t NextElementBegin = std::find(HTMLToParse+ParseOffset,HTMLToParse+DataSize,'<')-HTMLToParse;
				if (ParseOffset == 0)
				{
					std::cout << "Konstig" << std::endl;
				}
				if (NextElementBegin == DataSize)
				{
					//ENBART HÄR FÖR ATT STÖDJA FELAKTIG DATA
					//m_ParseError = false;
					//m_ParseError.ErrorMessage = "No tag end detected";
					ParseOffset = NextElementBegin;
					break;
				}
				if (std::string(HTMLToParse+NextElementBegin, 2) == "<!")
				{
					ParseOffset = (size_t)(std::find(HTMLToParse+NextElementBegin,HTMLToParse+DataSize,'>') + 1-HTMLToParse);
					continue;
				}
				if (NextElementBegin != ParseOffset)
				{
					//kanske finns rå text mellan
					std::string RawText = std::string(HTMLToParse+ParseOffset, NextElementBegin - ParseOffset);
					std::string NormalizedText = RawText;
					std::vector<std::string> FillerCharacters = { "\t","\n"," ","\r" };
					size_t NextInterestingCharacter = NextElementBegin;
					for (size_t i = 0; i < FillerCharacters.size(); i++)
					{
						NormalizedText = MBUtility::ReplaceAll(NormalizedText, FillerCharacters[i], "");
					}
					if (NormalizedText.size() != 0)
					{
						//std::string RawText = HTMLToParse.substr(ParseOffset, NextElementBegin - ParseOffset);
						ReturnValue.m_Children.push_back(HTMLNode(RawText));
						ReturnValue.m_Children.back().m_Parent = &ReturnValue;
					}
				}
				if (std::string(HTMLToParse+ NextElementBegin, 2 + ReturnValue.m_NodeTag.size()) == "</" + ReturnValue.m_NodeTag)
				{
					ParseOffset = std::find(HTMLToParse+NextElementBegin,HTMLToParse+DataSize,'>') + 1-HTMLToParse;
					break;
				}

				//m_Children.push_back(HTMLNode(HTMLToParse, NextElementBegin));
				ReturnValue.m_Children.push_back(HTMLNode::ParseNode(HTMLToParse,DataSize,NextElementBegin,&ParseOffset,&EvaluationError));
				//ParseOffset = m_Children.back().m_GetEndtagOffset();
				//m_ParseError = (m_Children.back().m_ParseError == true);
				ReturnValue.m_Children.back().m_Parent = &ReturnValue;
				if (!EvaluationError)
				{
					break;
				}
				if (ParseOffset >= DataSize)
				{
					//ENBART HÄR FÖR ATT STÖDJA FELAKTIG DATA
					//m_ElementEndOffset = ParseOffset;
					break;
				}
			}
		}
		else
		{
			//lägger in all data tills vi kommer till en ny tag
			assert(false);
		}
		MBParsing::UpdateParseState(ParseOffset, EvaluationError, OutOffset, OutError);
		return(ReturnValue);
	}
	HTMLNode HTMLNode::ParseNode(std::string const& DataToParse, size_t InOffset, size_t* OutOffset, MBError* OutError)
	{
		return(ParseNode(DataToParse.data(), DataToParse.size(), InOffset, OutOffset, OutError));
	}
	HTMLNode HTMLNode::CreateElement(std::string const& ElementTag)
	{
		HTMLNode ReturnValue;
		ReturnValue.m_NodeTag = ElementTag;
		return(ReturnValue);
	}
	void HTMLNode::p_UpdateChildParents()
	{
		for (auto& Child : m_Children)
		{
			Child.m_Parent = this;
			Child.p_UpdateChildParents();
		}
	}
	HTMLNode::HTMLNode(HTMLNode const& NodeToCopy)
	{
		m_RawText = NodeToCopy.m_RawText;
		m_Attributes = NodeToCopy.m_Attributes;
		m_NodeTag = NodeToCopy.m_NodeTag;
		m_IsRawtext = NodeToCopy.m_IsRawtext;
		m_Parent = nullptr; //kan inte kopiera att den är ett barn
		for(auto const& Child : NodeToCopy.m_Children)
		{
			m_Children.push_back(HTMLNode(Child));
			m_Children.back().m_Parent = this;
		}

	}
	HTMLNode::HTMLNode(HTMLNode&& NodeToSteal) noexcept
	{
		swap(*this, NodeToSteal);
	}
	void HTMLNode::AppendChild(HTMLNode NewChild)
	{
		m_Children.push_back(std::move(NewChild));
		m_Children.back().m_Parent = this;
	}
	//void HTMLNode::AppendChild(HTMLNode&& ChildToMove)
	//{
	//	m_Children.push_back(ChildToMove);
	//	m_Children.back().m_Parent = this;
	//}
	//std::vector<HTMLNode> m_Children = {};
	//std::string m_RawText = "";
	//std::unordered_map<std::string, std::string> m_Attributes = {};
	//std::string m_NodeTag = "";
	//HTMLNode* m_Parent = nullptr;
	//bool m_IsRawtext = false;
	void swap(HTMLNode& LeftNode, HTMLNode& RightNode)
	{
		std::swap(LeftNode.m_Children, RightNode.m_Children);
		std::swap(LeftNode.m_RawText, RightNode.m_RawText);
		std::swap(LeftNode.m_Attributes, RightNode.m_Attributes);
		std::swap(LeftNode.m_NodeTag, RightNode.m_NodeTag);
		std::swap(LeftNode.m_IsRawtext, RightNode.m_IsRawtext);
		std::swap(LeftNode.m_Parent, RightNode.m_Parent);
	}

	//END HTMLNode

	//BEGIN StaticRequestHandler
	StaticRequestHandler::StaticRequestHandler(RequestHandler HandlerToConvert)
	{
		m_InternalHandler = HandlerToConvert;
	}
	bool StaticRequestHandler::HandlesRequest(HTTPClientRequest const& RequestToHandle, HTTPClientConnectionState const& ConnectionState, HTTPServer* AssociatedServer)
	{
		return(m_InternalHandler.RequestPredicate(RequestToHandle.RawRequestData));
	}
	MrPostOGet::HTTPDocument StaticRequestHandler::GenerateResponse(HTTPClientRequest const& Request, HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket* Server, HTTPServer* Connection)
	{
		return(m_InternalHandler.RequestResponse(Request.RawRequestData, Connection, Server));
	}
	//END StaticRequestHandler

	//BEGIN FileIntervallExtracter
	FileIntervallExtracter::FileIntervallExtracter(std::string const& FilePath, std::vector<FiledataIntervall> const& Intervalls, size_t MaxDataInMemory)
		: FileToRead(FilePath, std::ifstream::in | std::ifstream::binary)
	{
		IntervallsToRead = Intervalls;
		//FileSize = std::filesystem::file_size(FilePath);
		FileSize = MBGetFileSize(FilePath);
		this->MaxDataInMemory = MaxDataInMemory;
	}
	std::string FileIntervallExtracter::GetNextIntervall()
	{
		if (IntervallIndex >= IntervallsToRead.size())
		{
			return("");
		}
		uint64_t NumberOfBytesToRead = IntervallsToRead[IntervallIndex].LastByte - IntervallsToRead[IntervallIndex].FirstByte + 1;
		if (IntervallsToRead[IntervallIndex].LastByte == -1)
		{
			NumberOfBytesToRead = FileSize - IntervallsToRead[IntervallIndex].FirstByte;
		}
		uint64_t FirstByteToReadPosition = IntervallsToRead[IntervallIndex].FirstByte;
		if (FirstByteToReadPosition < 0)
		{
			NumberOfBytesToRead -= 1; //vi subtraherade med -1 över
			FirstByteToReadPosition = FileSize - NumberOfBytesToRead;
		}

		if (NumberOfBytesToRead >= MaxDataInMemory)
		{
			uint64_t BytesWantedToRead = NumberOfBytesToRead;
			NumberOfBytesToRead = MaxDataInMemory;
			IntervallsToRead[IntervallIndex].LastByte = FirstByteToReadPosition + BytesWantedToRead - 1;
			IntervallsToRead[IntervallIndex].FirstByte = FirstByteToReadPosition + NumberOfBytesToRead;
		}
		else
		{
			IntervallIndex += 1;
		}
		std::string ReturnValue = std::string(NumberOfBytesToRead, 0);
		FileToRead.seekg(FirstByteToReadPosition);
		FileToRead.read(&ReturnValue[0], NumberOfBytesToRead);
		TotalDataRead += NumberOfBytesToRead;
		return(ReturnValue);
	}
	bool FileIntervallExtracter::IsDone()
	{
		if (IntervallIndex >= IntervallsToRead.size())
		{
			return(true);
		}
		return(false);
	}
	//END FileIntervallExtracter

		//BEGIN HTTPServerSocket
	//class HTTPServerSocket:: : public ServerSocket
	bool HTTPServerSocket::DataIsAvailable()
	{
		if (ChunksRemaining || (ParsedContentData != CurrentContentLength))
		{
			return(true);
		}
		else
		{
			std::cout << "Data is not available " << std::endl;
			return(false);
		}
	}
	MBError HTTPServerSocket::SendRawData(const void* DataToSend, size_t DataToLength)
	{
		return(m_UnderlyingSocket->SendData(DataToSend, DataToLength));
	}
	MBError HTTPServerSocket::SendRawData(std::string const& DataToSend)
	{
		return(SendRawData(DataToSend.data(), DataToSend.size()));
	}

	int HTTPServerSocket::Close()
	{
		m_UnderlyingSocket->Close();
		return(0);
	}
	bool HTTPServerSocket::IsConnected()
	{
		return(m_UnderlyingSocket->IsConnected());
	}
	bool HTTPServerSocket::IsValid()
	{
		return(m_UnderlyingSocket->IsValid());
	}
	MBError HTTPServerSocket::EstablishTLSConnection()
	{
		return(m_UnderlyingSocket->EstablishTLSConnection(true,""));
	}
	HTTPServerSocket::HTTPServerSocket(std::unique_ptr<MBSockets::ConnectSocket> ConntectedSocket)
	{
		m_UnderlyingSocket = std::unique_ptr<MBSockets::TLSConnectSocket>(new MBSockets::TLSConnectSocket(std::move(ConntectedSocket)));
	}
	int HTTPServerSocket::p_GetNextChunkSize(int ChunkHeaderPosition, std::string const& Data, int& OutChunkDataBeginning)
	{
		size_t ChunkHeaderEnd = Data.find("\r\n", ChunkHeaderPosition);
		std::string NumberOfChunkBytes = Data.substr(ChunkHeaderPosition, ChunkHeaderEnd - ChunkHeaderPosition);
		OutChunkDataBeginning = ChunkHeaderEnd + 2;
		return(std::stoi(NumberOfChunkBytes));
	}
	std::string HTTPServerSocket::p_UpdateAndDeChunkData(std::string const& ChunkedData)
	{
		std::string ReturnValue = "";
		int ChunkDataToReadPosition = 0;
		if (CurrentChunkSize == 0)
		{
			//detta innebär att vi är i den första chunken som innehåller headern
			int NextChunkHeaderPosition = ChunkedData.find("\n\r\n") + 3;
			ReturnValue += ChunkedData.substr(0, NextChunkHeaderPosition);
			int ChunkHeaderEnd = ChunkedData.find("\r\n", NextChunkHeaderPosition);
			std::string NumberOfChunkBytes = ChunkedData.substr(NextChunkHeaderPosition, ChunkHeaderEnd - NextChunkHeaderPosition);
			CurrentChunkSize = std::stoi(NumberOfChunkBytes);
			CurrentChunkParsed = 0;

			int ChunkDataToReadPosition = ChunkHeaderEnd + 2;
		}
		else
		{
			int ChunkDataToReadPosition = CurrentChunkParsed;

		}
		while (ChunkDataToReadPosition != ChunkedData.size() && CurrentChunkSize != 0)
		{
			int MaxByteToParse = CurrentChunkSize - CurrentChunkParsed;
			int AvailableBytes = ChunkedData.size() - ChunkDataToReadPosition;
			int BytesToRead = std::min(MaxByteToParse, AvailableBytes);
			//vi antar alltid att chunked headers skickas i helhet
			ReturnValue += ChunkedData.substr(ChunkDataToReadPosition, BytesToRead);
			//nu header
			CurrentChunkParsed += BytesToRead;
			if (CurrentChunkParsed == CurrentChunkSize)
			{
				CurrentChunkSize = p_GetNextChunkSize(ChunkDataToReadPosition + BytesToRead, ChunkedData, ChunkDataToReadPosition);
			}
			if (CurrentChunkSize == 0)
			{
				ChunksRemaining = false;
				RequestIsChunked = false;
				CurrentChunkSize = 0;
				CurrentChunkParsed = 0;
			}
		}
		if (CurrentChunkSize != 0)
		{
			ChunksRemaining = true;
			RequestIsChunked = true;
		}
		return(ReturnValue);
	}
	std::string HTTPServerSocket::GetHTTPRequest()
	{
		std::string ReturnValue = "";

		std::string NewData = "";

		std::string ContentLengthString = "NULL";
		size_t ContentLength = 0;
		size_t HeaderLength = 0;
		size_t MaxDataInMemory = 1650000 * 2;
		size_t TotalRecievedData = 0;
		while (true)
		{
			NewData = m_UnderlyingSocket->RecieveData(MaxDataInMemory - TotalRecievedData);
			if (!this->IsValid())
			{
				//något är fel, returna det vi fick och resetta, socketen kan inte användas mer
				CurrentContentLength = 0;
				ParsedContentData = 0;
				return(ReturnValue);
			}
			ReturnValue += NewData;
			TotalRecievedData += NewData.size();
			if (CurrentContentLength == 0)
			{
				if (ContentLengthString == "NULL")
				{
					HeaderLength = ReturnValue.find("\r\n\r\n") + 4;
					ContentLengthString = GetHeaderValue("Content-Length", ReturnValue);
					if (ContentLengthString != "")
					{
						ContentLength = std::stoi(ContentLengthString);
					}
				}
				if (ContentLengthString == "" || ReturnValue.size() - HeaderLength >= ContentLength)
				{
					break;
				}
				if (ReturnValue.size() > MaxDataInMemory)
				{
					if (ContentLength != 0)
					{
						CurrentContentLength = ContentLength;
						ParsedContentData = ReturnValue.size() - HeaderLength;
					}
					break;
				}
			}
			else
			{
				//ParsedContentData += ReturnValue.size();
				if (ParsedContentData + ReturnValue.size() >= CurrentContentLength)
				{
					CurrentContentLength = 0;
					ParsedContentData = 0;
					break;
				}
				if (ReturnValue.size() > MaxDataInMemory)
				{
					ParsedContentData += ReturnValue.size();
					break;
				}
			}
		}
		//kollar huruvida vi har en content-length tag, har vi det så appendearr vi den till det blir rätt
		if (RequestIsChunked == false)
		{
			//innebär att detta är den första requesten, vilket innebär att vi vill se om den är hel eller chunked
			std::vector<std::string> TransferEncodings = GetHeaderValues("Transfer-Encoding", ReturnValue);
			for (size_t i = 0; i < TransferEncodings.size(); i++)
			{
				if (TransferEncodings[i] == "chunked")
				{
					RequestIsChunked = true;
					break;
				}
			}
		}
		return(ReturnValue);
	}
	void HTTPServerSocket::SendDataAsHTTPBody(const std::string& Data)
	{
		std::string Body = "<html>\n<body>\n" + Data + "</body>\n</html>";
		std::string Request = p_GenerateRequest(Body);
		m_UnderlyingSocket->SendData(Request);
	}
	void HTTPServerSocket::SendHTTPBody(const std::string& Data)
	{
		std::string Request = p_GenerateRequest(Data);
		m_UnderlyingSocket->SendData(Request);
	}
	//void HTTPServerSocket::SendFullResponse(std::string const& DataToSend)
	//{
	//	ConnectSocket::SendData(DataToSend);
	//}
	std::string HTTPServerSocket::p_HTTPRequestStatusToString(HTTPRequestStatus StatusToConvert)
	{
		if (StatusToConvert == HTTPRequestStatus::OK)
		{
			return("200 OK");
		}
		else if (StatusToConvert == HTTPRequestStatus::PartialContent)
		{
			return("206 Partial Content");
		}
		else if (StatusToConvert == HTTPRequestStatus::NotFound)
		{
			return("404 Not Found");
		}
		else if (StatusToConvert == HTTPRequestStatus::Conflict)
		{
			return("409 Conflict");
		}
		else if (StatusToConvert == HTTPRequestStatus::Authenticate)
		{
			return("401 Authenticate");
		}
		assert(false);
		return("");
	}
	std::string HTTPServerSocket::p_GenerateRequest(HTTPDocument const& DocumentToSend)
	{
		std::string Request = "";
		Request += "HTTP/1.1 " + p_HTTPRequestStatusToString(DocumentToSend.RequestStatus) + "\r\n";
		if (DocumentToSend.ExtraHeaders.find("Content-Type") != DocumentToSend.ExtraHeaders.end())
		{
			Request += "Content-Type: " + DocumentToSend.ExtraHeaders.at("Content-Type").front();
		}
		else
		{
			Request += "Content-Type: " + MBMIME::GetMIMEStringFromType(DocumentToSend.Type) + "\r\n";
		}
		Request += "Accept-Ranges: bytes\r\n";
		Request += "Content-Length: ";
		if (DocumentToSend.DocumentDataFileReference != "")
		{
			//datan är sparad som en referns istället
			//Request += "Transfer-Encoding: chunked";
			if (DocumentToSend.IntervallsToRead.size() == 0)
			{
				//Request += std::to_string(std::filesystem::file_size(DocumentToSend.DocumentDataFileReference));
				Request += std::to_string(MBGetFileSize(DocumentToSend.DocumentDataFileReference));
			}
			else
			{
				//size_t FileSize = std::filesystem::file_size(DocumentToSend.DocumentDataFileReference);
				uint64_t FileSize = MBGetFileSize(DocumentToSend.DocumentDataFileReference);
				uint64_t TotalIntervallSize = 0;
				for (size_t i = 0; i < DocumentToSend.IntervallsToRead.size(); i++)
				{
					if (DocumentToSend.IntervallsToRead[i].FirstByte == -1)
					{
						TotalIntervallSize += DocumentToSend.IntervallsToRead[i].LastByte;
					}
					else if (DocumentToSend.IntervallsToRead[i].LastByte == -1)
					{
						TotalIntervallSize += FileSize - DocumentToSend.IntervallsToRead[i].FirstByte;
					}
					else
					{
						TotalIntervallSize += DocumentToSend.IntervallsToRead[i].LastByte - DocumentToSend.IntervallsToRead[i].FirstByte + 1;
					}
				}
				Request += std::to_string(TotalIntervallSize);
			}
		}
		else
		{
			//Request += "Content-Length: ";
			Request += std::to_string(DocumentToSend.DocumentData.size());
		}
		Request += "\r\n";
		for (auto const& Header : DocumentToSend.ExtraHeaders)
		{
			for (auto const& HeaderValue : Header.second)
			{
				Request += Header.first + ": " + HeaderValue + "\r\n";
			}
		}
		Request += "\r\n";
		if (DocumentToSend.DocumentDataFileReference == "")
		{
			Request += DocumentToSend.DocumentData;
		}
		return(Request);
	}
	std::string HTTPServerSocket::p_GenerateRequest(const std::string& HTMLBody)
	{
		/*
		HTTP/1.1 200 OK
		Content-Type: text/html
		Accept-Ranges: bytes
		//Vary: Accept-Encoding
		Content-Length: 390
		*/
		std::string Request = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccept-Ranges: bytes\r\nContent-Length: " + std::to_string(HTMLBody.size()) + "\r\n\r\n" + HTMLBody;
		return(Request);
	}
	void HTTPServerSocket::SendHTTPDocument(HTTPDocument const& DocumentToSend)
	{
		//TODO egentligen vill vi väll ha support för flera byta ranges (?) men det innebär att man kommer skicka dem som en multipart form, vilket inte är det vi vill
		if (DocumentToSend.RequestStatus == HTTPRequestStatus::PartialContent)
		{
			//enkel range request med specifikt intervall
			HTTPDocument NewDocument = DocumentToSend;
			uint64_t StartByte = NewDocument.IntervallsToRead[0].FirstByte;
			uint64_t LastByte = NewDocument.IntervallsToRead[0].LastByte;
			//size_t FileSize = std::filesystem::file_size(NewDocument.DocumentDataFileReference);
			uint64_t FileSize = MBGetFileSize(NewDocument.DocumentDataFileReference);
			if (StartByte == -1)
			{
				StartByte = FileSize - LastByte;
				LastByte = FileSize - 1;
			}
			if (LastByte == -1)
			{
				LastByte = FileSize - 1;
			}
			std::string ContentRangeHeader = "bytes " + std::to_string(StartByte) + "-" + std::to_string(LastByte) + "/" + std::to_string(FileSize);
			NewDocument.ExtraHeaders["Content-Range"].push_back(ContentRangeHeader);
			std::string DataToSend = p_GenerateRequest(NewDocument);
			m_UnderlyingSocket->SendData(DataToSend);
		}
		else
		{
			std::string DataToSend = p_GenerateRequest(DocumentToSend);
			m_UnderlyingSocket->SendData(DataToSend);
		}
		if (DocumentToSend.DocumentDataFileReference != "")
		{
			//vi ska skicka fildatan somn är där, och läser in den gradvis
			//std::ifstream DocumentFile(DocumentToSend.DocumentDataFileReference, std::ios::in | std::ios::binary);
			std::vector<FiledataIntervall> DocumentInterValls = DocumentToSend.IntervallsToRead;
			uint64_t MaxChunkSize = 16384;
			if (DocumentInterValls.size() == 0)
			{
				//vi skapar intervall
				//size_t FileSize = std::filesystem::file_size(DocumentToSend.DocumentDataFileReference);
				uint64_t FileSize = MBGetFileSize(DocumentToSend.DocumentDataFileReference);
				uint64_t CurrentOffset = 0;
				while (true)
				{
					FiledataIntervall NewIntervall = { CurrentOffset,CurrentOffset + MaxChunkSize - 1 };
					CurrentOffset += MaxChunkSize;
					if (NewIntervall.LastByte >= FileSize)
					{
						NewIntervall.LastByte = FileSize - 1;
					}
					DocumentInterValls.push_back(NewIntervall);
					if (NewIntervall.LastByte == FileSize - 1)
					{
						break;
					}
				}
			}
			FileIntervallExtracter DataExtracter(DocumentToSend.DocumentDataFileReference, DocumentInterValls, MaxChunkSize);
			while (!DataExtracter.IsDone())
			{
				m_UnderlyingSocket->SendData(DataExtracter.GetNextIntervall());
			}
			//int hej = 2;
		}
	}
	std::string HTTPServerSocket::GetNextChunkData()
	{
		std::string NewData = GetHTTPRequest();
		if (RequestIsChunked)
		{
			std::string ReturnValue = p_UpdateAndDeChunkData(NewData);
			return(ReturnValue);
		}
		else
		{
			return(NewData);
		}
	}
	//END HTTPServerSocket
};