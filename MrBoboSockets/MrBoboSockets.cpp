#include "MrBoboSockets.h"
#include <MBStrings.h>
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
#include <csignal>
//#include <stdatomic.h>
//#include <sys\types.h>
#endif

#ifdef __linux__
#define _FILE_OFFSET_BITS = 64
#include <sys/stat.h>
#endif
inline uint64_t MBGetFileSize(std::string const& PathToCheck)
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
		signal(SIGPIPE, SIG_IGN);
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




	//BEGIN Socket
	std::string Socket::p_GetLastError()
	{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		return(std::to_string(WSAGetLastError()));
#elif __linux__
		return(std::string(strerror(errno)));
#endif
	}
	void Socket::p_HandleError(std::string const& ErrorMessage, bool IsLethal)
	{
		//DEBUG GREJER
		std::cout << ErrorMessage << std::endl;
		m_LastErrorMessage = ErrorMessage;
		if (IsLethal == true)
		{
			m_Invalid = true;
		}
	}
	bool Socket::IsValid()
	{
		return(!m_Invalid);
	}
	//bool Socket::IsConnected()
	//{
	//	return(!ConnectionClosed);
	//}
	void Socket::Close()
	{
		MBCloseSocket(m_UnderlyingHandle);
		m_Invalid = true;
		m_UnderlyingHandle = MBInvalidSocket;
	}
	//int Socket::SendData(const char* DataPointer, int DataLength)
	//{
	//	try
	//	{
	//		int TotalDataSent = 0;
	//		while (TotalDataSent != DataLength)
	//		{
	//			m_ErrorResult = send(m_UnderlyingHandle, DataPointer, DataLength, 0);
	//			if (m_ErrorResult == MBSocketError())
	//			{
	//				p_HandleError("send failed with error: " + p_GetLastError(), true);
	//				return(0);
	//			}
	//			TotalDataSent += m_ErrorResult;
	//		}
	//	}
	//	catch (const std::exception&)
	//	{
	//		p_HandleError("send failed with unknown error", true);
	//		return(-1);
	//	}
	//	return(0);
	//}
	//std::string Socket::GetIpOfConnectedSocket()
	//{
	//	sockaddr_in  AddresData;
	//	AddresData.sin_family = AF_INET;
	//	int SizeOfData = sizeof(sockaddr_in);
	//	m_ErrorResult = getpeername(m_UnderlyingHandle, (sockaddr*)&AddresData, (socklen_t*)&SizeOfData);
	//	if (m_ErrorResult == MBSocketError())
	//	{
	//		p_HandleError("Get ip failed: " + p_GetLastError(), false);
	//		return("");
	//	}
	//	char ActualAdress[100];
	//	unsigned long DataReturned = 100;
	//	//ErrorResults = WSAAddressToStringA((SOCKADDR*)&AddresData, sizeof(SOCKADDR), NULL, ActualAdress, &DataReturned);
	//	//TODO fixa s� det h�r fungerar
	//	std::string Result = inet_ntoa(AddresData.sin_addr);
	//	return(std::string(ActualAdress));
	//}
	//int Socket::RecieveData(char* Buffer, int BufferSize)
	//{
	//	m_ErrorResult = recv(m_UnderlyingHandle, Buffer, BufferSize, 0);
	//	if (m_ErrorResult > 0)
	//	{
	//		//printf("Bytes received: %d\n", ErrorResults);
	//		return(m_ErrorResult);
	//	}
	//	else if (m_ErrorResult == 0)
	//	{
	//		//printf("Connection closed\n");
	//		ConnectionClosed = true;
	//		return(m_ErrorResult);
	//	}
	//	else
	//	{
	//		p_HandleError("recv failed: " + p_GetLastError(), true);
	//		return(m_ErrorResult);
	//	}
	//}
	//std::string Socket::RecieveData()
	//{
	//	size_t InitialBufferSize = 16500;
	//	char* Buffer = (char*)malloc(InitialBufferSize);
	//	size_t MaxRecieveSize = InitialBufferSize;
	//	size_t LengthOfDataRecieved = 0;
	//	size_t TotalLengthOfData = 0;
	//	while ((LengthOfDataRecieved = RecieveData(&Buffer[TotalLengthOfData], MaxRecieveSize)) > 0)
	//	{
	//		TotalLengthOfData += LengthOfDataRecieved;
	//		if (LengthOfDataRecieved == MaxRecieveSize)
	//		{
	//			MaxRecieveSize = 500;
	//			Buffer = (char*)realloc(Buffer, TotalLengthOfData + 500);
	//			assert(Buffer != nullptr);
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//	std::string ReturnValue(Buffer, TotalLengthOfData);
	//	free(Buffer);
	//	return(ReturnValue);
	//}
	//std::string Socket::RecieveData(int MaxNumberOfBytes)
	//{
	//	int InitialBufferSize = std::min(16500, MaxNumberOfBytes);
	//	char* Buffer = (char*)malloc(InitialBufferSize);
	//	int MaxRecieveSize = InitialBufferSize;
	//	int LengthOfDataRecieved = 0;
	//	int TotalLengthOfData = 0;
	//	while ((LengthOfDataRecieved = RecieveData(&Buffer[TotalLengthOfData], MaxRecieveSize)) > 0)
	//	{
	//		TotalLengthOfData += LengthOfDataRecieved;
	//		if (TotalLengthOfData >= MaxNumberOfBytes)
	//		{
	//			break;
	//		}
	//		if (LengthOfDataRecieved == MaxRecieveSize)
	//		{
	//			MaxRecieveSize = InitialBufferSize;
	//			if (TotalLengthOfData + MaxRecieveSize > MaxNumberOfBytes)
	//			{
	//				MaxRecieveSize = MaxNumberOfBytes - TotalLengthOfData;
	//			}
	//			Buffer = (char*)realloc(Buffer, TotalLengthOfData + MaxRecieveSize);
	//			assert(Buffer != nullptr);
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//	std::string ReturnValue(Buffer, TotalLengthOfData);
	//	free(Buffer);
	//	return(ReturnValue);
	//}
	Socket::Socket()
	{
		m_UnderlyingHandle = MBInvalidSocket;
	}
	Socket::~Socket()
	{
		MBCloseSocket(m_UnderlyingHandle);
	}
	//END Socket

	//BEGIN UDPSocket
	//class UDPSocket : public Socket
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
		return(0);
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
	//END UdpSocket

	//BEGIN ConnectSocket	
	//class ConnectSocket:: : public Socket
	ConnectSocket::~ConnectSocket()
	{
		delete _m_addr;
	}
	bool ConnectSocket::IsConnected()
	{
		return(m_IsConnected);
	}
	std::string ConnectSocket::GetIpOfConnectedSocket()
	{
		return("");
	}
	int ConnectSocket::SendRawData(const void* DataPointer, size_t DataLength)
	{
		size_t TotalDataSent = 0;
		while (TotalDataSent != DataLength)
		{
			m_ErrorResult = send(m_UnderlyingHandle, (const char*)DataPointer, DataLength, 0);
			if (m_ErrorResult == MBSocketError())
			{
				p_HandleError("send failed with error: " + p_GetLastError(), true);
				return(0);
			}
			TotalDataSent += m_ErrorResult;
		}
	}
	int ConnectSocket::SendData(const void* DataPointer, size_t DataLength)
	{
		if (!m_TLSConnectionEstablished)
		{
			try
			{
				SendRawData(DataPointer, DataLength);
			}
			catch (const std::exception&)
			{
				p_HandleError("send failed with unknown error", true);
				return(-1);
			}
		}
		else
		{
			m_TLSHandler.SendDataAsRecord(DataPointer, DataLength,this);
		}
		return(0);
	}
	int ConnectSocket::SendData(std::string const& DataToSend)
	{
		return(SendData(DataToSend.c_str(), DataToSend.size()));
	}
	std::string ConnectSocket::RecieveRawData(size_t MaxNumberOfBytes)
	{
		size_t InitialBufferSize = std::min((size_t)16500, MaxNumberOfBytes);
		char* Buffer = (char*)malloc(InitialBufferSize);
		size_t MaxRecieveSize = InitialBufferSize;
		size_t LengthOfDataRecieved = 0;
		size_t TotalLengthOfData = 0;
		while ((LengthOfDataRecieved = recv(m_UnderlyingHandle, &Buffer[TotalLengthOfData], MaxRecieveSize, 0)) > 0)
		{
			TotalLengthOfData += LengthOfDataRecieved;
			if (TotalLengthOfData >= MaxNumberOfBytes)
			{
				break;
			}
			if (LengthOfDataRecieved == MaxRecieveSize)
			{
				MaxRecieveSize = InitialBufferSize;
				if (TotalLengthOfData + MaxRecieveSize > MaxNumberOfBytes)
				{
					MaxRecieveSize = MaxNumberOfBytes - TotalLengthOfData;
				}
				Buffer = (char*)realloc(Buffer, TotalLengthOfData + MaxRecieveSize);
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
	std::string ConnectSocket::RecieveData(size_t MaxNumberOfBytes)
	{
		if (!m_TLSConnectionEstablished)
		{
			return(RecieveRawData(MaxNumberOfBytes));
		}
		else
		{
			return(m_TLSHandler.GetApplicationData(this, MaxNumberOfBytes));
		}
	}
	ConnectSocket& ConnectSocket::operator<<(std::string const& DataToSend)
	{
		SendData(DataToSend);
		return(*this);
	}
	ConnectSocket& ConnectSocket::operator>>(std::string& DataBuffer)
	{
		DataBuffer = RecieveData(-1);
		return(*this);
	}
	MBError ConnectSocket::EstablishTLSConnection()
	{
		//MBError ReturnValue(false);
		//try
		//{
		//	ReturnValue = m_TLSHandler.EstablishTLSConnection(this);
		//}
		//catch (const std::exception&)
		//{
		//	ReturnValue = false;
		//	ReturnValue.ErrorMessage = "Unknown error in establishing TLS connection";
		//}
		//if (!ReturnValue)
		//{
		//	//om det fuckade vill vi reseta vårt tls object
		//	m_TLSHandler = TLSHandler();
		//}
		//else
		//{
		//	m_TLSConnectionEstablished = true;
		//}
		//return(ReturnValue);
		return(MBError(false));
	}
	//END ConnectSocket

	//BEGIN ClientSocket
	MBError ClientSocket::EstablishTLSConnection()
	{
		MBError ReturnValue(false);
		try
		{
			ReturnValue = m_TLSHandler.EstablishTLSConnection(this);
		}
		catch (const std::exception&)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Unknown error in establishing TLS connection";
		}
		if (!ReturnValue)
		{
			//om det fuckade vill vi reseta vårt tls object
			m_TLSHandler = TLSHandler();
		}
		else
		{
			m_TLSConnectionEstablished = true;
		}
		return(ReturnValue);
	}
	ClientSocket::ClientSocket(std::string const& Adress, std::string const& Port)
	{
		HostName = Adress;
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
			return;
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
			//freeaddrinfo(result);
		}
	}
	int ClientSocket::Connect()
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
	//END ClientSocket

	//BEGIN ServerSocket
	MBError ServerSocket::EstablishTLSConnection()
	{
		MBError ReturnValue(true);
		try
		{
			ReturnValue = m_TLSHandler.EstablishHostTLSConnection(this);
		}
		catch (const std::exception&)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Unknown error in establishing host tls connection";
			std::cout << "Unknown error in establishing host tls connection" << std::endl;
		}
		if (!ReturnValue)
		{
			m_TLSHandler = TLSHandler();
		}
		else
		{
			m_TLSConnectionEstablished = true;
		}
		return(ReturnValue);
	}
	int ServerSocket::Bind()
	{
		m_ErrorResult = bind(m_ListenerSocket, _m_addr, _m_ai_addrlen);
		if (m_ErrorResult == MBSocketError()) {
			p_HandleError("bind failed with error: " + p_GetLastError(), false);
			//freeaddrinfo(result);
			//MBCloseSocket(ConnectedSocket);
		}
		return(0);
	}
	int ServerSocket::Listen()
	{
		m_ErrorResult = listen(m_ListenerSocket, SOMAXCONN);
		if (m_ErrorResult == MBSocketError())
		{
			p_HandleError("listen failed with error: " + p_GetLastError(), false);
			//MBCloseSocket(ConnectedSocket);
		}
		return(0);
	}
	void ServerSocket::TransferConnectedSocket(ServerSocket& OtherSocket)
	{
		OtherSocket.m_UnderlyingHandle = m_UnderlyingHandle;
		m_UnderlyingHandle = MBInvalidSocket;
		OtherSocket.SocketTlsHandler = SocketTlsHandler;
		SocketTlsHandler = TLSHandler();
		OtherSocket.m_IsConnected = m_IsConnected;
	}
	ServerSocket::ServerSocket() : m_ListenerSocket(MBInvalidSocket)
	{
		
	}
	int ServerSocket::Accept()
	{
		m_UnderlyingHandle = accept(m_ListenerSocket, NULL, NULL);
		if (m_UnderlyingHandle == MBInvalidSocket) {
			p_HandleError("accept failed with error: " + p_GetLastError(), true);
			//MBCloseSocket(ConnectedSocket);
			m_IsConnected = false;
		}
		else
		{
			m_IsConnected = true;
		}
		return(0);
	}
	//MBError ServerSocket::EstablishSecureConnection()
	//{
	//	MBError ReturnValue(false);
	//	try
	//	{
	//		ReturnValue = SocketTlsHandler.EstablishHostTLSConnection(this);
	//	}
	//	catch (const std::exception&)
	//	{
	//		ReturnValue = false;
	//		ReturnValue.ErrorMessage = "Unknown error in establishing TLS connection";
	//	}
	//	if (!ReturnValue)
	//	{
	//		//om det fuckade vill vi reseta vårt tls object
	//		SocketTlsHandler = TLSHandler();
	//	}
	//	return(ReturnValue);
	//}

	ServerSocket::ServerSocket(std::string const& Port) : m_ListenerSocket(MBInvalidSocket)
	{
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
			return;
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
			m_ErrorResult = setsockopt(m_ListenerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&Enable, sizeof(int));
			if (m_ErrorResult < 0)
			{
				p_HandleError("Error at socket() when setting SO_REUSEADDR:" + p_GetLastError(), true);
			}
		}
	}
	ServerSocket::~ServerSocket()
	{
		MBCloseSocket(m_ListenerSocket);
	}
	//END ServerSocket

	//BEGIN HTTPConnectSocket
	//class HTTPConnectSocket:: : public ConnectSocket
	//std::string HTTPConnectSocket::GetNextDecryptedData()
	//{
	//	return( .GetApplicationData(this));
	//}
	//sint HTTPConnectSocket::HTTPSendData(std::string const& DataToSend)
	//s{
	//s	if (IsValid())
	//s	{
	//s		if (UsingHTTPS)
	//s		{
	//s			if (TLSConnectionHandler.ConnectionIsActive())
	//s			{
	//s				TLSConnectionHandler.SendDataAsRecord(DataToSend, this);
	//s			}
	//s			else
	//s			{
	//s				m_Invalid = true;
	//s			}
	//s		}
	//s		else
	//s		{
	//s			SendData(DataToSend.c_str(), DataToSend.size());
	//s		}
	//s	}
	//s	return(0);
	//s}
	bool HTTPConnectSocket::DataIsAvailable()
	{
		return(!RequestFinished);
	}
	void HTTPConnectSocket::ResetRequestRecieveState()
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
	void HTTPConnectSocket::UpdateAndDechunkData(std::string& DataToDechunk, size_t Offset)
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
	std::string HTTPConnectSocket::HTTPGetData()
	{
		std::string ReturnValue = "";
		while (true)
		{
			size_t PreviousDataSize = ReturnValue.size();
			if (UsingHTTPS == false)
			{
				ReturnValue += RecieveData(MaxBytesInMemory);
			}
			else
			{
				ReturnValue += TLSConnectionHandler.GetApplicationData(this, MaxBytesInMemory);
			}
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
				std::string ContentLength = GetHeaderValue("Content-Length", ReturnValue);
				//TODO Antar att varje gång vi inte har contentlength så är det chunked, kanske inte alltid stämmer men får fixa det sen
				if (ContentLength == "" && GetHeaderValue("Transfer-Encoding", ReturnValue) != "")
				{
					size_t BodyBegin = HeaderEnd + 4;
					//size_t ChunkSizeHeaderEnd = ReturnValue.find("\r\n", BodyBegin);
					//std::string HexChunkLengthString = ReturnValue.substr(BodyBegin, ChunkSizeHeaderEnd - BodyBegin);
					//CurrentChunkLength = std::stoi(HexChunkLengthString,nullptr,16);
					//CurrentRecievedChunkData = ReturnValue.size() - (ChunkSizeHeaderEnd + 2);
					//if(CurrentRecievedChunkData 
					//ReturnValue = ReturnValue.substr(0, BodyBegin) + ReturnValue.substr(ChunkSizeHeaderEnd + 2);
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
				if (ContentLength == "" && GetHeaderValue("Transfer-Encoding", ReturnValue) == "")
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
	int HTTPConnectSocket::Get(std::string Resource)
	{
		std::string Meddelandet = "GET /" + Resource + " " + "HTTP/1.1\r\nHost: " + URl + "\r\n" + "accept-encoding: identity" + "\r\n" + "\r\n";
		//SendData(Meddelandet.c_str(), Meddelandet.length());
		ConnectSocket::SendData(Meddelandet);
		return(0);
	}
	int HTTPConnectSocket::Head(std::string Resource)
	{
		std::string Meddelandet = "HEAD /" + Resource + " " + "HTTP/1.1\r\nHost: " + URl + "\r\n" + "accept-encoding: identity" + "\r\n" + "\r\n";
		//SendData(Meddelandet.c_str(), Meddelandet.length());
		ConnectSocket::SendData(Meddelandet);
		return(0);
	}
	std::string HTTPConnectSocket::GetDataFromRequest(const std::string& RequestType, std::string Resource)
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
	//HTTPConnectSocket::HTTPConnectSocket(std::string const URL, std::string const Port) : ClientSocket(URL, Port)
	//{
	//	this->URl = URL;
	//}
	HTTPConnectSocket::HTTPConnectSocket(std::string const& URL, std::string const& Port) : ClientSocket(URL,Port)
	{
		this->URl = URL;
	}

	//void HTTPConnectSocket::EstablishSecureConnetion()
	//{
	//	TLSConnectionHandler.EstablishTLSConnection(this);
	//}
	HTTPConnectSocket::~HTTPConnectSocket()
	{

	}
	//END HTTPConnectSocket
	std::string GetRequestType(const std::string& RequestData)
	{
		int FirstSpace = RequestData.find(" ");
		return(RequestData.substr(0, FirstSpace));
	}
	std::string GetReqestResource(const std::string& RequestData)
	{
		int FirstSlashPos = RequestData.find("/");
		int FirstSpaceAfterSlash = RequestData.find(" ", FirstSlashPos);
		std::string ReturnValue = RequestData.substr(FirstSlashPos + 1, FirstSpaceAfterSlash - FirstSlashPos - 1);
		bool Error = true; // borde göra något med denna med aja
		ReturnValue = MBUtility::URLDecodeData(ReturnValue, &Error);
		return(ReturnValue);
	}
	//BEGIN HTTPTypeTuple
	HTTPTypeTuple HTTPTypesConnector::GetTupleFromExtension(std::string const& Extension)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			for (size_t j = 0; j < SuppportedTupples[i].FileExtensions.size(); j++)
			{
				if (SuppportedTupples[i].FileExtensions[j] == Extension)
				{
					return(SuppportedTupples[i]);
				}
			}
		}
		return(NullTupple);
	}
	HTTPTypeTuple HTTPTypesConnector::GetTupleFromDocumentType(HTTPDocumentType DocumentType)
	{
		for (size_t i = 0; i < SuppportedTupples.size(); i++)
		{
			if (SuppportedTupples[i].FileHTTPDocumentType == DocumentType)
			{
				return(SuppportedTupples[i]);
			}
		}
		return(NullTupple);
	}
	//END HTTPTypeTuple
	HTTPDocumentType DocumentTypeFromFileExtension(std::string const& FileExtension)
	{
		HTTPTypesConnector TypeConnector;
		HTTPTypeTuple DocumentTuple = TypeConnector.GetTupleFromExtension(FileExtension);
		return(DocumentTuple.FileHTTPDocumentType);
	}
	MediaType GetMediaTypeFromExtension(std::string const& FileExtension)
	{
		HTTPTypesConnector TypeConnector;
		HTTPTypeTuple DocumentTuple = TypeConnector.GetTupleFromExtension(FileExtension);
		return(DocumentTuple.FileMediaType);
	}
	std::string GetMIMEFromDocumentType(HTTPDocumentType TypeToConvert)
	{
		HTTPTypesConnector TypeConnector;
		HTTPTypeTuple DocumentTuple = TypeConnector.GetTupleFromDocumentType(TypeToConvert);
		return(DocumentTuple.MIMEMediaString);
	}
	std::string HTTPRequestStatusToString(HTTPRequestStatus StatusToConvert)
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
	}
	std::string GenerateRequest(HTTPDocument const& DocumentToSend)
	{
		std::string Request = "";
		Request += "HTTP/1.1 " + HTTPRequestStatusToString(DocumentToSend.RequestStatus) + "\r\n";
		Request += "Content-Type: " + GetMIMEFromDocumentType(DocumentToSend.Type) + "\r\n";
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
		for (size_t i = 0; i < DocumentToSend.ExtraHeaders.size(); i++)
		{
			Request += DocumentToSend.ExtraHeaders[i] + "\r\n";
		}
		Request += "\r\n";
		if (DocumentToSend.DocumentDataFileReference == "")
		{
			Request += DocumentToSend.DocumentData;
		}
		return(Request);
	}
	std::string GenerateRequest(const std::string& HTMLBody)
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
	int HexToDec(std::string NumberToConvert)
	{
		int ReturnValue = 0;
		for (size_t i = 0; i < NumberToConvert.size(); i++)
		{
			ReturnValue += std::stoi(NumberToConvert.substr(NumberToConvert.size() - 1 - i, 1)) * pow(16, i);
		}
		return(ReturnValue);
	}
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
	//void HTTPServerSocket::SendWithTls(std::string const& DataToSend)
	//{
	//	if (IsValid())
	//	{
	//		if (SocketTlsHandler.EstablishedSecureConnection() == true)
	//		{
	//			if (SocketTlsHandler.ConnectionIsActive())
	//			{
	//				SocketTlsHandler.SendDataAsRecord(DataToSend, this);
	//			}
	//			else
	//			{
	//				m_Invalid = true;
	//			}
	//		}
	//		else
	//		{
	//			SendData(DataToSend.c_str(), DataToSend.size());
	//		}
	//	}
	//}
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
	HTTPServerSocket::HTTPServerSocket(std::string const& Port) : ServerSocket(Port)
	{

	}
	int HTTPServerSocket::p_GetNextChunkSize(int ChunkHeaderPosition, std::string const& Data, int& OutChunkDataBeginning)
	{
		int ChunkHeaderEnd = Data.find("\r\n", ChunkHeaderPosition);
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
		std::string ContentLengthString = "NULL";
		int ContentLength = 0;
		int HeaderLength = 0;
		int MaxDataInMemory = 1650000 * 2;
		int TotalRecievedData = 0;
		while (true)
		{
			if (!SocketTlsHandler.EstablishedSecureConnection())
			{
				ReturnValue += RecieveData(MaxDataInMemory - TotalRecievedData);
			}
			else
			{
				ReturnValue += SocketTlsHandler.GetApplicationData(this, MaxDataInMemory - TotalRecievedData);
				//Kollar lite att det stämmer osv
			}
			if (!this->IsValid())
			{
				//något är fel, returna det vi fick och resetta, socketen kan inte användas mer
				CurrentContentLength = 0;
				ParsedContentData = 0;
				return(ReturnValue);
			}
			TotalRecievedData = ReturnValue.size();
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
		std::string Request = GenerateRequest(Body);
		SendData(Request);
	}
	void HTTPServerSocket::SendHTTPBody(const std::string& Data)
	{
		std::string Request = GenerateRequest(Data);
		SendData(Request);
	}
	//void HTTPServerSocket::SendFullResponse(std::string const& DataToSend)
	//{
	//	ConnectSocket::SendData(DataToSend);
	//}
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
			std::string ContentRangeHeader = "Content-Range: bytes " + std::to_string(StartByte) + "-" + std::to_string(LastByte) + "/" + std::to_string(FileSize);
			NewDocument.ExtraHeaders.push_back(ContentRangeHeader);
			std::string DataToSend = GenerateRequest(NewDocument);
			SendData(DataToSend);
		}
		else
		{
			std::string DataToSend = GenerateRequest(DocumentToSend);
			SendData(DataToSend);
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
				SendData(DataExtracter.GetNextIntervall());
			}
			int hej = 2;
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