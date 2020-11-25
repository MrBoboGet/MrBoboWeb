#include <cstdint>
#include <MrPostOGet/TLSHandler.h>
#include <atomic>
#include <MrBoboChatt/MBCProtocols.h>
/*
enum class MBTCPRecordType : uint8_t
{
	InitiateMessageTransfer,
	MessageData,
	RequestResponse,
	NATBreakMessage,
	Null
};
enum class MBTCPError : uint8_t
{
	OK, Error
};
enum class MBTCPRecordSendType : uint8_t
{
	Send,
	Respond, Null
};
*/

//class MBCMessage
MBCMessage::MBCMessage(std::string StringToParse)
{
	TLS1_2::NetWorkDataHandler DataExtracter(reinterpret_cast<const uint8_t*>(StringToParse.c_str()));
	MBCMessageType = MBCPMessageType(DataExtracter.Extract8());
	RecordNumber = DataExtracter.Extract32();
	ParseOffset = DataExtracter.GetPosition();
}
MBCMessage::MBCMessage()
{

}
std::string MBCMessage::ToString()
{
	std::string ReturnString = "";
	ReturnString += char(MBCMessageType);
	for (int i = 3; i >= 0; i--)
	{
		ReturnString += char((RecordNumber >> i * 8) % 256);
	}
	return(ReturnString);
}
MBCMessage::~MBCMessage()
{

}

//class MBCInitatieConnectionMessage : public MBCMessage
MBCInitatieConnectionMessage::MBCInitatieConnectionMessage(std::string StringToParse) : MBCMessage(StringToParse)
{
	uint32_t OffsetInString = ParseOffset;
	TLS1_2::NetWorkDataHandler DataExtracter(reinterpret_cast<const uint8_t*>(StringToParse.c_str()));
	DataExtracter.SetPosition(ParseOffset);
	InitationMessageType = MBCPInitatioMessageType(DataExtracter.Extract8());
	PortSuggestion = DataExtracter.Extract16();
	SendAdress = StringToParse.substr(DataExtracter.GetPosition());
	ParseOffset = DataExtracter.GetPosition();
}
std::string MBCInitatieConnectionMessage::ToString()
{
	std::string ReturnValue = MBCMessage::ToString();
	ReturnValue += char(InitationMessageType);
	ReturnValue += char(PortSuggestion >> 8);
	ReturnValue += char(PortSuggestion % 256);
	ReturnValue += SendAdress;
	return(ReturnValue);
}
MBCInitatieConnectionMessage::MBCInitatieConnectionMessage() : MBCMessage()
{

}

//class MBCConfirmConnectionMessage : public MBCMessage
MBCConfirmConnectionMessage::MBCConfirmConnectionMessage(std::string StringToParse) : MBCMessage(StringToParse)
{
	UserName = StringToParse.substr(ParseOffset);
}
MBCConfirmConnectionMessage::MBCConfirmConnectionMessage()
{

}
std::string MBCConfirmConnectionMessage::ToString()
{
	std::string ReturnValue = MBCMessage::ToString();
	ReturnValue += UserName;
	return(ReturnValue);
}


//class MBTCPRecord
MBTCPRecord::MBTCPRecord() 
{

}
MBTCPRecord::MBTCPRecord(std::string StringToParse)
{
	TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
	DataHandler.SetPosition(ParseOffset);

	RecordType = (MBTCPRecordType)DataHandler.Extract8();
	SendType = (MBTCPRecordSendType)DataHandler.Extract8();
	RecordNumber = DataHandler.Extract32();

	ParseOffset = DataHandler.GetPosition();
}
std::string MBTCPRecord::ToString()
{
	char DataBuffer[6];
	TLS1_2::NetWorkDataHandler DataHandler((uint8_t*)DataBuffer);
	DataHandler << uint8_t(RecordType);
	DataHandler << uint8_t(SendType);
	DataHandler << uint32_t(RecordNumber);
	return(std::string(DataBuffer, 6));
}


