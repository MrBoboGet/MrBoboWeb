#include <MrBoboChatt/MBChatConnection.h>
#include <MrBoboChatt/MBCProtocols.h>
//#include <MBRandom.h>
#include <MrBoboChatt/MrBoboChatt.h>
//struct MBChatConnection
std::string MBChatConnection::GetNextResponseMessage(float SecondsBeforeTimeout)
{
	clock_t TimeOut = clock();
	while (!RecievedResponseMessage)
	{
		if ((clock() - TimeOut) / float(CLOCKS_PER_SEC) > SecondsBeforeTimeout)
		{
			return("");
		}
	}
	//fått meddelande, recieved response message är då nästa meddelande
	std::string ReturnString = "";
	{
		std::lock_guard<std::mutex> Lock(PeerResponseMutex);
		ReturnString = PeerResponseMessages.front();
		PeerResponseMessages.pop_front();
		if (PeerResponseMessages.size() == 0)
		{
			RecievedResponseMessage = false;
		}
	}
	return(ReturnString);
}
std::string MBChatConnection::GetNextSendMessage(float SecondsBeforeTimeout)
{
	clock_t TimeOut = clock();
	while (!RecievedSendMessage)
	{
		if ((clock() - TimeOut) / float(CLOCKS_PER_SEC) > SecondsBeforeTimeout)
		{
			return("");
		}
	}
	//fått meddelande, recieved response message är då nästa meddelande
	std::string ReturnString = "";
	{
		std::lock_guard<std::mutex> Lock(PeerSendMutex);
		ReturnString = PeerSendMessages.front();
		PeerSendMessages.pop_front();
		if (PeerSendMessages.size() == 0)
		{
			RecievedSendMessage = false;
		}
	}
	return(ReturnString);
}
MrBigInt MBChatConnection::GenerateRandomValue(uint16_t MaxNumberOfBytes)
{
	MrBigInt ReturnValue(0);
	for (size_t i = 0; i < MaxNumberOfBytes; i++)
	{
		ReturnValue = ReturnValue + MrBigInt(256).Pow(i) * MBRandom::GetRandomByte();
	}
	return(ReturnValue);
}
bool MBChatConnection::IpAddresIsLower(std::string LeftIPAdress, std::string RightIPAdress)
{
	return(LeftIPAdress.compare(RightIPAdress) > 0);
}
std::string MBChatConnection::EncryptData(std::string& DataToEncrypt)
{
	//vi generarar en random iv som vi encryptar datan med 
	unsigned char IV[16];
	for (size_t i = 0; i < 16; i++)
	{
		IV[i] = MBRandom::GetRandomByte();
	}
	int SizeOfEncryptedData = DataToEncrypt.size() + (16 - (DataToEncrypt.size() % 16));
	std::string EncryptedData(SizeOfEncryptedData, 0);
	plusaes::encrypt_cbc((const unsigned char*)DataToEncrypt.c_str(), DataToEncrypt.size(), (const unsigned char*)SecurityParameters.SharedSecret.c_str(), SecurityParameters.SharedSecret.size(), &IV, (unsigned char*)EncryptedData.c_str(), SizeOfEncryptedData, true);
	EncryptedData = std::string((char*)IV, 16) + EncryptedData;
	return(EncryptedData);
}
std::string MBChatConnection::DecryptData(std::string& DataToDecrypt)
{
	//anta alltid att IV:n är i början
	unsigned char IV[16];
	for (size_t i = 0; i < 16; i++)
	{
		IV[i] = DataToDecrypt[i];
	}
	unsigned long PaddingRemoved = 1;
	std::string DecryptedData(DataToDecrypt.size() - 16, 0);
	plusaes::decrypt_cbc((const unsigned char*)DataToDecrypt.substr(16).c_str(), DataToDecrypt.size() - 16, (const unsigned char*)SecurityParameters.SharedSecret.c_str(), SecurityParameters.SharedSecret.size(), &IV, (unsigned char*)DecryptedData.c_str(), DecryptedData.size(), &PaddingRemoved);
	return(DecryptedData.substr(0, DecryptedData.size() - PaddingRemoved));
}
MBChatConnection::MBChatConnection(std::string PeerIP, int Port) : ConnectionSocket(PeerIP, std::to_string(Port), MBSockets::TraversalProtocol::TCP)
{
	std::string PeerMessage = "";
	ConnectionSocket.UDPMakeSocketNonBlocking(StandardResponseWait);
	ConnectionSocket.Bind(Port);
	ConnectionListenThread = std::thread(MBChatConnection_ListenFunc, this);
}

