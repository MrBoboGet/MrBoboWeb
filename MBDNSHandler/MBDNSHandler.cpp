#include "MBDNSHandler.h"
#include <MBStrings.h>
namespace MBDNS
{
	DNSMessageHeader MBDNSHandler::p_ParseMessageHeader(std::string const& DataToParse, size_t InOffset, size_t* OutOffset)
	{
		DNSMessageHeader ReturnValue;
		size_t ParseOffset = InOffset;
		ReturnValue.ID = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		uint16_t FlagMask = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		ReturnValue.IsQuerry = (FlagMask >> 15) == 0;
		ReturnValue.OPCode = QuerryOPCode((FlagMask >> 11) % 16);
		ReturnValue.AuthorativeAnswer = ((FlagMask >> 10) & 1) == 1;
		ReturnValue.Truncated = ((FlagMask >> 9) & 1) == 1;
		ReturnValue.RecursiveDesired = ((FlagMask >> 8) & 1) == 1;
		ReturnValue.RecursionSupported = ((FlagMask >> 7) & 1) == 1;
		ReturnValue.ResponseCode = QuerryResponseCode(FlagMask % 16);
		ReturnValue.QuestionCount = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		ReturnValue.AnswerCount = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		ReturnValue.AuthorityCount	 = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		ReturnValue.AdditionalCount = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		*OutOffset = ParseOffset;
		return(ReturnValue);
	}
	uintmax_t MBDNSHandler::p_ParseBigEndianInteger(std::string const& DataToParse, size_t IntegerSize, size_t ParseOffset, size_t* OutParseOffset)
	{
		uintmax_t ReturnValue = 0;
		for (size_t i = 0; i < IntegerSize; i++)
		{
			ReturnValue <<= 8;
			ReturnValue += (unsigned char)DataToParse[ParseOffset + i];
		}
		*OutParseOffset = ParseOffset + IntegerSize;		
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_ParseLabel(std::string const& DataToParse, size_t ParseOffset, size_t* OutOffset)
	{
		unsigned char NumberOfCharacters = DataToParse[ParseOffset];
		if (NumberOfCharacters > 63)
		{
			assert(false);
			//dett är en pointer
			//uint16_t PointerOffset = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset)&(~(3<<14));
			//size_t TempOffset = 0;
			//std::string ReturnValue = p_ParseDomain(DataToParse, PointerOffset, &TempOffset);
			//*OutOffset = ParseOffset;
			//return(ReturnValue);
			return("");
		}
		else
		{
			std::string ReturnValue = DataToParse.substr(ParseOffset + 1, NumberOfCharacters);
			*OutOffset = ParseOffset + 1 + NumberOfCharacters;
			return(ReturnValue);
		}
	}
	std::string MBDNSHandler::p_ParseDomain(std::string const& DataToParse, size_t ParseOffset, size_t* OutOffset)
	{
		std::string ReturnValue = "";
		bool WasPointer = false;
		while(DataToParse[ParseOffset] != 0 && !WasPointer)
		{
			WasPointer = (((unsigned char)DataToParse[ParseOffset]) > 63);
			if (ReturnValue != "")
			{
				ReturnValue += ".";
			}
			if (WasPointer)
			{
				uint16_t PointerOffset = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset) & (~(3 << 14));
				size_t TempOffset = 0;
				ReturnValue += p_ParseDomain(DataToParse, PointerOffset, &TempOffset);
			}
			else
			{
				ReturnValue += p_ParseLabel(DataToParse, ParseOffset, &ParseOffset);
			}
		}
		if (!WasPointer)
		{
			ParseOffset += 1;
		}
		*OutOffset = ParseOffset;
		return(ReturnValue);
	}
	DNSMessageQuestion MBDNSHandler::p_ParseMessageQuestion(std::string const& DataToParse, size_t InOffset, size_t* OutOffset)
	{
		DNSMessageQuestion ReturnValue;
		size_t ParseOffset = InOffset;
		ReturnValue.DomainName = p_ParseDomain(DataToParse, ParseOffset,&ParseOffset);
		ReturnValue.QuerryType = RRQType(p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset));
		ReturnValue.QuerryClass = RRQClass(p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset));
		*OutOffset = ParseOffset;
		return(ReturnValue);
	}
	DNSResourceRecord MBDNSHandler::p_ParseResourceRecord(std::string const& DataToParse, size_t InOffset, size_t* OutOffset)
	{
		DNSResourceRecord ReturnValue;
		size_t ParseOffset = InOffset;
		ReturnValue.Name = p_ParseDomain(DataToParse, ParseOffset, &ParseOffset);
		ReturnValue.Type = RRType(p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset));
		ReturnValue.Class = RRClass(p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset));
		ReturnValue.TTL = p_ParseBigEndianInteger(DataToParse, 4, ParseOffset, &ParseOffset);
		uint16_t DataLength = p_ParseBigEndianInteger(DataToParse, 2, ParseOffset, &ParseOffset);
		ReturnValue.DataOffset = ParseOffset;
		ReturnValue.RecordData = DataToParse.substr(ParseOffset, DataLength);
		ParseOffset += DataLength;
		*OutOffset = ParseOffset;
		return(ReturnValue);
	}
	DNSMessage MBDNSHandler::p_ParseDNSMessage(std::string const& DataToParse)
	{
		DNSMessage ReturnValue;
		size_t ParseOffset = 0;
		ReturnValue.Header = p_ParseMessageHeader(DataToParse, ParseOffset, &ParseOffset);
		for (size_t i = 0; i < ReturnValue.Header.QuestionCount; i++)
		{
			ReturnValue.Question.push_back(p_ParseMessageQuestion(DataToParse, ParseOffset, &ParseOffset));
		}
		for (size_t i = 0; i < ReturnValue.Header.AnswerCount; i++)
		{
			ReturnValue.Answers.push_back(p_ParseResourceRecord(DataToParse, ParseOffset, &ParseOffset));
		}
		for (size_t i = 0; i < ReturnValue.Header.AuthorityCount; i++)
		{
			ReturnValue.Authority.push_back(p_ParseResourceRecord(DataToParse, ParseOffset, &ParseOffset));
		}
		for (size_t i = 0; i < ReturnValue.Header.AdditionalCount; i++)
		{
			ReturnValue.Additional.push_back(p_ParseResourceRecord(DataToParse, ParseOffset, &ParseOffset));
		}
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeDomain(std::string const& DomainToEncode)
	{
		std::string ReturnValue = "";
		std::vector<std::string> DomainComponents = MBUtility::Split(DomainToEncode, ".");
		for (size_t i = 0; i < DomainComponents.size(); i++)
		{
			ReturnValue += (unsigned char)DomainComponents[i].size();
			ReturnValue += DomainComponents[i];
		}
		ReturnValue += (char)0;
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeBigEndianInteger(uintmax_t IntegerToEncode, unsigned char IntegerSize)
	{
		std::string ReturnValue = "";
		for (int i = IntegerSize-1; i >= 0; i--)
		{
			ReturnValue += (unsigned char)((IntegerToEncode>>(i*8))%256);
		}
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeMessageHeader(DNSMessageHeader const& HeaderToEncode)
	{
		std::string ReturnValue = "";
		ReturnValue += p_EncodeBigEndianInteger(HeaderToEncode.ID, 2);
		uint16_t FlagMask = 0;
		FlagMask += (HeaderToEncode.IsQuerry != true) << 15;	
		FlagMask += uint16_t((HeaderToEncode.OPCode)) << 11;
		FlagMask += uint16_t((HeaderToEncode.AuthorativeAnswer)) << 10;
		FlagMask += uint16_t((HeaderToEncode.Truncated)) << 9;
		FlagMask += uint16_t((HeaderToEncode.RecursiveDesired)) << 8;
		FlagMask += uint16_t((HeaderToEncode.RecursiveDesired)) << 7;
		FlagMask += uint16_t(HeaderToEncode.ResponseCode);
		ReturnValue += p_EncodeBigEndianInteger(FlagMask, 2);
		ReturnValue += p_EncodeBigEndianInteger(HeaderToEncode.QuestionCount, 2);
		ReturnValue += p_EncodeBigEndianInteger(HeaderToEncode.AnswerCount, 2);
		ReturnValue += p_EncodeBigEndianInteger(HeaderToEncode.AuthorativeAnswer, 2);
		ReturnValue += p_EncodeBigEndianInteger(HeaderToEncode.AdditionalCount, 2);
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeResourceRecord(DNSResourceRecord const& RecordToEncode)
	{
		std::string ReturnValue = "";
		ReturnValue += p_EncodeDomain(RecordToEncode.Name);
		ReturnValue += p_EncodeBigEndianInteger((uint16_t)RecordToEncode.Type,2);
		ReturnValue += p_EncodeBigEndianInteger((uint16_t)RecordToEncode.Class,2);
		ReturnValue += p_EncodeBigEndianInteger(RecordToEncode.TTL,4);
		ReturnValue += p_EncodeBigEndianInteger(RecordToEncode.RecordData.size(),2);
		ReturnValue += RecordToEncode.RecordData;
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeDNSQuerry(DNSMessageQuestion const& QuerryToEncode)
	{
		std::string ReturnValue = "";
		ReturnValue += p_EncodeDomain(QuerryToEncode.DomainName);
		ReturnValue += p_EncodeBigEndianInteger((uint16_t)QuerryToEncode.QuerryType,2);
		ReturnValue += p_EncodeBigEndianInteger((uint16_t)QuerryToEncode.QuerryClass,2);
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_EncodeDNSMessage(DNSMessage const& MessageToEncode,bool IsTCP)
	{
		std::string ReturnValue = "";
		ReturnValue += p_EncodeMessageHeader(MessageToEncode.Header);
		for (size_t i = 0; i < MessageToEncode.Question.size(); i++)
		{
			ReturnValue += p_EncodeDNSQuerry(MessageToEncode.Question[i]);
		}
		for (size_t i = 0; i < MessageToEncode.Answers.size(); i++)
		{
			ReturnValue += p_EncodeResourceRecord(MessageToEncode.Answers[i]);
		}
		for (size_t i = 0; i < MessageToEncode.Authority.size(); i++)
		{
			ReturnValue += p_EncodeResourceRecord(MessageToEncode.Authority[i]);
		}
		for (size_t i = 0; i < MessageToEncode.Authority.size(); i++)
		{
			ReturnValue += p_EncodeResourceRecord(MessageToEncode.Authority[i]);
		}
		if (IsTCP)
		{
			ReturnValue = p_EncodeBigEndianInteger(ReturnValue.size(), 2) + ReturnValue;
		}
		return(ReturnValue);
	}
	MXRecord MBDNSHandler::p_GetMXRecord(DNSResourceRecord const& RecordToParse,std::string const& OriginalMessageData)
	{
		MXRecord ReturnValue;
		size_t ParseOffset = 0;
		ReturnValue.Preference = p_ParseBigEndianInteger(RecordToParse.RecordData, 2, ParseOffset, &ParseOffset);
		ReturnValue.Exchange = p_ParseDomain(OriginalMessageData, RecordToParse.DataOffset+2, &ParseOffset);
		return(ReturnValue);
	}
	NSRecord MBDNSHandler::p_GetNSRecord(DNSResourceRecord const& RecordToParse, std::string const& OriginialMessageData)
	{
		NSRecord ReturnValue;
		size_t TempInt = 0;
		ReturnValue.DomainName = p_ParseDomain(OriginialMessageData, RecordToParse.DataOffset, &TempInt);
		return(ReturnValue);
	}
	std::string MBDNSHandler::p_GetQuerryResponse(DNSMessageQuestion const& Querry, std::string const& DNSServerIP,bool UseTCP)
	{
		DNSMessage MessageToSend;
		MessageToSend.Header.IsQuerry = true;
		MessageToSend.Header.QuestionCount = 1;
		MessageToSend.Header.RecursiveDesired = true;
		MessageToSend.Header.OPCode = QuerryOPCode::Standard;
		MessageToSend.Question.push_back(Querry);
		std::string StringToSend = p_EncodeDNSMessage(MessageToSend, UseTCP);
		std::string ReturnValue = "";
		if (UseTCP)
		{
			//m_TCPSocket = MBSockets::ConnectSocket(DNSServerIP, "53", MBSockets::TraversalProtocol::TCP);
			//m_TCPSocket.Connect();
			//m_TCPSocket.SendData(StringToSend.c_str(), StringToSend.size());
			MBSockets::ClientSocket SocketToUse(DNSServerIP, "53");
			SocketToUse.Connect();
			SocketToUse.SendData(StringToSend.c_str(), StringToSend.size());
			ReturnValue = SocketToUse.RecieveData();
		}
		else
		{
			m_UDPSocket.UDPSendData(StringToSend, DNSServerIP, 53);
			ReturnValue = m_UDPSocket.UDPGetData();
		}
		if (ReturnValue.size() < 2)
		{
			return("");
		}
		if (UseTCP)
		{
			size_t TempInt = 0;
			size_t MessageSize = p_ParseBigEndianInteger(ReturnValue, 2, 0, &TempInt);
			while (ReturnValue.size() < MessageSize+2)
			{
				ReturnValue += m_TCPSocket.RecieveData();
			}
			return(ReturnValue.substr(2));
		}
		return(ReturnValue);
	}
	std::string IPv4AdressToString(uint32_t Adress)
	{
		std::string ReturnValue = "";
		for (size_t i = 0; i < 4; i++)
		{
			ReturnValue += std::to_string(((Adress >> (8 * (3 - i))) % 256));
			if (i < 3)
			{
				ReturnValue += ".";
			}
		}
		return(ReturnValue);
	}
	DNSMessage MBDNSHandler::p_ResolveQuerryRecursivly(DNSMessageQuestion const& Querry,std::string& OutResponseData)
	{
		std::string DNSServerToQuerry = "83.255.255.1";
		DNSMessage ResponseMessage;
		std::string ResponseData = "";
		while (true)
		{
			ResponseData = p_GetQuerryResponse(Querry, DNSServerToQuerry, true);
			ResponseMessage = p_ParseDNSMessage(ResponseData);
			if (ResponseMessage.Authority.size() != 0)
			{
				std::string NewServer = p_GetNSRecord(ResponseMessage.Authority[0], ResponseData).DomainName;
				if (NewServer == DNSServerToQuerry)
				{
					break;
				}
				DNSServerToQuerry = NewServer;
			}
			else
			{
				break;
			}
		}
		std::swap(OutResponseData,ResponseData);
		return(ResponseMessage);
	}
	ARecord MBDNSHandler::p_GetARecord(DNSResourceRecord const& RecordToParse)
	{
		size_t TempInt = 0;
		ARecord ReturnValue;
		ReturnValue.IPAdress = p_ParseBigEndianInteger(RecordToParse.RecordData, 4, 0, &TempInt);
		return(ReturnValue);
	}
	PTRRecord MBDNSHandler::p_GetPTRRecord(DNSResourceRecord const& RecordToParse, std::string const& OriginalMessageData)
	{
		PTRRecord ReturnValue;
		size_t TempInt = 0;
		ReturnValue.DomainName = p_ParseDomain(OriginalMessageData, RecordToParse.DataOffset, &TempInt);
		return(ReturnValue);
	}
	TXTRecord MBDNSHandler::p_GetTXTRecords(DNSResourceRecord const& RecordToParse)
	{
		TXTRecord ReturnValue;
		size_t ParseOffset = 0;
		while (ParseOffset < RecordToParse.RecordData.size())
		{
			uint8_t StringLength = RecordToParse.RecordData[ParseOffset];
			ParseOffset += 1;
			ReturnValue.RecordStrings.push_back(RecordToParse.RecordData.substr(ParseOffset, StringLength));
			ParseOffset += StringLength;
		}
		return(ReturnValue);
	}
	std::vector <ARecord> MBDNSHandler::GetDomainIPAdresses(std::string DomainName)
	{
		std::vector<ARecord> ReturnValue = {};
		DNSMessageQuestion Querry;
		Querry.DomainName = DomainName;
		Querry.QuerryType = RRQType::A;
		Querry.QuerryClass = RRQClass::In;
		std::string ResponseData = "";
		DNSMessage ResponseMessage = p_ResolveQuerryRecursivly(Querry, ResponseData);
		for (size_t i = 0; i < ResponseMessage.Answers.size(); i++)
		{
			ReturnValue.push_back(p_GetARecord(ResponseMessage.Answers[i]));
		}
		return(ReturnValue);
	}
	std::string h_ReverseIPAdress(std::string const& IPToReverse)
	{
		std::string ReturnValue = "";
		std::vector<std::string> IPAdressParts = MBUtility::Split(IPToReverse, ".");
		for (size_t i = 0; i < IPAdressParts.size(); i++)
		{
			ReturnValue += IPAdressParts[IPAdressParts.size() - 1 - i];
			if (i + 1 < IPAdressParts.size())
			{
				ReturnValue += ".";
			}
		}
		return(ReturnValue);
	}
	std::vector<PTRRecord> MBDNSHandler::GetIPAdressDomains(std::string const& IPAdress)
	{
		std::vector<PTRRecord> ReturnValue = {};
		DNSMessageQuestion Querry;
		Querry.DomainName = h_ReverseIPAdress(IPAdress) + ".in-addr.arpa";
		Querry.QuerryType = RRQType::PTR;
		Querry.QuerryClass = RRQClass::In;
		std::string ResponseData = "";
		DNSMessage ResponseMessage = p_ResolveQuerryRecursivly(Querry, ResponseData);
		for (size_t i = 0; i < ResponseMessage.Answers.size(); i++)
		{
			ReturnValue.push_back(p_GetPTRRecord(ResponseMessage.Answers[i],ResponseData));
		}
		return(ReturnValue);
	}
	std::vector<MXRecord> MBDNSHandler::GetDomainMXRecords(std::string DomainName)
	{
		std::vector<MXRecord> ReturnValue = {};
		DNSMessageQuestion Querry;
		Querry.DomainName = DomainName;
		Querry.QuerryType = RRQType::MX;
		Querry.QuerryClass = RRQClass::In;
		std::string ResponseData = "";
		DNSMessage ResponseMessage = p_ResolveQuerryRecursivly(Querry,ResponseData);
		for (size_t i = 0; i < ResponseMessage.Answers.size(); i++)
		{
			ReturnValue.push_back(p_GetMXRecord(ResponseMessage.Answers[i],ResponseData));
		}
		return(ReturnValue);
	}
	std::vector<TXTRecord> MBDNSHandler::GetTXTRecords(std::string const& DomainName)
	{
		std::vector<TXTRecord> ReturnValue = {};
		DNSMessageQuestion Querry;
		Querry.DomainName = DomainName;
		Querry.QuerryType = RRQType::TXT;
		Querry.QuerryClass = RRQClass::In;
		std::string ResponseData = "";
		DNSMessage ResponseMessage = p_ResolveQuerryRecursivly(Querry, ResponseData);
		for (size_t i = 0; i < ResponseMessage.Answers.size(); i++)
		{
			ReturnValue.push_back(p_GetTXTRecords(ResponseMessage.Answers[i]));
		}
		return(ReturnValue);
	}
};