#pragma once
#include <MrBoboSockets.h>
#include <TextReader.h>
namespace MrPostOGet 
{
	class HTTPServer;
	struct RequestHandler
	{
		bool (*RequestPredicate)(const std::string& RequestData);
		MBSockets::HTTPDocument(*RequestResponse)(const std::string& ResoureData, HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedSocket);
	};
	void HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient, std::vector<RequestHandler> RequestHandlers, std::string ResourcesPath, HTTPServer* AssociatedServer);
	
	
	
	//convinience funktioner
	inline std::string GetRequestContent(std::string const& RequestData)
	{
		size_t ContentStart = RequestData.find("\r\n\r\n")+4;
		return(RequestData.substr(ContentStart));
	}
	
	class HTTPServer
	{
	private:
		std::string ContentPath = "";
		int Port = -1;
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
			size_t size = std::filesystem::file_size(FilePath);
			std::string FileDataBuffer(size, ' ');
			t.read(&FileDataBuffer[0], size);
			size_t ReadCharacters = t.gcount();
			std::string FileData(FileDataBuffer.c_str(), ReadCharacters);
			return(FileData);
		}
		MBSockets::HTTPDocument GetResource(std::string const& ResourcePath)
		{
			MBSockets::HTTPDocument ReturnValue;
			std::string ResourceExtension = ResourcePath.substr(ResourcePath.find_last_of(".")+1);
			if (ResourceExtension == "png")
			{
				ReturnValue.Type = MBSockets::HTTPDocumentType::png;
			}
			else if (ResourceExtension == "html" || ResourceExtension == "htm")
			{
				ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
			}
			else if (ResourceExtension == "jpg")
			{
				ReturnValue.Type = MBSockets::HTTPDocumentType::jpg;
			}
			else if (ResourceExtension == "ts")
			{
				ReturnValue.Type = MBSockets::HTTPDocumentType::ts;
			}
			else if (ResourceExtension == "m3u8")
			{
				ReturnValue.Type = MBSockets::HTTPDocumentType::m3u8;
			}
			else
			{
				std::cout << ResourcePath.substr(ResourcePath.find_last_of(".")) << std::endl;
			}
			ReturnValue.DocumentData = LoadWholeFile(ResourcePath);
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
				std::filesystem::path ActualResourcePath = std::filesystem::current_path().concat("/" + ResourceToGet);
				//std::cout << ResourceToGet << std::endl;
				MBSockets::HTTPDocument DocumentToSend;
				if (std::filesystem::exists(ActualResourcePath))
				{
					if (ResourceToGet == ResourcesPath)
					{
						DocumentToSend = AssociatedServer->GetResource(ResourcesPath + "index.htm");
						ConnectedClient->SendHTTPDocument(DocumentToSend);
					}
					else
					{
						//väldigt ful undantagsfall för att få acme protokollet att fungera
						std::string ChallengeFolder = "./ServerResources/MrBoboGet/HTMLResources/.well-known/acme-challenge/";
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
							DocumentToSend = AssociatedServer->GetResource(ResourceToGet);
							ConnectedClient->SendHTTPDocument(DocumentToSend);
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