#define NOMINMAX
#define _CRT_RAND_S 
#pragma once
#include <cstdint>
#include <string>
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

enum class MBCPMessageType : uint8_t
{
	InitiateConnection, AcceptConnection = 2, Null
};

enum class MBCPInitatioMessageType : uint8_t
{
	FirstMessage,
	PortSuggestion,
	AcceptPort,
	Null
};

class MBCMessage
{
protected:
	//variabel som underlättar parsing
	int ParseOffset = 0;
public:
	MBCPMessageType MBCMessageType = MBCPMessageType::Null;
	uint32_t RecordNumber = 0;
	MBCMessage(std::string StringToParse);
	MBCMessage();
	virtual std::string ToString();
	~MBCMessage();
};

class MBCInitatieConnectionMessage : public MBCMessage
{
public:
	MBCPInitatioMessageType InitationMessageType = MBCPInitatioMessageType::Null;
	uint16_t PortSuggestion = 2700;
	std::string SendAdress = "";
	MBCInitatieConnectionMessage(std::string StringToParse); //: MBCMessage(StringToParse);
	std::string ToString() override;
	MBCInitatieConnectionMessage(); //: MBCMessage();
};

class MBCConfirmConnectionMessage : public MBCMessage
{
public:
	std::string UserName = "";
	MBCConfirmConnectionMessage(std::string StringToParse); //: MBCMessage(StringToParse);
	MBCConfirmConnectionMessage();
	std::string ToString() override;
};



class MBTCPRecord
{
protected:
	int ParseOffset = 0;
public:
	MBTCPRecordType RecordType = MBTCPRecordType::Null;
	MBTCPRecordSendType SendType = MBTCPRecordSendType::Null;
	uint32_t RecordNumber = 0;
	MBTCPRecord();
	MBTCPRecord(std::string StringToParse);
	virtual std::string ToString();
};
class MBTCPInitatieMessageTransfer : public MBTCPRecord
{
public:
	uint32_t MessageId = 0;//sätter ett id som varje record i detta message har
	uint16_t NumberOfMessages = 0;//avgör hur många data messages som förväntas skickas med denna funktion
	uint32_t TotalDataLength = 0;//avgör förväntad total mängd data
	uint8_t DataCheckIntervall = 10;//avgör hur många protokoll max som kan skickas innan en integirets check förväntas
	MBTCPError RespondError = MBTCPError::OK;
	MBTCPInitatieMessageTransfer();
	MBTCPInitatieMessageTransfer(std::string StringToParse); //: MBTCPRecord(StringToParse);
	bool ParametersMatch(MBTCPInitatieMessageTransfer ObjectToCompare);
	std::string ToString() override;
};
class MBTCPMessage : public MBTCPRecord
{
public:
	uint32_t MessageId = 0;
	uint16_t MessageNumber = 1;
	uint16_t DataLength = 0;
	std::string Data;
	MBTCPMessage();
	MBTCPMessage(std::string StringToParse); //: MBTCPRecord(StringToParse);
	std::string ToString() override;
};
class MBTCPMessageVerification : public MBTCPRecord
{
public:
	MBTCPError RespondError = MBTCPError::OK;
	uint32_t MessageId = 0;
	uint16_t ByteLengthOfResendRecords = 0;
	std::vector<uint16_t> RecordsToResend = {};

	MBTCPMessageVerification();
	MBTCPMessageVerification(std::string StringToParse); //: MBTCPRecord(StringToParse);
	std::string ToString() override;
};