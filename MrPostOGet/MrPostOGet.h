#pragma once
#include <MrBoboSockets.h>
#include <TextReader.h>
#include <cstring>
//linux specifika grejer


namespace MrPostOGet 
{
	class HTTPServer;
	struct RequestHandler
	{
		bool (*RequestPredicate)(const std::string& RequestData);
		MBSockets::HTTPDocument(*RequestResponse)(const std::string& ResoureData, HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedSocket);
	};
	void HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient, std::vector<RequestHandler> RequestHandlers, std::string ResourcesPath, HTTPServer* AssociatedServer);
	
	
	inline MBSockets::HTTPDocumentType MimeSniffDocument(std::string const& FilePath)
	{
		std::fstream DocumentToSniff = std::fstream(FilePath);
		std::string DocumentHeader(100,0);
		size_t BytesRead = DocumentToSniff.read(&DocumentHeader[0], 100).gcount();
		if (DocumentHeader.substr(0,6) == "<html>")
		{
			return(MBSockets::HTTPDocumentType::HTML);
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
				return(MBSockets::HTTPDocumentType::HTML);
			}
		}
		return(MBSockets::HTTPDocumentType::Null);
	}
	//convinience funktioner
	inline std::string GetRequestContent(std::string const& RequestData)
	{
		size_t ContentStart = RequestData.find("\r\n\r\n")+4;
		return(RequestData.substr(ContentStart));
	}
	inline std::string LoadWholeFile(std::string const& FilePath)
	{
		std::ifstream t(FilePath, std::ifstream::in | std::ifstream::binary);
		//size_t size = std::filesystem::file_size(FilePath);
		size_t size = MBGetFileSize(FilePath);
		std::string FileDataBuffer(size, ' ');
		t.read(&FileDataBuffer[0], size);
		size_t ReadCharacters = t.gcount();
		std::string FileData(FileDataBuffer.c_str(), ReadCharacters);
		return(FileData);
	}
	inline std::string ReplaceMPGVariables(std::string const& FileData, std::unordered_map<std::string, std::string> const& VariablesMap)
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
	inline std::string LoadFileWithVariables(std::string const& Filepath, std::unordered_map<std::string, std::string> const& VariablesMap)
	{
		if (!std::filesystem::exists(Filepath))
		{
			return("");
		}
		std::string FileData = LoadWholeFile(Filepath);
		return(ReplaceMPGVariables(FileData, VariablesMap));
	}
	inline std::string LoadFileWithPreprocessing(std::string const& Filepath, std::string const& ResourcesPath) 
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
					ReturnValue += FileData.substr(ParsePosition, NextEndSequence - ParsePosition+1);
					ParsePosition = NextEndSequence + 1;
					continue;
				}
				ReturnValue += LoadFileWithPreprocessing(ResourcesPath + DirectiveArguments, ResourcesPath);
			}
			ParsePosition = NextEndSequence + 1;
		}
		return(ReturnValue);
	}
	inline std::string GetFileExtension(std::string const& StringData)
	{
		std::string ReturnValue = "";
		size_t DotPosition = StringData.find_last_of(".");
		if (DotPosition != StringData.npos)
		{
			ReturnValue = StringData.substr(DotPosition + 1);
		}
		return(ReturnValue);
	}
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
		HTTPServer(std::string PathToResources, int PortToListenTo)
		{
			ServerSocketen = new MBSockets::HTTPServerSocket(std::to_string(PortToListenTo), MBSockets::TraversalProtocol::TCP);
			Port = PortToListenTo;
			ContentPath = PathToResources;
		}
		std::string GenerateResponse(MBSockets::HTTPDocument const& Document)
		{
			return(MBSockets::GenerateRequest(Document));
		}
		std::string GetResourcePath(std::string const& DomainName)
		{
			return(ContentPath);
		}
		std::string LoadWholeFile(std::string const& FilePath)
		{
			std::ifstream t(FilePath, std::ifstream::in | std::ifstream::binary);
			//size_t size = std::filesystem::file_size(FilePath);
			size_t size = MBGetFileSize(FilePath);
			std::string FileDataBuffer(size, ' ');
			t.read(&FileDataBuffer[0], size);
			size_t ReadCharacters = t.gcount();
			std::string FileData(FileDataBuffer.c_str(), ReadCharacters);
			return(FileData);
		}
		std::string LoadFileWithIntervalls(std::string const& FilePath, std::vector<FiledataIntervall> const& ByteRanges)
		{
			std::string ReturnValue = "";
			std::ifstream FileToRead(FilePath, std::ifstream::in | std::ifstream::binary);
			//size_t LastBytePosition = std::filesystem::file_size(FilePath) - 1;
			size_t LastBytePosition = MBGetFileSize(FilePath) - 1;
			for (size_t i = 0; i < ByteRanges.size(); i++)
			{
				int NumberOfBytesToRead = ByteRanges[i].LastByte - ByteRanges[i].FirstByte;
				if (NumberOfBytesToRead < 0)
				{
					NumberOfBytesToRead = LastBytePosition - ByteRanges[i].FirstByte+1;
				}
				int FirstByteToReadPosition = ByteRanges[i].FirstByte;
				if (FirstByteToReadPosition < 0)
				{
					NumberOfBytesToRead -= 1; //vi subtraherade med -1 över
					
				}
				char* NewData = new char[500];
			}
		}
		MBSockets::HTTPDocument GetResource(std::string const& ResourcePath, std::vector<FiledataIntervall> const& Byteranges = {})
		{
			MBSockets::HTTPDocument ReturnValue;
			if (!std::filesystem::exists(ResourcePath))
			{
				ReturnValue.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
				ReturnValue.DocumentData = "";
				ReturnValue.DocumentDataFileReference = "";
				std::cout << "get file doesnt exist "+ResourcePath << std::endl;
			}

			if (Byteranges.size() != 0)
			{
				ReturnValue.RequestStatus = MBSockets::HTTPRequestStatus::PartialContent;
			}

			std::string ResourceExtension = ResourcePath.substr(ResourcePath.find_last_of(".")+1);
			ReturnValue.Type = MBSockets::DocumentTypeFromFileExtension(ResourceExtension);
			if(ReturnValue.Type == MBSockets::HTTPDocumentType::Null)
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
		void AddRequestHandler(RequestHandler HandlerToAdd)
		{
			ServerRequestHandlers.push_back(HandlerToAdd);
		}
		void StartListening()
		{
			//huvud loopen som tar och faktiskt sk�ter hur allt funkar internt
			int NumberOfConnections = 0;
			std::vector<std::thread*> CurrentActiveThreads = {};
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
					MBSockets::HTTPServerSocket* NewSocket = new MBSockets::HTTPServerSocket(std::to_string(Port), MBSockets::TraversalProtocol::TCP);
					ServerSocketen->TransferConnectedSocket(*NewSocket);
					std::thread* NewThread = new std::thread(HandleConnectedSocket, NewSocket, ServerRequestHandlers, ContentPath, this);
					CurrentActiveThreads.push_back(NewThread);
					std::cout << NumberOfConnections << std::endl;
				}
			}
		}
		~HTTPServer()
		{
		}
	};
	inline void HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient, std::vector<RequestHandler> RequestHandlers, std::string ResourcesPath, HTTPServer* AssociatedServer)
	{
		MBError ConnectError = ConnectedClient->EstablishSecureConnection();
		if (!ConnectError)
		{
			std::cout << ConnectError.ErrorMessage << std::endl;
		}
		std::string RequestData;
		std::cout << RequestData << std::endl;
		//v�ldigt enkelt system, tar bara emot get requests, och skickar bara datan som kopplad till filepathen
		while ((RequestData = ConnectedClient->GetNextChunkData()) != "")
		{
			if (!ConnectedClient->IsConnected())
			{
				continue;
			}
			if (!ConnectedClient->IsValid())
			{

			}
			//vi kollar om request handlersen har n�gon �sikt om datan, annars l�ter vi v�ran default getrequest handlar sk�ta allt 
			bool HandlerHasHandled = false;
			for (size_t i = 0; i < RequestHandlers.size(); i++)
			{
				if (RequestHandlers[i].RequestPredicate(RequestData))
				{
					//vi ska g�ra grejer med denna data, s� vi tar och skapar stringen som vi sen ska skicka
					MBSockets::HTTPDocument RequestResponse = RequestHandlers[i].RequestResponse(RequestData, AssociatedServer,ConnectedClient);
					ConnectedClient->SendHTTPDocument(RequestResponse);
					HandlerHasHandled = true;
					break;
				}
			}
			if (HandlerHasHandled)
			{
				continue;
			}
			//om v�ra handlers inte har handlat datan s� g�r v�r default handler det. Denna funkar enbart f�r get, eftersom dem andra requestsen inte makar mycket sense i sammanhanget
			if (MBSockets::GetRequestType(RequestData) != "GET")
			{
				ConnectedClient->SendDataAsHTTPBody("Bad Request");
			}
			else
			{
				//std::cout << RequestData << std::endl;
				std::string ResourceToGet = ResourcesPath + MBSockets::GetReqestResource(RequestData);
				std::string ResourceExtension = GetFileExtension(ResourceToGet);
				std::filesystem::path ActualResourcePath = std::filesystem::current_path().concat("/" + ResourceToGet);
				//std::cout << ResourceToGet << std::endl;
				MBSockets::HTTPDocument DocumentToSend;
				if (std::filesystem::exists(ActualResourcePath))
				{
					if (ResourceToGet == ResourcesPath)
					{
						DocumentToSend.Type = MBSockets::HTTPDocumentType::HTML;
						DocumentToSend.DocumentData = LoadFileWithPreprocessing(ResourcesPath + "index.htm",AssociatedServer->GetResourcePath("mrboboget.se"));
						ConnectedClient->SendHTTPDocument(DocumentToSend);
					}
					else
					{
						//väldigt ful undantagsfall för att få acme protokollet att fungera
						std::string ChallengeFolder = "./ServerResources/mrboboget.se/HTMLResources/.well-known/acme-challenge/";
						if (ResourceToGet.substr(0, ChallengeFolder.size()) == ChallengeFolder)
						{
							MBSockets::HTTPDocument NewDocument;
							NewDocument.Type = MBSockets::HTTPDocumentType::OctetString;
							std::string DocumentData = "";
							TextReader Data(ResourceToGet);
							for (int i = 0; i < Data.Size(); i++)
							{
								DocumentData += Data[i];
							}
							NewDocument.DocumentData = DocumentData;
							ConnectedClient->SendHTTPDocument(NewDocument);
						}
						else
						{
							//undantagsfall utifall det är ett hmtl dokument, då vill vi ta och skicka den med includes
							if (ResourceExtension == "html" || ResourceExtension == "htm")
							{
								DocumentToSend.Type = MBSockets::HTTPDocumentType::HTML;
								//TODO fixa så att hosten faktiskt är en del av detta, nu hardcodas det
								DocumentToSend.DocumentData = LoadFileWithPreprocessing(ResourceToGet, AssociatedServer->GetResourcePath("mrboboget.se"));
								ConnectedClient->SendHTTPDocument(DocumentToSend);
							}
							else
							{
								DocumentToSend = AssociatedServer->GetResource(ResourceToGet);
								ConnectedClient->SendHTTPDocument(DocumentToSend);
							}
						}
					}
				}
				else
				{
					ConnectedClient->SendDataAsHTTPBody("Bad Request");
				}
			}
		}
		//delete ConnectedClient;
	}
}