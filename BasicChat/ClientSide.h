#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
void SendingPart(const std::string IPAdressToConnectTo,const std::string PortToListenAndConnectTo)
{
	std::cout << "Sender Started" << std::endl;
	WSAData SocketData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &SocketData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(IPAdressToConnectTo.c_str(), PortToListenAndConnectTo.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//connecta till en annan socket
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	//för att clienten ska skicka och få data

	int recvbuflen = DEFAULT_BUFLEN;

	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];

	//int iResult;
	while (true)
	{
		std::string MessageToSend;
		std::getline(std::cin, MessageToSend);
		// Send an initial buffer
		iResult = send(ConnectSocket, MessageToSend.c_str(), (int)MessageToSend.size(), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return;
		}
	}


	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return;
}