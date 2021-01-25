#pragma once
#include <MrBoboSockets.h>
#include <TextReader.h>
namespace MrPostOGet 
{
	struct RequestHandler
	{
		bool (*RequestPredicate)(const std::string& RequestData);
		std::string (*RequestResponse)(const std::string& ResoureData,const std::string& ResourcePath);
	};
	void HandleConnectedSocket(MBSockets::HTTPServerSocket* ConnectedClient,std::vector<RequestHandler> RequestHandlers,std::string ResourcesPath)
	{
		std::string RequestData;
		//v�ldigt enkelt system, tar bara emot get requests, och skickar bara datan som kopplad till filepathen
 		while ((RequestData = ConnectedClient->GetNextRequestData()) != "")
		{
			//vi kollar om request handlersen har n�gon �sikt om datan, annars l�ter vi v�ran default getrequest handlar sk�ta allt 
			bool HandlerHasHandled = false;
			for (size_t i = 0; i < RequestHandlers.size(); i++)
			{
				if (RequestHandlers[i].RequestPredicate(RequestData))
				{
					//vi ska g�ra grejer med denna data, s� vi tar och skapar stringen som vi sen ska skicka
					std::string RequestResponse = RequestHandlers[i].RequestResponse(RequestData, ResourcesPath);
					ConnectedClient->SendHTTPBody(RequestResponse);
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
				std::string ResourceToGet = ResourcesPath + MBSockets::GetReqestResource(RequestData);
				std::filesystem::path ActualResourcePath = std::filesystem::current_path().concat("/"+ResourceToGet);
				std::cout<<ResourceToGet<<std::endl;
				if (std::filesystem::exists(ActualResourcePath))
				{
					if (ResourceToGet == ResourcesPath)
					{
						TextReader Data(ResourcesPath + "index.htm");
						std::string HTMLBody = "";
						for (int i = 0; i < Data.Size(); i++)
						{
							HTMLBody += Data[i] + "\n";
						}
						ConnectedClient->SendHTTPBody(HTMLBody);
					}
					else
					{
						std::string ChallengeFolder = "./ServerResources/MrBoboGet/HTMLResources/.well-known/acme-challenge/";
						if(ResourceToGet.substr(0,ChallengeFolder.size()) == ChallengeFolder)
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
							TextReader Data(ResourceToGet);
							std::string HTMLBody = "";
							for (int i = 0; i < Data.Size(); i++)
							{
								HTMLBody += Data[i] + "\n";
							}
							ConnectedClient->SendHTTPBody(HTMLBody);
						}
					}
				}
				else
				{
					ConnectedClient->SendDataAsHTTPBody("Bad Request");
				}
			}
		}
		delete ConnectedClient;
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
				NumberOfConnections += 1;
				ServerSocketen->Accept();
				//TODO detecta huruvida det är http eller https
				ServerSocketen->EstablishSecureConnection();
				MBSockets::HTTPServerSocket* NewSocket = new MBSockets::HTTPServerSocket(std::to_string(Port), MBSockets::TraversalProtocol::TCP);
				ServerSocketen->TransferConnectedSocket(*NewSocket);
				std::thread* NewThread = new std::thread(HandleConnectedSocket, NewSocket,ServerRequestHandlers, ContentPath);
				CurrentActiveThreads.push_back(NewThread);
				std::cout << NumberOfConnections << std::endl;
			}
		}
		~HTTPServer()
		{
		}
	};
}