#pragma once
#define NOMINMAX
#define _CRT_RAND_S
#include <string>
#include <MrBoboSockets/MrBoboSockets.h>
#include <MBMime/MBMime.h>
#include <MBSearchEngine/MBUnicode.h>
#include <MBStrings.h>
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
	std::string BASE64Decode(std::string const& DataToDecode);
	std::string BASE64Encode(std::string const& DataToEncode);
	
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
	struct SMTPAuthenticationData
	{
		std::string RelayDomain = "";
		std::string PlaintextUser = "";
		std::string PlaintextPassword = "";
	};
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
			ReturnValue.StatusCode = (SMTPStatusCode) std::stoi(ResponseData);
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
			SMTPResponse Response = p_GetCommandResponse("EHLO " + SendInfo.Domain,CommunicationStream);
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
			SMTPResponse Response = p_GetCommandResponse("MAIL FROM:<" + SendInfo.User + "@" + SendInfo.Domain + ">",&CommunicationStream);
			if (SMTPStatusIsError(Response.StatusCode))
			{
				StateToUpdate.InTransaction = false;
				StateToUpdate.LatestCommandStatus = Response.StatusCode;
				StateToUpdate.LatestErrorString = Response.ResponseLines[0];
				return;
			}
			for (size_t i = 0; i < SendInfo.Recievers.size(); i++)
			{
				Response = p_GetCommandResponse("RCPT TO:<" + SendInfo.Recievers[i]+ ">",&CommunicationStream);
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
			SMTPResponse Response = p_GetCommandResponse("\r\n.\r\n", &CommunciationStream);
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
			SMTPResponse Response = p_GetCommandResponse("QUIT", &CommunciationStream);
			StateToUpdate.Connected = false;
			StateToUpdate.InTransaction = false;
			StateToUpdate.MaxMessageSize = 0;
			StateToUpdate.ServerExtensions = {};
		}
		template<typename MBOctetCommunication> void p_Authenticate(SMTPConnectionState& StateToUpdate, MBOctetCommunication& CommunicationStream)
		{
			SMTPAuthenticationData AuthenticationData = p_GetAuthenticationData();
			SMTPResponse AuthResponse = p_GetCommandResponse("AUTH LOGIN",&CommunicationStream);
			AuthResponse = p_GetCommandResponse(BASE64Encode(AuthenticationData.PlaintextUser.data(), AuthenticationData.PlaintextPassword.size()), &CommunicationStream);
			AuthResponse = p_GetCommandResponse(BASE64Encode(AuthenticationData.PlaintextPassword.data(), AuthenticationData.PlaintextPassword.size()), &CommunicationStream);
		}
		template<typename MBOctetStream> void p_InsertAttachment(Mail const& MailToSend, size_t AttachmentIndex,MBOctetStream& DataStream)
		{
			assert(false);
			return;
		}
		static std::string p_GetDateString();
		MBSockets::ClientSocket p_GetServerConnection(std::string const& DomainName);
		SMTPAuthenticationData p_GetAuthenticationData();
	public:
		void SetMailbox(std::string const& NewMailbox);
		MailError SendMail(Mail MailToSend,std::string const& PostBoxReiever,SMTPSendInfo const& SendInfo);
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

	class MBMailReciever;
	struct SMTPTransactionState
	{
		bool InTransaction = false;
		bool InDataTransfer = false;
		bool SenderValid = true;
		std::string SenderUsername = "";
		std::string SenderDomain = "";
		std::vector<std::string> Recipients = {};
	};
	class MBMailSMTPServerConnection
	{
	private:
		friend class MBMailReciever;
		size_t m_ConnectionID = -1;
		std::shared_ptr<MBSockets::ConnectSocket> m_AssociatedSocket = nullptr;
		MBMailReciever* m_AssociatedReciever = nullptr;
		std::string m_ClientDomain = "";
		bool m_UsedImplicitTLS = false;
		bool m_ConnectionIsOpen = true;
		SMTPTransactionState m_TransactionState;
		std::string m_FileToSaveToPath = "";
		std::ofstream m_FileToSaveTo;
		void p_HandleConnection();
		void p_HandleCommand(std::string const& CommandData);
		static void p_GetPostboxParts(std::string& Username, std::string& Domain, std::string const& LineContainingPostbox);
		static bool p_VerifyPRF(std::string const& DomainToVerify,std::string const& ConnectionIP);
		static bool p_VerifyDKIM(std::string const& DomainToVerify,std::string const& MailFilePath);
		static bool p_VerifyRDNS(std::string const& ConnectionIP);
		static void p_SendServerHello(MBSockets::ConnectSocket* AssociatedSocket);
		static void p_SendExtensions(MBSockets::ConnectSocket* AssociatedSocket);
		void p_CloseConnection();
		MBMailSMTPServerConnection(std::shared_ptr<MBSockets::ConnectSocket> AssociatedConnection, size_t ConnectionID, bool UsedImplicitTLS);
	public:
	};
	class MBMailReciever
	{
		friend class MBMailSMTPServerConnection;
	private:
		std::mutex m_InternalsMutex;
		std::unordered_map<size_t, std::shared_ptr<MBMailSMTPServerConnection>> m_ActiveConnections = {};
		void p_AddConnection(size_t ConnectionID, std::shared_ptr<MBMailSMTPServerConnection> ConnectionToAdd);
		void p_RemoveConnection(size_t ConnectionID);
		bool p_VerifyUser(std::string const& UsernameToVerify);
		std::string p_GetTempDirectory();
		std::string p_GetUserDirectory(std::string const& Username);
		std::string p_GetTimeString();
		//static void p_SendHello(MBSockets::ConnectSocket* ConnectionSocket);
	public:
		void StartListening(std::string const& AssociatedDomain, std::string const& PortToListenTo,bool UseImplicitTLS);
	};
}