//class MBTCPInitatieMessageTransfer : public MBTCPRecord
MBTCPInitatieMessageTransfer::MBTCPInitatieMessageTransfer() 
{
	
}
MBTCPInitatieMessageTransfer::MBTCPInitatieMessageTransfer(std::string StringToParse) : MBTCPRecord(StringToParse)
{
	TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
	DataHandler.SetPosition(ParseOffset);

	MessageId = DataHandler.Extract32();
	NumberOfMessages = DataHandler.Extract16();
	TotalDataLength = DataHandler.Extract32();
	DataCheckIntervall = DataHandler.Extract8();
	RespondError = (MBTCPError)DataHandler.Extract8();

	ParseOffset = DataHandler.GetPosition();
}
bool MBTCPInitatieMessageTransfer::ParametersMatch(MBTCPInitatieMessageTransfer ObjectToCompare)
{
	if (MessageId != ObjectToCompare.MessageId)
	{
		return(false);
	}
	if (NumberOfMessages != ObjectToCompare.NumberOfMessages)
	{
		return(false);
	}
	if (TotalDataLength != ObjectToCompare.TotalDataLength)
	{
		return(false);
	}
	if (DataCheckIntervall != ObjectToCompare.DataCheckIntervall)
	{
		return(false);
	}
	return(true);
}
std::string MBTCPInitatieMessageTransfer::ToString()
{
	std::string ReturnValue = MBTCPRecord::ToString();
	char DataBuffer[12];
	TLS1_2::NetWorkDataHandler DataHandler((uint8_t*)DataBuffer);
	DataHandler << uint32_t(MessageId);
	DataHandler << uint16_t(NumberOfMessages);
	DataHandler << uint32_t(TotalDataLength);
	DataHandler << uint8_t(DataCheckIntervall);
	DataHandler << uint8_t(RespondError);
	ReturnValue += std::string(DataBuffer, 12);
	return(ReturnValue);
}


//class MBTCPMessage : public MBTCPRecord

MBTCPMessage::MBTCPMessage()
{

}
MBTCPMessage::MBTCPMessage(std::string StringToParse) : MBTCPRecord(StringToParse)
{
	TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
	DataHandler.SetPosition(ParseOffset);

	MessageId = DataHandler.Extract32();
	MessageNumber = DataHandler.Extract16();
	DataLength = DataHandler.Extract16();

	Data = StringToParse.substr(DataHandler.GetPosition(), DataLength);

	ParseOffset = DataHandler.GetPosition() + Data.size();
}
std::string MBTCPMessage::ToString()
{
	std::string ReturnValue = MBTCPRecord::ToString();

	DataLength = Data.size();
	for (int i = 3; i >= 0; i--)
	{
		ReturnValue += char((MessageId >> (i * 8)) % 256);
	}
	for (int i = 1; i >= 0; i--)
	{
		ReturnValue += char((MessageNumber >> (i * 8)) % 256);
	}
	for (int i = 1; i >= 0; i--)
	{
		ReturnValue += char((DataLength >> (i * 8)) % 256);
	}
	ReturnValue += Data;
	return(ReturnValue);
}


//class MBTCPMessageVerification : public MBTCPRecord
MBTCPMessageVerification::MBTCPMessageVerification() 
{

}
MBTCPMessageVerification::MBTCPMessageVerification(std::string StringToParse) : MBTCPRecord(StringToParse)
{
	TLS1_2::NetWorkDataHandler DataHandler((const uint8_t*)StringToParse.c_str());
	DataHandler.SetPosition(ParseOffset);

	RespondError = (MBTCPError)DataHandler.Extract8();
	MessageId = DataHandler.Extract32();
	ByteLengthOfResendRecords = DataHandler.Extract16();
	for (size_t i = 0; i < ByteLengthOfResendRecords / 2; i++)
	{
		RecordsToResend.push_back(DataHandler.Extract16());
	}

	ParseOffset = DataHandler.GetPosition();
}
std::string MBTCPMessageVerification::ToString()
{
	std::string ReturnValue = MBTCPRecord::ToString();

	ReturnValue += char(RespondError);
	for (int i = 3; i >= 0; i--)
	{
		ReturnValue += char((MessageId >> (i * 8)) % 256);
	}
	for (int i = 1; i >= 0; i--)
	{
		ReturnValue += char((ByteLengthOfResendRecords >> (i * 8)) % 256);
	}
	for (size_t i = 0; i < RecordsToResend.size(); i++)
	{
		ReturnValue += char(RecordsToResend[i] >> 8);
		ReturnValue += char(RecordsToResend[i] % 256);
	}
	return(ReturnValue);
}