#pragma once
#include <MrBoboSockets/MrBoboSockets.h>
#include <cinttypes>
#include <string>
#include <vector>
namespace MBDNS
{
	enum class RRType : uint16_t
	{
		A = 1, //host ip adress
		NS = 2, // authorative name server
		MD = 3, //OBSOLETE
		MF = 4, //OBSOLETE
		CNAME = 5, //the canoncial name of an alias
		SAO = 6, //marks the start of a zone of authorit
		MB = 7, //a mailbox domain name(EXPERIMENTAL)
		MG = 8,// a mail group member(EXPERIMENTAL)
		MR = 9,//a mail rename domain name(EXPERIMENTAL)
		Null = 10,//a null RR(EXPERIMENTAL)
		WKS = 11,//a well known service description
		PTR = 12,//a domain name pointer
		HINFO = 13,//host information
		MINFO = 14,//mailbox or mail list information
		MX = 15,//mail exchange
		TXT = 16,//text strings
	};
	enum class RRQType : uint16_t
	{
		A = 1, //host ip adress
		NS = 2, // authorative name server
		MD = 3, //OBSOLETE
		MF = 4, //OBSOLETE
		CNAME = 5, //the canoncial name of an alias
		SAO = 6, //marks the start of a zone of authorit
		MB = 7, //a mailbox domain name(EXPERIMENTAL)
		MG = 8,// a mail group member(EXPERIMENTAL)
		MR = 9,//a mail rename domain name(EXPERIMENTAL)
		Null = 10,//a null RR(EXPERIMENTAL)
		WKS = 11,//a well known service description
		PTR = 12,//a domain name pointer
		HINFO = 13,//host information
		MINFO = 14,//mailbox or mail list information
		MX = 15,//mail exchange
		TXT = 16,//text strings


