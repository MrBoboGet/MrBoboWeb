#include "MrBoboMail.h"
#include <chrono>
#include <MBCrypto/MBCrypto.h>
#include <MBUnicode/MBUnicode.h>
#include <iostream>
#include <fstream>
namespace MBMail
{
	//void MBMailSender::p_SendAttachmentData(SMTPConnectionState& AssociatedConnection, MailAttachment const& AttachmentToSend)
	//{
	//	AssociatedConnection
	//}
	//void MBMailSender::p_StartMailTranser(SMTPConnectionState& StateToUpdate)
	//{
	//
	//}
	//void MBMailSender::p_EndDataTransfer(SMTPConnectionState& StateToUpdate)
	//{
	//
	//}
	//void MBMailSender::p_EndSMTPConnection(SMTPConnectionState& StateToUpdate)
	//{
	//
	//}
	void MBMailSender::SetMailbox(std::string const& NewMailbox)
	{
		m_SenderMailbox = NewMailbox;
	}
	void MBMailSender::t_SaveMail(Mail MailToSend, std::string const& PostBoxReciever, std::string const& MailOutputPath)
	{
		std::ofstream OutputFile = std::ofstream(MailOutputPath,std::ios::binary);
		t_InsertMail(MailToSend, PostBoxReciever, OutputFile);
	}
	std::unique_ptr<MBSockets::TCPClient> MBMailSender::p_GetServerConnection(std::string const& DomainName)
	{
		std::unique_ptr<MBSockets::TCPClient> ReturnValue = std::unique_ptr<MBSockets::TCPClient>(new MBSockets::TCPClient("smtp-relay.gmail.com", "465"));
		return(ReturnValue);
	}
	SMTPAuthenticationData MBMailSender::p_GetAuthenticationData()
	{
		SMTPAuthenticationData ReturnValue;
		std::ifstream AuthenticationInfoFile = std::ifstream("./ServerResources/mrboboget.se/SMTPResources/RelayInfo.txt",std::ios::in|std::ios::binary);
		std::string DataBuffer = std::string(MBGetFileSize("./ServerResources/mrboboget.se/SMTPResources/RelayInfo.txt"), 0);
		AuthenticationInfoFile.read(DataBuffer.data(), MBGetFileSize("./ServerResources/mrboboget.se/SMTPResources/RelayInfo.txt"));
		size_t ParseOffset = 0;
		ReturnValue.RelayDomain = DataBuffer.substr(DataBuffer.find(':', ParseOffset) + 1, std::min(DataBuffer.find('\n', ParseOffset), DataBuffer.find('\r', ParseOffset)) - ParseOffset);
		ParseOffset = std::min(DataBuffer.find('\n', ParseOffset), DataBuffer.find('\r', ParseOffset)) + 2;
		ReturnValue.PlaintextUser = DataBuffer.substr(DataBuffer.find(':', ParseOffset) + 1, std::min(DataBuffer.find('\n', ParseOffset), DataBuffer.find('\r', ParseOffset)) - ParseOffset);
		ParseOffset = std::min(DataBuffer.find('\n', ParseOffset), DataBuffer.find('\r', ParseOffset)) + 2;
		ReturnValue.PlaintextPassword = DataBuffer.substr(DataBuffer.find(':', ParseOffset) + 1, std::min(DataBuffer.find('\n', ParseOffset), DataBuffer.find('\r', ParseOffset)) - ParseOffset);
		return(ReturnValue);
	}
	MailError MBMailSender::SendMail(Mail MailToSend, std::string const& PostBoxReciever,SMTPSendInfo const& SendInfo)
	{
		//return(MailError());
		MailError ReturnValue;
		std::string BodyContent = p_GetMailBody(MailToSend);
		m_DKIMSigner.SignMail(MailToSend, BodyContent);
		std::unique_ptr<MBSockets::TCPClient> MailServerConnection = p_GetServerConnection(PostBoxReciever.substr(PostBoxReciever.find_last_of('@') + 1));
		m_CurrentSMTPConnectionState = p_StartSMTPConnection<MBSockets::ConnectSocket>(PostBoxReciever.substr(PostBoxReciever.find("@") + 1), MailServerConnection.get(),SendInfo);
		p_Authenticate(m_CurrentSMTPConnectionState, MailServerConnection);
		p_StartMailTranser(m_CurrentSMTPConnectionState, MailServerConnection,SendInfo);
		for (size_t i = 0; i < MailToSend.MimeHeaders.size(); i++)
		{
			std::string CurrentHeaderData = GetMIMEHeaderLines(MailToSend.MimeHeaders[i]);
			MailServerConnection->SendData(CurrentHeaderData);
		}
		MailServerConnection->SendData("\r\n");
		assert(BodyContent.find("\r\n.\r\n") == BodyContent.npos);
		//TODO fix dot-stuffing
		MailServerConnection->SendData(BodyContent);
		//for (size_t i = 0; i < MailToSend.Attachments.size(); i++)
		//{
		//	p_SendAttachmentData(m_CurrentSMTPConnectionState, MailToSend.Attachments[i]);
		//}
		p_EndDataTransfer(m_CurrentSMTPConnectionState,MailServerConnection);
		p_EndSMTPConnection(m_CurrentSMTPConnectionState, MailServerConnection);
		return(ReturnValue);
	}
	std::vector<MIMEHeader> MBMailSender::p_GetMIMEContentMIMEHeaders(MBMIME::MIMEType MIMETypeToCheck)
	{
		std::vector<MIMEHeader> ReturnValue = {};
		
		return(ReturnValue);
	}
	std::string MBMailSender::p_GetAttachmentNonOcurringString(MailAttachment const& AttachmentToCheck, std::string const& CurrentString)
	{
		return(p_GetMimeHeadersNonOcurringString(p_GetAttachmentMIMEHeaders(AttachmentToCheck), CurrentString));
	}
	std::vector<MIMEHeader> MBMailSender::p_GetMailTopLevelMimeHeaders(Mail const& MailToParse)
	{
		MBMIME::MIMETypeConnector MimeConnector;
		std::vector<MIMEHeader> ReturnValue = {};
		MIMEHeader MIMEVersion;
		MIMEVersion.HeaderName = "MIME-Version";
		MIMEVersion.HeaderBody = "1.0";
		ReturnValue.push_back(MIMEVersion);
		//MIMEHeader ContentType;
		//ContentType.HeaderName = "Content-Type";
		if (MailToParse.Attachments.size() == 0)
		{
			//ContentType.HeaderBody = MimeConnector.GetTupleFromDocumentType(MailToParse.Body.BodyType).MIMEMediaString;
			std::vector<MIMEHeader> BodyHeaders = p_GetMIMEContentMIMEHeaders(MailToParse.Body.BodyType);
			for (size_t i = 0; i < BodyHeaders.size(); i++)
			{
				ReturnValue.push_back(BodyHeaders[i]);
			}
		}
		else
		{
			MIMEHeader MultiPart;
			MultiPart.HeaderName = "Content-Type";
			MultiPart.HeaderBody = "multipart/mixed; boundary=\"";
			std::string Boundary = p_GetBASE64NonOcurringString();
			for (size_t i = 0; i < MailToParse.Attachments.size(); i++)
			{
				Boundary = p_GetAttachmentNonOcurringString(MailToParse.Attachments[i], Boundary);
			}
			MultiPart.HeaderBody += Boundary + "\"";
		}
		return(ReturnValue);
	}
	std::string MBMailSender::p_GetDateString()
	{
		return("Date: Tue, 18 Nov 2014 15:57:11 +0000");
	}
	void MBMailSender::FillMailHeaders(Mail& MailToEdit, StandardUserEmailHeaders const& UserInputHeaders)
	{
		std::vector<MIMEHeader> MIMEProtocolHeaders = p_GetMailTopLevelMimeHeaders(MailToEdit);
		std::vector<MIMEHeader> SMTPUserInputHeaders = {};
		SMTPUserInputHeaders.push_back({ "From",UserInputHeaders.From });
		SMTPUserInputHeaders.push_back({ "To",UserInputHeaders.To });
		SMTPUserInputHeaders.push_back({ "Subject",UserInputHeaders.Subject });
		std::vector<MIMEHeader> SMTPClientHeaders = {};
		SMTPClientHeaders.push_back({ "Date",p_GetDateString() });
		MailToEdit.MimeHeaders = {};
		for (size_t i = 0; i < SMTPUserInputHeaders.size(); i++)
		{
			MailToEdit.MimeHeaders.push_back(SMTPUserInputHeaders[i]);
		}
		for (size_t i = 0; i < SMTPClientHeaders.size(); i++)
		{
			MailToEdit.MimeHeaders.push_back(SMTPClientHeaders[i]);
		}
		for (size_t i = 0; i < MIMEProtocolHeaders.size(); i++)
		{
			MailToEdit.MimeHeaders.push_back(MIMEProtocolHeaders[i]);
		}
	}
	std::vector<MIMEHeader> MBMailSender::p_GetAttachmentMIMEHeaders(MailAttachment const& AttachmentToCheck)
	{
		MBMIME::MIMETypeConnector TypeConnector;
		std::vector<MIMEHeader> ReturnValue = {};
		MIMEHeader ContentType;
		ContentType.HeaderName = "Content-Type";
		ContentType.HeaderBody = TypeConnector.GetTupleFromDocumentType(AttachmentToCheck.AttachmentMIMEType).MIMEMediaString + "; name\"" + AttachmentToCheck.AttachmentName + "\"";
		//Content-Transfer-Encoding: base64
		//Content-Disposition: attachment; filename="Emanuel 20 år.pdf"
		ReturnValue.push_back(ContentType);
		MIMEHeader TransferEncoding;
		TransferEncoding.HeaderName = "Content-Transfer-Encoding";
		TransferEncoding.HeaderBody = "base64";
		ReturnValue.push_back(TransferEncoding);
		MIMEHeader ContentDisposition;
		ContentDisposition.HeaderName = "Content-Disposition";
		ContentDisposition.HeaderBody = "attachment; filename=\"" + AttachmentToCheck.AttachmentName + "\"";
		ReturnValue.push_back(ContentDisposition);
		return(ReturnValue);
	}
	//std::string MBMailSender::p_GetMailHeaderNonOcurringString(Mail const& MailToCheck, std::string const& CurrentString)
	//{
	//
	//}
	void h_ModifyNonOcurringStringHit(std::string& NonOcurringStringToModify, std::string const& StringToCheck, size_t HitParseOffset)
	{
		if (HitParseOffset + NonOcurringStringToModify.size() == StringToCheck.size())
		{
			NonOcurringStringToModify += "0";
		}
		else
		{
			//Antagande, stringen vi kollar är printable ascii
			NonOcurringStringToModify += (StringToCheck[HitParseOffset + NonOcurringStringToModify.size()] + 1);
			if (NonOcurringStringToModify.back() > 0x7E)
			{
				NonOcurringStringToModify.back() = 0x20;
			}
			if (NonOcurringStringToModify.back() < 0x20)
			{
				NonOcurringStringToModify.back() = 0x7E;
			}
		}
	}
	std::string MBMailSender::p_GetMimeHeadersNonOcurringString(std::vector<MIMEHeader> const& HeadersToCheck, std::string const& CurrentString)
	{
		std::string ReturnValue = CurrentString;
		for (size_t i = 0; i < HeadersToCheck.size(); i++)
		{
			std::string StringToCheck = HeadersToCheck[i].HeaderName + ":" + HeadersToCheck[i].HeaderBody + "\r\n";
			ReturnValue = p_GetNonOcurringString(StringToCheck, ReturnValue);
		}
		return(ReturnValue);
	}
	std::string MBMailSender::p_GetBASE64NonOcurringString()
	{
		return("-");
	}
	std::string MBMailSender::p_GetNonOcurringString(std::string const& StringToCheck, std::string const& CurrentString)
	{
		std::string ReturnValue = CurrentString;
		size_t ParseOffset = 0;
		while (ParseOffset < StringToCheck.size())
		{
			ParseOffset = StringToCheck.find(ReturnValue,ParseOffset);
			if (ParseOffset != StringToCheck.npos)
			{
				h_ModifyNonOcurringStringHit(ReturnValue, StringToCheck, ParseOffset);
			}
		}
		return(ReturnValue);
	}
	void DKIMSigner::SignMail(Mail& MailToSign,std::string const& BodyData) //adds headers and header values to the mail to send
	{
		DKIMSigningData SigningData;
		SigningData.BodyNormalizationMethod = DKIMNormalizationMethod::Simple;
		SigningData.HeaderNormalizationMethod = DKIMNormalizationMethod::Simple;
		SigningData.DomainKeySelector = "privatemail";
		SigningData.KeyQuerryMethod = DKIMDomainkeyQuerryMethod::DNS_TXT;
		SigningData.SigningDomain = "mrboboget.se";
		SigningData.SigningMethod = DKIMSigningMethod::RSA_SHA256;
		SigningData.NumberOfBodyOctetsToSign = -1;
		std::vector<std::string> HeadersToSign = { "From","Date","To","Subject" };
		SigningData.HeadersToSign = HeadersToSign;
		SignMail(MailToSign, BodyData, SigningData);
	}
	std::string DKIMSigner::p_GetSigningMethodString(DKIMSigningMethod MethodToEvaluate)
	{
		if (MethodToEvaluate == DKIMSigningMethod::RSA_SHA256)
		{
			return("rsa-sha256");
		}
	}
	std::string DKIMSigner::p_GetQuerryMethodString(DKIMDomainkeyQuerryMethod MethodToEvaluate)
	{
		if (MethodToEvaluate == DKIMDomainkeyQuerryMethod::DNS_TXT)
		{
			return("dns/txt");
		}
	}
	std::string DKIMSigner::p_GetHashSignature(std::string const& HashToSign, DKIMSigningData const& SigningData, std::string const& PrivateKeyPath)
	{
		std::string ReturnValue = "";
		if (SigningData.SigningMethod != DKIMSigningMethod::RSA_SHA256)
		{
			assert(false);
		}
		ReturnValue = MBCrypto::RSASSA_PKCS1_V1_5_SIGN(HashToSign,PrivateKeyPath,MBCrypto::HashFunction::SHA256);
		ReturnValue = BASE64Encode(ReturnValue.data(), ReturnValue.size());
		return(ReturnValue);
	}
	std::string MBMailSender::p_GetMailBody(Mail const& MailToParse)
	{
		//TODO fixa så det faktiskt här hela kroppen inte bara texten, attachments dvs
		return(MailToParse.Body.BodyData);
	}
	MIMEHeader DKIMSigner::p_GenerateInitialDKIMHeader(DKIMSigningData const& SigningData)
	{
		if (SigningData.SigningMethod == DKIMSigningMethod::RSA_SHA256)
		{
			MIMEHeader ReturnValue;
			ReturnValue.HeaderName = "DKIM-Signature";
			ReturnValue.HeaderBody = "v=1; a="+p_GetSigningMethodString(SigningData.SigningMethod) +"; d=" + SigningData.SigningDomain + "; s=" + SigningData.DomainKeySelector + "; ";
			ReturnValue.HeaderBody += "q=" + p_GetQuerryMethodString(SigningData.KeyQuerryMethod) + "; ";
			const auto CurrentTime = std::chrono::system_clock::now();
			uint32_t UnixTime = std::chrono::duration_cast<std::chrono::seconds>(CurrentTime.time_since_epoch()).count();
			ReturnValue.HeaderBody += "t=" + std::to_string(UnixTime) + "; ";
			ReturnValue.HeaderBody += "h=";
			for (size_t i = 0; i < SigningData.HeadersToSign.size(); i++)
			{
				ReturnValue.HeaderBody += SigningData.HeadersToSign[i];
				if (i + 1 < SigningData.HeadersToSign.size())
				{
					ReturnValue.HeaderBody += ":";
				}
			}
			ReturnValue.HeaderBody += "; bh=";
			return(ReturnValue);
		}
		else
		{
			assert(false);
		}
		return(MIMEHeader());
	}
	char ByteToBASE64(uint8_t ByteToEncode)
	{
		if (ByteToEncode >= 0 && ByteToEncode <= 25)
		{
			return(ByteToEncode + 65);
		}
		else if (ByteToEncode >= 26 && ByteToEncode <= 51)
		{
			return(ByteToEncode + 71);
		}
		else if (ByteToEncode >= 52 && ByteToEncode <= 61)
		{
			return(ByteToEncode - 4);
		}
		else if (ByteToEncode == 62)
		{
			return('+');
		}
		else if (ByteToEncode == 63)
		{
			return('/');
		}
		assert(false);
	}
	std::string BASE64Decode(const void* CharactersToRead, size_t NumberOfCharacters)
	{
		return(MBCrypto::Base64ToBinary(std::string((char*)CharactersToRead, NumberOfCharacters)));
	}
	std::string BASE64Encode(const void* DataToEncode, size_t DataLength)
	{
		std::string ReturnValue = "";
		uint32_t DataBuffer = 0;
		size_t BitsInBuffer = 0;
		size_t CurrentByteOffset = 0;
		uint8_t* DataPointer = (uint8_t*)DataToEncode;
		while (CurrentByteOffset < DataLength)
		{
			DataBuffer <<= 8;
			DataBuffer += DataPointer[CurrentByteOffset];
			BitsInBuffer += 8;
			while(BitsInBuffer >= 6)
			{
				uint8_t BitsBeforeData = (BitsInBuffer - 6);
				ReturnValue += ByteToBASE64((DataBuffer) >> BitsBeforeData);
				BitsInBuffer -= 6;
				DataBuffer = DataBuffer & (~(63 << BitsBeforeData));
			}
			CurrentByteOffset += 1;
		}
		while (BitsInBuffer != 0)
		{
			int8_t BitsBeforeData = (BitsInBuffer - 6);
			if (BitsBeforeData >= 0)
			{
				BitsInBuffer -= 6;
			}
			else
			{
				DataBuffer = DataBuffer << (-BitsBeforeData);
				BitsInBuffer = 0;
			}
			if (BitsBeforeData < 0)
			{
				BitsBeforeData = 0;
			}
			ReturnValue += ByteToBASE64((DataBuffer) >> BitsBeforeData);
			DataBuffer = DataBuffer & (~(63 << BitsBeforeData));
		}
		while(ReturnValue.size() %4 != 0)
		{
			ReturnValue += "=";
		}
		return(ReturnValue);
	}
	std::string BASE64Decode(std::string const& DataToDecode)
	{
		return(BASE64Decode(DataToDecode.data(), DataToDecode.size()));
	}
	std::string BASE64Encode(std::string const& DataToEncode)
	{
		return(BASE64Encode(DataToEncode.data(), DataToEncode.size()));
	}
	std::string GetMIMEHeaderLines(MIMEHeader const& HeaderToEncode)
	{
		std::string ReturnValue = HeaderToEncode.HeaderName + ": " + HeaderToEncode.HeaderBody + "\r\n";
		return(ReturnValue);
	}
	std::string DKIMSigner::p_GetBodyHash(std::string const& BodyData, DKIMSigningData const& SigningData)
	{
		//TODO läg till support för support mer än simple
		if (SigningData.BodyNormalizationMethod != DKIMNormalizationMethod::Simple)
		{
			assert(false);
		}
		std::string ReturnValue = "";
		if (SigningData.SigningMethod == DKIMSigningMethod::RSA_SHA256)
		{
			ReturnValue = MBCrypto::HashData(BodyData, MBCrypto::HashFunction::SHA256);
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}
	MIMEHeader DKIMSigner::p_GetHeader(std::vector<MIMEHeader> const& HeadersToSearch, std::string const& HeaderName)
	{
		MIMEHeader ReturnValue;
		for (size_t i = 0; i < HeadersToSearch.size(); i++)
		{
			if (MBUnicode::UnicodeStringToLower(HeadersToSearch[i].HeaderName) == MBUnicode::UnicodeStringToLower(HeaderName))
			{
				ReturnValue = HeadersToSearch[i];
				break;
			}
		}
		return(ReturnValue);
	}
	std::string DKIMSigner::p_GetHeaderDataToHash(Mail const& MailToSign, DKIMSigningData const& SigningData)
	{
		std::string DataToHash = "";
		if (SigningData.HeaderNormalizationMethod != DKIMNormalizationMethod::Simple)
		{
			assert(false);
		}
		for (size_t i = 0; i < SigningData.HeadersToSign.size(); i++)
		{
			DataToHash += GetMIMEHeaderLines(p_GetHeader(MailToSign.MimeHeaders, SigningData.HeadersToSign[i]));
		}
		//ANTAGANDE första headern här är alltid 
		DataToHash += GetMIMEHeaderLines(p_GetHeader(MailToSign.MimeHeaders, "DKIM-Signature"));
		//Because the last \r\n shouldnt be included in the hash
		DataToHash.resize(DataToHash.size() - 2);
		//if (SigningData.SigningMethod == DKIMSigningMethod::RSA_SHA256)
		//{
		//	ReturnValue = MBCrypto::HashData(DataToHash, MBCrypto::HashFunction::SHA256);
		//}
		//else
		//{
		//	assert(false);
		//}
		return(DataToHash);
	}
	std::string DKIMSigner::p_GetPrivateKeyPath(std::string const& DomainToSign, std::string const& SignSelector)
	{
		//debug
		return("ServerResources/mrboboget.se/EncryptionResources/KeyfileRSA2096.key");
		//return("./ServerResources/SMTPResources/" + DomainToSign + "/DKIM/" + SignSelector + "/PrivateKey.key");
	}
	void DKIMSigner::SignMail(Mail& MailToSign,std::string const& BodyData, DKIMSigningData const& SigningData)
	{
		MIMEHeader NewHeader = p_GenerateInitialDKIMHeader(SigningData);
		std::string BodyHash = p_GetBodyHash(BodyData, SigningData);
		NewHeader.HeaderBody += BASE64Encode(BodyHash.data(),BodyHash.size())+"; b=";
		std::vector<MIMEHeader> NewHeaders = {};
		NewHeaders.push_back(NewHeader);
		for (size_t i = 0; i < MailToSign.MimeHeaders.size(); i++)
		{
			NewHeaders.push_back(MailToSign.MimeHeaders[i]);
		}
		MailToSign.MimeHeaders = NewHeaders;
		std::string HeaderHash = p_GetHeaderDataToHash(MailToSign, SigningData);
		MailToSign.MimeHeaders.front().HeaderBody += p_GetHashSignature(HeaderHash, SigningData,p_GetPrivateKeyPath(SigningData.SigningDomain,SigningData.DomainKeySelector));
	}
	//END DKIMSigner

	//BEGIN MBMailReciever
	bool MBMailReciever::p_VerifyUser(std::string const& UsernameToVerify)
	{
		assert(false);
		return(true);
	}
	void MBMailReciever::p_AddConnection(size_t ConnectionID, std::shared_ptr<MBMailSMTPServerConnection> ConnectionToAdd)
	{
		std::lock_guard<std::mutex> InternalsLock(m_InternalsMutex);
		m_ActiveConnections[ConnectionID] = ConnectionToAdd;
	}
	void MBMailReciever::p_RemoveConnection(size_t ConnectionID)
	{
		std::lock_guard<std::mutex> InternalsLock(m_InternalsMutex);
		m_ActiveConnections.erase(ConnectionID);
	}
	void MBMailReciever::StartListening(std::string const& AssociatedDomain, std::string const& PortToListenTo, bool UseImplicitTLS)
	{
		MBSockets::TCPServer ListenSocket(PortToListenTo);
		ListenSocket.Bind();
		size_t CurrentConnectionID = 0;
		while (true)
		{
			ListenSocket.Listen();
			ListenSocket.Accept();
			MBSockets::TCPServer* NewSocket = new MBSockets::TCPServer();
			ListenSocket.TransferConnectedSocket(NewSocket);
			MBSockets::TLSConnectSocket* NewConnection =new MBSockets::TLSConnectSocket(std::unique_ptr<MBSockets::TCPServer>(NewSocket));
			if (UseImplicitTLS)
			{
				NewConnection->EstablishTLSConnection(true,"");
			}
			std::shared_ptr<MBSockets::TLSConnectSocket> SharedSocketPointer(NewConnection);
			MBMailSMTPServerConnection* ConnectionPointer = new MBMailSMTPServerConnection(SharedSocketPointer, CurrentConnectionID, UseImplicitTLS);
			p_AddConnection(CurrentConnectionID, std::shared_ptr<MBMailSMTPServerConnection>(ConnectionPointer));
			CurrentConnectionID += 1;
		}
	}
	std::string MBMailReciever::p_GetTempDirectory()
	{
		return("");
	}
	std::string MBMailReciever::p_GetUserDirectory(std::string const& Username)
	{
		return("");
	}
	std::string MBMailReciever::p_GetTimeString()
	{
		return("");
	}
	//END MBMailReciever
	//BEGIN MBMailSMTPConnection
	bool MBMailSMTPServerConnection::p_VerifyPRF(std::string const& DomainToVerify, std::string const& ConnectionIP)
	{
		return(true);
	}
	bool MBMailSMTPServerConnection::p_VerifyDKIM(std::string const& DomainToVerify, std::string const& MailFilePath)
	{
		return(true);
	}
	bool MBMailSMTPServerConnection::p_VerifyRDNS(std::string const& ConnectionIP)
	{
		return(true);
	}
	void MBMailSMTPServerConnection::p_SendServerHello(MBSockets::ConnectSocket* AssociatedSocket)
	{
		AssociatedSocket->SendData("220 OwO");
	}
	void MBMailSMTPServerConnection::p_SendExtensions(MBSockets::ConnectSocket* AssociatedSocket)
	{
		AssociatedSocket->SendData("250 STARTTLS");
	}
	void MBMailSMTPServerConnection::p_GetPostboxParts(std::string& Username, std::string& Domain, std::string const& LineData)
	{
		Username = "";
		Domain = "";
		size_t RecieverBegin = LineData.find('<');
		size_t AtLocation = LineData.find('@');
		if (RecieverBegin != LineData.npos)
		{
			RecieverBegin += 1;
		}
		if (RecieverBegin == LineData.npos || AtLocation == LineData.npos)
		{
			return;
		}
		Username = LineData.substr(RecieverBegin, AtLocation - RecieverBegin);
		Domain = LineData.substr(AtLocation + 1, LineData.find_last_of('>') - AtLocation - 1);
	}
	void MBMailSMTPServerConnection::p_CloseConnection()
	{
		m_ConnectionIsOpen = false;
		m_AssociatedSocket->Close();
		m_TransactionState = SMTPTransactionState();
	}
	void MBMailSMTPServerConnection::p_HandleCommand(std::string const& CommandData)
	{
		std::string CommandVerb = MBUnicode::UnicodeStringToLower(CommandData.substr(0, CommandData.find(' ')));
		if (CommandVerb == "ehlo")
		{
			m_ClientDomain = CommandData.substr(CommandVerb.size() + 1,CommandData.size()-2-CommandVerb.size()-1);
			if (m_ClientDomain == "")
			{
				m_AssociatedSocket->SendData("501 Invalid syntax: empty HELO/EHLO argument not allowed\r\n");
				return;
			}
			else
			{
				p_SendExtensions(m_AssociatedSocket.get());
				return;
			}
		}
		else if (CommandVerb == "starttls")
		{
			m_AssociatedSocket->SendData("250 Ready to start TLS\r\n");
			m_AssociatedSocket->EstablishTLSConnection(true,"");
			return;
		}
		else if (CommandVerb == "mail")
		{
			std::string SenderUsername;
			std::string SenderDomain;
			p_GetPostboxParts(SenderUsername, SenderDomain, CommandData);
			if (SenderUsername == "" || SenderDomain == "")
			{
				m_AssociatedSocket->SendData("501 Invalid postbox\r\n");
				return;
			}
			//bool PRFPassed = p_VerifyPRF(SenderDomain,m_AssociatedSocket->GetIpOfConnectedSocket());
			bool PRFPassed = false;
			if (!PRFPassed)
			{
				m_AssociatedSocket->SendData("550 PRF not passed, closing connection\r\n");
				p_CloseConnection();
				return;
			}
			m_TransactionState.InTransaction = true;
			m_TransactionState.SenderDomain = SenderDomain;
			m_TransactionState.SenderUsername = SenderUsername;
			m_FileToSaveToPath = m_AssociatedReciever->p_GetTempDirectory() + m_AssociatedReciever->p_GetTimeString()+".eml";
			m_FileToSaveTo.open(m_FileToSaveToPath, std::ios::binary | std::ios::out);
			m_AssociatedSocket->SendData("250 Allowed sender\r\n");
			return;
		}
		else if (CommandVerb == "rcpt")
		{
			std::string RecieverUsername;
			std::string RecieverDomain;
			p_GetPostboxParts(RecieverUsername, RecieverDomain, CommandData);
			if (RecieverUsername == "" || RecieverDomain == "")
			{
				m_AssociatedSocket->SendData("501 Invalid postbox\r\n");
				return;
			}
			else
			{
				if (!m_AssociatedReciever->p_VerifyUser(RecieverUsername))
				{
					m_AssociatedSocket->SendData("450 Invalid postbox: user not found\r\n");
					return;
				}
				if (RecieverDomain != "mrboboget.se")
				{
					m_AssociatedSocket->SendData("550 Only accepts mail to same domain\r\n");
					return;
				}
				m_TransactionState.Recipients.push_back(RecieverUsername);
				m_AssociatedSocket->SendData("250 Recipient accepted\r\n");
				return;
			}
		}
		else if (CommandVerb == "data")
		{
			m_TransactionState.InDataTransfer = true;
			m_AssociatedSocket->SendData("354 Ready to recieve data\r\n");
		}
		else if (CommandVerb == "quit")
		{
			m_AssociatedSocket->SendData("221 Bye\r\n");
			p_CloseConnection();
			return;
		}
		else
		{
			m_AssociatedSocket->SendData("502 \r\n");
			return;
		}
	}
	void MBMailSMTPServerConnection::p_HandleConnection()
	{
		p_SendServerHello(m_AssociatedSocket.get());
		std::string NextClientData;
		size_t TotalDataRecieved = 0;
		while (m_AssociatedSocket->IsValid() && m_AssociatedSocket->IsValid() && m_ConnectionIsOpen)
		{
			//rätt så godtyckligt tal
			NextClientData = m_AssociatedSocket->RecieveData(1000000);
			if (!m_TransactionState.InDataTransfer)
			{
				p_HandleCommand(NextClientData);
			}
			else
			{
				TotalDataRecieved += NextClientData.size();
				m_FileToSaveTo << NextClientData;
				if (NextClientData.find("\r\n.\r\n") != NextClientData.npos)
				{
					m_TransactionState.InDataTransfer = false;
					m_TransactionState.InTransaction = false;
					m_FileToSaveTo.flush();
					m_FileToSaveTo.close();
					bool DKIMIsValid = p_VerifyDKIM(m_TransactionState.SenderDomain, m_FileToSaveToPath);
					if (DKIMIsValid)
					{
						m_AssociatedSocket->SendData("250 Data recieved");
					}
					else
					{
						m_AssociatedSocket->SendData("450 DKIM Invalid, closing connection");
						p_CloseConnection();
						return;
					}
				}
			}
		}
		m_AssociatedReciever->p_RemoveConnection(m_ConnectionID);
	}
	MBMailSMTPServerConnection::MBMailSMTPServerConnection(std::shared_ptr<MBSockets::TLSConnectSocket> AssociatedConnection, size_t ConnectionID, bool UsedImplicitTLS)
	{
		m_AssociatedSocket = AssociatedConnection;
		m_ConnectionID = ConnectionID;
		m_UsedImplicitTLS = UsedImplicitTLS;
	}
}