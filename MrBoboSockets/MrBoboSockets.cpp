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
		return(m_IsConnected && !m_Invalid);
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
	//int ConnectSocket::SendData(std::string const& DataToSend)
	//{
	//	return(SendData(DataToSend.c_str(), DataToSend.size()));
	//}
	std::string ConnectSocket::RecieveRawData(size_t MaxNumberOfBytes)
	{
		size_t InitialBufferSize = std::min((size_t)16500, MaxNumberOfBytes);
		char* Buffer = (char*)malloc(InitialBufferSize);
		size_t MaxRecieveSize = InitialBufferSize;
		int LengthOfDataRecieved = 0;
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
	//ConnectSocket& ConnectSocket::operator<<(std::string const& DataToSend)
	//{
	//	SendData(DataToSend);
	//	return(*this);
	//}
	//ConnectSocket& ConnectSocket::operator>>(std::string& DataBuffer)
	//{
	//	DataBuffer = RecieveData(-1);
	//	return(*this);
	//}
	MBError ConnectSocket::EstablishTLSConnection()
	{
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
	bool HTTPConnectSocket::DataIsAvailable()
	{
		return(!RequestFinished);
	}
	std::string  HTTPConnectSocket::p_GetHeaderValue(std::string const& Header, const std::string& HeaderContent)
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
	HTTPConnectSocket::HTTPConnectSocket(std::string const& URL, std::string const& Port) : ClientSocket(URL,Port)
	{
		this->URl = URL;
	}
	HTTPConnectSocket::~HTTPConnectSocket()
	{

	}
	//END HTTPConnectSocket
};