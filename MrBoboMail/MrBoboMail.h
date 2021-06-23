#pragma once
#include <string>
#include <MrBoboSockets.h>
#include <MBMime/MBMime.h>
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
	enum class SMTPStatusCodes
	{
		OK = 250,
		Null,
	};
	struct MailError
	{
		std::string ErrorCode = "";
		SMTPStatusCodes StatusCode = SMTPStatusCodes::Null;
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
	std::string BASE64Decode(void* CharactersToRead, size_t NumberOfCharacters);
	std::string BASE64Encode(void* DataToEncode, size_t DataLength);

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
		static std::string p_GetHeadersHash(Mail const& MailToSign, DKIMSigningData const& SigningData);
		static std::string p_GetPrivateKeyPath(std::string const& DomainToSign, std::string const& SignSelector);
		static std::string p_GetHashSignature(std::string const& HashToSign, DKIMSigningData const& SigningData,std::string const& PrivateKeyPath);
		static MIMEHeader p_GetHeader(std::vector<MIMEHeader> const& HeadersToSearch, std::string const& HeaderName);
	public:
		void SignMail(Mail& MailToSign,std::string const& BodyData); //adds headers and header values to the mail to send
		static void SignMail(Mail& MailToSign,std::string const& BodyData, DKIMSigningData const& SigningData);
	};
	struct SMTPConnectionState
	{
		MBSockets::ConnectSocket* DataSocket = nullptr;
		bool Connected = false;
		bool InTransaction = false;
		uintmax_t MaxMessageSize = 0;
		std::unordered_map<std::string,bool> ServerExtensions = {};
		MailError LastError;
	};
	struct StandardUserEmailHeaders
	{
		std::string To = "";
		std::string From = "";
		std::string Subject = "";
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
		SMTPConnectionState p_StartSMTPConnection(std::string const& DomainToSendTo);
		void p_SendAttachmentData(SMTPConnectionState& AssociatedConnection, MailAttachment const& AttachmentToSend);
		void p_StartMailTranser(SMTPConnectionState& StateToUpdate);
		void p_EndDataTransfer(SMTPConnectionState& StateToUpdate);
		void p_EndSMTPConnection(SMTPConnectionState& StateToUpdate);
		static std::string p_GetDateString();
		template<typename MBOctetStream> void p_InsertAttachment(Mail const& MailToSend, size_t AttachmentIndex,MBOctetStream& DataStream)
		{
			assert(false);
			return;
		}
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