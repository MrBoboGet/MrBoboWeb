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
#include <MBCrypto/MBCrypto.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");
	MBSockets::Init();
	//<!-- sp:eh:0NeoaKmFjBKH4oimbSprXXINNJbO2DBGl0uTpjOggp0uD8/1G2AkxBRJzYeuMdglx0+TRmMkkDpr+OUudn/vdKifIiLCgl8iTIv9c2ms8RaQkwgmaopWHS2jAxM= -->
	//std::string AmazonEasterEgg = "fhkUxNast3AF+wV5uFyTMkY01EmUV3vAgzYv46uVJ/KDVIkf+yLwwh+rjYVTYfUy1TqDVqQf3uMOuk8f/i3yQjnfz0mq1NmeSKJpM9MEcyapWYzb+CvJ8q56JWY=";
	//std::cout << MBMail::BASE64Decode(AmazonEasterEgg.data(), AmazonEasterEgg.size())<<std::endl;
	//exit(0);
	MBSockets::HTTPConnectSocket TestConnectSocket("google.com", "443", MBSockets::TraversalProtocol::TCP,MBSockets::ApplicationProtocols::HTTPS);
	TestConnectSocket.Connect();
	TestConnectSocket.EstablishSecureConnetion();
	//std::cout << TestConnectSocket.GetDataFromRequest("GET", "")<<std::endl;
	exit(0);
	////test för att signera enbart
	//std::string TestSignature = MBCrypto::RSASSA_PKCS1_V1_5_SIGN("Test Test", "ServerResources/mrboboget.se/EncryptionResources/KeyfileRSA2096.key", MBCrypto::HashFunction::SHA256);
	//std::cout << MBMail::BASE64Encode(TestSignature.data(),TestSignature.size())<< std::endl;
	//std::cout << "Base64 encoded size: " << MBMail::BASE64Encode(TestSignature.data(), TestSignature.size()).size() << std::endl;
	//std::ofstream OutFile = std::ofstream("ServerResources/mrboboget.se/EncryptionResources/MBSignedFile.txt", std::ios::binary);
	//OutFile << TestSignature;
	////exit(0);
	//std::string StringToEncode = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
	//std::string Base64EncodedString = MBMail::BASE64Encode(StringToEncode.data(), StringToEncode.size());
	//std::cout << "BASE64 Encoding: " << Base64EncodedString << std::endl;
	//std::cout << "Decoded string; " << MBMail::BASE64Decode(Base64EncodedString.data(), Base64EncodedString.size()) << std::endl;
	//
	//MBMail::MBMailSender TestSender;
	//MBMail::Mail MailToSave;
	//MailToSave.Body.BodyType = MBMIME::MIMEType::Text;
	//MailToSave.Body.BodyData = "Test mail hej hej\r\n";
	//MBMail::StandardUserEmailHeaders StandardHeaders;
	//StandardHeaders.From = "test@mrboboget.se";
	//StandardHeaders.To = "emanuelberggren01@gmail.com";
	//StandardHeaders.Subject = "Test";
	//TestSender.FillMailHeaders(MailToSave, StandardHeaders);
	//TestSender.t_SaveMail(MailToSave, "test@mrboboget.se", "MailToTest.eml");
	//exit(0);
	

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
	std::vector<MBDNS::ARecord> DomainIPAdresses = TestHandler.GetDomainIPAdresses("gmail.com");
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
	std::string DomainToText = "_dmarc.gmail.com";
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
	//exit(0);
	//MBSockets::HTTPConnectSocket TestSocket1("gmail-smtp-in.l.google.com", "465", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	//TestSocket1.Connect();
	//TestSocket1.EstablishSecureConnetion();
	//alt1.gmail-smtp-in.l.google.com
	MBSockets::HTTPConnectSocket TestSocket("smtp.gmail.com", "465", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	TestSocket.Connect();
	TestSocket.EstablishSecureConnetion();
	//TestSocket.EstablishSecureConnetion();
	TestSocket.HTTPSendData("EHLO mrboboget.se");
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
