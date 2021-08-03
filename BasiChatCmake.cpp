#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets/MrBoboSockets.h>
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
#include <MBTorrent/MBBitTorrent.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//#include <Hash/src/sha1.h>

int main()
{
	MBSockets::Init();
	std::filesystem::current_path("C:/Users/emanu/Desktop/Program/C++/BasicChatCmake/");

	
	MBDB::MBDB_Object TestObject;
	TestObject.LoadObject("TestMBDBO.mbdbo", "Guest", nullptr);
	std::ofstream OutputTest = std::ofstream("TestOutputMBDBO",std::ios::out|std::ios::binary);
	std::string ObjectJSON = TestObject.ToJason();
	OutputTest << ObjectJSON;
	OutputTest.flush();
	OutputTest.close();
	std::cout << ObjectJSON<<std::endl;
	TestObject.LoadObject("TestOutputMBDBO", "Guest", nullptr);
	std::cout << TestObject.ToJason() << std::endl;
	std::cout << (TestObject.ToJason() == ObjectJSON) << std::endl;

	MBDB::MBDB_Object TestObjectScript;
	TestObjectScript.LoadObject("TestMBDBO_ObjectScript.mbdbo", "Guest", nullptr);
	std::cout << TestObjectScript.ToJason() << std::endl;
	exit(0);
	//std::ofstream SparseFileTest("SparseFileTest",std::ios::out|std::ios::binary);
	//SparseFileTest.seekp(10000);
	//SparseFileTest.write("TestTestTest", 12);
	//SparseFileTest.flush();
	//SparseFileTest.close();
	//exit(0);
	MBTorrent::MBBitTorrentHandler TestTorrentHandler;
	TestTorrentHandler.LoadTorrentInfo("Serial Experiments Lain Storyboard.torrent");
	TestTorrentHandler.StartDownload();
	exit(0);
	MBSockets::ClientSocket GmailConnection("smtp-relay.gmail.com", "587");
	GmailConnection.Connect();
	//GmailConnection.EstablishTLSConnection();
	std::cout << GmailConnection.RecieveData();
	while (true)
	{
		std::string LineToSend;
		std::getline(std::cin, LineToSend);
		LineToSend += "\r\n";
		GmailConnection.SendData(LineToSend);
		std::cout << GmailConnection.RecieveData();
		if (LineToSend == "STARTTLS\r\n")
		{
			GmailConnection.EstablishTLSConnection();
		}
	}
	exit(0);
	//<!-- sp:eh:0NeoaKmFjBKH4oimbSprXXINNJbO2DBGl0uTpjOggp0uD8/1G2AkxBRJzYeuMdglx0+TRmMkkDpr+OUudn/vdKifIiLCgl8iTIv9c2ms8RaQkwgmaopWHS2jAxM= -->
	//std::string AmazonEasterEgg = "fhkUxNast3AF+wV5uFyTMkY01EmUV3vAgzYv46uVJ/KDVIkf+yLwwh+rjYVTYfUy1TqDVqQf3uMOuk8f/i3yQjnfz0mq1NmeSKJpM9MEcyapWYzb+CvJ8q56JWY=";
	//std::cout << MBMail::BASE64Decode(AmazonEasterEgg.data(), AmazonEasterEgg.size())<<std::endl;
	//exit(0);
	MBSockets::HTTPConnectSocket TestConnectSocket("www.google.com", "443");
	TestConnectSocket.Connect();
	TestConnectSocket.EstablishTLSConnection();
	std::cout << TestConnectSocket.GetDataFromRequest("GET", "")<<std::endl;
	exit(0);
	//MBSockets::HTTPConnectSocket TestSocket("alt1.gmail-smtp-in.l.google.com", "465", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	//TestSocket.Connect();
	//TestSocket.EstablishSecureConnetion();
	////TestSocket.EstablishSecureConnetion();
	//TestSocket.HTTPSendData("EHLO mrboboget.se");
	////std::cout << TestSocket.GetNextDecryptedData();
	//std::cout << TestSocket.GetNextDecryptedData();
	//while (true)
	//{
	//	std::string StringToSend;
	//	std::getline(std::cin, StringToSend);
	//	TestSocket.HTTPSendData((StringToSend + "\r\n").c_str());
	//	std::cout << TestSocket.GetNextDecryptedData();
	//}
	//exit(0);



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
	std::vector<MBDNS::MXRecord> MXToUse = TestHandler.GetDomainMXRecords("tele2.com");
	size_t HighestPriortiyMXIndex = 0;
	size_t LowestPreferance = uint16_t(~0)>>1;
	for (size_t i = 0; i < MXToUse.size(); i++)
	{
		if (MXToUse[i].Preference < LowestPreferance)
		{
			HighestPriortiyMXIndex = i;
			LowestPreferance = MXToUse[i].Preference;
		}
		std::cout << MXToUse[i].Preference << " " << MXToUse[i].Exchange << std::endl;
	}
	std::string DomainToUse = MXToUse[HighestPriortiyMXIndex].Exchange;
	std::cout << "Domain to use: " << DomainToUse << std::endl;
	std::cout << "Number of MXRecords: " << MXToUse.size() << std::endl;
	


	//reverse dns test grejer
	std::vector<MBDNS::ARecord> DomainIPAdresses = TestHandler.GetDomainIPAdresses("mx1.wk.se");
	for (size_t i = 0; i < DomainIPAdresses.size(); i++)
	{
		std::cout << MBDNS::IPv4AdressToString(DomainIPAdresses[i].IPAdress) << std::endl;
	}
	//std::vector<MBDNS::PTRRecord> IPDomains = TestHandler.GetIPAdressDomains(MBDNS::IPAdressToString(DomainIPAdresses[0].IPAdress));
	//for (size_t i = 0; i < IPDomains.size(); i++)
	//{
	//	std::cout << IPDomains[i].DomainName << std::endl;
	//}
	//std::vector<MBDNS::ARecord> Domain2IPAdresses = TestHandler.GetDomainIPAdresses(IPDomains[0].DomainName);
	//for (size_t i = 0; i < Domain2IPAdresses.size(); i++)
	//{
	//	std::cout << MBDNS::IPAdressToString(Domain2IPAdresses[i].IPAdress) << std::endl;
	//}

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
	//MBSockets::HTTPConnectSocket TestSocket1("gmail-smtp-in.l.google.com", "465", MBSockets::TraversalProtocol::TCP, MBSockets::ApplicationProtocols::HTTPS);
	//TestSocket1.Connect();
	//TestSocket1.EstablishSecureConnetion();
	//alt1.gmail-smtp-in.l.google.com
	
	for (size_t i = 0; i < MXToUse.size(); i++)
	{
		MBSockets::ClientSocket TestSocket(MXToUse[i].Exchange, "587");
		TestSocket.Connect();
		//TestSocket.EstablishSecureConnetion();
		//TestSocket.HTTPSendData("EHLO mrboboget.se");
		////std::cout << TestSocket.GetNextDecryptedData();
		//std::cout << TestSocket.GetNextDecryptedData();
		//while (true)
		//{
		//	std::string StringToSend;
		//	std::getline(std::cin, StringToSend);
		//	TestSocket.SendData((StringToSend + "\r\n").c_str(), StringToSend.size() + 2);
		//	std::cout << TestSocket.GetNextDecryptedData();
		//}
		//TestSocket.SendData("EHLO mrboboget.se", 17);
		std::cout << TestSocket.RecieveData() << std::endl;
	}
	return(MBGWebsiteMain());
}
