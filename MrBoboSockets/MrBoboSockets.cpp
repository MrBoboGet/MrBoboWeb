#include "MrBoboSockets.h"
#include <MBUtility/MBStrings.h>
#include <MBMime/MBMime.h>
#include <filesystem>
//#include <MBUtility/MBErrorHandling.h>
//operativ system specifika grejer
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <winsock2.h>
#include <ws2tcpip.h>
//linux
#elif __linux__
#include <execinfo.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <csignal>
#include <signal.h>
//#include <stdatomic.h>
//#include <sys\types.h>
#endif

#ifdef __linux__
#define _FILE_OFFSET_BITS = 64
#include <sys/stat.h>
#endif
uint64_t MBGetFileSize(std::string const& PathToCheck)
{
#ifdef __linux__
	struct stat64 FileStats;
	stat64(PathToCheck.c_str(), &FileStats);
	//std::cout << size_t(FileStats.st_size) << std::endl;
	return(FileStats.st_size);
#else
	return(std::filesystem::file_size(PathToCheck));
#endif // __linux__
}

namespace MBSockets
{
#ifdef __linux__

    void handler(int sig) 
	{
		void *array[10];
		size_t size;

		// get void*'s for all entries on the stack
		size = backtrace(array, 10);

		// print out all the frames to stderr
		fprintf(stderr, "Error: signal %d:\n", sig);
		backtrace_symbols_fd(array, size, STDERR_FILENO);
		exit(1);
	}
#endif
	void Init()
	{
#if defined(WIN32) || defined(_WIN32)
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return;
		}
#else
		//signal(SIGPIPE, SIG_IGN);
		struct sigaction SIGPIPE_Handler;
		SIGPIPE_Handler.sa_flags = SA_RESTART;
		SIGPIPE_Handler.sa_handler = SIG_IGN;
		sigaction(SIGPIPE, &SIGPIPE_Handler, NULL);
		signal(SIGSEGV,handler);
#endif
		return;
	}
#if defined(WIN32) || defined(_WIN32)
	int MBCloseSocket(int SocketToClose)
	{
		closesocket(SocketToClose);
		return(0);
	}
#elif defined(__linux__)
	int MBCloseSocket(int SocketToClose)
	{
		close(SocketToClose);
		return(0);
	}
#endif
	int MBSocketError()
	{
#if defined(WIN32) || defined(_WIN32)
		return(SOCKET_ERROR);
#elif defined(__linux__)
		return(-1);
#endif
	}
#if defined(WIN32) || defined(_WIN32)
	const unsigned int MBInvalidSocket = INVALID_SOCKET;
#elif defined(__linux__)
	const int MBInvalidSocket = -1;
