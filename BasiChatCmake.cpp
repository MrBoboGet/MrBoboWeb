#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets.h>
#include <Crawler.h>
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MrPostOGet/SearchEngineImplementation.h>
#include <MrPostOGet/TLSHandler.h>
#include <MrBoboChatt/MrBoboChatt.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MrBoboMedia/MBMedia.h>
#include <MBSearchEngine/MBUnicode.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <time.h>
#include <Crawler/MBCrawler.h>
#include <Crawler/MBCrawlerSite.h>
#include <MrPostOGet/MBHTMLParser.h>
#include <MBDNSHandler/MBDNSHandler.h>
#include <MrBoboMail/MrBoboMail.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	std::string StringToEncode = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
	std::string Base64EncodedString = MBMail::BASE64Encode(StringToEncode.data(), StringToEncode.size());
	std::cout << "BASE64 Encoding: " << Base64EncodedString << std::endl;
	std::cout << "Decoded string; " << MBMail::BASE64Decode(Base64EncodedString.data(), Base64EncodedString.size()) << std::endl;

	MBMail::MBMailSender TestSender;
	MBMail::Mail MailToSave;
	MailToSave.Body.BodyType = MBMIME::MIMEType::Text;
	MailToSave.Body.BodyData = "Test mail hej hej\r\n";
	MBMail::StandardUserEmailHeaders StandardHeaders;
	StandardHeaders.From = "test@mrboboget.se";
	StandardHeaders.To = "emanuelberggren01@gmail.com";
	StandardHeaders.Subject = "Test";
	TestSender.FillMailHeaders(MailToSave, StandardHeaders);
	TestSender.t_SaveMail(MailToSave, "test@mrboboget.se", "MailToTest.eml");
	exit(0);
	

	MBSockets::Init();
	MBDNS::MBDNSHandler TestHandler;
	std::vector<MBDNS::MXRecord> MXToUse = TestHandler.GetDomainMXRecords("gmail.com");
	size_t HighestPriortiyMXIndex = 0;
	size_t LowestPreferance = uint16_t(~0)>>1;
	for (size_t i = 0; i < MXToUse.size(); i++)
	{
		if (MXToUse[i].Preference < LowestPreferance)
		{
			HighestPriortiyMXIndex = i;
			LowestPreferance = MXToUse[i].Preference;
		}
	}
	std::string DomainToUse = MXToUse[HighestPriortiyMXIndex].Exchange;
	std::cout << "Domain to use: " << DomainToUse << std::endl;
	std::cout << "Number of MXRecords: " << MXToUse.size() << std::endl;
	


	//reverse dns test grejer
	std::vector<MBDNS::ARecord> DomainIPAdresses = TestHandler.GetDomainIPAdresses("mrboboget.se");
	for (size_t i = 0; i < DomainIPAdresses.size(); i++)
	{
		std::cout << MBDNS::IPAdressToString(DomainIPAdresses[i].IPAdress) << std::endl;
	}
	std::vector<MBDNS::PTRRecord> IPDomains = TestHandler.GetIPAdressDomains(MBDNS::IPAdressToString(DomainIPAdresses[0].IPAdress));
	for (size_t i = 0; i < IPDomains.size(); i++)
	{
		std::cout << IPDomains[i].DomainName << std::endl;
	}
	std::vector<MBDNS::ARecord> Domain2IPAdresses = TestHandler.GetDomainIPAdresses(IPDomains[0].DomainName);
	for (size_t i = 0; i < Domain2IPAdresses.size(); i++)
	{
		std::cout << MBDNS::IPAdressToString(Domain2IPAdresses[i].IPAdress) << std::endl;
	}

	//TXT records
	std::string DomainToText = "brisbane._domainkey.example.com";
	std::vector<MBDNS::TXTRecord> DomainTXTRecords = TestHandler.GetTXTRecords(DomainToText);
	std::cout << DomainToText<<" TXT records:"<<std::endl;
	for (size_t i = 0; i < DomainTXTRecords.size(); i++)
	{
		std::cout << "Record:" << i << std::endl;
		for (size_t j = 0; j < DomainTXTRecords[i].RecordStrings.size(); j++)
		{
			std::cout << DomainTXTRecords[i].RecordStrings[j] << std::endl;
		}
	}
	exit(0);


	//MBSockets::HTTPConnectSocket TestSocket1("gmail-smtp-in.l.google.com", "465", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	//TestSocket1.Connect();
	//TestSocket1.EstablishSecureConnetion();
	MBSockets::HTTPConnectSocket TestSocket("alt3.gmail-smtp-in.l.google.com", "587", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	TestSocket.Connect();
	//TestSocket.EstablishSecureConnetion();
	//TestSocket.HTTPSendData("EHLO mrboboget.se");
	//std::cout << TestSocket.GetNextDecryptedData();
	std::cout << TestSocket.GetNextRequestData();
	while (true)
	{
		std::string StringToSend;
		std::getline(std::cin,StringToSend);
		TestSocket.SendData((StringToSend + "\r\n").c_str(),StringToSend.size()+2);
		std::cout << TestSocket.GetNextRequestData();
	}
	return(MBGWebsiteMain());
}
