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
		//väldigt enkelt system, tar bara emot get requests, och skickar bara datan som kopplad till filepathen
 		while ((RequestData = ConnectedClient->GetNextRequestData()) != "")
		{
			//vi kollar om request handlersen har någon åsikt om datan, annars låter vi våran default getrequest handlar sköta allt 
			bool HandlerHasHandled = false;
			for (size_t i = 0; i < RequestHandlers.size(); i++)
			{
				if (RequestHandlers[i].RequestPredicate(RequestData))
				{
					//vi ska göra grejer med denna data, så vi tar och skapar stringen som vi sen ska skicka
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
			//om våra handlers inte har handlat datan så gör vår default handler det. Denna funkar enbart för get, eftersom dem andra requestsen inte makar mycket sense i sammanhanget
			if (MBSockets::GetRequestType(RequestData) != "GET")
			{
				ConnectedClient->SendDataAsHTTPBody("Bad Request");
			}
			else
			{
				std::string ResourceToGet = ResourcesPath + MBSockets::GetReqestResource(RequestData);
				std::filesystem::path ActualResourcePath = std::filesystem::current_path().concat("/"+ResourceToGet);
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
						TextReader Data(ResourceToGet);
						std::string HTMLBody = "";
						for (int i = 0; i < Data.Size(); i++)
						{
							HTMLBody += Data[i] + "\n";
						}
						ConnectedClient->SendHTTPBody(HTMLBody);
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
			//huvud loopen som tar och faktiskt sköter hur allt funkar internt
			int NumberOfConnections = 0;
			std::vector<std::thread*> CurrentActiveThreads = {};
			ServerSocketen->Bind();
			ServerSocketen->Listen();
			while (true)
			{
				NumberOfConnections += 1;
				ServerSocketen->Accept();
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