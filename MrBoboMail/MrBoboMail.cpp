#include "MrBoboMail.h"
#include <chrono>
#include <MBCrypto/MBCrypto.h>
#include <MBSearchEngine/MBUnicode.h>
namespace MBMail
{
	void MBMailSender::SetMailbox(std::string const& NewMailbox)
	{
		m_SenderMailbox = NewMailbox;
	}
	void MBMailSender::t_SaveMail(Mail MailToSend, std::string const& PostBoxReciever, std::string const& MailOutputPath)
	{
		std::ofstream OutputFile = std::ofstream(MailOutputPath,std::ios::binary);
		t_InsertMail(MailToSend, PostBoxReciever, OutputFile);
	}
	MailError MBMailSender::SendMail(Mail MailToSend, std::string const& PostBoxReiever)
	{
		return(MailError());
		//MailError ReturnValue;
		//std::string BodyContent = p_GetMailBody(MailToSend);
		//m_DKIMSigner.SignMail(MailToSend, BodyContent);
		//m_CurrentSMTPConnectionState = p_StartSMTPConnection(PostBoxReiever.substr(PostBoxReiever.find("@") + 1));
		//p_StartMailTranser(m_CurrentSMTPConnectionState);
		//for (size_t i = 0; i < MailToSend.MimeHeaders.size(); i++)
		//{
		//	std::string CurrentHeaderData = GetMIMEHeaderLines(MailToSend.MimeHeaders[i]);
		//	m_CurrentSMTPConnectionState.DataSocket->SendData(CurrentHeaderData.data(), CurrentHeaderData.size());
		//}
		//m_CurrentSMTPConnectionState.DataSocket->SendData("\r\n", 2);
		//assert(BodyContent.find("\r\n.\r\n") == BodyContent.npos);
		////TODO fix dot-stuffing
		//m_CurrentSMTPConnectionState.DataSocket->SendData(BodyContent.data(),BodyContent.size());
		//for (size_t i = 0; i < MailToSend.Attachments.size(); i++)
		//{
		//	p_SendAttachmentData(m_CurrentSMTPConnectionState, MailToSend.Attachments[i]);
		//}
		//p_EndDataTransfer(m_CurrentSMTPConnectionState);
		//p_EndSMTPConnection(m_CurrentSMTPConnectionState);
		//return(ReturnValue);
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
		SigningData.DomainKeySelector = "PrivateMail";
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
	std::string BASE64Decode(void* CharactersToRead, size_t NumberOfCharacters)
	{
		return(MBCrypto::Base64ToBinary(std::string((char*)CharactersToRead, NumberOfCharacters)));
	}
	std::string BASE64Encode(void* DataToEncode, size_t DataLength)
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
	std::string DKIMSigner::p_GetHeadersHash(Mail const& MailToSign, DKIMSigningData const& SigningData)
	{
		std::string ReturnValue = "";
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
		if (SigningData.SigningMethod == DKIMSigningMethod::RSA_SHA256)
		{
			ReturnValue = MBCrypto::HashData(DataToHash, MBCrypto::HashFunction::SHA256);
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
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
		std::string HeaderHash = p_GetHeadersHash(MailToSign, SigningData);
		MailToSign.MimeHeaders.front().HeaderBody += p_GetHashSignature(HeaderHash, SigningData,p_GetPrivateKeyPath(SigningData.SigningDomain,SigningData.DomainKeySelector));
	}

}