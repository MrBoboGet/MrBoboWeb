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

		//borde finnas en variabel för att vissa att den är invalid
		std::string GetLastError()
		{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
			return(std::to_string(WSAGetLastError()));
#elif __linux__
			return(std::string(strerror(errno)));
#endif
		}
	public:
		//returnar en int för eventuellt framtida error hantering
		int SendData(const char* DataPointer, int DataLength)
		{
			int TotalDataSent = 0;
			while (TotalDataSent != DataLength)
			{
				ErrorResults = send(ConnectedSocket, DataPointer, DataLength, 0);
				if (ErrorResults == MBSocketError())
				{
					std::cout << "send failed "<<GetLastError();
					MBCloseSocket(ConnectedSocket);
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
				std::cout << "Get ip failed: " << GetLastError();
			}
			char ActualAdress[100];
			unsigned long DataReturned = 100;
			//ErrorResults = WSAAddressToStringA((SOCKADDR*)&AddresData, sizeof(SOCKADDR), NULL, ActualAdress, &DataReturned);
			//TODO fixa så det här fungerar
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
				printf("Connection closed\n");
				return(ErrorResults);
			}
			else
			{
				std::cout << "recv failed: " << GetLastError();
				return(ErrorResults);
			}
		}
		std::string GetNextRequestData()
		{
			int InitialBufferSize = 1000;
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
				//error grejer, helst våra egna också
				printf("getaddrinfo failed: %d\n", ErrorResults);
				std::cout << Adress << std::endl;
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
				std::cout << "Error at socket(): " << GetLastError();
				freeaddrinfo(result);
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
				std::cout << "UDP send data failed with error: " << GetLastError() << std::endl;
				freeaddrinfo(result);
				MBCloseSocket(ConnectedSocket);
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
				std::cout << "Bind failed with error: " << GetLastError() << std::endl;
				freeaddrinfo(result);
				MBCloseSocket(ConnectedSocket);
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
			LengthOfDataRecieved = recvfrom(ConnectedSocket, Buffer, MaxRecieveSize,0,nullptr,0);
			if (LengthOfDataRecieved == MBSocketError())
			{
				std::cout << "UDPgetdata failed with error: " << GetLastError() << std::endl;
				freeaddrinfo(result);
				MBCloseSocket(ConnectedSocket);
			}
			std::string ReturnValue(Buffer, LengthOfDataRecieved);
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
				std::cout << "listen in UDP socket failed with error: " << GetLastError() << std::endl;
				MBCloseSocket(ConnectedSocket);
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
				std::cout << "Error Att connecta "<<ErrorResults << std::endl;
				MBCloseSocket(ConnectedSocket);
				ConnectedSocket = MBInvalidSocket;
			}
			// freeaddrinfo(result);
			// Should really try the next address returned by getaddrinfo
			// if the connect call failed
			// But for this simple example we just free the resources
			// returned by getaddrinfo and print an error message

			// vi vill nog egentligen spara detta så vi inte behöver göra det flera gånger
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
				//error grejer, helst våra egna också
				printf("getaddrinfo failed: %d\n", ErrorResults);
				std::cout << Adress << std::endl;
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
				std::cout << "Error at socket(): " << GetLastError() << std::endl;
				freeaddrinfo(result);
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
	public:
		int Bind()
		{
			ErrorResults = bind(ListenerSocket, result->ai_addr, (int)result->ai_addrlen);
			if (ErrorResults == MBSocketError()) {
				std::cout << "bind failed with error: " << GetLastError() << std::endl;
				freeaddrinfo(result);
				MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		int Listen()
		{
			ErrorResults = listen(ListenerSocket, SOMAXCONN);
			if (ErrorResults == MBSocketError())
			{
				std::cout << "listen failed with error: " << GetLastError() << std::endl;
				MBCloseSocket(ConnectedSocket);
			}
			return(0);
		}
		void TransferConnectedSocket(ServerSocket& OtherSocket)
		{
			OtherSocket.ConnectedSocket = ConnectedSocket;
			ConnectedSocket = MBInvalidSocket;
		}
		int Accept()
		{
			ConnectedSocket = accept(ListenerSocket, NULL, NULL);
			if (ConnectedSocket == MBInvalidSocket) {
				std::cout << "accept failed with error: " << GetLastError() << std::endl;
				MBCloseSocket(ConnectedSocket);
			}
			return(0);
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
				//error grejer, helst våra egna också
				printf("getaddrinfo failed: %d\n", ErrorResults);
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
				std::cout << "error at socket(): " << GetLastError() << std::endl;
				freeaddrinfo(result);
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
			if (UsingHTTPS)
			{
				TLSConnectionHandler.SendDataAsRecord(DataToSend,this);
			}
			else
			{
				SendData(DataToSend.c_str(), DataToSend.size());
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
	
	public:
		HTTPServerSocket(std::string Port, TraversalProtocol TraversalProto) : ServerSocket(Port, TraversalProto)
		{

		}
		std::string  GetNextRequestData()
		{
			int InitialBufferSize = 1000;
			char* Buffer = (char*)malloc(InitialBufferSize);
			int MaxRecieveSize = InitialBufferSize;
			int LengthOfDataRecieved = 0;
			int TotalLengthOfData = 0;
			assert(Buffer != nullptr);
			assert(sizeof(Buffer) != 8 * InitialBufferSize);
			assert(Buffer == (char*)&Buffer[TotalLengthOfData]);
			while ((LengthOfDataRecieved = RecieveData(&Buffer[TotalLengthOfData],MaxRecieveSize)) > 0)
			{
				TotalLengthOfData += LengthOfDataRecieved;
				if (LengthOfDataRecieved == MaxRecieveSize)
				{
					MaxRecieveSize = 500;
					Buffer = (char*)realloc(Buffer,TotalLengthOfData+500);
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
		void SendDataAsHTTPBody(const std::string& Data)
		{
			std::string Body = "<html>\n<body>\n" + Data + "</body>\n</html>";
			std::string Request = GenerateRequest(Body);
			SendData(Request.c_str(), Request.size());
		}
		void SendHTTPBody(const std::string& Data)
		{
			std::string Request = GenerateRequest(Data);
			SendData(Request.c_str(), Request.size());
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
		//tänkt för användning av enbart vår worker, manages annars av threadpoolen
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
			//resettar tråden till ursrpungs tillståndet
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
			//om vi nu har en worker vi kan assigna arbete så gör vi det, annars så lägger vi till den i vår queue av grejer som behöver göras
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
			//vi har en wait variabel som vi väntar på och kollar huruvida våra grejer på standby är lika med antalet, då är det ju klart
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

			//vi poppar det längst ner på listan, executar, tar en ny
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
							//lägger till den i listan av grejer på standby
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

/*
			ProtocolForTraversal = TraversalProto;
			ZeroMemory(&hints, sizeof(hints));
			if (SocketT == SocketType::Client)
			{
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
			}
			else if (SocketT == SocketType::Server)
			{
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags = AI_PASSIVE;
			}
			if (SocketT == SocketType::Client)
			{
				ErrorResults = getaddrinfo(Adress.c_str(), Port.c_str(), &hints, &result);
			}
			else if (SocketT == SocketType::Server)
			{
				ErrorResults = getaddrinfo(NULL, Port.c_str(), &hints, &result);
			}
			if (ErrorResults != 0)
			{
				//error grejer, helst våra egna också
				printf("getaddrinfo failed: %d\n", ErrorResults);
			}

			ConnectedSocket = INVALID_SOCKET;

			// Attempt to connect to the first address returned by
			// the call to getaddrinfo
			ptr = result;
			// Create a SOCKET for connecting to server
			ConnectedSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectedSocket == INVALID_SOCKET)
			{
				//egen error hantering
				printf("Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(result);
			}
*/





/*Gamla grejer
			const int MAXHEADERSIZE = 1024;
			Head(Resource);
			char* Buffer = new char[MAXHEADERSIZE];
			int LengthOfData;
			int BodySize = 0;
			std::string HeaderContent;
			const int DefaultBodySize = 2024;
			while ((LengthOfData = RecieveData(Buffer, MAXHEADERSIZE)) > 0 && BodySize == 0)
			{
				//nu har vi fått headern i buffer
				HeaderContent = std::string(Buffer, LengthOfData);
				std::string ContentLength = MBSockets::GetHeaderValue("Content-Length", HeaderContent);
				if(RequestType == "HEAD")
				{
					delete[] Buffer;
					return(HeaderContent);
				}
				if (ContentLength == "")
				{
					if (MBSockets::GetHeaderValue("Location",HeaderContent) != "")
					{
						delete[] Buffer;
						return(HeaderContent);
					}
					else
					{
						BodySize = DefaultBodySize;
					}
				}
				else
				{
					BodySize = std::stoi(ContentLength);
				}
				break;
			}
			std::cout << HeaderContent.size() << std::endl;
			//nu har vi exakt hur lång vår request är, headerns storlek + bodysize
			int RequestSize = HeaderContent.size() + BodySize;
			int TotalSize = RequestSize;
			int TotalRecievedSize = 0;
			delete[] Buffer;
			bool TransferIsChunked = false;
			Buffer = (char*)malloc(RequestSize);
			if (RequestType == "GET")
			{
				Get(Resource);
			}
			while ((LengthOfData = RecieveData(&Buffer[TotalRecievedSize], RequestSize)) > 0)
			{
				//vi kollar huruvida vi får att datan kommer som chunked eller inte, gör den det behöver vi om allokera vår buffer data
				/*
				std::string CurrentData = std::string(Buffer,)
				if (MBSockets::GetHeaderValue("Transfer-Encoding",std::string(Buffer, HeaderContent.size())) == "chunked")
				{
					//vi reallocatar minnet så vi får plats med resten
					//första raden med ett dubbel \n\n, sen börjar bodyn

					//nu vill vi ha hur mycket data som behövs
					int Position =
				}
				
if (LengthOfData == WSAEMSGSIZE)
{
	//assert(false);
}
TotalRecievedSize += LengthOfData;
/*
if (TransferIsChunked == false)
{
	HeaderContent = std::string(Buffer, LengthOfData);
	std::string TransferEncoding = MBSockets::GetHeaderValue("Transfer-Encoding", HeaderContent);
	ReplaceAll(&TransferEncoding, "\r", "");
	if (TransferEncoding == "chunked")
	{
		TransferIsChunked = true;
	}
}

if (LengthOfData == RequestSize)
{
	RequestSize = 500;
	Buffer = (char*)realloc(Buffer, TotalRecievedSize + 500);
	continue;
}
/*
if (TransferEncoding == "chunked" || TransferIsChunked)
{
	TransferIsChunked = true;
	int CRLNPosition = HeaderContent.find("\r");
	int StartOfBody = HeaderContent.find("<");
	std::string SizeInHexString = HeaderContent.substr(CRLNPosition + 2, StartOfBody - (CRLNPosition + 2));
	int SizeInBytes = HexToDec(SizeInHexString);
	RequestSize = SizeInBytes;
	TotalSize += RequestSize;
	realloc(Buffer, TotalSize);
	continue;
}

break;
			}
			std::string ärde = std::string(Buffer, TotalRecievedSize);
			free(Buffer);
			return(ReturVärde);
*/