#endif



	//BEGIN UDPSocket
	//class UDPSocket : public Socket
	std::string UDPSocket::p_GetLastError()
	{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		return(std::to_string(WSAGetLastError()));
#elif __linux__
		return(std::string(strerror(errno)));
#endif
	}
	void UDPSocket::p_HandleError(std::string const& ErrorMessage, bool IsLethal)
	{
		//DEBUG GREJER
		std::cout << ErrorMessage << std::endl;
		m_LastErrorMessage = ErrorMessage;
		if (IsLethal == true)
		{
			m_Invalid = true;
		}
	}
    UDPSocket::UDPSocket(uint32_t Adress,uint16_t TargetPort,uint16_t LocalPort)
    {
        m_DestinationAddres = Adress;
        m_DestinationPort = TargetPort;
        m_LocalPort = LocalPort;

		m_UnderlyingHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_UnderlyingHandle == MBInvalidSocket)
		{
			p_HandleError("Error at socket(): " + p_GetLastError(), true);
			return;
		}
		sockaddr_in RecvAddr;
        std::memset(&RecvAddr,0,sizeof(sockaddr_in));
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(LocalPort);
        Bind(LocalPort);
		//auto BindResult = bind(m_UnderlyingHandle,(const sockaddr*)&RecvAddr,sizeof(RecvAddr));
		//if (BindResult != 0)
		//{
		//	p_HandleError("Error at socket(): " + p_GetLastError(), true);
		//	return;
		//}
    }
    UDPSocket::UDPSocket(uint16_t ListenPort)
    {
        m_LocalPort = ListenPort;

		m_UnderlyingHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_UnderlyingHandle == MBInvalidSocket)
		{
			p_HandleError("Error at socket(): " + p_GetLastError(), true);
			return;
		}
		sockaddr_in RecvAddr;
        std::memset(&RecvAddr,0,sizeof(sockaddr_in));
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(ListenPort);

		auto BindResult = bind(m_UnderlyingHandle,(const sockaddr*)&RecvAddr,sizeof(RecvAddr));
		if (BindResult != 0)
		{
			p_HandleError("Error at socket(): " + p_GetLastError(), true);
			return;
		}
    }
	UDPSocket::UDPSocket(std::string const& Adress, std::string const& Port)
	{
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		m_ErrorResult = getaddrinfo(Adress.c_str(), Port.c_str(), &hints, &result);
		if (m_ErrorResult != 0)
		{
			//error grejer, helst v�ra egna ocks�
			p_HandleError("getaddrinfo on adress: " + Adress + " failed with error: " + p_GetLastError(), true);
			return;
		}

		m_UnderlyingHandle = MBInvalidSocket;
		// Attempt to connect to the first address returned by
		// the call to getaddrinfo
		ptr = result;
		// Create a SOCKET for connecting to server
		m_UnderlyingHandle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		freeaddrinfo(result);
		if (m_UnderlyingHandle == MBInvalidSocket)
		{
			//egen error hantering
			p_HandleError("Error at socket(): " + p_GetLastError(), true);
			//freeaddrinfo(result);
			return;
		}
	}
	bool UDPSocket::IsValid()
	{
		return(!m_Invalid);
	}
	void UDPSocket::Close()
	{
		if (!m_SocketClosed)
		{
			MBCloseSocket(m_UnderlyingHandle);
			m_SocketClosed = true;
		}
	}
	void UDPSocket::UDPSendData(std::string const& DataToSend, std::string const& HostAdress, int PortNumber)
	{
		sockaddr_in RecvAddr;
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(PortNumber);
		RecvAddr.sin_addr.s_addr = inet_addr(HostAdress.c_str());
		m_ErrorResult = sendto(m_UnderlyingHandle, DataToSend.c_str(), DataToSend.size(), 0, (sockaddr*)&RecvAddr, sizeof(sockaddr_in));
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("UDP send data failed with error: " + p_GetLastError(), false);
			//freeaddrinfo(result);
			//MBCloseSocket(ConnectedSocket);
		}
	}
    void UDPSocket::SetDstIP(uint32_t DstIp)
    {
        m_DestinationAddres = DstIp;
    }
    void UDPSocket::SetDstPort(uint16_t DstPort)
    {
        m_DestinationPort = DstPort;
    }
    void UDPSocket::UDPSendData(const char* Data,size_t DataSize, uint32_t HostAdress,uint16_t Port)
    {
		sockaddr_in RecvAddr;
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(Port);
		RecvAddr.sin_addr.s_addr = htonl(HostAdress);
		m_ErrorResult = sendto(m_UnderlyingHandle, Data, DataSize, 0, (sockaddr*)&RecvAddr, sizeof(sockaddr_in));
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("UDP send data failed with error: " + p_GetLastError(), false);
			//freeaddrinfo(result);
			//MBCloseSocket(ConnectedSocket);
		}
    }
	int UDPSocket::Bind(int PortToAssociateWith)
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = htonl(INADDR_ANY);
		service.sin_port = htons(PortToAssociateWith);
		m_ErrorResult = bind(m_UnderlyingHandle, (sockaddr*)&service, sizeof(service));
		//ErrorResults = bind(ConnectedSocket, result->ai_addr, (int)result->ai_addrlen);
		if (m_ErrorResult == MBSocketError()) {
			p_HandleError("Bind failed with error: " + p_GetLastError(), true);
			//freeaddrinfo(result);
			//MBCloseSocket(ConnectedSocket);
		}
        if(PortToAssociateWith == 0)
        {
            m_LocalPort = GetBoundPort();
        }
		return(0);
	}
    uint16_t UDPSocket::GetBoundPort()
    {
        uint16_t ReturnValue;
        sockaddr_in Adress;
        socklen_t Length = sizeof(Adress);
        std::memset(&Adress,0,Length);
        auto Result = getsockname(m_UnderlyingHandle,(sockaddr*)&Adress,&Length);
        if(Result == MBSocketError())
        {
            p_HandleError("Error getting bound port: "+p_GetLastError(),false);
            return 0;
        }
        return ntohs(Adress.sin_port);
    }
	std::string UDPSocket::UDPGetData()
	{
		int InitialBufferSize = 65535;
		char* Buffer = (char*)malloc(InitialBufferSize);
		int MaxRecieveSize = InitialBufferSize;
		int LengthOfDataRecieved = 0;
		//assert(Buffer != nullptr);
		//assert(sizeof(Buffer) != 8 * InitialBufferSize);
		//assert(Buffer == (char*)&Buffer[TotalLengthOfData]);
		std::string ReturnValue = "";
		LengthOfDataRecieved = recvfrom(m_UnderlyingHandle, Buffer, MaxRecieveSize, 0, nullptr, 0);
		if (LengthOfDataRecieved == MBSocketError())
		{
			p_HandleError("UDPgetdata failed with error: " + p_GetLastError(), false);
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
	void UDPSocket::UDPMakeSocketNonBlocking(float SecondsToWait)
	{
		struct timeval read_timeout;
		read_timeout.tv_sec = SecondsToWait - std::fmod(SecondsToWait, 1);
		read_timeout.tv_usec = std::fmod(SecondsToWait, 1);
		setsockopt(m_UnderlyingHandle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&read_timeout, sizeof read_timeout);
	}
	void UDPSocket::Listen(std::string const& PortNumber)
	{
		m_ErrorResult = listen(m_UnderlyingHandle, SOMAXCONN);
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("listen in UDP socket failed with error: " + p_GetLastError(), false);
			//MBCloseSocket(ConnectedSocket);
			//WSACleanup();
		}
	}
    size_t UDPSocket::ReadMaxPacketSize() 
    {
        //Arbitrary AF
        return 500;
    }
    size_t UDPSocket::ReadPacket(void* OutBuffer,size_t BufferSize,double Timeout) 
    {
		auto Result = recvfrom(m_UnderlyingHandle, (char*)OutBuffer, BufferSize, 0, nullptr, 0);
        if (Result == MBSocketError())
        {
            p_HandleError("UDPgetdata failed with error: " + p_GetLastError(), false);
            return 0;
        }
        return Result;
    }
    size_t UDPSocket::WriteMaxPacketSize() 
    {
        //Arbitrary AF
        return 500;
    }
    void UDPSocket::WritePacket(const void* InBuffer,size_t BufferSize,double Timout) 
    {
		sockaddr_in RecvAddr;
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(m_DestinationPort);
		RecvAddr.sin_addr.s_addr = htonl(m_DestinationAddres);
		m_ErrorResult = sendto(m_UnderlyingHandle,(const char*)InBuffer ,BufferSize, 0, (sockaddr*)&RecvAddr, sizeof(sockaddr_in));
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("UDP send data failed with error: " + p_GetLastError(), false);
		}
    }
    size_t UDPSocket::ReadPacket(UDPSource& Source,void* OutBuffer,size_t BufferSize,double Timeout)
    {
        sockaddr_in SourceAdress;
        socklen_t SourceLen =  sizeof(SourceAdress);
		auto Result = recvfrom(m_UnderlyingHandle, (char*)OutBuffer, BufferSize, 0,(sockaddr*)&SourceAdress,&SourceLen);
        if (Result == MBSocketError())
        {
            p_HandleError("UDPgetdata failed with error: " + p_GetLastError(), false);
            return 0;
        }
        Source.Port = ntohs(SourceAdress.sin_port);
        Source.IP = ntohl(SourceAdress.sin_addr.s_addr);
        return Result;
    }
	//END UdpSocket


	//BEGIN OSSocket
	void OSSocket::p_Swap(OSSocket& SocketToSwapWith)
	{
		std::swap(m_UnderlyingHandle, SocketToSwapWith.m_UnderlyingHandle);
		std::swap(m_ErrorResult, SocketToSwapWith.m_ErrorResult);
		std::swap(m_OSSocketClosed, SocketToSwapWith.m_OSSocketClosed);
		std::swap(m_Invalid, SocketToSwapWith.m_Invalid);
		std::swap(m_LastErrorMessage, SocketToSwapWith.m_LastErrorMessage);
	}
	std::string OSSocket::p_GetLastError()
	{
		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
			return(std::to_string(WSAGetLastError()));
		#elif __linux__
			return(std::string(strerror(errno)));
		#endif
	}
	void OSSocket::p_HandleError(std::string const& ErrorMessage, bool IsLethal)
	{
		//DEBUG GREJER
		std::cout << ErrorMessage << std::endl;
		m_LastErrorMessage = ErrorMessage;
		if (IsLethal == true)
		{
			m_Invalid = true;
		}
	}
	void OSSocket::p_SendAllData(MB_OS_Socket ConnectedSocket, const void* DataToSend, size_t DataSize)
	{
		size_t TotalDataSent = 0;
		while (TotalDataSent != DataSize)
		{
			m_ErrorResult = send(m_UnderlyingHandle, ((const char*)DataToSend)+TotalDataSent, DataSize-TotalDataSent, 0);
			if (m_ErrorResult == MBSocketError())
			{
				p_HandleError("send failed with error: " + p_GetLastError(), true);
				break;
			}
			TotalDataSent += m_ErrorResult;
		}
	}
	std::string OSSocket::p_RecieveData(MB_OS_Socket ConnectedSocket, size_t MaxNumberOfBytes)
	{
		size_t BufferSize = std::min((size_t)4096 * 16, MaxNumberOfBytes);//totalt godtyckligt
		std::string ReturnValue = std::string(BufferSize,0);
		int LengthOfDataRecieved = 0;
		LengthOfDataRecieved = recv(m_UnderlyingHandle, ReturnValue.data(), BufferSize, 0);
		if (LengthOfDataRecieved < 0)
		{
			ReturnValue.resize(0);
			p_HandleError("Recieved negative error code on recv: " + p_GetLastError(), true);
		}
		else
		{
			ReturnValue.resize(LengthOfDataRecieved);
			if (LengthOfDataRecieved == 0)
			{
                m_Invalid = true;
			}
		}
		return(ReturnValue);
	}
	void OSSocket::p_CloseOSSocket()
	{
		m_Invalid = true;
		if (m_OSSocketClosed == false)
		{
			if (m_UnderlyingHandle != MBInvalidSocket)
			{
				MBCloseSocket(m_UnderlyingHandle);
			}
			m_OSSocketClosed = true;
			//DEBUG
			//std::cout << "Closed socket handle: " + std::to_string(m_UnderlyingHandle) << std::endl;
		}
	}
	OSSocket::OSSocket()
	{
		m_UnderlyingHandle = MBInvalidSocket;
	}
	OSSocket::~OSSocket()
	{
		p_CloseOSSocket();
	}
	//END OSSocket

	//BEGIN TCPClient
	TCPClient::TCPClient()
	{
		
	}
	void TCPClient::p_Swap(TCPClient& SocketToSwapWith)
	{
		std::swap(m_IsConnected, SocketToSwapWith.m_IsConnected);
		std::swap(m_Closed, SocketToSwapWith.m_Closed);
		std::swap(_m_ai_addrlen, SocketToSwapWith._m_ai_addrlen);
		std::swap(_m_addr, SocketToSwapWith._m_addr);
	}
	MBError TCPClient::Initialize(std::string const& Adress, std::string const& Port)
	{
		MBError ReturnValue = true;
		TCPClient SocketToSwap;
		OSSocket::p_Swap(SocketToSwap);
		TCPClient::p_Swap(SocketToSwap);

		struct addrinfo* result = NULL, * ptr = NULL, hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		m_ErrorResult = getaddrinfo(Adress.c_str(), Port.c_str(), &hints, &result);
		if (m_ErrorResult != 0)
		{
			//error grejer, helst v�ra egna ocks�
			p_HandleError("getaddrinfo with adress " + Adress + " failed: " + p_GetLastError(), true);
			//std::cout << Adress << std::endl;
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
			return ReturnValue;
		}

		m_UnderlyingHandle = MBInvalidSocket;
		// Attempt to connect to the first address returned by
		// the call to getaddrinfo
		ptr = result;
		// Create a SOCKET for connecting to server
		m_UnderlyingHandle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		_m_ai_addrlen = ptr->ai_addrlen;
		_m_addr = new sockaddr;
		*_m_addr = *(ptr->ai_addr);
		freeaddrinfo(result);
		if (m_UnderlyingHandle == MBInvalidSocket)
		{
			//egen error hantering
			p_HandleError("Error at socket(): " + p_GetLastError(), true);
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
			//freeaddrinfo(result);
		}
		return(ReturnValue);
	}
	TCPClient::TCPClient(std::string const& Adress, std::string const& Port)
	{
		Initialize(Adress, Port);
	}
	int TCPClient::Connect()
	{
		m_ErrorResult = connect(m_UnderlyingHandle, _m_addr, _m_ai_addrlen);
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("Error Att connecta " + p_GetLastError(), false);
		}
		else
		{
			m_IsConnected = true;
		}
		return(0);
	}
	bool TCPClient::IsValid()
	{
		return(!m_Invalid);
	}
	void TCPClient::Close()
	{
		p_CloseOSSocket();
		m_IsConnected = false;
	}

	bool TCPClient::IsConnected()
	{
		return(m_IsConnected);
	}
	std::string TCPClient::RecieveData(size_t MaxNumberOfBytes)
	{
		std::string ReturnValue = p_RecieveData(m_UnderlyingHandle);
		if (!IsValid())
		{
			m_IsConnected = false;
		}
		return(ReturnValue);
	}
	MBError TCPClient::SendData(const void* DataPointer, size_t DataLength)
	{
		MBError ReturnValue = true;
		p_SendAllData(m_UnderlyingHandle,DataPointer,DataLength);
		if (!IsValid())
		{
			m_IsConnected = false;
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
		}
		return(ReturnValue);
	}
	MBError TCPClient::SendData(std::string const& DataToSend)
	{
		return(SendData(DataToSend.data(), DataToSend.size()));
	}
	TCPClient::~TCPClient()
	{
		Close();
		delete _m_addr;
	}
	//

	//BEGIN TCPServer
	TCPServer::TCPServer()
	{
		m_ListenerClosed = MBInvalidSocket;
	}
	void TCPServer::p_Swap(TCPServer& SocketToSwapWith)
	{
		std::swap(m_IsConnected, SocketToSwapWith.m_IsConnected);
		std::swap(m_ListenerClosed, SocketToSwapWith.m_ListenerClosed);
		std::swap(_m_ai_addrlen, SocketToSwapWith._m_ai_addrlen);
		std::swap(_m_addr, SocketToSwapWith._m_addr);
	}
	void TCPServer::TransferConnectedSocket(TCPServer* ServerToRecieve)
	{
		OSSocket::p_Swap(*ServerToRecieve);
		ServerToRecieve->m_IsConnected = m_IsConnected;
		m_IsConnected = false;
	}
	MBError TCPServer::Initialize(std::string const& Port)
	{
		//utifall fi initializer en socket som redan existerar
		MBError ReturnValue = true;
		TCPServer SocketToSwap;
		OSSocket::p_Swap(SocketToSwap);
		TCPServer::p_Swap(SocketToSwap);

		m_ListenerSocket = MBInvalidSocket;
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		m_ErrorResult = getaddrinfo(NULL, Port.c_str(), &hints, &result);
		if (m_ErrorResult != 0)
		{
			//error grejer, helst v�ra egna ocks�
			p_HandleError("getaddrinfo failed: " + p_GetLastError(), true);
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
			return ReturnValue;
		}

		m_ListenerSocket = MBInvalidSocket;
		// Attempt to connect to the first address returned by
		// the call to getaddrinfo
		ptr = result;
		// Create a SOCKET for connecting to server
		m_ListenerSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		_m_ai_addrlen = ptr->ai_addrlen;
		_m_addr = new sockaddr;
		*_m_addr = *(ptr->ai_addr);
		freeaddrinfo(result);
		if (m_ListenerSocket == MBInvalidSocket)
		{
			//egen error hantering
			p_HandleError("error at socket(): " + p_GetLastError(), true);
			//freeaddrinfo(result);
		}
		else
		{
			//nu fixar vi specifika options, som bbland annat SO_REUSEADRR
			//TODO Detta var copy pastat från stack overflow, men kan det vara så att det faktiskt beror på endianessen av ens dator?
			int Enable = 1;
			m_ListenerClosed = false;
			m_ErrorResult = setsockopt(m_ListenerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&Enable, sizeof(int));
			if (m_ErrorResult < 0)
			{
				p_HandleError("Error at socket() when setting SO_REUSEADDR: " + p_GetLastError(), true);
			}
		}
		if (!IsValid())
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
		}
		return(ReturnValue);
	}
	TCPServer::TCPServer(std::string const& Port)
	{
		Initialize(Port);
	}
	int TCPServer::Bind()
	{
		m_ErrorResult = bind(m_ListenerSocket, _m_addr, _m_ai_addrlen);
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("bind failed with error: " + p_GetLastError(), false);
		}
		return(0);
	}
	int TCPServer::Listen()
	{
		m_ErrorResult = listen(m_ListenerSocket, SOMAXCONN);
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("listen failed with error: " + p_GetLastError(), false);
			//MBCloseSocket(ConnectedSocket);
		}
		return(0);
	}
	int TCPServer::Accept()
	{
		m_UnderlyingHandle = accept(m_ListenerSocket, NULL, NULL);
		if (m_UnderlyingHandle == MBInvalidSocket) {
			p_HandleError("accept failed with error: " + p_GetLastError(), true);
			//MBCloseSocket(ConnectedSocket);
			m_IsConnected = false;
			m_Invalid = true;
		}
		else
		{
			m_IsConnected = true;
		}
		return(0);
	}
	bool TCPServer::IsValid()
	{
		return(!m_Invalid);
	}
	void TCPServer::Close()
	{
		p_CloseOSSocket();
		m_IsConnected = false;
		if (m_ListenerClosed == false)
		{
			MBCloseSocket(m_ListenerSocket);
			m_ListenerClosed = true;
		}
	}
	bool TCPServer::IsConnected()
	{
		return(m_IsConnected);
	}
	std::string TCPServer::RecieveData(size_t MaxNumberOfBytes)
	{
		std::string ReturnValue = p_RecieveData(m_UnderlyingHandle, MaxNumberOfBytes);
		if (!IsValid())
		{
			m_IsConnected = false;
		}
		return(ReturnValue);
	}
	MBError TCPServer::SendData(const void* DataPointer, size_t DataLength)
	{
		MBError ReturnValue = true;
		p_SendAllData(m_UnderlyingHandle, DataPointer, DataLength);
		if (!IsValid())
		{
			m_IsConnected = false;
			ReturnValue = false;
			ReturnValue.ErrorMessage = p_GetLastError();
		}
		return(ReturnValue);
	}
	MBError TCPServer::SendData(std::string const& DataToSend)
	{
		return(SendData(DataToSend.data(), DataToSend.size()));
	}
	TCPServer::~TCPServer()
	{
		delete _m_addr;
		Close();
	}
	//END TCPServer
	//BEGIN TLSConnectSocket
	void TLSConnectSocket::p_Initialize(std::unique_ptr<ConnectSocket> NewSocket)
	{
		m_UnderlyingSocket = std::move(NewSocket);
	}
	TLSConnectSocket::TLSConnectSocket(std::unique_ptr<ConnectSocket> NewSocket)
	{
		p_Initialize(std::move(NewSocket));
	}

    MBError TLSConnectSocket::EstablishClientTLSConnection(std::string const& RemoteHostName)
    {
		MBError ReturnValue = true;
		if (m_UnderlyingSocket == nullptr)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "No internal ConnecSocket to connect";
			return(ReturnValue);
		}
		if (m_TLSHandler.EstablishedSecureConnection())
		{
			return(ReturnValue);
		}
        ReturnValue = m_TLSHandler.EstablishTLSConnection(m_UnderlyingSocket.get(),RemoteHostName);
		return(ReturnValue);
    }
    MBError TLSConnectSocket::EstablishHostTLSConnection(std::unique_ptr<DomainHandler> Retriever)
    {
		MBError ReturnValue = true;
		if (m_UnderlyingSocket == nullptr)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "No internal ConnecSocket to connect";
			return(ReturnValue);
		}
		if (m_TLSHandler.EstablishedSecureConnection())
		{
			return(ReturnValue);
		}
        ReturnValue = m_TLSHandler.EstablishHostTLSConnection(m_UnderlyingSocket.get(),std::move(Retriever));
		return(ReturnValue);
    }
	bool TLSConnectSocket::IsValid()
	{
		if(m_UnderlyingSocket == nullptr)
		{
			return(false);
		}
		else
		{
			return(m_UnderlyingSocket->IsValid());
		}
	}
	void TLSConnectSocket::Close()
	{
		if (m_UnderlyingSocket == nullptr)
		{
			return;
		}
		else
		{
			return(m_UnderlyingSocket->Close());
		}
	}
	bool TLSConnectSocket::IsConnected()
	{
		if (m_UnderlyingSocket == nullptr)
		{
			return false;
		}
		else
		{
			return(m_UnderlyingSocket->IsConnected());
		}
	}
	std::string TLSConnectSocket::RecieveData(size_t MaxNumberOfBytes)
	{
		std::string ReturnValue = "";
		if (m_UnderlyingSocket == nullptr)
		{
			return ReturnValue;
		}
		else
		{
			if (m_TLSHandler.EstablishedSecureConnection() && m_TLSHandler.ConnectionIsActive())
			{
				ReturnValue = m_TLSHandler.GetApplicationData(m_UnderlyingSocket.get(),MaxNumberOfBytes);
			}
			else
			{
				ReturnValue = m_UnderlyingSocket->RecieveData(MaxNumberOfBytes);
			}
		}
		return(ReturnValue);
	}
	MBError TLSConnectSocket::SendData(const void* DataPointer, size_t DataLength)
	{
		MBError ReturnValue = true;
		if (m_UnderlyingSocket == nullptr)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "No underyling ConnectSocket";
			return(ReturnValue);
		}
		else
		{
			if (m_TLSHandler.EstablishedSecureConnection() && m_TLSHandler.ConnectionIsActive())
			{
				m_TLSHandler.SendDataAsRecord(DataPointer, DataLength, m_UnderlyingSocket.get());
			}
			else
			{
				return(m_UnderlyingSocket->SendData(DataPointer,DataLength));
			}
		}
		return(ReturnValue);
	}
	MBError TLSConnectSocket::SendData(std::string const& DataToSend) 
	{
		return(SendData(DataToSend.data(), DataToSend.size()));
	}
	//END TLSConnetSocket

	//BEGIN HTTPConnectSocket
	bool HTTPClientSocket::DataIsAvailable()
	{
		return(!RequestFinished);
	}
	std::string  HTTPClientSocket::p_GetHeaderValue(std::string const& Header, const std::string& HeaderContent)
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
	void HTTPClientSocket::ResetRequestRecieveState()
	{
		CurrentContentLength = -1;
		RecievedContentData = 0;
		HeadRecieved = false;
		IsChunked = false;
		RequestFinished = true;

		CurrentChunkLength = -1;
		CurrentRecievedChunkData = 0;
		size_t ChunkParseOffset = 0;
	}
	void HTTPClientSocket::UpdateAndDechunkData(std::string& DataToDechunk, size_t Offset)
	{
		while (true)
		{
			if (CurrentChunkLength <= CurrentRecievedChunkData || CurrentChunkLength == -1)
			{
				//nu är vi på en header som vi ska ta och fixa längden på
				bool IncludeTailLineFeed = false;
				if (CurrentChunkLength == -1)
				{
					IncludeTailLineFeed = true;
				}

				size_t ChunkHeaderEnd = DataToDechunk.find("\r\n", Offset + 2);
				if (ChunkHeaderEnd == DataToDechunk.npos)
				{
					ChunkParseOffset = DataToDechunk.size();
					break;
				}
				ChunkHeaderEnd += 2;
				std::string ChunkLengthData = DataToDechunk.substr(Offset + 2, ChunkHeaderEnd - 4 - Offset);
				CurrentChunkLength = std::stoi(ChunkLengthData, nullptr, 16);
				CurrentRecievedChunkData = 0;
				if (IncludeTailLineFeed)
				{
					Offset += 2;
				}
				DataToDechunk = DataToDechunk.substr(0, Offset) + DataToDechunk.substr(ChunkHeaderEnd);
				//offset behöver inte ändras, för nu pekar den helt enkelt med den datan som kommer efter chunk headern
			}
			if (CurrentChunkLength == 0)
			{
				RequestFinished = true;
				ChunkParseOffset = 0;
				break;
			}
			int PreviousRecievedChunkData = CurrentRecievedChunkData;
			CurrentRecievedChunkData += DataToDechunk.size() - Offset;
			if (CurrentRecievedChunkData >= CurrentChunkLength)
			{
				CurrentRecievedChunkData = CurrentChunkLength;
				Offset += (CurrentRecievedChunkData - PreviousRecievedChunkData);
				continue;
			}
			else
			{
				ChunkParseOffset = DataToDechunk.size();
				break;
			}
		}
	}
	std::string HTTPClientSocket::HTTPGetData()
	{
		std::string ReturnValue = "";
		while (true)
		{
			size_t PreviousDataSize = ReturnValue.size();
			ReturnValue += m_UnderlyingSocket->RecieveData(MaxBytesInMemory);
			size_t HeaderSize = 0;
			if (!HeadRecieved)
			{
				size_t HeaderEnd = ReturnValue.find("\r\n\r\n");
				HeaderSize = HeaderEnd + 4;
				if (HeaderEnd == ReturnValue.npos)
				{
					continue;
				}
				HeadRecieved = true;
				RequestFinished = false;
				std::string ContentLength = p_GetHeaderValue("Content-Length", ReturnValue);
				//TODO Antar att varje gång vi inte har contentlength så är det chunked, kanske inte alltid stämmer men får fixa det sen
				if (ContentLength == "" && p_GetHeaderValue("Transfer-Encoding", ReturnValue) != "")
				{
					size_t BodyBegin = HeaderEnd + 4;
					ChunkParseOffset = BodyBegin - 2; //vi vill inkludera \r\n som avslutar headern eftersom varje chunk också avslutas med dens
					IsChunked = true;
				}
				if (ContentLength != "")
				{
					try
					{
						CurrentContentLength = std::stoi(ContentLength);
						RecievedContentData += ReturnValue.size() - (HeaderEnd + 4);
					}
					catch (const std::exception&)
					{
						assert(false);
					}
				}
				if (ContentLength == "" && p_GetHeaderValue("Transfer-Encoding", ReturnValue) == "")
				{
					//vi har fått all data typ
					ResetRequestRecieveState();
					break;
				}
			}
			if (!IsChunked)
			{
				//extremt äcklig work around för vad som händer om den här händer efter head
				if (HeaderSize != 0)
				{
					RecievedContentData = ReturnValue.size() - HeaderSize;
				}
				else
				{
					RecievedContentData += ReturnValue.size() - PreviousDataSize;
				}
				if (RecievedContentData >= CurrentContentLength)
				{
					RequestFinished = true;
				}
			}
			else
			{
				UpdateAndDechunkData(ReturnValue, ChunkParseOffset);
			}
			HeaderSize = 0;
			if (RequestFinished)
			{
				ResetRequestRecieveState();
				break;
			}
			if (ReturnValue.size() >= MaxBytesInMemory)
			{
				break;
			}
		}
		ChunkParseOffset = 0;
		return(ReturnValue);
	}
	int HTTPClientSocket::Get(std::string Resource)
	{
		std::string Meddelandet = "GET /" + Resource + " " + "HTTP/1.1\r\nHost: " + URl + "\r\n" + "accept-encoding: identity" + "\r\n" + "\r\n";
		//SendData(Meddelandet.c_str(), Meddelandet.length());
		m_UnderlyingSocket->SendData(Meddelandet);
		return(0);
	}
	int HTTPClientSocket::Head(std::string Resource)
	{
		std::string Meddelandet = "HEAD /" + Resource + " " + "HTTP/1.1\r\nHost: " + URl + "\r\n" + "accept-encoding: identity" + "\r\n" + "\r\n";
		//SendData(Meddelandet.c_str(), Meddelandet.length());
		m_UnderlyingSocket->SendData(Meddelandet);
		return(0);
	}
	std::vector<std::pair<std::string,std::string>> HTTPClientSocket::p_GetDefaultHeaders()
	{
		std::vector<std::pair<std::string, std::string>> ReturnValue = {};
		ReturnValue.push_back(std::pair<std::string, std::string>("accept-encoding", "identity"));
		return(ReturnValue);
	}
	void HTTPClientSocket::p_SendRequest(std::string const& RequestType, std::string const& ResourceToGet, std::vector<std::pair<std::string, std::string>> const& ExtraHeaders)
	{
		std::string StringToSend = RequestType + ResourceToGet + " " + "HTTP/1.1\r\nHost: "+m_RemoteHost+"\r\n";
		std::vector<std::pair<std::string, std::string>> DefaultHeaders = p_GetDefaultHeaders();
		for (size_t i = 0; i < DefaultHeaders.size(); i++)
		{
			StringToSend += DefaultHeaders[i].first + ": " + DefaultHeaders[i].second + "\r\n";
		}
		for (size_t i = 0; i < ExtraHeaders.size(); i++)
		{
			StringToSend += ExtraHeaders[i].first + ": " + ExtraHeaders[i].second + "\r\n";
		}
		StringToSend += "\r\n";
		m_UnderlyingSocket->SendData(StringToSend);
	}
	std::string HTTPClientSocket::GetDataFromRequest(const std::string& RequestType, std::string Resource)
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
	std::string HTTPClientSocket::GetDataFromRequest(const std::string& RequestType, std::string const& Resource, std::vector<std::pair<std::string, std::string>> const& ExtraHeaders)
	{
		p_SendRequest(RequestType, Resource, ExtraHeaders);
		std::string ReturnValue = HTTPGetData();
		return(ReturnValue);
	}
	MBError HTTPClientSocket::EstablishTLSConnection()
	{
		return(m_UnderlyingSocket->EstablishClientTLSConnection(m_RemoteHost));
	}
	HTTPClientSocket::HTTPClientSocket(std::unique_ptr<TLSConnectSocket> ConnectedSocket,std::string const& Host)
	{
		m_RemoteHost = Host;
		m_UnderlyingSocket = std::move(ConnectedSocket);
	}
	bool HTTPClientSocket::IsConnected()
	{
		if (m_UnderlyingSocket != nullptr)
		{
			return(false);
		}
		return(m_UnderlyingSocket->IsConnected());
	}
	bool HTTPClientSocket::IsValid()
	{
		if (m_UnderlyingSocket != nullptr)
		{
			return(false);
		}
		return(m_UnderlyingSocket->IsValid());
	}
	MBError HTTPClientSocket::Connect(std::string const& Host, std::string const& Port)
	{
		MBError ReturnValue = true;
		this->URl = Host;
		std::unique_ptr<TCPClient> TCPConnection = std::unique_ptr<TCPClient>(new TCPClient(Host, Port));
		TCPConnection->Connect();
		m_UnderlyingSocket = std::unique_ptr<TLSConnectSocket>(new TLSConnectSocket(std::move(TCPConnection)));
		m_RemoteHost = Host;
		return(ReturnValue);
	}
	HTTPClientSocket::HTTPClientSocket(std::string const& URL, std::string const& Port)
	{
		this->URl = URL;
		std::unique_ptr<TCPClient> TCPConnection = std::unique_ptr<TCPClient>(new TCPClient(URL, Port));
		TCPConnection->Connect();
		m_UnderlyingSocket = std::unique_ptr<TLSConnectSocket>(new TLSConnectSocket(std::move(TCPConnection)));
		m_RemoteHost = URL;
	}
	HTTPClientSocket::~HTTPClientSocket()
	{

	}
	//END HTTPConnectSocket
    std::string IPToString(uint32_t IP)
    {
        std::string ReturnValue;
        for(int i = 0; i < 4;i++)
        {
            ReturnValue += std::to_string((unsigned int)((unsigned char)(IP>> ((3-i)*8))));
            if(i != 3)
            {
                ReturnValue += '.';
            }
        }
        return ReturnValue;
    }
    uint32_t StringToIP(std::string const& String)
    {
        uint32_t ReturnValue = 0;
        size_t ParseOffset = 0;
        MBParsing::SkipWhitespace(String,ParseOffset,&ParseOffset);
        for(int i = 0; i < 4;i++)
        {
            std::string CurrentNumber;
            while(ParseOffset < String.size())
            {
                if(String[ParseOffset] == '.')
                {
                    ParseOffset += 1;
                    break;
                }
                CurrentNumber += String[ParseOffset];
                ParseOffset += 1;
            }
            uint8_t Number = std::stoi(CurrentNumber);
            ReturnValue += Number << ((3-i)*8);
        }
        return ReturnValue;
    }
};
