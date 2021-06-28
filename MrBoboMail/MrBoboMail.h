#pragma once
#include <string>
#include <MrBoboSockets.h>
#include <MBMime/MBMime.h>
#include <MBSearchEngine/MBUnicode.h>
namespace MBMail
{
	struct MailAttachment
	{
		std::string AttachmentName = "";
		std::string AttachmentFilepath = "";
		MBMIME::MIMEType AttachmentMIMEType = MBMIME::MIMEType::Null;
	};
	struct MailHeaders
	{

	};
	struct MIMEHeader
	{
		std::string HeaderName = "";
		std::string HeaderBody = "";
	};
	struct MailBody
	{
		MBMIME::MIMEType BodyType = MBMIME::MIMEType::Text;
		std::string BodyData = "";
	};
	struct Mail
	{
		//MailHeaders Headers;
		std::vector<MIMEHeader> MimeHeaders = {};
		MailBody Body;
		std::vector<MailAttachment> Attachments = {};
	};
	enum class SMTPStatusCode
	{
		OK = 250,
		Null,
	};
	struct MailError
	{
		std::string ErrorMessage = "";
		SMTPStatusCode StatusCode = SMTPStatusCode::Null;
	};
	class BASE64Encoder
	{
	private:

	public:
	};
	struct MIMEDocument
	{
		std::vector<MIMEHeader> Headers = {};
		std::string BodyContent = "";
	};
	class BASE64Decoder
	{
	private:

	public:
	};
	char ByteToBASE64(uint8_t ByteToEncode);
	std::string BASE64Decode(const void* CharactersToRead, size_t NumberOfCharacters);
	std::string BASE64Encode(const void* DataToEncode, size_t DataLength);

	enum class DKIMNormalizationMethod
	{
		Simple,
		Relaxed,
		Null
	};
	enum class DKIMSigningMethod
	{
		RSA_SHA1,
		RSA_SHA256,
		Null
	};
	enum class DKIMDomainkeyQuerryMethod
	{
		DNS_TXT,
		Null
	};
	struct DKIMSigningData
	{
		DKIMNormalizationMethod HeaderNormalizationMethod = DKIMNormalizationMethod::Simple;
		DKIMNormalizationMethod BodyNormalizationMethod = DKIMNormalizationMethod::Simple;
		DKIMSigningMethod SigningMethod = DKIMSigningMethod::RSA_SHA256;
		std::string SigningDomain = "";
		std::string DomainKeySelector = "";
		DKIMDomainkeyQuerryMethod KeyQuerryMethod = DKIMDomainkeyQuerryMethod::DNS_TXT;
		std::vector<std::string> HeadersToSign = {};
		uintmax_t NumberOfBodyOctetsToSign = -1;//= oändligt
	};
	std::string GetMIMEHeaderLines(MIMEHeader const& HeaderToEncode);
	class DKIMSigner
	{
	private:
		static std::string p_GetSigningMethodString(DKIMSigningMethod MethodToEvaluate);
		static std::string p_GetQuerryMethodString(DKIMDomainkeyQuerryMethod MethodToEvaluate);
		static MIMEHeader p_GenerateInitialDKIMHeader(DKIMSigningData const& SigningData);
		static std::string p_GetBodyHash(std::string const& BodyData, DKIMSigningData const& SigningData);
		static std::string p_GetBodyHash(Mail const& MailToSign, DKIMSigningData const& SigningData);
		static std::string p_GetHeaderDataToHash(Mail const& MailToSign, DKIMSigningData const& SigningData);
		static std::string p_GetPrivateKeyPath(std::string const& DomainToSign, std::string const& SignSelector);
		static std::string p_GetHashSignature(std::string const& HashToSign, DKIMSigningData const& SigningData,std::string const& PrivateKeyPath);
		static MIMEHeader p_GetHeader(std::vector<MIMEHeader> const& HeadersToSearch, std::string const& HeaderName);
	public:
		void SignMail(Mail& MailToSign,std::string const& BodyData); //adds headers and header values to the mail to send
		static void SignMail(Mail& MailToSign,std::string const& BodyData, DKIMSigningData const& SigningData);
	};
	struct SMTPConnectionState
	{
		//MBSockets::ConnectSocket* DataSocket = nullptr;
		bool Connected = false;
		bool InTransaction = false;
		uintmax_t MaxMessageSize = 0;
		std::unordered_map<std::string,bool> ServerExtensions = {};
		SMTPStatusCode LatestCommandStatus = SMTPStatusCode::Null;
		std::string LatestErrorString = "";
	};
	struct StandardUserEmailHeaders
	{
		std::string To = "";
		std::string From = "";
		std::string Subject = "";
	};
	struct SMTPResponse
	{
		SMTPStatusCode StatusCode = SMTPStatusCode::Null;
		std::vector<std::string> ResponseLines = {};
	};
	struct SMTPSendInfo
	{
		std::string Domain = "";
		std::string User = "";
		std::vector<std::string> Recievers = {};
	};
	inline bool SMTPStatusIsError(SMTPStatusCode StatusToCheck)
	{
		if ((uint32_t)StatusToCheck >= 200)
		{
			return(false);
		}
		else
		{
			return(true);
		}
	}
	class MBMailSender
	{
	private:
		std::string m_SenderMailbox = "";
		DKIMSigner m_DKIMSigner;
		SMTPConnectionState m_CurrentSMTPConnectionState;
		static std::vector<MIMEHeader> p_GetAttachmentMIMEHeaders(MailAttachment const& AttachmentToCheck);
		static std::vector<MIMEHeader> p_GetMailTopLevelMimeHeaders(Mail const& MailToParse);
		static std::vector<MIMEHeader> p_GetMIMEContentMIMEHeaders(MBMIME::MIMEType MIMETypeToCheck);
		//std::string p_GetMailHeaderNonOcurringString(Mail const& MailToCheck,std::string const& CurrentString);
		static std::string p_GetAttachmentNonOcurringString(MailAttachment const& AttachmentToCheck, std::string const& CurrentString);
		static std::string p_GetMimeHeadersNonOcurringString(std::vector<MIMEHeader> const& HeadersToCheck, std::string const& CurrentString);
		static std::string p_GetNonOcurringString(std::string const& StringToCheck, std::string const& CurrentString);
		static std::string p_GetBASE64NonOcurringString();
		static std::string p_GetMailBody(Mail const& MailToParse);

