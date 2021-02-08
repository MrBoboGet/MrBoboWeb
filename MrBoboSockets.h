#pragma once
//operativ system specifika grejer
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#include <winsock2.h>
	#include <ws2tcpip.h>
//linux
#elif __linux__
	#include <errno.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	//#include <stdatomic.h>
	//#include <sys\types.h>
#endif
#include <string>
#include <string.h>
#include <stdio.h>

#include <thread>
#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <mutex>
#include <functional>
#include <assert.h>
#include <StringGrejer.h>
#include <condition_variable>
#include <MrPostOGet/TLSHandler.h>
#include <math.h>
#if defined(WIN32)
typedef SOCKET MB_OS_Socket;
#elif defined(__linux__)
typedef int MB_OS_Socket;
#endif

namespace MBSockets
{
	inline void Init()
	{
#if defined(WIN32)
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return;
		}
#endif
		return;
	}
#if defined(WIN32)
	inline int MBCloseSocket(int SocketToClose)
	{
		closesocket(SocketToClose);
		return(0);
	}
#elif defined(__linux__)
	inline int MBCloseSocket(int SocketToClose)
	{
		close(SocketToClose);
		return(0);
	}
#endif
	inline int MBSocketError()
	{
#if defined(WIN32)
		return(SOCKET_ERROR);
#elif defined(__linux__)
		return(-1);
#endif
	}
#if defined(WIN32)
	const unsigned int MBInvalidSocket = INVALID_SOCKET;
#elif defined(__linux__)
	const int MBInvalidSocket = -1;