MBError MBChatConnection::EstablishSecureConnection()
{
	MBError ReturnValue(true);
	if (SecurityParameters.ConnectionIsSecure)
	{
		return(ReturnValue);
	}
	std::string PeerResponse = "";
	MrBigInt PeerIntAfterExponenitation(0);
	MrBigInt Generator = MrBigInt(AssociatedChatObject->StaticResources.GetDiffieHellmanGenerator(), 16);
	MrBigInt Modolu = MrBigInt(AssociatedChatObject->StaticResources.GetDiffieHellmanModolu(), 16);
	MrBigInt LocalRandomNumber = GenerateRandomValue(2048 / 8);
	MrBigInt LocalRandomNumberAfterExponentiation = Generator.PowM(LocalRandomNumber, Modolu);

	//test grejer
	MrBigInt TestInt;
	std::string StringBefore;
	if (IpAddresIsLower(AssociatedChatObject->GetExtIp(), PeerIPAddress))
	{
		//vi skickar datan först
		//skickar den genom att tolka det som en 256 byte big endian string
		std::string StringToSend = "";
		for (int i = 255; i >= 0; i--)
		{
			StringToSend += char((LocalRandomNumberAfterExponentiation >> (i * 8)) % 256);
		}
		//AssociatedChatObject->PrintLine("Sent data " + ReplaceAll(HexEncodeString(StringToSend), " ", ""));
		SendData(StringToSend);
		PeerResponse = GetData();
	}
	else if (!(AssociatedChatObject->GetExtIp() == "127.0.0.1" && PeerIPAddress == "127.0.0.1"))
	{
		PeerResponse = GetData();
		std::string StringToSend = "";
		for (int i = 255; i >= 0; i--)
		{
			StringToSend += char((LocalRandomNumberAfterExponentiation >> (i * 8)) % 256);
		}
		//AssociatedChatObject->PrintLine("Sent data "+ReplaceAll(HexEncodeString(StringToSend), " ", ""));
		SendData(StringToSend);
	}
	else
	{
		//vi har enbart denna clause för att testa konceptet
		TestInt = GenerateRandomValue(256);
		TestInt = Generator.PowM(TestInt, Modolu);
		StringBefore = TestInt.GetString();
		for (int i = 255; i >= 0; i--)
		{
			PeerResponse += char((TestInt >> (i * 8)) % 256);
		}
	}
	for (int i = 255; i >= 0; i--)
	{
		PeerIntAfterExponenitation = PeerIntAfterExponenitation + MrBigInt(256).Pow(i) * MrBigInt(unsigned char(PeerResponse[255-i]));
	}
	//AssociatedChatObject->PrintLine("recieved data " + ReplaceAll(HexEncodeString(PeerResponse), " ", ""));
	//annan testgrej
	MrBigInt TestIgen(4);
	TestIgen = TestIgen.Pow(4);
	std::string TestIgenString = TestIgen.GetString();
	std::string TestConversionString = "";
	for (int i = 255; i >= 0; i--)
	{
		TestConversionString += char((PeerIntAfterExponenitation >> (i * 8)) % 256);
	}
	bool TestIntEqual = PeerIntAfterExponenitation == TestInt;
	std::string StringAfter = PeerIntAfterExponenitation.GetString();
	//
	//nu ska vi etablera master secreten
	MrBigInt MasterInteger = PeerIntAfterExponenitation.PowM(LocalRandomNumber, Modolu);
	//AssociatedChatObject->PrintLine("Master integer infered " + ReplaceAll(HexEncodeString(MasterInteger.GetString()), " ", ""));
	//hashar denna inte 
	SecurityParameters.SharedSecret = std::string(32, 0);
	picosha2::hash256(MasterInteger.GetString(), SecurityParameters.SharedSecret);
	//vi är nu klara, vi har etablerat ett en gemensam secret
	SecurityParameters.ConnectionIsSecure = true;
	return(ReturnValue);
}
void MBChatConnection::SetDescription(std::string NewDescription)
{
	std::lock_guard<std::mutex> Lock(ConnectionMutex);
	ConnectionDescription = NewDescription;
}
std::string MBChatConnection::GetDescription()
{
	std::lock_guard<std::mutex> Lock(ConnectionMutex);
	return(ConnectionDescription);
}
MBError MBChatConnection::SendData(std::string DataToSend)
{
	//vi initierat MBTCP protokollet
	//vi börjar med att skicka initiate meddelandet och väntar sedan på svaret från motparten
	//headers för protokollen är 16 byte, men vi tar 20 bytes för lite marginal
	MBError ReturnError(true);
	if (DataToSend == "")
	{
		ReturnError.Type = MBErrorType::Error;
		ReturnError.ErrorMessage = "No data to send";
		return(ReturnError);
	}
	if (SecurityParameters.ConnectionIsSecure)
	{
		DataToSend = EncryptData(DataToSend);
	}
	MBTCPInitatieMessageTransfer FirstMessage;
	FirstMessage.SendType = MBTCPRecordSendType::Send;
	FirstMessage.RecordNumber = PrivateRecordNumber;
	PrivateRecordNumber += 1;
	FirstMessage.NumberOfMessages = DataToSend.size() / MaxSendLength;
	if (DataToSend.size() % MaxSendLength != 0)
	{
		FirstMessage.NumberOfMessages += 1;
	}
	FirstMessage.DataCheckIntervall = 10;
	if (FirstMessage.DataCheckIntervall > FirstMessage.NumberOfMessages)
	{
		FirstMessage.DataCheckIntervall = FirstMessage.NumberOfMessages;
	}
	FirstMessage.TotalDataLength = DataToSend.size();
	FirstMessage.RecordType = MBTCPRecordType::InitiateMessageTransfer;
	//vi borde faktiskt ha en bra randomfunktion
	FirstMessage.MessageId = rand();
	//nu tar vi och faktiskt skickar messaget
	bool RecievedFirstMessageResponse = false;
	int FirstMessagesSent = 0;
	std::string ResponseData = "";
	while (!RecievedFirstMessageResponse)
	{
		ConnectionSocket.UDPSendData(FirstMessage.ToString(), PeerIPAddress, ConnectionPort);
		ResponseData = GetNextResponseMessage();
		if (ResponseData != "")
		{
			break;
		}
		else
		{
			FirstMessagesSent += 1;
			if (FirstMessagesSent > MessagesBeforeDisconnection)
			{
				ReturnError.Type = MBErrorType::Error;
				ReturnError.ErrorMessage = "No response from host";
				return(ReturnError);
			}
		}
	}
	std::string DataSent = FirstMessage.ToString();
	MBTCPInitatieMessageTransfer ResponseMessage(ResponseData);
	if (ResponseMessage.RespondError != MBTCPError::OK || !FirstMessage.ParametersMatch(ResponseMessage))
	{
		ReturnError.Type = MBErrorType::Error;
		ReturnError.ErrorMessage = "Error in response message";
		return(ReturnError);
	}

	//när vi fått bekräftat att vi kan börja skicka grejer så tar vi och gör det tills vi antingen nått den bestämda MessageCheck intervallet eller skickat alla meddelanden
	bool Finished = false;
	int MessageBatchesSent = 0;
	while (!Finished)
	{
		bool AllDataRecieved = false;
		std::vector<std::string> PartitionedMessageData = std::vector<std::string>(FirstMessage.DataCheckIntervall);
		for (size_t i = 0; i < PartitionedMessageData.size(); i++)
		{
			if (i * MaxSendLength + MessageBatchesSent * FirstMessage.DataCheckIntervall > DataToSend.size())
			{
				PartitionedMessageData[i] = "";
				continue;
			}
			PartitionedMessageData[i] = DataToSend.substr(i * MaxSendLength + MessageBatchesSent * FirstMessage.DataCheckIntervall, MaxSendLength);
		}
		//först så skickar vi alla data, väntar sedan på responset och ser om vi ska skicka något igen
		for (size_t i = 0; i < PartitionedMessageData.size(); i++)
		{
			MBTCPMessage MessageToSend;
			MessageToSend.Data = PartitionedMessageData[i];
			MessageToSend.DataLength = MessageToSend.Data.size();
			MessageToSend.MessageId = FirstMessage.MessageId;
			MessageToSend.MessageNumber = i + 1 + MessageBatchesSent * FirstMessage.DataCheckIntervall;
			MessageToSend.RecordNumber = PrivateRecordNumber;
			PrivateRecordNumber += 1;
			MessageToSend.RecordType = MBTCPRecordType::MessageData;
			MessageToSend.SendType = MBTCPRecordSendType::Send;
			ConnectionSocket.UDPSendData(MessageToSend.ToString(), PeerIPAddress, ConnectionPort);
		}
		int WaitCycles = 0;
		bool PeerDisconeccted = false;
		while (!AllDataRecieved)
		{
			std::string ResponseData = GetNextResponseMessage();
			if (ResponseData == "")
			{
				//timeout, vi väntar igen en given tid
				WaitCycles += 1;
				if (WaitCycles > MessagesBeforeDisconnection)
				{
					PeerDisconeccted = true;
					break;
				}
				continue;
			}
			else
			{
				MBTCPMessageVerification VerificationResponse(ResponseData);
				if (VerificationResponse.RespondError != MBTCPError::OK || VerificationResponse.MessageId != FirstMessage.MessageId)
				{
					//vet inte vad protokollet ska göra här
				}
				else
				{
					if (VerificationResponse.RecordsToResend.size() == 0)
					{
						//alla messages har blivit reciavade, vi är klara med denna batch
						AllDataRecieved = true;
					}
					else
					{
						//vi skickar recordsen den inte fick, går till toppen av loopen och väntar på respons data
						for (size_t i = 0; i < VerificationResponse.RecordsToResend.size(); i++)
						{
							int RecordToResendIndex = VerificationResponse.RecordsToResend[i] % FirstMessage.DataCheckIntervall;
							MBTCPMessage MessageToSend;
							MessageToSend.Data = PartitionedMessageData[RecordToResendIndex - 1];
							MessageToSend.DataLength = MessageToSend.Data.size();
							MessageToSend.MessageId = FirstMessage.MessageId;
							MessageToSend.MessageNumber = VerificationResponse.RecordsToResend[i];
							MessageToSend.RecordNumber = PrivateRecordNumber;
							PrivateRecordNumber += 1;
							MessageToSend.RecordType = MBTCPRecordType::MessageData;
							MessageToSend.SendType = MBTCPRecordSendType::Send;
							ConnectionSocket.UDPSendData(MessageToSend.ToString(), PeerIPAddress, ConnectionPort);
						}
					}
				}
			}
		}
		MessageBatchesSent += 1;
		if (MessageBatchesSent * FirstMessage.DataCheckIntervall >= FirstMessage.NumberOfMessages)
		{
			Finished = true;
		}
	}
	return(ReturnError);
}
std::string MBChatConnection::GetData(float TimeoutInSeconds)
{
	//den första recorden får ska alltid vara en initate connection meddelande
	std::string ReturnValue = "";
	std::string PeerFirstMessageData;
	int FirstWaitTime = 0;
	//skickar lite data så vi breakar that nat
	while (true)
	{
		MBTCPRecord BreakNatMessage;
		BreakNatMessage.SendType = MBTCPRecordSendType::Respond;
		BreakNatMessage.RecordType = MBTCPRecordType::NATBreakMessage;
		BreakNatMessage.RecordNumber = 0;
		ConnectionSocket.UDPSendData(BreakNatMessage.ToString(), PeerIPAddress, ConnectionPort);
		PeerFirstMessageData = GetNextSendMessage();
		if (PeerFirstMessageData != "")
		{
			break;
		}
		else
		{
			FirstWaitTime += 1;
			if (FirstWaitTime > MessagesBeforeDisconnection)
			{
				return("");
			}
		}
	}
	MBTCPInitatieMessageTransfer FirstMessage(PeerFirstMessageData);
	MBTCPInitatieMessageTransfer FirstMessageResponse(FirstMessage.ToString());
	FirstMessageResponse.SendType = MBTCPRecordSendType::Respond;
	ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
	//vi skickar data igen om vi inte får någon data
	bool Finished = false;
	bool PeerRecievedResponseMessage = false;
	int FirstMessageResponseResends = 0;
	while (!Finished)
	{
		std::string NextSendData = GetNextSendMessage();
		if (NextSendData == "")
		{
			ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
			FirstMessageResponseResends += 1;
			if (FirstMessageResponseResends > MessagesBeforeDisconnection)
			{
				return("");
			}
		}
		else
		{
			MBTCPRecord NextResponseType(NextSendData);
			if (NextResponseType.RecordType == MBTCPRecordType::InitiateMessageTransfer)
			{
				//vi vet att vi behöver skicka får response igen, den spåg inte den
				ConnectionSocket.UDPSendData(FirstMessageResponse.ToString(), PeerIPAddress, ConnectionPort);
			}
			else if (NextResponseType.RecordType == MBTCPRecordType::MessageData)
			{
				//första meddelandet är bekräftat, vi går nu in i en loop där vi faktiskt ska bygga på retur värdet
				MBTCPMessage NextResponse(NextSendData);
				std::unordered_map<uint16_t, std::string> RecievedRecordsMap = {};
				RecievedRecordsMap[NextResponse.MessageNumber] = NextResponse.Data;
				int MessagesInBatchRecieved = 1;
				int CompleteBatchesRecieved = 0;
				std::string LastDataSent = "";
				while (true)
				{
					int ExpectedMessages = FirstMessage.NumberOfMessages - CompleteBatchesRecieved * FirstMessage.DataCheckIntervall;
					if (ExpectedMessages > FirstMessage.DataCheckIntervall)
					{
						ExpectedMessages = FirstMessage.DataCheckIntervall;
					}
					if (ExpectedMessages < 0)
					{
						ExpectedMessages = 1;
					}
					if (MessagesInBatchRecieved < ExpectedMessages)
					{
						NextSendData = GetNextSendMessage();
						if (NextSendData == "")
						{
							//personen har timat ut, vi ber den att skicka igen
							MBTCPMessageVerification MissingRecordVerification;
							MissingRecordVerification.MessageId = FirstMessage.MessageId;
							MissingRecordVerification.SendType = MBTCPRecordSendType::Respond;
							MissingRecordVerification.RecordType = MBTCPRecordType::MessageData;
							MissingRecordVerification.RecordNumber = 0;
							for (size_t i = 0; i < ExpectedMessages; i++)
							{
								if (RecievedRecordsMap.find(i + 1 + CompleteBatchesRecieved * FirstMessage.DataCheckIntervall) == RecievedRecordsMap.end())
								{
									MissingRecordVerification.RecordsToResend.push_back(i + 1 + CompleteBatchesRecieved * FirstMessage.DataCheckIntervall);
								}
							}
							MissingRecordVerification.ByteLengthOfResendRecords = (2 * MissingRecordVerification.RecordsToResend.size());
							ConnectionSocket.UDPSendData(MissingRecordVerification.ToString(), PeerIPAddress, ConnectionPort);
							LastDataSent = MissingRecordVerification.ToString();
						}
						else
						{
							//kollar om messaget vi får är av typen "ge respons"
							if (MBTCPMessage(NextSendData).RecordType == MBTCPRecordType::RequestResponse)
							{
								ConnectionSocket.UDPSendData(LastDataSent, PeerIPAddress, ConnectionPort);
								continue;
							}
							MBTCPMessage NextResponseMessage(NextSendData);
							if (RecievedRecordsMap.find(NextResponseMessage.MessageNumber) == RecievedRecordsMap.end())
							{
								RecievedRecordsMap[NextResponseMessage.MessageNumber] = NextResponseMessage.Data;
								MessagesInBatchRecieved += 1;
							}
						}
					}
					else
					{
						//vi appendar datan till vår retur string och resettar för nästa batch
						for (size_t i = 0; i < ExpectedMessages; i++)
						{
							ReturnValue += RecievedRecordsMap[i + 1 + CompleteBatchesRecieved * FirstMessage.DataCheckIntervall];
						}
						RecievedRecordsMap.clear();
						MessagesInBatchRecieved = 0;
						CompleteBatchesRecieved += 1;
						MBTCPMessageVerification MissingRecordVerification;
						MissingRecordVerification.MessageId = FirstMessage.MessageId;
						MissingRecordVerification.SendType = MBTCPRecordSendType::Respond;
						MissingRecordVerification.RecordType = MBTCPRecordType::MessageData;
						MissingRecordVerification.RecordNumber = 0;
						MissingRecordVerification.ByteLengthOfResendRecords = 0;
						ConnectionSocket.UDPSendData(MissingRecordVerification.ToString(), PeerIPAddress, ConnectionPort);
						LastDataSent = MissingRecordVerification.ToString();
						if (CompleteBatchesRecieved * FirstMessage.DataCheckIntervall >= FirstMessage.NumberOfMessages)
						{
							Finished = true;
							break;
						}
					}
				}
			}
		}
	}
	if (SecurityParameters.ConnectionIsSecure)
	{
		ReturnValue = DecryptData(ReturnValue);
	}
	return(ReturnValue);
}
void MBChatConnection_ListenFunc(MBChatConnection* AssociatedChatObject)
{
	while (!AssociatedChatObject->ShouldStop)
	{
		std::string NextDataRecieved = AssociatedChatObject->ConnectionSocket.UDPGetData();
		if (NextDataRecieved != "")
		{
			MBTCPRecord DataRecord(NextDataRecieved);
			if (DataRecord.RecordType == MBTCPRecordType::NATBreakMessage)
			{

			}
			else if (DataRecord.SendType == MBTCPRecordSendType::Send)
			{
				std::lock_guard<std::mutex> Lock(AssociatedChatObject->PeerSendMutex);
				AssociatedChatObject->PeerSendMessages.push_back(NextDataRecieved);
				AssociatedChatObject->RecievedSendMessage = true;
			}
			else if (DataRecord.SendType == MBTCPRecordSendType::Respond)
			{
				std::lock_guard<std::mutex> Lock(AssociatedChatObject->PeerResponseMutex);
				AssociatedChatObject->PeerResponseMessages.push_back(NextDataRecieved);
				AssociatedChatObject->RecievedResponseMessage = true;
			}
		}
	}
}