		AXFR = 252, //A request for a transfer of an entire zone
		MAILB = 253, //A request for mailbox - related records(MB, MG or MR)
		MAILA = 254, // A request for mail agent RRs(Obsolete - see MX)
		ALL = 255,  //A request for all records
	};
	enum class RRClass : uint16_t
	{
		In = 1,// the Internet
		CS = 2,// the CSNET class (Obsolete - used only for examples in	some obsolete RFCs)
		CH = 3,// the CHAOS class
		HS = 4,// Hesiod[Dyer 87]
		Null,
	};
	enum class RRQClass : uint16_t
	{
		In = 1,// the Internet
		CS = 2,// the CSNET class (Obsolete - used only for examples in	some obsolete RFCs)
		CH = 3,// the CHAOS class
		HS = 4,// Hesiod[Dyer 87]
		ALL = 255, //Any class
		Null,
	};
	struct DNSResourceRecord
	{
		std::string Name = "";
		RRType Type = RRType::Null;
		RRClass Class = RRClass::Null;
		int32_t TTL = 0;
		std::string RecordData = "";
		size_t DataOffset = 0;
	};
	struct MXRecord
	{
		int16_t Preference = 0;
		std::string Exchange = "";
	};
	enum class QuerryOPCode : uint8_t
	{
		Standard = 0,
		Inverse = 1,
		Status = 2,
		Null
	};
	enum class QuerryResponseCode : uint8_t
	{
		NoError = 0,
		FormatError = 1,
		ServerFailure = 2,
		NameError = 3,
		NotImplemented = 4,
		Refused = 5,
		Null,
	};
	struct DNSMessageHeader
	{
		uint16_t ID = 0;
		bool IsQuerry = false;
		QuerryOPCode OPCode = QuerryOPCode::Null;
		bool AuthorativeAnswer = 0;
		bool Truncated = false;
		bool RecursiveDesired = false;
		bool RecursionSupported = false;
		QuerryResponseCode ResponseCode = QuerryResponseCode::NoError;
		uint16_t QuestionCount = 0;
		uint16_t AnswerCount = 0;
		uint16_t AuthorityCount = 0;
		uint16_t AdditionalCount = 0;
	};
	struct DNSMessageQuestion
	{
		std::string DomainName = "";
		RRQType QuerryType = RRQType::Null;
		RRQClass QuerryClass = RRQClass::Null;
	};
	struct DNSMessage
	{
		DNSMessageHeader Header;
		std::vector<DNSMessageQuestion> Question = {};
		std::vector<DNSResourceRecord> Answers = {};
		std::vector<DNSResourceRecord> Authority = {};
		std::vector<DNSResourceRecord> Additional = {};
	};
	struct NSRecord
	{
		std::string DomainName = "";
	};
	//VIKTIGT om truncated flaggen är satt så ska det göras om via TCP
	//VIKTIGT om det är tcp så är meddelandent prefixat med en (såklart bigendian) 2 byte som säger längden
	//varför inte bara köra på tcp hela tiden, aningen smidigare i början
	struct ARecord
	{
		uint32_t IPAdress;
	};
	struct PTRRecord
	{
		std::string DomainName = "";
	};
	struct TXTRecord
	{
		std::vector<std::string> RecordStrings = {};
	};
	std::string IPv4AdressToString(uint32_t Adress);
	class MBDNSHandler
	{
	private:
		MBSockets::UDPSocket m_UDPSocket;
		MBSockets::ClientSocket m_TCPSocket;
		static std::string p_EncodeDomain(std::string const& DomainToEncode);
		static std::string p_EncodeBigEndianInteger(uintmax_t IntegerToEncode,unsigned char IntegerSize);
		static std::string p_EncodeMessageHeader(DNSMessageHeader const& HeaderToEncode);
		static std::string p_EncodeDNSMessage(DNSMessage const& MessageToEncode,bool IsTCP);
		static std::string p_EncodeResourceRecord(DNSResourceRecord const& RecordToEncode);
		static std::string p_EncodeDNSQuerry(DNSMessageQuestion const& QuerryToEncode);
		static std::string p_ParseLabel(std::string const& DataToParse, size_t ParseOffset, size_t* OutOffset);
		static std::string p_ParseDomain(std::string const& DataToParse, size_t ParseOffset, size_t* OutOffset);
		static uintmax_t p_ParseBigEndianInteger(std::string const& DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset);
		static DNSMessageHeader p_ParseMessageHeader(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
		static DNSMessageQuestion p_ParseMessageQuestion(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
		static DNSResourceRecord p_ParseResourceRecord(std::string const& DataToParse, size_t InOffset, size_t* OutOffset);
		static DNSMessage p_ParseDNSMessage(std::string const& DataToParse);
		static MXRecord p_GetMXRecord(DNSResourceRecord const& RecordToParse,std::string const& OriginalMessageData);
		static NSRecord p_GetNSRecord(DNSResourceRecord const& RecordToParse, std::string const& OriginialMessageData);
		static ARecord p_GetARecord(DNSResourceRecord const& RecordToParse);
		static PTRRecord p_GetPTRRecord(DNSResourceRecord const& RecordToParse, std::string const& OriginalMessageData);
		static TXTRecord p_GetTXTRecords(DNSResourceRecord const& RecordToParse);
		DNSMessage p_ResolveQuerryRecursivly(DNSMessageQuestion const& Querry,std::string& OutResponseData);
		std::string p_GetQuerryResponse(DNSMessageQuestion const& Querry,std::string const& DNSServerIP,bool UseTCP);
	public:
		MBDNSHandler()
			//: m_UDPSocket("83.255.255.1", "53", MBSockets::TraversalProtocol::TCP), m_TCPSocket("83.255.255.1", "53", MBSockets::TraversalProtocol::TCP)
			: m_UDPSocket("8.8.8.8", "53"), m_TCPSocket("8.8.8.8", "53")
		{

		};
		std::vector<MXRecord> GetDomainMXRecords(std::string DomainName);
		std::vector<ARecord> GetDomainIPAdresses(std::string DomainName);
		std::vector<PTRRecord> GetIPAdressDomains(std::string const& DomainName);
		std::vector<TXTRecord> GetTXTRecords(std::string const& DomainName);
	};
}