#endif
	enum class TraversalProtocol
	{
		TCP
	};
	enum class SocketType
	{
		Server,
		Client
	};
	enum class MBSocketErrors :uint8_t
	{
		fatal,
		nonfatal
	};
	class Socket
	{
	protected:
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		MB_OS_Socket ConnectedSocket = MBInvalidSocket;
		int ErrorResults = 0;
		//SocketType TypeOfSocket;
		TraversalProtocol ProtocolForTraversal;
		bool Invalid = false;
		bool ConnectionClosed = false;
		std::string LastErrorMessage = "";
		//borde finnas en variabel f�r att vissa att den �r invalid
		std::string GetLastError()
		{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
			return(std::to_string(WSAGetLastError()));
#elif __linux__
			return(std::string(strerror(errno)));
#endif
		}
		void HandleError(std::string const& ErrorMessage, bool IsLethal)
		{
			//DEBUG GREJER
			std::cout << ErrorMessage << std::endl;
			LastErrorMessage = ErrorMessage;
			if (IsLethal == true)
			{
				Invalid = true;
			}
		}
	private:
	public:
		bool IsValid()
		{
			return(!Invalid);
		}
		bool IsConnected()
		{
			return(!ConnectionClosed);
		}
		//returnar en int f�r eventuellt framtida error hantering
		int SendData(const char* DataPointer, int DataLength)
		{
			int TotalDataSent = 0;
			while (TotalDataSent != DataLength)
			{
				ErrorResults = send(ConnectedSocket, DataPointer, DataLength, 0);
				if (ErrorResults == MBSocketError())
				{
					HandleError("send failed with error: "+GetLastError(),true);
					return(0);
				}
				TotalDataSent += ErrorResults;
			}
			return(0);
		}
		std::string GetIpOfConnectedSocket()
		{
			sockaddr_in  AddresData;
			AddresData.sin_family = AF_INET;
			int SizeOfData = sizeof(sockaddr_in);
			ErrorResults = getpeername(ConnectedSocket, (sockaddr*)&AddresData, (socklen_t*)&SizeOfData);
			if (ErrorResults == MBSocketError())
			{
				HandleError("Get ip failed: "+ GetLastError(),false);
				return("");
			}
			char ActualAdress[100];
			unsigned long DataReturned = 100;
			//ErrorResults = WSAAddressToStringA((SOCKADDR*)&AddresData, sizeof(SOCKADDR), NULL, ActualAdress, &DataReturned);
			//TODO fixa s� det h�r fungerar
			std::string Result = inet_ntoa(AddresData.sin_addr);
			return(std::string(ActualAdress));
		}
		int RecieveData(char* Buffer,int BufferSize)
		{
			ErrorResults = recv(ConnectedSocket, Buffer, BufferSize, 0);
			if (ErrorResults > 0)
			{
				//printf("Bytes received: %d\n", ErrorResults);
				return(ErrorResults);
			}
			else if (ErrorResults == 0)
			{
				//printf("Connection closed\n");
				ConnectionClosed = true;
				return(ErrorResults);
			}
			else
			{
				HandleError("recv failed: " + GetLastError(),true);
				return(ErrorResults);
			}
		}
		std::string GetNextRequestData()
		{
			//int InitialBufferSize = 1000;
			int InitialBufferSize = 16500;
			char* Buffer = (char*)malloc(InitialBufferSize);
			int MaxRecieveSize = InitialBufferSize;
			int LengthOfDataRecieved = 0;
			int TotalLengthOfData = 0;
			//assert(Buffer != nullptr);
			//assert(sizeof(Buffer) != 8 * InitialBufferSize);
			//assert(Buffer == (char*)&Buffer[TotalLengthOfData]);
			while ((LengthOfDataRecieved = RecieveData(&Buffer[TotalLengthOfData], MaxRecieveSize)) > 0)
			{
				TotalLengthOfData += LengthOfDataRecieved;
				if (LengthOfDataRecieved == MaxRecieveSize)
				{
					MaxRecieveSize = 500;
					Buffer = (char*)realloc(Buffer, TotalLengthOfData + 500);
					assert(Buffer != nullptr);
				}
				else
				{
					break;
				}
			}
			std::string ReturnValue(Buffer, TotalLengthOfData);
			free(Buffer);
			return(ReturnValue);
		}
		Socket()
		{

		}
		~Socket()
		{
			freeaddrinfo(result);
			MBCloseSocket(ConnectedSocket);
		}
		
	};
	class UDPSocket : public Socket
	{
	public:
		UDPSocket(std::string Adress, std::string Port, TraversalProtocol TraversalProto)
		{
			ProtocolForTraversal = TraversalProto;

			memset(&hints, 0,sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;

			ErrorResults = getaddrinfo(Adress.c_str(), Port.c_str(), &hints, &result);
			if (ErrorResults != 0)
			{
				//error grejer, helst v�ra egna ocks�
				HandleError("getaddrinfo on adress: " +Adress+ " failed with error: "+ GetLastError(),true);
				return;
			}

			ConnectedSocket = MBInvalidSocket;
			// Attempt to connect to the first address returned by
			// the call to getaddrinfo
			ptr = result;
			// Create a SOCKET for connecting to server
			ConnectedSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectedSocket == MBInvalidSocket)
			{
				//egen error hantering
				HandleError("Error at socket(): " + GetLastError(),true);
				//freeaddrinfo(result);
				return;
			}
		}
		void UDPSendData(std::string DataToSend, std::string HostAdress, int PortNumber)
		{
			sockaddr_in RecvAddr;
			RecvAddr.sin_family = AF_INET;
			RecvAddr.sin_port = htons(PortNumber);
			RecvAddr.sin_addr.s_addr = inet_addr(HostAdress.c_str());
			ErrorResults = sendto(ConnectedSocket, DataToSend.c_str(), DataToSend.size(), 0, (sockaddr*)&RecvAddr, sizeof(sockaddr_in));
			if (ErrorResults == MBSocketError()) 
			{
				HandleError("UDP send data failed with error: " + GetLastError(),false);
				//freeaddrinfo(result);
				//MBCloseSocket(ConnectedSocket);
			}
		}
		int Bind(int PortToAssociateWith)
		{
			sockaddr_in service;
			service.sin_family = AF_INET;
			service.sin_addr.s_addr = htonl(INADDR_ANY);
			service.sin_port = htons(PortToAssociateWith);
			ErrorResults = bind(ConnectedSocket, (sockaddr*)&service, sizeof(service));
			//ErrorResults = bind(ConnectedSocket, result->ai_addr, (int)result->ai_addrlen);
			if (ErrorResults == MBSocketError()) {
				HandleError("Bind failed with error: " + GetLastError(),true);
				//freeaddrinfo(result);
				//MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		std::string UDPGetData()
		{
			int InitialBufferSize = 65535;
			char* Buffer = (char*)malloc(InitialBufferSize);
			int MaxRecieveSize = InitialBufferSize;
			int LengthOfDataRecieved = 0;
			//assert(Buffer != nullptr);
			//assert(sizeof(Buffer) != 8 * InitialBufferSize);
			//assert(Buffer == (char*)&Buffer[TotalLengthOfData]);
			std::string ReturnValue = "";
			LengthOfDataRecieved = recvfrom(ConnectedSocket, Buffer, MaxRecieveSize,0,nullptr,0);
			if (LengthOfDataRecieved == MBSocketError())
			{
				HandleError("UDPgetdata failed with error: " + GetLastError(),false);
				//freeaddrinfo(result);
				//MBCloseSocket(ConnectedSocket);
			}
			else
			{
				ReturnValue = std::string(Buffer, LengthOfDataRecieved);
			}
			free(Buffer);
			return(ReturnValue);
		}
		void UDPMakeSocketNonBlocking(float SecondsToWait = 0.5)
		{
			struct timeval read_timeout;
			read_timeout.tv_sec = SecondsToWait-std::fmod(SecondsToWait, 1);
			read_timeout.tv_usec = std::fmod(SecondsToWait, 1);
			setsockopt(ConnectedSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&read_timeout, sizeof read_timeout);
		}
		void Listen(std::string PortNumber)
		{
			ErrorResults = listen(ConnectedSocket, SOMAXCONN);
			if (ErrorResults == MBSocketError())
			{
				HandleError("listen in UDP socket failed with error: " + GetLastError(),false);
				//MBCloseSocket(ConnectedSocket);
				//WSACleanup();
			}
		}
	};
	class ConnectSocket : public Socket
	{
	private:
	public:
		std::string HostName;
		int Connect()
		{
			ErrorResults = connect(ConnectedSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (ErrorResults == MBSocketError())
			{
				HandleError("Error Att connecta "+GetLastError(),false);
				//MBCloseSocket(ConnectedSocket);
				//ConnectedSocket = MBInvalidSocket;
			}
			// freeaddrinfo(result);
			// Should really try the next address returned by getaddrinfo
			// if the connect call failed
			// But for this simple example we just free the resources
			// returned by getaddrinfo and print an error message

			// vi vill nog egentligen spara detta s� vi inte beh�ver g�ra det flera g�nger
			return(0);
		}
		ConnectSocket(std::string Adress, std::string Port, TraversalProtocol TraversalProto)
		{
			HostName = Adress;
			ProtocolForTraversal = TraversalProto;

			memset(&hints, 0,sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			ErrorResults = getaddrinfo(Adress.c_str(), Port.c_str(), &hints, &result);
			if (ErrorResults != 0)
			{
				//error grejer, helst v�ra egna ocks�
				HandleError("getaddrinfo with adress "+Adress+ " failed: "+ GetLastError(),true);
				//std::cout << Adress << std::endl;
				return;
			}

			ConnectedSocket = MBInvalidSocket;
			// Attempt to connect to the first address returned by
			// the call to getaddrinfo
			ptr = result;
			// Create a SOCKET for connecting to server
			ConnectedSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectedSocket == MBInvalidSocket)
			{
				//egen error hantering
				HandleError("Error at socket(): " + GetLastError(),true);
				//freeaddrinfo(result);
			}
		}
		~ConnectSocket()
		{

		}

	};
	class ServerSocket : public Socket
	{
	private:
		MB_OS_Socket ListenerSocket = MBInvalidSocket;
	protected:
		TLSHandler SocketTlsHandler = TLSHandler();
		//bool SecureConnectionEstablished = false;
	public:
		int Bind()
		{
			ErrorResults = bind(ListenerSocket, result->ai_addr, (int)result->ai_addrlen);
			if (ErrorResults == MBSocketError()) {
				HandleError("bind failed with error: " + GetLastError(),false);
				//freeaddrinfo(result);
				//MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		int Listen()
		{
			ErrorResults = listen(ListenerSocket, SOMAXCONN);
			if (ErrorResults == MBSocketError())
			{
				HandleError("listen failed with error: " + GetLastError(),false);
				//MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		void TransferConnectedSocket(ServerSocket& OtherSocket)
		{
			OtherSocket.ConnectedSocket = ConnectedSocket;
			ConnectedSocket = MBInvalidSocket;
			OtherSocket.SocketTlsHandler = SocketTlsHandler;
			SocketTlsHandler = TLSHandler();
		}
		int Accept()
		{
			ConnectedSocket = accept(ListenerSocket, NULL, NULL);
			if (ConnectedSocket == MBInvalidSocket) {
				HandleError("accept failed with error: " + GetLastError(),true);
				//MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		MBError EstablishSecureConnection()
		{
			MBError ReturnValue(false);
			try
			{
				ReturnValue = SocketTlsHandler.EstablishHostTLSConnection(this);
			}
			catch (const std::exception&)
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "Unknown error in establishing TLS connection";
			}
			if (!ReturnValue)
			{
				//om det fuckade vill vi reseta vårt tls object
				SocketTlsHandler = TLSHandler();
			}
			return(ReturnValue);
		}
		ServerSocket(std::string Port, TraversalProtocol TraversalProto)
		{
			ProtocolForTraversal = TraversalProto;
			memset(&hints, 0,sizeof(hints));

			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			ErrorResults = getaddrinfo(NULL, Port.c_str(), &hints, &result);
			if (ErrorResults != 0)
			{
				//error grejer, helst v�ra egna ocks�
				HandleError("getaddrinfo failed: "+ GetLastError(),true);
				return;
			}

			ListenerSocket = MBInvalidSocket;
			// Attempt to connect to the first address returned by
			// the call to getaddrinfo
			ptr = result;
			// Create a SOCKET for connecting to server
			ListenerSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ListenerSocket == MBInvalidSocket)
			{
				//egen error hantering
				HandleError("error at socket(): " + GetLastError(),true);
				//freeaddrinfo(result);
			}
		}
		~ServerSocket()
		{
			MBCloseSocket(ListenerSocket);
		}
	};
	inline std::string GetHeaderValue(std::string Header, const std::string& HeaderContent)
	{
		int HeaderPosition = HeaderContent.find(Header + ": ");
		int FirstEndlineAfterContentPos = HeaderContent.find("\n", HeaderPosition);
		if (HeaderPosition == HeaderContent.npos)
		{
			return("");
		}
		else
		{
			return(HeaderContent.substr(HeaderPosition + Header.size() + 2, FirstEndlineAfterContentPos - (HeaderPosition + Header.size() + 2)));
		}
	}
	inline int HexToDec(std::string NumberToConvert)
	{
		int ReturnValue = 0;
		for (size_t i = 0; i < NumberToConvert.size(); i++)
		{
			ReturnValue += std::stoi(NumberToConvert.substr(NumberToConvert.size() - 1 - i, 1)) *pow(16, i);
		}
		return(ReturnValue);
	}
	enum class ApplicationProtocols
	{
		HTTP,
		HTTPS
	};
	class HTTPConnectSocket : public ConnectSocket
	{
	private:
		std::string URl;
		bool UsingHTTPS = false;
		TLSHandler TLSConnectionHandler = TLSHandler();
	public:
		int HTTPSendData(std::string DataToSend)
		{
			if (IsValid())
			{
				if (UsingHTTPS)
				{
					if (TLSConnectionHandler.ConnectionIsActive())
					{
						TLSConnectionHandler.SendDataAsRecord(DataToSend, this);
					}
					else
					{
						Invalid = true;
					}
				}
				else
				{
					SendData(DataToSend.c_str(), DataToSend.size());
				}
			}
			return(0);
		}
		std::string HTTPGetData()
		{
			if (UsingHTTPS == false)
			{
				return(GetNextRequestData());
			}
			else
			{
				return(TLSConnectionHandler.GetApplicationData(this));
			}
		}
		int Get(std::string Resource = "")
		{
			std::string Meddelandet = "GET /"+Resource +" "+ "HTTP/1.1\nHost: " + URl+"\n"+"accept-encoding: identity"+"\n"+"\n";
			//SendData(Meddelandet.c_str(), Meddelandet.length());
			HTTPSendData(Meddelandet);
			return(0);
		}
		int Head(std::string Resource = "")
		{
			std::string Meddelandet = "HEAD /" + Resource + " " + "HTTP/1.1\nHost: " + URl + "\n" + "accept-encoding: identity" + "\n" + "\n";
			//SendData(Meddelandet.c_str(), Meddelandet.length());
			HTTPSendData(Meddelandet);
			return(0);
		}
		std::string GetDataFromRequest(const std::string& RequestType, std::string Resource)
		{
			if (RequestType == "HEAD")
			{
				Head(Resource);
			}
			else if (RequestType == "GET")
			{
				Get(Resource);
			}
			std::string ReturnValue = HTTPGetData();
			return(ReturnValue);
		}

		HTTPConnectSocket(std::string URL, std::string Port, TraversalProtocol TraversalProto,ApplicationProtocols ApplicationProtocol = ApplicationProtocols::HTTP) : ConnectSocket(URL,Port,TraversalProto)
		{
			this->URl = URL;
			if (ApplicationProtocol == ApplicationProtocols::HTTPS)
			{
				UsingHTTPS = true;
			}
		}
		void EstablishSecureConnetion()
		{
			TLSConnectionHandler.EstablishTLSConnection(this);
		}
		~HTTPConnectSocket()
		{

		}
	};
	inline std::string GetRequestType(const std::string& RequestData)
	{
		int FirstSpace = RequestData.find(" ");
		return(RequestData.substr(0, FirstSpace));
	}
	inline std::string GetReqestResource(const std::string& RequestData)
	{
		int FirstSlashPos = RequestData.find("/");
		int FirstSpaceAfterSlash = RequestData.find(" ", FirstSlashPos);
		return(RequestData.substr(FirstSlashPos + 1, FirstSpaceAfterSlash - FirstSlashPos - 1));	
	}
	enum class HTTPDocumentType
	{
		OctetString,
		HTML,
		png,
		jpg,
		json,
		Null
	};
	struct HTTPDocument
	{
		HTTPDocumentType Type = HTTPDocumentType::Null;
		std::string DocumentData;
	};
	inline std::string GenerateRequest(HTTPDocument const& DocumentToSend)
	{
		std::string Request = "";
		Request += "HTTP/1.1 200 OK\n";
		Request += "Content-Type: ";
		if(DocumentToSend.Type == HTTPDocumentType::OctetString)
		{
			Request += "application/octet-stream\n";
		}
		else if(DocumentToSend.Type == HTTPDocumentType::HTML)
		{
			Request += "text/html\n";
		}
		else if (DocumentToSend.Type == HTTPDocumentType::png)
		{
			Request += "image/png";
		}
		else if (DocumentToSend.Type == HTTPDocumentType::jpg)
		{
			Request += "image/jpg";
		}
		else if (DocumentToSend.Type == HTTPDocumentType::json)
		{
			Request += "application/json";
		}
		Request += "Accept-Ranges: bytes\n";
		Request += "Content-Length: " + std::to_string(DocumentToSend.DocumentData.size()) + "\n\r\n";
		Request += DocumentToSend.DocumentData;
		return(Request);
	}
	inline std::string GenerateRequest(const std::string& HTMLBody)
	{
		/*
		HTTP/1.1 200 OK
		Content-Type: text/html
		Accept-Ranges: bytes
		//Vary: Accept-Encoding
		Content-Length: 390
		*/
		std::string Request = "HTTP/1.1 200 OK\nContent-Type: text/html\nAccept-Ranges: bytes\nContent-Length: " + std::to_string(HTMLBody.size()) + "\n\r\n" + HTMLBody;
		return(Request);
	}
	class HTTPServerSocket : public ServerSocket
	{
	private:
		void SendWithTls(std::string const& DataToSend)
		{
			if (IsValid())
			{
				if (SocketTlsHandler.EstablishedSecureConnection() == true)
				{
					if (SocketTlsHandler.ConnectionIsActive())
					{
						SocketTlsHandler.SendDataAsRecord(DataToSend, this);
					}
					else
					{
						Invalid = true;	
					}
				}
				else
				{
					SendData(DataToSend.c_str(), DataToSend.size());
				}
			}
		}
	public:
		HTTPServerSocket(std::string Port, TraversalProtocol TraversalProto) : ServerSocket(Port, TraversalProto)
		{

		}
		std::string  GetNextRequestData()
		{
			std::string ReturnValue = "";
			if (!SocketTlsHandler.EstablishedSecureConnection())
			{
				int InitialBufferSize = 1000;
				char* Buffer = (char*)malloc(InitialBufferSize);
				int MaxRecieveSize = InitialBufferSize;
				int LengthOfDataRecieved = 0;
				int TotalLengthOfData = 0;
				assert(Buffer != nullptr);
				assert(sizeof(Buffer) != 8 * InitialBufferSize);
				assert(Buffer == (char*)&Buffer[TotalLengthOfData]);
				while ((LengthOfDataRecieved = RecieveData(&Buffer[TotalLengthOfData], MaxRecieveSize)) > 0)
				{
					TotalLengthOfData += LengthOfDataRecieved;
					if (LengthOfDataRecieved == MaxRecieveSize)
					{
						MaxRecieveSize = 500;
						Buffer = (char*)realloc(Buffer, TotalLengthOfData + 500);
						assert(Buffer != nullptr);
					}
					else
					{
						break;
					}
				}
				ReturnValue = std::string(Buffer, TotalLengthOfData);
				free(Buffer);
			}
			else
			{
				ReturnValue = SocketTlsHandler.GetApplicationData(this);
				//Kollar lite att det stämmer osv
			}
			return(ReturnValue);
		}
		void SendDataAsHTTPBody(const std::string& Data)
		{
			std::string Body = "<html>\n<body>\n" + Data + "</body>\n</html>";
			std::string Request = GenerateRequest(Body);
			SendWithTls(Request);
		}
		void SendHTTPBody(const std::string& Data)
		{
			std::string Request = GenerateRequest(Data);
			SendWithTls(Request);
		}
		void SendFullResponse(std::string const& DataToSend)
		{
			SendWithTls(DataToSend);
		}
		void SendHTTPDocument(HTTPDocument const& DocumentToSend)
		{
			std::string DataToSend = GenerateRequest(DocumentToSend);
			SendWithTls(DataToSend);
		}
	};
	class ThreadPool;
	class Worker;
	void WorkerThreadFunction(Worker*);
	class Worker
	{
		friend class ThreadPool;
		friend void WorkerThreadFunction(Worker*);
	private:
		std::mutex ObjectValuesMutex;
		//t�nkt f�r anv�ndning av enbart v�r worker, manages annars av threadpoolen
		std::deque<std::function<void()>> TaskQueue = {};
		std::condition_variable CanExecuteConditional;
		std::thread WorkingThread;
		ThreadPool* ConnectedThreadPool = nullptr;
		bool AllTasksFinished = true;
		bool Running = false;
		bool ShouldStop = false;
		int WorkerID = -1;

		void StartWorking()
		{
			Running = true;
			WorkingThread = std::thread(WorkerThreadFunction,this);
		}
	public:
		Worker()
		{
			
		}
		Worker(ThreadPool* ThreadPoolToAttachTo)
		{
			ConnectedThreadPool = ThreadPoolToAttachTo;
		}
		~Worker()
		{
			Stop();
		}
		template<typename... Args>
		void AddTask(Args&&... args)
		{
			{
				std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
				TaskQueue.push_back(std::bind(std::forward<Args>(args)...));
			}
			CanExecuteConditional.notify_all();
		}
		void Start()
		{
			if (Running == true)
			{
				return;
			}
			else
			{
				StartWorking();
			}
		}
		void Stop()
		{
			{
				std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
				if (Running == false)
				{
					return;
				}
				ShouldStop = true;
				Running = false;
			}
			CanExecuteConditional.notify_all();
			WorkingThread.join();
			//resettar tr�den till ursrpungs tillst�ndet
			ShouldStop = false;
			AllTasksFinished = true;
		}
		bool InStandby()
		{
			std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
			if (Running && AllTasksFinished)
			{
				return(true);
			}
			else
			{
				return(false);
			}
		}
	};
	class ThreadPool
	{
		friend 	void WorkerThreadFunction(Worker* WorkerAttachedTo);
	private:
		std::vector<Worker> AvailableWorkers;
		std::deque<std::function<void()>> TaskQueue = {};
		std::condition_variable WaitForThreadsToFinishConditional;
		std::deque<Worker*> ThreadsOnStandby = std::deque<Worker*>(0);
		std::mutex ThreadPoolMutex;
		bool Running = false;
		//std::deque<
	public:
		ThreadPool(int NumberOfThreads)
		{
			AvailableWorkers = std::vector<Worker>(NumberOfThreads);
		}
		template<typename... Args>
		void AddTask(Args&&... args)
		{
			Worker* WorkerToAssignWork = nullptr;
			{
				std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
				if (ThreadsOnStandby.size() > 0)
				{
					WorkerToAssignWork = ThreadsOnStandby[0];
					ThreadsOnStandby.pop_front();
				}
			}
			//om vi nu har en worker vi kan assigna arbete s� g�r vi det, annars s� l�gger vi till den i v�r queue av grejer som beh�ver g�ras
			if (WorkerToAssignWork != nullptr)
			{
				WorkerToAssignWork->AddTask(args...);
			}
			else
			{
				std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
				TaskQueue.push_back(std::bind(std::forward<Args>(args)...));
			}
		}
		void WaitForThreadsToFinish()
		{
			//vi har en wait variabel som vi v�ntar p� och kollar huruvida v�ra grejer p� standby �r lika med antalet, d� �r det ju klart
			std::unique_lock<std::mutex> Locken(ThreadPoolMutex);
			while (ThreadsOnStandby.size() != AvailableWorkers.size())
			{
				WaitForThreadsToFinishConditional.wait(Locken);
			}
			int i = 1;
		}
		void Start()
		{
			if (Running)
			{
				return;
			}
			else
			{
				Running = true;
				for (int i = 0; i < AvailableWorkers.size(); i++)
				{
					std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
					AvailableWorkers[i].ConnectedThreadPool = this;
					if (TaskQueue.size() > 0)
					{
						AvailableWorkers[i].AddTask(TaskQueue.front());
						TaskQueue.pop_front();
						AvailableWorkers[i].Start();
					}
					else
					{
						AvailableWorkers[i].Start();
					}
				}
			}
		}
		void Stop()
		{
			for (int i = 0; i < AvailableWorkers.size(); i++)
			{
				AvailableWorkers[i].Stop();
			}
		}
		int NumberOfActiveWorkers()
		{
			std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
			return(AvailableWorkers.size() - ThreadsOnStandby.size());
		}
		int NumberOfQueuedTasks()
		{
			std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
			return(TaskQueue.size());
		}
		~ThreadPool()
		{
			Stop();
		}

	};
	inline void WorkerThreadFunction(Worker* WorkerAttachedTo)
	{
		while (true)
		{

			//vi poppar det l�ngst ner p� listan, executar, tar en ny
			std::function<void()> TaskToExecute;
			decltype(WorkerAttachedTo->TaskQueue) SeperateTaskQueue;
			{
				std::unique_lock<std::mutex> Locken(WorkerAttachedTo->ObjectValuesMutex);
				if (WorkerAttachedTo->TaskQueue.empty() && WorkerAttachedTo->ConnectedThreadPool != nullptr)
				{
					{
						std::lock_guard<std::mutex> TPLocken(WorkerAttachedTo->ConnectedThreadPool->ThreadPoolMutex);
						if (!WorkerAttachedTo->ConnectedThreadPool->TaskQueue.empty())
						{
							WorkerAttachedTo->TaskQueue.push_back(WorkerAttachedTo->ConnectedThreadPool->TaskQueue.front());
							WorkerAttachedTo->ConnectedThreadPool->TaskQueue.pop_front();
						}
						else
						{
							//l�gger till den i listan av grejer p� standby
							//assert(WorkerAttachedTo != nullptr);
							WorkerAttachedTo->ConnectedThreadPool->ThreadsOnStandby.push_back(WorkerAttachedTo);
						}
					}
					WorkerAttachedTo->ConnectedThreadPool->WaitForThreadsToFinishConditional.notify_all();

				}
				while (WorkerAttachedTo->TaskQueue.empty() && !WorkerAttachedTo->ShouldStop)
				{
					WorkerAttachedTo->AllTasksFinished = true;
					WorkerAttachedTo->CanExecuteConditional.wait(Locken);
				}
				if (WorkerAttachedTo->ShouldStop)
				{
					SeperateTaskQueue = WorkerAttachedTo->TaskQueue;
					WorkerAttachedTo->TaskQueue.clear();
				}
				else
				{
					WorkerAttachedTo->AllTasksFinished = false;
				}
			}
			if (WorkerAttachedTo->ShouldStop)
			{
				for (int i = 0; i < SeperateTaskQueue.size(); i++)
				{
					SeperateTaskQueue[i]();
				}
				return;
			}
			else
			{
				std::unique_lock<std::mutex> Locken(WorkerAttachedTo->ObjectValuesMutex);
				TaskToExecute = WorkerAttachedTo->TaskQueue.front();
				TaskToExecute();
				WorkerAttachedTo->TaskQueue.pop_front();
			}
		}
	}
};