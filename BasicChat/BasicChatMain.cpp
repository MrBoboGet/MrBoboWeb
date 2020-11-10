/*
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <ServerSide.h>
#include <ClientSide.h>
#include <thread>
#include <string>
#include <iostream>
int main()
{
	//initialiserar lite lokala variablar så vi kan få grejer att connecta utan att jag specificerar 
	std::cout << "Enter IP adress to connect to" << std::endl;
	std::string IPAdressen;
	std::getline(std::cin, IPAdressen);
	std::cout << "Enter Port to connect and listen to" << std::endl;
	std::string PortAdress;
	std::getline(std::cin, PortAdress);

	std::thread Lyssnaren(ListeningPart, PortAdress);
	std::thread Skickaren(SendingPart, IPAdressen, PortAdress);
	Lyssnaren.join();
	Skickaren.join();
	std::cout << "Programmet Avslutade" << std::endl;
}
*/