		template<typename MBOctetCommunication>
		SMTPResponse p_GetCommandResponse(std::string const& CommandToSend, MBOctetCommunication* CommunicationStream)
		{
			SMTPResponse ReturnValue;
			(*CommunicationStream) << CommandToSend;
			std::string ResponseData;
			(*CommunicationStream) >> ResponseData;
			std::string StatusString = ResponseData.substr(0, 3);
			ReturnValue.StatusCode = (SMTPResponse) std::stoi(ResponseData);
			bool MultiLine = (ResponseData[3] == ' ');
			if (MultiLine)
			{
				while (true)
				{
					size_t LastResponseCodePosition = 0;
					size_t StringSize = ResponseData.size();
					for (size_t i = 2; i < ResponseData.size() - 1; i++)
					{
						if (ResponseData[StringSize - 1 - i] == '\n' && ResponseData[StringSize - 2 - i] == '\r')
						{
							LastResponseCodePosition = StringSize - i;
						}
					}
					if (ResponseData[LastResponseCodePosition + 3] != ' ' || ResponseData.substr(ResponseData.size() - 3) != "\r\n")
					{
						std::string NewData;
						(*CommunicationStream) >> NewData;
						ResponseData += NewData;
					}
					else
					{
						break;
					}
				}
			}
			std::vector<std::string> ResponseLines = MBUtility::Split(ResponseData, "\r\n");
			for (size_t i = 0; i < ResponseLines.size(); i++)
			{
				ReturnValue.ResponseLines.push_back(ResponseLines[i].substr(4));
			}
			return(ReturnValue);
		}
		template<typename MBOctetCommunication>
		SMTPConnectionState p_StartSMTPConnection(std::string const& DomainToSendTo, MBOctetCommunication* CommunicationStream,SMTPSendInfo const& SendInfo)
		{
			SMTPConnectionState ReturnValue;
			SMTPResponse Response = p_GetCommandResponse("EHLO " + SendInfo.Domain);
			ReturnValue.LatestCommandStatus = Response.StatusCode;
			for (size_t i = 0; i < Response.ResponseLines.size(); i++)
			{
				if (MBUnicode::UnicodeStringToLower(Response.ResponseLines[i].substr(0, 4)) == "size")
				{
					ReturnValue.MaxMessageSize = std::stoi(Response.ResponseLines[i].substr(5));
					ReturnValue.ServerExtensions[Response.ResponseLines[i].substr(0, 4)] = true;
				}
				else
				{
					ReturnValue.ServerExtensions[Response.ResponseLines[i]] = true;
				}
			}
			if (SMTPStatusIsError(Response.StatusCode))
			{
				ReturnValue.LatestErrorString = Response.ResponseLines[0];
			}
			ReturnValue.Connected = true;
			return(ReturnValue);
		}
		template<typename MBOctetCommunication> void p_SendDotStuffedData(SMTPConnectionState& AssociatedConnection, std::string const& DataToSend, MBOctetCommunication* CommunicationStream)
		{
			size_t ParseOffset = 0;
			while (ParseOffset < DataToSend.size())
			{
				size_t NextLineToStuff = DataToSend.find("\r\n.\r\n", ParseOffset);
				if (NextLineToStuff == DataToSend.npos)
				{
					(*CommunicationStream) << DataToSend.substr(ParseOffset);
					break;
				}
				else
				{
					(*CommunicationStream) << DataToSend.substr(ParseOffset,NextLineToStuff-ParseOffset);
					(*CommunicationStream) << "\r\n..\r\n";
					ParseOffset = NextLineToStuff + 5;
				}
			}
		}
		template<typename MBOctetCommunication> void p_SendAttachmentData(SMTPConnectionState& AssociatedConnection,MBOctetCommunication* CommunicationStream, MailAttachment const& AttachmentToSend)
		{

		}
		template<typename MBOctetCommunication>
		void p_StartMailTranser(SMTPConnectionState& StateToUpdate,MBOctetCommunication& CommunicationStream,SMTPSendInfo const& SendInfo)
		{
			StateToUpdate.InTransaction = true;
			SMTPResponse Response = p_GetCommandResponse("MAIL FROM:" + "<" + SendInfo.User + "@" + SendInfo.Domain + ">");
			if (SMTPStatusIsError(Response.StatusCode))
			{
				StateToUpdate.InTransaction = false;
				StateToUpdate.LatestCommandStatus = Response.StatusCode;
				StateToUpdate.LatestErrorString = Response.ResponseLines[0];
				return;
			}
			for (size_t i = 0; i < SendInfo.Recievers.size(); i++)
			{
				Response = p_GetCommandResponse("RCPT TO:" + "<" + SendInfo.Recievers[i]+ ">");
				if (SMTPStatusIsError(Response.StatusCode))
				{
					StateToUpdate.InTransaction = false;
					StateToUpdate.LatestCommandStatus = Response.StatusCode;
					StateToUpdate.LatestErrorString = Response.ResponseLines[0];
					return;
				}
			}
		}
		template<typename MBOctetCommunication>
		void p_EndDataTransfer(SMTPConnectionState& StateToUpdate,MBOctetCommunication& CommunciationStream)
		{
			SMTPResponse Response = p_GetCommandResponse("\r\n.\r\n", CommunciationStream);
			StateToUpdate.InTransaction = false;
			StateToUpdate.LatestCommandStatus = Response.StatusCode;
			if (SMTPStatusIsError(Response.StatusCode))
			{
				StateToUpdate.LatestErrorString = Response.ResponseLines[0];
			}
		}
		template<typename MBOctetCommunication>
		void p_EndSMTPConnection(SMTPConnectionState& StateToUpdate, MBOctetCommunication& CommunciationStream)
		{
			SMTPResponse Response = p_GetCommandResponse("QUIT", CommunciationStream);
			StateToUpdate.Connected = false;
			StateToUpdate.InTransaction = false;
			StateToUpdate.MaxMessageSize = 0;
			StateToUpdate.ServerExtensions = {};
		}
		template<typename MBOctetStream> void p_InsertAttachment(Mail const& MailToSend, size_t AttachmentIndex,MBOctetStream& DataStream)
		{
			assert(false);
			return;
		}
		static std::string p_GetDateString();
	public:
		void SetMailbox(std::string const& NewMailbox);
		MailError SendMail(Mail MailToSend,std::string const& PostBoxReiever);
		void FillMailHeaders(Mail& MailToEdit, StandardUserEmailHeaders const& UserInputHeaders);
		void t_SaveMail(Mail MailToSend, std::string const& PostBoxReiever, std::string const& MailOutputPath);
		template<typename MBOctetStream> void t_InsertMail(Mail MailToSend, std::string const& PostBoxReiever,MBOctetStream& DataStream)
		{
			std::string BodyContent = p_GetMailBody(MailToSend);
			m_DKIMSigner.SignMail(MailToSend, BodyContent);
			for (size_t i = 0; i < MailToSend.MimeHeaders.size(); i++)
			{
				DataStream<<GetMIMEHeaderLines(MailToSend.MimeHeaders[i]);
			}
			DataStream << "\r\n";
			assert(BodyContent.find("\r\n.\r\n") == BodyContent.npos);
			//TODO fix dot-stuffing
			DataStream << BodyContent;
			for (size_t i = 0; i < MailToSend.Attachments.size(); i++)
			{
				p_InsertAttachment(MailToSend, i, DataStream);
			}
		}
	};
	class MBMailReciever
	{
	private:

	public:

	};
}