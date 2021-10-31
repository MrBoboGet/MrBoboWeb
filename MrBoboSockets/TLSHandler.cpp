#define NOMINMAX
#define _CRT_RAND_S
#include <MrBoboSockets/MrBoboSockets.h>
#include "TLSHandler.h"
#include <math.h>
#include <MBUtility/MBRandom.h>
#include <MBUtility/MBStrings.h>
#include <filesystem>
#include <mutex>
#include <ostream>
#include <iostream>
//MBSockets::Socket* AssociatedSocket = nullptr;

//TODO Vi vill helst bli av med denna depenadncy, exempelvis genom att optimera min klass
#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>
#include <cryptopp/algebra.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>
#include <cryptopp/gcm.h>


std::string MBSha256(std::string const& StringToHash)
{
	return(MBCrypto::HashData(StringToHash, MBCrypto::HashFunction::SHA256));
}

//std::string MBSha1(std::string const& StringToHash)
//{
//	Chocobo1::SHA1 Hasher;
//	Hasher.addData(StringToHash.c_str(), StringToHash.size());
//	Hasher.finalize();
//	return(std::string((char*)&Hasher.toArray()[0],20));
//	//sha1::SHA1 Sha1;
//	//Sha1.processBytes(StringToHash.data(), StringToHash.size());
//	//std::string ReturnValue(20, 0);
//	//Sha1.getDigestBytes((uint8_t*)ReturnValue.data());
//	//return(ReturnValue);
//}
namespace TLS1_2
{
	ECDHEServerKeyExchange GetServerECDHEKey(SecurityParameters& ConnectionParams,std::string const& CompleteRecordData)
	{
		ECDHEServerKeyExchange ReturnValue;
		std::string ServerKeyExchangeData = CompleteRecordData.substr(5 + 4);
		NetWorkDataHandler Extracter((const uint8_t*)ServerKeyExchangeData.c_str());
		size_t StartOfServerECDHEParams = Extracter.GetPosition();
		ReturnValue.CurveType = (ECCurveType) Extracter.Extract8();
		ReturnValue.NamedCurve = (MBCrypto::NamedElipticCurve) Extracter.Extract16();
		size_t CurveLength = Extracter.Extract8();
		ReturnValue.ECPoint = ServerKeyExchangeData.substr(Extracter.GetPosition(), CurveLength);
		Extracter.SetPosition(CurveLength + Extracter.GetPosition());
		size_t EndOfServerKeyExchangeParams = Extracter.GetPosition();
		std::string ServerParamsData = ServerKeyExchangeData.substr(StartOfServerECDHEParams, EndOfServerKeyExchangeParams - StartOfServerECDHEParams);
		std::string DataToHash = std::string((char*)ConnectionParams.client_random, 32) + std::string((char*)ConnectionParams.server_random, 32) + ServerParamsData;
		ReturnValue.ClientHashedServerECDHEParams = ConnectionParams.HashAlgorithm.Hash(DataToHash);
		ReturnValue.SignatureAlgorithm.Hash =(HashAlgorithm) Extracter.Extract8();
		ReturnValue.SignatureAlgorithm.Signature =(SignatureAlgorithm) Extracter.Extract8();
		size_t LengthOfSignatureData = Extracter.Extract16();
		ReturnValue.ServerSignedData = ServerKeyExchangeData.substr(Extracter.GetPosition(), LengthOfSignatureData);
		return(ReturnValue);
	}
	std::string GetAlertErrorDescription(uint8_t ErrorValue)
	{
		if (ErrorValue == close_notify)
		{
			return("close_notify");
		}
		else if (ErrorValue == unexpected_message)
		{
			return("unexpected_message");
		}
		else if (ErrorValue == bad_record_mac)
		{
			return("bad_record_mac");
		}
		else if (ErrorValue == decryption_failed_RESERVED)
		{
			return("decryption_failed_RESERVED");
		}
		else if (ErrorValue == record_overflow)
		{
			return("record_overflow");
		}
		else if (ErrorValue == decompression_failure)
		{
			return("decompression_failure");
		}
		else if (ErrorValue == handshake_failure)
		{
			return("handshake_failure");
		}
		else if (ErrorValue == no_certificate_RESERVED)
		{
			return("no_certificate_RESERVED");
		}
		else if (ErrorValue == bad_certificate)
		{
			return("bad_certificate");
		}
		else if (ErrorValue == unsupported_certificate)
		{
			return("unsupported_certificate");
		}
		else if (ErrorValue == certificate_revoked)
		{
			return("certificate_revoked");
		}
		else if (ErrorValue == certificate_expired)
		{
			return("certificate_expired");
		}
		else if (ErrorValue == certificate_unknown)
		{
			return("certificate_unknown");
		}
		else if (ErrorValue == illegal_parameter)
		{
			return("illegal_parameter");
		}
		else if (ErrorValue == unknown_ca)
		{
			return("unknown_ca");
		}
		else if (ErrorValue == access_denied)
		{
			return("access_denied");
		}
		else if (ErrorValue == decode_error)
		{
			return("decode_error");
		}
		else if (ErrorValue == decrypt_error)
		{
			return("decrypt_error");
		}
		else if (ErrorValue == export_restriction_RESERVED)
		{
			return("export_restriction_RESERVED");
		}
		else if (ErrorValue == protocol_version)
		{
			return("protocol_version");
		}
		else if (ErrorValue == insufficient_security)
		{
			return("insufficient_security");
		}
		else if (ErrorValue == internal_error)
		{
			return("internal_error");
		}
		else if (ErrorValue == user_canceled)
		{
			return("user_canceled");
		}
		else if (ErrorValue == no_renegotiation)
		{
			return("no_renegotiation");
		}
		else if (ErrorValue == unsupported_extension)
		{
			return("unsupported_extension");
		}
		else
		{
			return("ErrorCodeNotValid");
		}
	}
	bool ErrorIsFatal(uint8_t ErrorValue)
	{
		switch (ErrorValue) {
		case close_notify:					return(true);
		case unexpected_message:			return(true);
		case bad_record_mac:				return(true);
		case decryption_failed_RESERVED:	return(true);
		case record_overflow:				return(true);
		case decompression_failure:			return(true);
		case handshake_failure:				return(true);
		case no_certificate_RESERVED:		return(true);
		case bad_certificate:				return(true);
		case unsupported_certificate:		return(true);
		case certificate_revoked:			return(true);
		case certificate_expired:			return(true);
		case certificate_unknown:			return(true);
		case illegal_parameter:				return(true);
		case unknown_ca:					return(true);
		case access_denied:					return(true);
		case decode_error:					return(true);
		case decrypt_error:					return(true);
		case export_restriction_RESERVED:	return(true);
		case protocol_version:				return(true);
		case insufficient_security:			return(true);
		case internal_error:				return(true);
		case user_canceled:					return(true);
		case no_renegotiation:				return(true);
		case unsupported_extension:			return(true);
		}
	}
	int SizeOfProccessedHelloClientStruct(TLS1_2HelloClientStruct& StructToProcess)
	{
		int ReturnValue = 0;
		ReturnValue += sizeof(ProtocolVersion);
		ReturnValue += sizeof(Random);
		ReturnValue += 1 + sizeof(uint8_t) * StructToProcess.SessionId.size();
		ReturnValue += 2 + sizeof(CipherSuite) * StructToProcess.CipherSuites.size();
		ReturnValue += 1 + sizeof(uint8_t) * StructToProcess.CompressionMethods.size();
		//eftersom varje extension nu sedan kan innahålla varierande mängd data behöver vi loopa igen alla
		if (StructToProcess.OptionalExtensions.size() > 0)
		{
			ReturnValue += sizeof(uint16_t);
		}
		for (int i = 0; i < StructToProcess.OptionalExtensions.size(); i++)
		{
			ReturnValue += sizeof(ExtensionTypes) + 2 + sizeof(uint8_t) * StructToProcess.OptionalExtensions[i].ExtensionData.size();
		}
		return(ReturnValue);
	}
	void FillArrayWithHelloClientData(uint8_t* ArrayToFill, TLS1_2HelloClientStruct& Data)
	{
		NetWorkDataHandler ArrayFiller(ArrayToFill);
		ArrayFiller << Data.ProtocolVers.Major;
		ArrayFiller << Data.ProtocolVers.Minor;
		//ArrayFiller << uint32_t(Data.RandomStruct.GMT_UNIX_TIME);
		for (size_t i = 0; i < 32; i++)
		{
			ArrayFiller << uint8_t(Data.RandomStruct.RandomBytes[i]);
		}
		ArrayFiller << uint8_t(Data.SessionId.size());
		for (int i = 0; i < Data.SessionId.size(); i++)
		{
			ArrayFiller << Data.SessionId[i];
		}
		ArrayFiller << uint16_t(Data.CipherSuites.size() * 2);
		for (int i = 0; i < Data.CipherSuites.size(); i++)
		{
			ArrayFiller << Data.CipherSuites[i].Del1;
			ArrayFiller << Data.CipherSuites[i].Del2;
		}
		ArrayFiller << uint8_t(Data.CompressionMethods.size());
		for (int i = 0; i < Data.CompressionMethods.size(); i++)
		{
			ArrayFiller << Data.CompressionMethods[i];
		}
		//extensiones säger inte sin längd innan om dem om inte finns
		if (Data.OptionalExtensions.size() > 0)
		{
			uint16_t LengthOfExtensions = 0;
			for (int i = 0; i < Data.OptionalExtensions.size(); i++)
			{
				LengthOfExtensions += sizeof(ExtensionTypes) + sizeof(uint16_t) + Data.OptionalExtensions[i].ExtensionData.size();
			}
			ArrayFiller << LengthOfExtensions;
		}
		for (int i = 0; i < Data.OptionalExtensions.size(); i++)
		{
			ArrayFiller << uint16_t(Data.OptionalExtensions[i].ExtensionType);
			ArrayFiller << uint16_t(Data.OptionalExtensions[i].ExtensionData.size());
			for (int j = 0; j < Data.OptionalExtensions[i].ExtensionData.size(); j++)
			{
				ArrayFiller << Data.OptionalExtensions[i].ExtensionData[j];
			}
		}
	}
	Extension GenerateServernameExtension(std::string DomainName)
	{
		Extension ExtensionToReturn;
		ExtensionToReturn.ExtensionType = ExtensionTypes::server_name;
		ExtensionToReturn.ExtensionData = std::vector<uint8_t>(0);
		//ReplaceAll(&DomainName, "www.", "");
		uint16_t ServerNameListLength = sizeof(ServerNameNameTypes) + sizeof(uint16_t) + DomainName.size();
		uint8_t* ServerNameData = new uint8_t[sizeof(uint16_t) + ServerNameListLength];
		NetWorkDataHandler ArrayFiller(ServerNameData);
		ArrayFiller << uint16_t(ServerNameListLength);
		ArrayFiller << uint8_t(host_name);
		ArrayFiller << uint16_t(DomainName.size());
		for (int i = 0; i < DomainName.size(); i++)
		{
			ArrayFiller << uint8_t(DomainName[i]);
		}
		for (int i = 0; i < sizeof(uint16_t) + ServerNameListLength; i++)
		{
			ExtensionToReturn.ExtensionData.push_back(ServerNameData[i]);
		}
		delete[] ServerNameData;
		return(ExtensionToReturn);
	}
	void ParseServerHelloToStruct(TLS1_2ServerHelloStruct* StructToPopulate, std::string& ServerHelloData)
	{
		NetWorkDataHandler NetworkHandler(reinterpret_cast<const uint8_t*>(ServerHelloData.c_str()));
		//StructToPopulate->Protocol = { NetworkHandler.Extract8(),NetworkHandler.Extract8() };
		StructToPopulate->Protocol.Major = NetworkHandler.Extract8();
		StructToPopulate->Protocol.Minor = NetworkHandler.Extract8();
		for (int i = 0; i < sizeof(Random); i++)
		{
			NetworkHandler >> static_cast<uint8_t&>(StructToPopulate->RandomStruct.RandomBytes[i]);
		}
		uint8_t LengthOfSessionId;
		NetworkHandler >> LengthOfSessionId;
		for (int i = 0; i < LengthOfSessionId; i++)
		{
			StructToPopulate->SessionId.push_back(NetworkHandler.Extract8());
		}
		//StructToPopulate->CipherSuiteToUse = { NetworkHandler.Extract8(),NetworkHandler.Extract8() };
		StructToPopulate->CipherSuiteToUse.Del1 = NetworkHandler.Extract8();
		StructToPopulate->CipherSuiteToUse.Del2 = NetworkHandler.Extract8();
		StructToPopulate->CompressionToUse = NetworkHandler.Extract8();
		int NumberOfExtensions = 0;
		uint16_t TotalLengthOfExtensions = 0;
		if (NetworkHandler.GetPosition() < ServerHelloData.size() - 1)
		{
			//finns data att extracta
			if (NumberOfExtensions == 0)
			{
				TotalLengthOfExtensions = NetworkHandler.Extract16();
			}
			Extension NyExtension;
			NyExtension.ExtensionType = static_cast<TLS1_2::ExtensionTypes>(NetworkHandler.Extract16());
			NyExtension.ExtensionData = std::vector<uint8_t>(0);
			uint16_t LengthOfExtension = NetworkHandler.Extract16();
			for (int i = 0; i < LengthOfExtension; i++)
			{
				NyExtension.ExtensionData.push_back(NetworkHandler.Extract8());
			}
			NumberOfExtensions += 1;
			StructToPopulate->OptionalExtensions.push_back(NyExtension);
		}
	}
	TLSServerPublickeyInfo GetServerPublicKey(std::string& ServerCertificateData)
	{
		uint64_t Offset = 0;
		TLSServerPublickeyInfo ReturnValue;
		uint64_t AbsoluteOffsetToNextCertficate = 0;
		ASN1Extracter Parser(reinterpret_cast<const uint8_t*>(&ServerCertificateData.c_str()[Offset]));
		ASN1TagValue  FirstTag = Parser.ExtractTagData();
		if (FirstTag.TagType != ASN1PrimitiveTagNumber::Sequence)
		{
			//yikers
			std::cout << uint16_t(FirstTag.TagType) << std::endl;
			assert(false);
		}
		/*
		   Certificate  ::=  SEQUENCE  {
			tbsCertificate       TBSCertificate,
			signatureAlgorithm   AlgorithmIdentifier,
			signatureValue       BIT STRING  }

	   TBSCertificate  ::=  SEQUENCE  {
			version         [0]  EXPLICIT Version DEFAULT v1,
			serialNumber         CertificateSerialNumber,
			signature            AlgorithmIdentifier,
			issuer               Name,
			validity             Validity,
			subject              Name,
			subjectPublicKeyInfo SubjectPublicKeyInfo,
			issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
			subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
			extensions      [3]  EXPLICIT Extensions OPTIONAL
			}

			SubjectPublicKeyInfo  ::=  SEQUENCE
			{
				algorithm            AlgorithmIdentifier,
				subjectPublicKey     BIT STRING
			}
		*/
		uint64_t LengthOfCurrentCertificate = Parser.ExtractLengthOfType();
		//skippar till signature algorithm fielden

		//först entrar vi tbscertificate fältet
		Parser.ExtractTagData();
		Parser.ExtractLengthOfType();

		//eftersom att den har ett default value kollar vi först och främst om den finns, gör den inte det så vet vi att vi skippar rätt grej i alla fall
		ASN1TagValue VersionTag = Parser.ExtractTagData();
		if (VersionTag.AlternateTagValue == 0)
		{
			//det är faktiskt en tag vi extractar och inte serialnumber

			//vi skippar helt enkelt typen vi börjar extracta, som vi nu vet är serialnumber
			uint64_t LengthOfTypeData = Parser.ExtractLengthOfType();
			Parser.SetOffset(Parser.GetOffset() + LengthOfTypeData);
		}
		else
		{
			uint64_t LengthOfVersionData = Parser.ExtractLengthOfType();
			Parser.SetOffset(Parser.GetOffset() + LengthOfVersionData);
			Parser.SkipToNextField();//Skippar SerialNumber
		}
		Parser.SkipToNextField();//Skippar Signature
		Parser.SkipToNextField();//Skippar Issuer
		Parser.SkipToNextField();//Skippar Validity
		Parser.SkipToNextField();//Skippar issuer
		Parser.SkipToNextField();//Skippar skippar name
		//nu vill vi entra subjectpublickeyinfo fältet
		ASN1TagValue SubjectPublicKeyInfoTag = Parser.ExtractTagData();
		uint64_t LengthOfKeyInfo = Parser.ExtractLengthOfType();
		Parser.SkipToNextField();//Skippar algorithm identifer i subjcet public key info
		//nu jävlar, nu är vi äntligen i själva RSA keyn som vi vill ha
		ASN1TagValue BitStringTag = Parser.ExtractTagData();
		uint64_t LenthOfBitString = Parser.ExtractLengthOfType();
		ReturnValue.ServerKeyData = "";
		for (size_t i = 0; i < LenthOfBitString; i++)
		{
			ReturnValue.ServerKeyData += Parser.ExtractByte();
		}
		//Relevant om den första certifikaten inte inehåller public keyn
		//AbsoluteOffsetToNextCertficate = Offset + Parser.GetOffset() + LengthOfCurrentCertificate;
		//nu tar vi och ser om vi faktiskt kan ta och hitta lite spicy public key data
		//Offset = AbsoluteOffsetToNextCertficate;
		//nu koller vi längden till nästa value
		return(ReturnValue);
	}
	std::string GetRecordString(TLS1_2GenericRecord const& RecordToEncode)
	{
		std::string ReturnValue = "";
		ReturnValue += char(RecordToEncode.Type);
		ReturnValue += char(RecordToEncode.Protocol.Major);
		ReturnValue += char(RecordToEncode.Protocol.Minor);
		ReturnValue += char(RecordToEncode.Data.size() >> 8);
		ReturnValue += char(RecordToEncode.Data.size() % 256);
		ReturnValue += RecordToEncode.Data;
		return(ReturnValue);
	}
	std::string GetRecordString(TLS1_2HanshakeMessage const& MessageToEncode)
	{
		std::string ReturnValue = "";
		ReturnValue += (char)MessageToEncode.Type;
		for (size_t i = 0; i < 3; i++)
		{
			ReturnValue += char((MessageToEncode.MessageData.size() >> ((2 - i) * 8)) % 256);
		}
		ReturnValue += MessageToEncode.MessageData;
		return(ReturnValue);
	}
};

std::mutex CachedSessionsMutex;
std::unordered_map<std::string, TLS1_2::SecurityParameters> CachedSessions = {};



void FillArrayWithRandomBytes(uint8_t* ArrayToFill, int LengthOfArray)
{
	srand(time(0));
	for (size_t i = 0; i < LengthOfArray; i++)
	{
		ArrayToFill[i] = (uint8_t)rand();
	}
}
CryptoPP::Integer MBGToCryptoPP(MrBigInt const& IntToConvert)
{
	std::string IntData = IntToConvert.GetLittleEndianString();
	CryptoPP::Integer ReturnValue = CryptoPP::Integer((const CryptoPP::byte*)IntData.c_str(), IntData.size(), CryptoPP::Integer::UNSIGNED, CryptoPP::LITTLE_ENDIAN_ORDER);
	return(ReturnValue);
}
MrBigInt CryptoPPToMBG(CryptoPP::Integer const& IntToConvert)
{
	std::string IntData = "";
	IntData.reserve(IntToConvert.ByteCount());
	for (int i = 0; i < IntToConvert.ByteCount(); i++)
	{
		IntData += IntToConvert.GetByte(i);
	}
	MrBigInt ReturnValue;
	ReturnValue.SetFromLittleEndianArray(IntData.c_str(), IntData.size());
	return(ReturnValue);
}
RSAPublicKey TLSHandler::ExtractRSAPublicKeyFromBitString(std::string& BitString)
{
	RSAPublicKey ReturnValue;
	MrBigInt PublicKeyPrime = 0;
	//parsa primen från datan, först kollar vi att paddingen är 0
	if (BitString[0] != 0)
	{
		std::cout << "Fel formatterad public key" << std::endl;
		assert(false);
		return ReturnValue;
	}
	ASN1Extracter Parser(reinterpret_cast<const uint8_t*>(BitString.c_str()));
	Parser.ExtractByte();
	Parser.ExtractTagData();
	Parser.ExtractLengthOfType();//vi vill skippa den del som bara encodar vår dubbel int typ
	Parser.ExtractTagData();
	uint64_t LengthOfPrime = Parser.ExtractLengthOfType();
	if (!(LengthOfPrime == 257 || LengthOfPrime == 513))
	{
		std::cout << "Prime is wrongly formatted :(" << std::endl;
		assert(false);
	}
	else
	{
		//första är alltid 0
		PublicKeyPrime.SetFromBigEndianArray(&BitString.c_str()[Parser.GetOffset() + 1], LengthOfPrime-1);
		for (size_t i = 0; i < LengthOfPrime; i++)
		{
			Parser.ExtractByte();
		}
	}
	//std::cout << PublicKeyPrime.get_str() << std::endl;
	//nu räknar vi ut exponenten
	Parser.ExtractTagData();
	uint64_t LengthOfExponent = Parser.ExtractLengthOfType();
	MrBigInt Exponent(0);
	Exponent.SetFromBigEndianArray(&BitString[Parser.GetOffset()], LengthOfExponent);
	ReturnValue.Modolu = PublicKeyPrime;
	ReturnValue.Exponent = Exponent;
	return(ReturnValue);
}
MrBigInt TLSHandler::OS2IP(const char* Data,uint64_t LengthOfData)
{
	MrBigInt ReturnValueTest(0);
	ReturnValueTest.SetFromBigEndianArray(Data, LengthOfData);
	return(ReturnValueTest);
}
MrBigInt TLSHandler::RSAEP(TLSServerPublickeyInfo& RSAInfo,MrBigInt const& MessageRepresentative)
{
	MrBigInt ReturnValue(0);
	RSAPublicKey PublicKey = ExtractRSAPublicKeyFromBitString(RSAInfo.ServerKeyData);
	CryptoPP::Integer TempRepresentative = MBGToCryptoPP(MessageRepresentative);
	CryptoPP::Integer TempExponent = MBGToCryptoPP(PublicKey.Exponent);
	CryptoPP::Integer TempModolu = MBGToCryptoPP(PublicKey.Modolu);
	CryptoPP::ModularArithmetic TempMA(TempModolu);
	CryptoPP::Integer TempResult = TempMA.Exponentiate(TempRepresentative, TempExponent);
	ReturnValue = CryptoPPToMBG(TempResult);
	//MrBigInt::PowM(MessageRepresentative, PublicKey.Exponent, PublicKey.Modolu, ReturnValue);
	return(ReturnValue);
}
std::string TLSHandler::I2OSP(MrBigInt NumberToConvert, uint64_t LengthOfString)
{
	std::string ReturnValue = "";
	MrBigInt MaxValueOfInteger(0);

	MrBigInt::Pow(LengthOfString, LengthOfString, MaxValueOfInteger);
	//std::cout << MaxValueOfInteger.GetString() << std::endl;
	if (NumberToConvert > MaxValueOfInteger)
	{
		std::cout << "Number to conver " << NumberToConvert.GetString() << std::endl;
		std::cout << "Max number " << MaxValueOfInteger.GetString() << std::endl;
		std::cout << "LengthOfString " << LengthOfString << std::endl;
		assert(false);
		return("ERROR");
	}
	else
	{
		//returnar en string som sett att grejen är big endian, vi appendar dem sista grejerna och till slut så reversar vi den
		std::string NumberAsString = NumberToConvert.GetBigEndianArray();
		for (size_t i = 0; i < LengthOfString-NumberAsString.size(); i++)
		{
			ReturnValue += char(0);
		}
		ReturnValue += NumberAsString;

		return(ReturnValue);
	}
}
std::string TLSHandler::RSAES_PKCS1_V1_5_ENCRYPT(TLSServerPublickeyInfo& RSAInfo, std::string& DataToEncrypt)
{
	uint16_t RSAPrimeSize = RSAInfo.ServerKeyData.size() - 15;
	if (DataToEncrypt.size() > RSAPrimeSize-11)
	{
		assert(false);
	}
	std::string EM = "";
	EM += char(0x00);
	EM += char(0x02);
	uint64_t PSLength = RSAPrimeSize - 3 - DataToEncrypt.size();
	srand(time(0));
	for (int i = 0; i < PSLength; i++)
	{
		char RandomChar = rand();
		if (RandomChar!= 0)
		{
			EM += RandomChar;
		}
		else
		{
			EM += char(1);
		}
	}
	EM += char(0x00);
	EM += DataToEncrypt;
	MrBigInt MessageRepresentative = OS2IP(EM.c_str(), RSAPrimeSize);
	MrBigInt CiphertextRepresentative = RSAEP(RSAInfo,MessageRepresentative);
	std::string CipherText = I2OSP(CiphertextRepresentative, RSAPrimeSize);
	return(CipherText);
}
void TLSHandler::SendClientKeyExchange(TLSServerPublickeyInfo& Data,MBSockets::ConnectSocket* SocketToConnect)
{
	//vi utgår alltid ifrån att det är rsa med specifika algoritmer
	uint16_t RSAPrimeSize = Data.ServerKeyData.size() - 15;
	uint8_t* DataToSend = static_cast<uint8_t*>(malloc(5 + 4 +2+ RSAPrimeSize));
	TLS1_2::NetWorkDataHandler ArrayFiller(DataToSend);
	ArrayFiller << uint8_t(TLS1_2::handshake);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint16_t(4+ RSAPrimeSize+2);
	ArrayFiller << uint8_t(TLS1_2::client_key_exchange);
	ArrayFiller << TLS1_2::uint24(RSAPrimeSize+2);
	ArrayFiller << uint16_t(RSAPrimeSize);
	uint8_t PreMasterSecret[48];
	FillArrayWithRandomBytes(PreMasterSecret, 48);
	//OBSS!"!"!"!"!"!!  dem första bytesen är variationen vi ville började att skicka, dvs jag hard codar 3 3
	PreMasterSecret[0] = 3;
	PreMasterSecret[1] = 3;
	for (size_t i = 0; i < 48; i++)
	{
		ConnectionParameters.PreMasterSecret[i] = PreMasterSecret[i];
	}
	std::string PremasterSecretString = std::string(reinterpret_cast<char*>(PreMasterSecret), 48);
	std::string EncryptedPremasterSecret = RSAES_PKCS1_V1_5_ENCRYPT(Data, PremasterSecretString);
	for (size_t i = 0; i < RSAPrimeSize; i++)
	{
		DataToSend[9 +2+ i] = EncryptedPremasterSecret[i];
	}
	ConnectionParameters.AllHandshakeMessages.push_back(std::string(reinterpret_cast<char*>(DataToSend), 5 + 4 + 2 + RSAPrimeSize));
	SocketToConnect->SendData(reinterpret_cast<char*>(DataToSend), 5 + 4+2 + RSAPrimeSize);
	free(DataToSend);
}





//TLS1_2::SecurityParameters ConnectionParameters;





std::string TLSHandler::XORedString(std::string String1, std::string String2)
{
	std::string Returnvalue = "";
	std::string MinString  = "";
	std::string MaxString  = "";
	if (String1.size() > String2.size())
	{
		MinString = String2;
		MaxString = String1;
	}
	else
	{
		MinString = String1;
		MaxString = String2;
	}
	for (size_t i = 0; i < MaxString.size(); i++)
	{
		if (i < MinString.size())
		{
			Returnvalue += char(MaxString[i] ^ MinString[i]);
		}
		else
		{
			Returnvalue += char(MaxString[i] ^ char(0));
		}
	}
	return(Returnvalue);
}
unsigned int MacIndexUsed = 0;
std::string TLSHandler::HMAC(std::string Secret, std::string Seed)
{
	uint64_t BlockLength = ConnectionParameters.HashAlgorithm.GetBlockSize(); //sha256
	uint64_t OutputSize = ConnectionParameters.HashAlgorithm.GetDigestSize(); //sha256
	std::string AppendedSecret = Secret;
	assert(Secret.size() <= BlockLength);
	if (!(Secret.size() <= BlockLength))
	{
		std::cout << "Problem in hmac " << std::endl;
	}
	std::string Ipad = "";
	std::string Opad = "";
	for (size_t i = 0; i < BlockLength; i++)
	{
		Ipad += char(0x36);
		Opad += char(0x5c);
	}
	for (int i = 0; i < BlockLength - Secret.size(); i++)
	{
		AppendedSecret += char(0x00);
	}
	std::string LeftMostAppend = XORedString(AppendedSecret, Opad);
	std::string InsideOfFirstHash = XORedString(AppendedSecret, Ipad) + Seed;

	std::string FirstHashData = ConnectionParameters.HashAlgorithm.Hash(InsideOfFirstHash);
	//std::ofstream DebugHash("DebugHash", std::ios::out | std::ios::binary);
	//DebugHash << InsideOfFirstHash;
	//std::cout << HexEncodeString(FirstHashData)<<std::endl;

	std::string FinalHashData = ConnectionParameters.HashAlgorithm.Hash(LeftMostAppend + FirstHashData);
	//std::ofstream DebugFile("DebugHmac" + std::to_string(MacIndexUsed),std::ios::out|std::ios::binary);
	//DebugFile << Seed;
	//std::cout << "Hmac " << MacIndexUsed << " Used: " << ReplaceAll(HexEncodeString(Secret), " ", "") << std::endl;
	//std::cout << "Hmac " << MacIndexUsed << " Result: " << ReplaceAll(HexEncodeString(FinalHashData), " ", "") << std::endl;
	//MacIndexUsed += 1;
	return(FinalHashData);
}
std::string TLSHandler::P_Hash(std::string Secret, std::string Seed,uint64_t AmountOfData)
{
	std::string TotalDigestedData = "";
	uint64_t OutputSizeOfHashFunction = ConnectionParameters.HashAlgorithm.GetDigestSize();
	std::string PreviousA(OutputSizeOfHashFunction, 0x00);
	PreviousA = HMAC(Secret, Seed);
	for (int i = 0; i < ceil(AmountOfData/float(OutputSizeOfHashFunction)); i++)
	{
		TotalDigestedData += HMAC(Secret, PreviousA+Seed);
		PreviousA = HMAC(Secret, PreviousA);	
	}
	return(TotalDigestedData.substr(0, AmountOfData));
}
std::string TLSHandler::PRF(std::string Secret, std::string Label, std::string Seed,uint64_t AmountOfData)
{
	//använder alltid sha256
	MBCrypto::HashObject PreviousHashObject = ConnectionParameters.HashAlgorithm;
	ConnectionParameters.HashAlgorithm = MBCrypto::HashObject(MBCrypto::HashFunction::SHA256);
	std::string ReturnValue = P_Hash(Secret, Label + Seed, AmountOfData);
	ConnectionParameters.HashAlgorithm = PreviousHashObject;
	return(ReturnValue);
	
}
TLSHandler::TLSHandler()
{
	//AssociatedSocket = SocketToAssociateWith;
}
MBError TLSHandler::EstablishTLSConnection(MBSockets::ConnectSocket* SocketToConnect,std::string const& HostName)
{
	return(InitiateHandShake(SocketToConnect, HostName));
}
TLS1_2::CipherSuiteData TLSHandler::p_GetCipherSuiteData(TLS1_2::CipherSuite CipherToEvaluate)
{
	TLS1_2::CipherSuiteData ReturnValue;
	if (CipherToEvaluate.Del1 == 0 && CipherToEvaluate.Del2 == 0x2f)
	{
		ReturnValue = { TLS1_2::KeyExchangeMethod::RSA,TLS1_2::CertificateAuthenticationMethod::RSA,16,0,0,TLS1_2::SymmetricEncryptionCipher::AES,TLS1_2::SymmetricCipherMode::CBC,MBCrypto::HashFunction::SHA1};
	}
	else if (CipherToEvaluate.Del1 == 0xc0 && CipherToEvaluate.Del2 == 0x2f)
	{
		ReturnValue = { TLS1_2::KeyExchangeMethod::ECDHE,TLS1_2::CertificateAuthenticationMethod::RSA,16,8,4,TLS1_2::SymmetricEncryptionCipher::AES,TLS1_2::SymmetricCipherMode::GCM,MBCrypto::HashFunction::SHA256 };
	}
	return(ReturnValue);
}
std::vector<TLS1_2::CipherSuite> TLSHandler::p_GetSupportedCipherSuites()
{
	std::vector<TLS1_2::CipherSuite> ReturnValue = { {0x00,0x3c},{0xc0,0x02f} };
	//std::vector<TLS1_2::CipherSuite> ReturnValue = { {0x00,0x3c} };
	return(ReturnValue);
}

std::vector<TLS1_2::Extension> TLSHandler::p_GetClientExtensions()
{
	std::vector<TLS1_2::Extension> ReturnValue = {};
	TLS1_2::Extension SignatureAlgoritmExtension;
	SignatureAlgoritmExtension.ExtensionType = TLS1_2::signature_algoritms;//{ TLS1_2::signature_algoritms,{TLS1_2::sha256,TLS1_2::rsa}};
	TLS1_2::SignatureAndHashAlgoritm ExtensionDataToUse;
	ExtensionDataToUse.Hash = TLS1_2::sha256;
	ExtensionDataToUse.Signature = TLS1_2::rsa;
	SignatureAlgoritmExtension.ExtensionData = { 0x00,0x02,ExtensionDataToUse.Hash,ExtensionDataToUse.Signature };
	TLS1_2::Extension ServerNameExtension = TLS1_2::GenerateServernameExtension(this->ConnectionParameters.HostToConnectToDomainName);

	ReturnValue.push_back(SignatureAlgoritmExtension);
	ReturnValue.push_back(ServerNameExtension);
	
	std::vector<TLS1_2::Extension> DHEExtensions = p_GenerateDHEExtensions();
	for (size_t i = 0; i < DHEExtensions.size(); i++)
	{
		ReturnValue.push_back(DHEExtensions[i]);
	}
	return(ReturnValue);
}
std::string TLSHandler::GenerateTLS1_2ClientHello(MBSockets::ConnectSocket* SocketToConnect)
{
	//vi börjar med att skicka ett client hello
	TLS1_2::TLS1_2HelloClientStruct HelloToSend;
	HelloToSend.ProtocolVers = { 3,3 };
	FillArrayWithRandomBytes(HelloToSend.RandomStruct.RandomBytes, 32);
	//passar även på att fylla våra parameters med våran random data
	for (int i = 0; i < 32; i++)
	{
		ConnectionParameters.client_random[i] = HelloToSend.RandomStruct.RandomBytes[i];
	}
	HelloToSend.SessionId = {};
	//TLS_RSA_WITH_NULL_SHA256
	HelloToSend.CipherSuites = p_GetSupportedCipherSuites();
	HelloToSend.CompressionMethods = { 0 };
	HelloToSend.OptionalExtensions = p_GetClientExtensions();
	//nu har vi initialisat allt data vi behöver för hellosend grejen, nu så ska vi skicka detta record till servern
	TLS1_2::HandShake HandShakeData;
	HandShakeData.MessageType = TLS1_2::client_hello;
	int LengthOfHandshakeData = TLS1_2::SizeOfProccessedHelloClientStruct(HelloToSend);
	int TotalDataLength = 5 + 4 + LengthOfHandshakeData;
	uint8_t* MessageData = new uint8_t[5 + 4 + LengthOfHandshakeData];
	TLS1_2::NetWorkDataHandler ArrayFiller(MessageData);
	ArrayFiller << uint8_t(TLS1_2::handshake);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint16_t(TotalDataLength - 5);
	ArrayFiller << uint8_t(TLS1_2::client_hello);
	ArrayFiller << TLS1_2::uint24(TotalDataLength - (5 + 4));
	TLS1_2::FillArrayWithHelloClientData(&MessageData[5 + 4], HelloToSend);
	std::string ReturnValue = std::string(reinterpret_cast<char*>(MessageData), TotalDataLength);
	delete[] MessageData;
	ConnectionParameters.AllHandshakeMessages.push_back(ReturnValue);
	return(ReturnValue);
}
std::vector<std::string> TLSHandler::GetCertificateList(std::string& AllCertificateData)
{
	assert(false);
	std::vector<std::string> ReturnValue = std::vector<std::string>(0);
	int Offset = 0;
	TLS1_2::NetWorkDataHandler DataReader(reinterpret_cast<const uint8_t*>(AllCertificateData.c_str()));
	while (DataReader.GetPosition() < AllCertificateData.size())
	{
		uint16_t LengthOfData = DataReader.Extract16();
		ReturnValue.push_back(AllCertificateData.substr(Offset, LengthOfData));
	}
	return(ReturnValue);
}
void TLSHandler::SendChangeCipherMessage(MBSockets::ConnectSocket* SocketToConnect)
{
	uint8_t* Data = (uint8_t*)malloc(5 + 1);
	TLS1_2::NetWorkDataHandler ArrayFiller(Data);
	ArrayFiller << uint8_t(TLS1_2::change_cipher_spec);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint8_t(3);
	ArrayFiller << uint8_t(0);
	ArrayFiller << uint8_t(1);
	ArrayFiller << uint8_t(1);
	SocketToConnect->SendData(reinterpret_cast<char*>(Data), 6);
	free(Data);
	if (!ConnectionParameters.IsHost)
	{
		ConnectionParameters.ClientSequenceNumber = 0;
	}
	else
	{
		//egentligen lite osäker på denna
		ConnectionParameters.ServerSequenceNumber = 0;
	}
}
void TLSHandler::GenerateKeys()
{
	uint8_t EncryptKeySize = ConnectionParameters.CipherSuiteInfo.CipherKeySize;
	uint8_t MacKeyLength = ConnectionParameters.HashAlgorithm.GetDigestSize();
	if (ConnectionParameters.CipherSuiteInfo.CipherMode == TLS1_2::SymmetricCipherMode::GCM)
	{
		MacKeyLength = 0;
	}
	uint8_t WriteIVLength = ConnectionParameters.CipherSuiteInfo.FixedIVLength;
	uint64_t DataNeeded = 2*EncryptKeySize+2*MacKeyLength+2*WriteIVLength;
	std::string MasterSecret = std::string(reinterpret_cast<char*>(ConnectionParameters.master_secret), 48);
	std::string ServerRandom = std::string(reinterpret_cast<char*>(ConnectionParameters.server_random), 32);
	std::string ClientRandom = std::string(reinterpret_cast<char*>(ConnectionParameters.client_random), 32);
	
	//std::cout << "Client radnom used: " << HexEncodeString(ClientRandom) << std::endl;
	//std::cout << "Server random used: " << HexEncodeString(ServerRandom) << std::endl;
	//std::cout << "Master secret used: " << HexEncodeString(MasterSecret) << std::endl;
	
	std::string GeneratedKeyData = PRF(MasterSecret, "key expansion", ServerRandom + ClientRandom, DataNeeded);
	
	//std::cout << "Generated PRF: " << HexEncodeString(GeneratedKeyData) << std::endl;
	
	ConnectionParameters.client_write_MAC_Key = GeneratedKeyData.substr(MacKeyLength *0, MacKeyLength);
	ConnectionParameters.server_write_MAC_Key = GeneratedKeyData.substr(MacKeyLength *1, MacKeyLength);
	ConnectionParameters.client_write_Key = GeneratedKeyData.substr((MacKeyLength*2)+(EncryptKeySize *0), EncryptKeySize);
	ConnectionParameters.server_write_Key = GeneratedKeyData.substr((MacKeyLength * 2)+(EncryptKeySize *1), EncryptKeySize);
	ConnectionParameters.client_write_IV = GeneratedKeyData.substr((EncryptKeySize *2)+(MacKeyLength*2), WriteIVLength);
	ConnectionParameters.server_write_IV = GeneratedKeyData.substr((EncryptKeySize *2)+(MacKeyLength*2)+WriteIVLength, WriteIVLength);

	//std::cout << "Client_write_MAC_Key: " << HexEncodeString(ConnectionParameters.client_write_MAC_Key) << std::endl;
	//std::cout << "Server_write_MAC_Key: " << HexEncodeString(ConnectionParameters.server_write_MAC_Key) << std::endl;
	//std::cout << "Client_write_Key: " << HexEncodeString(ConnectionParameters.client_write_Key) << std::endl;
	//std::cout << "Server_write_Key: " << HexEncodeString(ConnectionParameters.server_write_Key) << std::endl;
	//std::cout << "Client Write IV: " << MBUtility::ReplaceAll(MBUtility::HexEncodeString(ConnectionParameters.client_write_IV), " ", "") << std::endl;
	//std::cout << "Server Write IV: " << MBUtility::ReplaceAll(MBUtility::HexEncodeString(ConnectionParameters.server_write_IV), " ", "") << std::endl;
}
std::string TLSHandler::p_GetCBCEncryptedRecord(TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt)
{
	std::string TotalRecordData = "";
	unsigned char IV[16];
	FillArrayWithRandomBytes(IV, 16);
	std::string SequenceNumber = std::string(8, 0);
	uint64_t TempSequenceNumber;
	if (!ConnectionParameters.IsHost)
	{
		TempSequenceNumber = ConnectionParameters.ClientSequenceNumber;
	}
	else
	{
		TempSequenceNumber = ConnectionParameters.ServerSequenceNumber;
	}
	for (size_t i = 0; i < 8; i++)
	{
		SequenceNumber[7 - i] = TempSequenceNumber % 256;
		TempSequenceNumber = TempSequenceNumber >> 8;
	}
	std::string LengthString = std::string(2, 0);
	uint16_t TempLengthNumber = RecordToEncrypt.Length;
	for (int i = 0; i < 2; i++)
	{
		LengthString[1 - i] = TempLengthNumber % 256;
		TempLengthNumber = TempLengthNumber >> 8;
	}
	std::string LengthOfRecord = "";
	LengthOfRecord += RecordToEncrypt.Length >> 8;
	LengthOfRecord += RecordToEncrypt.Length % 256;
	std::string MACStringInput = SequenceNumber + char(RecordToEncrypt.Type) + char(RecordToEncrypt.Protocol.Major) + char(RecordToEncrypt.Protocol.Minor) + LengthString + RecordToEncrypt.Data;
	std::string MAC;
	std::string WriteKey;
	if (!ConnectionParameters.IsHost)
	{
		MAC = HMAC(ConnectionParameters.client_write_MAC_Key, MACStringInput);
		WriteKey = ConnectionParameters.client_write_Key;
	}
	else
	{
		MAC = HMAC(ConnectionParameters.server_write_MAC_Key, MACStringInput);
		WriteKey = ConnectionParameters.server_write_Key;
	}
	std::string DataToEncrypt = "";
	DataToEncrypt += RecordToEncrypt.Data;
	DataToEncrypt += MAC;
	std::string Padding = "";
	uint8_t PaddingLength = (16 - ((DataToEncrypt.size() + 1) % 16)) % 16;
	for (int i = 0; i < PaddingLength; i++)
	{
		Padding += char(PaddingLength);
	}
	DataToEncrypt += Padding;
	DataToEncrypt += char(PaddingLength);
	
	//std::string EncryptedData(DataToEncrypt.size(), 0);
	//plusaes::encrypt_cbc((unsigned char*)DataToEncrypt.data(), DataToEncrypt.size(), (unsigned char*)WriteKey.data(), WriteKey.size(), &IV, (unsigned char*)EncryptedData.data(), EncryptedData.size(), false);
	
	std::string EncryptedData = "";
	MBCrypto::BlockCipher_CBC_Handler Encrypter(MBCrypto::BlockCipher::AES, MBCrypto::CBC_PaddingScheme::Null);
	MBError EncryptionError = true;
	EncryptedData = Encrypter.EncryptData(DataToEncrypt.data(), DataToEncrypt.size(), WriteKey.data(), WriteKey.size(), IV, 16, &EncryptionError);
	assert(EncryptionError);
	
	TotalRecordData += char(RecordToEncrypt.Type);
	TotalRecordData += char(RecordToEncrypt.Protocol.Major);
	TotalRecordData += char(RecordToEncrypt.Protocol.Minor);
	std::string IVString = std::string(reinterpret_cast<char*>(IV), 16);
	uint16_t LengthOfCipherBlock = EncryptedData.size() + IVString.size();
	TotalRecordData += char(LengthOfCipherBlock >> 8);
	TotalRecordData += char(LengthOfCipherBlock % 256);
	TotalRecordData += IVString;
	TotalRecordData += std::move(EncryptedData);
	return(TotalRecordData);
} 
std::string TLSHandler::p_GetExplicitNonce()
{
	//m_NonceSequenceNumber += 1;
	std::string ReturnValue = "";
	//ReturnValue += "UwU\xbe\xef";
	size_t SequenceNumberToUse = 0;
	if (ConnectionParameters.IsHost)
	{
		SequenceNumberToUse = ConnectionParameters.ServerSequenceNumber;
	}
	else
	{
		SequenceNumberToUse = ConnectionParameters.ClientSequenceNumber;
	}
	for (size_t i = 0; i < ConnectionParameters.CipherSuiteInfo.NonceSize; i++)
	{
		ReturnValue += char(SequenceNumberToUse >> ((3 - i) * 8));
	}
	return(ReturnValue);
}
std::string TLSHandler::p_GetGCMEncryptedRecord(TLS1_2::SecurityParameters const& SecurityParams, TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt)
{
	if (ConnectionParameters.CipherSuiteInfo.EncryptionCipher == TLS1_2::SymmetricEncryptionCipher::AES)
	{
		return(p_Get_AES_GCM_EncryptedRecord(SecurityParams, RecordToEncrypt));
	}
}
std::string TLSHandler::p_GetAEADAdditionalData(TLS1_2::SecurityParameters const& ConnectionParams, std::string const& RecordData,bool Encrypt)
{
	if (ConnectionParams.CipherSuiteInfo.EncryptionCipher != TLS1_2::SymmetricEncryptionCipher::AES)
	{
		assert(false);
	}
	std::string ReturnValue = "";
	std::string SequenceNumber = std::string(8, 0);
	size_t ExplicitNounceLength = ConnectionParams.CipherSuiteInfo.NonceSize;
	size_t TagSize = 16;
	uintmax_t TempSequenceNumber;
	if (!ConnectionParams.IsHost)
	{
		if (Encrypt)
		{
			TempSequenceNumber = ConnectionParams.ClientSequenceNumber;
		}
		else
		{
			TempSequenceNumber = ConnectionParams.ServerSequenceNumber;
		}
	}
	else
	{
		if (Encrypt)
		{
			TempSequenceNumber = ConnectionParams.ServerSequenceNumber;
		}
		else
		{
			TempSequenceNumber = ConnectionParams.ClientSequenceNumber;
		}
	}
	for (size_t i = 0; i < 8; i++)
	{
		SequenceNumber[7 - i] = TempSequenceNumber % 256;
		TempSequenceNumber = TempSequenceNumber >> 8;
	}
	ReturnValue = SequenceNumber + RecordData.substr(0,3);
	if (Encrypt)
	{
		ReturnValue += RecordData.substr(3, 2);
	}
	else
	{
		size_t DecryptedRecordLength = 0;
		DecryptedRecordLength +=uint8_t(RecordData[3]) << 8;
		DecryptedRecordLength += uint8_t(RecordData[4]);
		DecryptedRecordLength -= TagSize;
		DecryptedRecordLength -= ExplicitNounceLength;
		ReturnValue += DecryptedRecordLength >> 8;
		ReturnValue += DecryptedRecordLength%256;
	}
	size_t RecordSize = 0;

	return(ReturnValue);
}
std::string TLSHandler::p_GetAEADAdditionalData(TLS1_2::SecurityParameters const& ConnectionParams, TLS1_2::TLS1_2GenericRecord const& AssociatedRecord,bool Encrypts)
{
	if (ConnectionParams.CipherSuiteInfo.EncryptionCipher != TLS1_2::SymmetricEncryptionCipher::AES)
	{
		assert(false);
	}
	std::string ReturnValue = "";
	std::string SequenceNumber = std::string(8, 0);
	size_t ExplicitNounceLength = ConnectionParams.CipherSuiteInfo.NonceSize;
	size_t TagSize = 16;
	uintmax_t TempSequenceNumber;
	if (!ConnectionParams.IsHost)
	{
		TempSequenceNumber = ConnectionParams.ClientSequenceNumber;
	}
	else
	{
		TempSequenceNumber = ConnectionParams.ServerSequenceNumber;
	}
	for (size_t i = 0; i < 8; i++)
	{
		SequenceNumber[7 - i] = TempSequenceNumber % 256;
		TempSequenceNumber = TempSequenceNumber >> 8;
	}
	std::string RecordLength = "";
	if (Encrypts)
	{
		RecordLength += char(AssociatedRecord.Data.size() >> 8);
		RecordLength += char(AssociatedRecord.Data.size() % 256);
	}
	else
	{
		RecordLength += char((AssociatedRecord.Data.size()-TagSize-ExplicitNounceLength) >> 8);
		RecordLength += char((AssociatedRecord.Data.size()-TagSize - ExplicitNounceLength) % 256);
	}
	ReturnValue = SequenceNumber + char(AssociatedRecord.Type) + char(AssociatedRecord.Protocol.Major) + char(AssociatedRecord.Protocol.Minor) + RecordLength;
	return(ReturnValue);
}
std::string TLSHandler::p_Get_AES_GCM_EncryptedRecord(TLS1_2::SecurityParameters const& SecurityParams, TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt)
{
	std::string ReturnValue = "";
	ReturnValue += char(RecordToEncrypt.Type);
	ReturnValue += char(RecordToEncrypt.Protocol.Major);
	ReturnValue += char(RecordToEncrypt.Protocol.Minor);
	std::string EncryptedData = "";
	std::string ImplicitNonce = "";
	if (ConnectionParameters.IsHost)
	{
		ImplicitNonce = ConnectionParameters.server_write_IV;
	}
	else
	{
		ImplicitNonce = ConnectionParameters.client_write_IV;
	}
	std::string ExplicitNonce = p_GetExplicitNonce();
	std::string NonceToUse = ImplicitNonce+ ExplicitNonce;
	std::string AuthenticatedData = p_GetAEADAdditionalData(SecurityParams,RecordToEncrypt,true);
	size_t TagSize = 16;

	std::string WriteKey = "";
	if (ConnectionParameters.IsHost)
	{
		WriteKey = ConnectionParameters.server_write_Key;
	}
	else
	{
		WriteKey = ConnectionParameters.client_write_Key;
	}

	//CryptoPP::GCM<CryptoPP::AES>::Encryption Encryptor;
	//Encryptor.SetKeyWithIV((CryptoPP::byte*)WriteKey.data(), WriteKey.size(), (CryptoPP::byte*)NonceToUse.data(), NonceToUse.size());
	//CryptoPP::AuthenticatedEncryptionFilter AEADFilter(Encryptor, new CryptoPP::StringSink(EncryptedData), false, TagSize);
	//AEADFilter.ChannelPut(CryptoPP::AAD_CHANNEL,(CryptoPP::byte*) AuthenticatedData.data(), AuthenticatedData.size());
	//AEADFilter.ChannelMessageEnd(CryptoPP::AAD_CHANNEL);
	//AEADFilter.ChannelPut(CryptoPP::DEFAULT_CHANNEL,(CryptoPP::byte*) RecordToEncrypt.Data.data(), RecordToEncrypt.Data.size());
	//AEADFilter.ChannelMessageEnd(CryptoPP::DEFAULT_CHANNEL);

	MBError EncryptionError = true;
	MBCrypto::BlockCipher_GCM_Handler Encryptor(MBCrypto::BlockCipher::AES);
	EncryptedData = Encryptor.EncryptData(RecordToEncrypt.Data.data(), RecordToEncrypt.Data.size(), WriteKey.data(), WriteKey.size(), NonceToUse.data(), NonceToUse.size(),
		AuthenticatedData.data(), AuthenticatedData.size(), &EncryptionError);
	assert(EncryptionError);


	size_t TotalMessageSize = ExplicitNonce.size() + EncryptedData.size();
	ReturnValue += char(TotalMessageSize >> 8);
	ReturnValue += char(TotalMessageSize %256);
	ReturnValue += ExplicitNonce;
	ReturnValue += EncryptedData;
	//std::string DEBUG_TEST = p_DecryptRecord(ReturnValue);
	return(ReturnValue);
}
std::string TLSHandler::GetEncryptedRecord(TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt)
{
	if (ConnectionParameters.CipherSuiteInfo.CipherMode == TLS1_2::SymmetricCipherMode::CBC)
	{
		return(p_GetCBCEncryptedRecord(RecordToEncrypt));
	}
	else if (ConnectionParameters.CipherSuiteInfo.CipherMode == TLS1_2::SymmetricCipherMode::GCM)
	{
		return(p_GetGCMEncryptedRecord(ConnectionParameters, RecordToEncrypt));
	}
}
std::string TLSHandler::GenerateVerifyDataMessage()
{
	std::string ConcattenatedHandshakeData = "";
	for (size_t i = 0; i < ConnectionParameters.AllHandshakeMessages.size(); i++)
	{
		ConcattenatedHandshakeData += ConnectionParameters.AllHandshakeMessages[i].substr(5);
	}
	std::string HashedHandshakeData = Sha256HashObject.Hash(ConcattenatedHandshakeData);
	std::string VerifyData;
	if (ConnectionParameters.IsHost)
	{
		VerifyData = PRF(std::string(reinterpret_cast<char*>(ConnectionParameters.master_secret), 48), "server finished", HashedHandshakeData, 12);
	}
	else
	{
		VerifyData = PRF(std::string(reinterpret_cast<char*>(ConnectionParameters.master_secret), 48), "client finished", HashedHandshakeData, 12);
	}
	TLS1_2::TLS1_2GenericRecord Record;
	Record.Type = TLS1_2::handshake;
	Record.Protocol = { 3,3 };
	Record.Length = 4+12;
	std::string HandshakeData = "";
	HandshakeData += char(TLS1_2::finished);
	HandshakeData += char(0);
	HandshakeData += char(0);
	HandshakeData += char(12);
	HandshakeData += VerifyData;
	Record.Data = HandshakeData;
	//string som representerar all data i denna
	std::string TotalUnencryptedRecordData = "";
	TotalUnencryptedRecordData += char(Record.Type);
	TotalUnencryptedRecordData += char(3);
	TotalUnencryptedRecordData += char(3);
	TotalUnencryptedRecordData += char(char(16)>>8);
	TotalUnencryptedRecordData += char(char(16)%256);
	TotalUnencryptedRecordData += HandshakeData;
	ConnectionParameters.AllHandshakeMessages.push_back(TotalUnencryptedRecordData);
	std::string ReturnValue = GetEncryptedRecord(Record);
	return(ReturnValue);
}
TLSServerPublickeyInfo GetServerPublicKey(std::string& ServerCertificateData)
{
	uint64_t Offset = 0;
	TLSServerPublickeyInfo ReturnValue;
	uint64_t AbsoluteOffsetToNextCertficate = 0;
	ASN1Extracter Parser(reinterpret_cast<const uint8_t*>(&ServerCertificateData.c_str()[Offset]));
	ASN1TagValue  FirstTag = Parser.ExtractTagData();
	if (FirstTag.TagType != ASN1PrimitiveTagNumber::Sequence)
	{
		//yikers
		std::cout << uint16_t(FirstTag.TagType) << std::endl;
		assert(false);
	}
	/*
	   Certificate  ::=  SEQUENCE  {
		tbsCertificate       TBSCertificate,
		signatureAlgorithm   AlgorithmIdentifier,
		signatureValue       BIT STRING  }

   TBSCertificate  ::=  SEQUENCE  {
		version         [0]  EXPLICIT Version DEFAULT v1,
		serialNumber         CertificateSerialNumber,
		signature            AlgorithmIdentifier,
		issuer               Name,
		validity             Validity,
		subject              Name,
		subjectPublicKeyInfo SubjectPublicKeyInfo,
		issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
		subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
		extensions      [3]  EXPLICIT Extensions OPTIONAL
		}

		SubjectPublicKeyInfo  ::=  SEQUENCE
		{
			algorithm            AlgorithmIdentifier,
			subjectPublicKey     BIT STRING
		}
	*/
	uint64_t LengthOfCurrentCertificate = Parser.ExtractLengthOfType();
	//skippar till signature algorithm fielden

	//först entrar vi tbscertificate fältet
	Parser.ExtractTagData();
	Parser.ExtractLengthOfType();

	//eftersom att den har ett default value kollar vi först och främst om den finns, gör den inte det så vet vi att vi skippar rätt grej i alla fall
	ASN1TagValue VersionTag = Parser.ExtractTagData();
	if (VersionTag.AlternateTagValue == 0)
	{
		//det är faktiskt en tag vi extractar och inte serialnumber

		//vi skippar helt enkelt typen vi börjar extracta, som vi nu vet är serialnumber
		uint64_t LengthOfTypeData = Parser.ExtractLengthOfType();
		Parser.SetOffset(Parser.GetOffset() + LengthOfTypeData);
	}
	else
	{
		uint64_t LengthOfVersionData = Parser.ExtractLengthOfType();
		Parser.SetOffset(Parser.GetOffset() + LengthOfVersionData);
		Parser.SkipToNextField();//Skippar SerialNumber
	}
	Parser.SkipToNextField();//Skippar Signature
	Parser.SkipToNextField();//Skippar Issuer
	Parser.SkipToNextField();//Skippar Validity
	Parser.SkipToNextField();//Skippar issuer
	Parser.SkipToNextField();//Skippar skippar name
	//nu vill vi entra subjectpublickeyinfo fältet
	ASN1TagValue SubjectPublicKeyInfoTag = Parser.ExtractTagData();
	uint64_t LengthOfKeyInfo = Parser.ExtractLengthOfType();
	Parser.SkipToNextField();//Skippar algorithm identifer i subjcet public key info
	//nu jävlar, nu är vi äntligen i själva RSA keyn som vi vill ha
	ASN1TagValue BitStringTag = Parser.ExtractTagData();
	uint64_t LenthOfBitString = Parser.ExtractLengthOfType();
	ReturnValue.ServerKeyData = "";
	for (size_t i = 0; i < LenthOfBitString; i++)
	{
		ReturnValue.ServerKeyData += Parser.ExtractByte();
	}
	//Relevant om den första certifikaten inte inehåller public keyn
	//AbsoluteOffsetToNextCertficate = Offset + Parser.GetOffset() + LengthOfCurrentCertificate;
	//nu tar vi och ser om vi faktiskt kan ta och hitta lite spicy public key data
	//Offset = AbsoluteOffsetToNextCertficate;
	//nu koller vi längden till nästa value
	return(ReturnValue);
}
void TLSHandler::p_UpdateConnectionParametersAfterServerHello(TLS1_2::SecurityParameters& ConnectionParameters, TLS1_2::TLS1_2ServerHelloStruct const& ServerHelloStruct)
{
	ConnectionParameters.NegotiatedProtocolVersion = ServerHelloStruct.Protocol;
	ConnectionParameters.CipherSuiteInfo = p_GetCipherSuiteData(ServerHelloStruct.CipherSuiteToUse);
	ConnectionParameters.HashAlgorithm = MBCrypto::HashObject(ConnectionParameters.CipherSuiteInfo.HashFunction);
	//if (ServerHelloStruct.CipherSuiteToUse.Del1 == 0x00 && ServerHelloStruct.CipherSuiteToUse.Del2 == 0x3c)
	//{
	//	ConnectionParameters.HashAlgorithm = TLSHashObject(MBSha256, 32, 64);
	//}
	//else
	//{
	//	ConnectionParameters.CipherSuiteInfo = p_GetCipherSuiteData(ServerHelloStruct.CipherSuiteToUse);
	//}
	for (int i = 0; i < 32; i++)
	{
		ConnectionParameters.server_random[i] = ServerHelloStruct.RandomStruct.RandomBytes[i];
	}
}
std::vector<std::string> TLSHandler::p_GetServerHelloResponseRecords(MBSockets::ConnectSocket* SocketToUse)
{
	std::vector<std::string> ReturnValue = GetNextPlaintextRecords(SocketToUse,3,m_MaxBytesInMemory);
	if (ReturnValue[ReturnValue.size() - 1].size() != (5 + 4))
	{
		std::vector<std::string> Hellostruct = GetNextPlaintextRecords(SocketToUse,1,m_MaxBytesInMemory);
		for (size_t i = 0; i < Hellostruct.size(); i++)
		{
			ReturnValue.push_back(Hellostruct[i]);
		}
	}
	for (int i = 0; i < ReturnValue.size(); i++)
	{
		ConnectionParameters.AllHandshakeMessages.push_back(ReturnValue[i]);
	}
	return(ReturnValue);
}
std::string TLSHandler::p_GenerateClientECDHEKeyExchangeMessage(TLS1_2::SecurityParameters const& ConnectionParams, MBCrypto::ECDHEPrivatePublicKeyPair& OutKeypair, TLS1_2::ECDHEServerKeyExchange const& ServerExchange)
{
	std::string ReturnValue = "";
	TLS1_2::TLS1_2GenericRecord RecordToSend;
	RecordToSend.Protocol = ConnectionParams.NegotiatedProtocolVersion;
	RecordToSend.Type = TLS1_2::handshake;
	TLS1_2::TLS1_2HanshakeMessage HandshakeMessage;
	HandshakeMessage.Type = TLS1_2::client_key_exchange;
	MBCrypto::ElipticCurve CurveToUse(ServerExchange.NamedCurve);
	OutKeypair = CurveToUse.GetPrivatePublicKeypair();
	std::string PointData = CurveToUse.EncodeECP_ANS1x92Encoding(OutKeypair.PublicPoint);
	HandshakeMessage.MessageData += char(PointData.size() % 256);
	HandshakeMessage.MessageData += PointData;
 	RecordToSend.Data = TLS1_2::GetRecordString(HandshakeMessage);
	ReturnValue = TLS1_2::GetRecordString(RecordToSend);
	return(ReturnValue);
}
void TLSHandler::p_ECDHECalculatePremasterSecret(TLS1_2::SecurityParameters& ConnectionParams, TLS1_2::ECDHEServerKeyExchange const& ServerExchange, MBCrypto::ECDHEPrivatePublicKeyPair const& ClientKeypair)
{
	MBCrypto::ElipticCurve CurveToUse(ServerExchange.NamedCurve);
	std::string ServerBigEndianPoint = ServerExchange.ECPoint;
	std::string PreMasterSecret = CurveToUse.CalculateSharedKey(MBCrypto::DecodeECP_ANS1x92Encoding(ServerBigEndianPoint), ClientKeypair.PrivateInteger);
	PreMasterSecret = PreMasterSecret.substr(PreMasterSecret.find_first_not_of((char)0));
	ConnectionParams.PreMasterSecret = PreMasterSecret;
}
void TLSHandler::p_EstablishPreMasterSecret(TLS1_2::SecurityParameters& ConnectionParams, std::vector<std::string> const& ServerHelloResponse, MBSockets::ConnectSocket* SocketTouse)
{
	std::string ServerCertificateMessage = ServerHelloResponse[1];
	std::string ServerCertificate = ServerCertificateMessage.substr(5 + 4 + 3 + 3/*första trean är eftersom den börjhar med en lengthy byte av alla data, andra är föör längen av första certfiikaten*/);
	TLSServerPublickeyInfo KeyFromCertificate = GetServerPublicKey(ServerCertificate); //keytype agnostic data
	if (ConnectionParams.CipherSuiteInfo.ExchangeMethod == TLS1_2::KeyExchangeMethod::RSA)
	{
		SendClientKeyExchange(KeyFromCertificate, SocketTouse);
		ConnectionParams.ClientSequenceNumber += 1;
		SendChangeCipherMessage(SocketTouse);
	}
	else if (ConnectionParams.CipherSuiteInfo.ExchangeMethod == TLS1_2::KeyExchangeMethod::ECDHE)
	{
		TLS1_2::ECDHEServerKeyExchange ServerKeyExchange = TLS1_2::GetServerECDHEKey(ConnectionParameters,ServerHelloResponse[2]);
		MBCrypto::RSAPublicKey ServerRSAPublicKey = MBCrypto::ParsePublicKeyDEREncodedData(KeyFromCertificate.ServerKeyData);
		bool SignatureValid = MBCrypto::RSASSA_PKCS1_V1_5_VERIFY(ServerKeyExchange.ServerSignedData, ServerKeyExchange.ClientHashedServerECDHEParams, ServerRSAPublicKey, ConnectionParams.CipherSuiteInfo.HashFunction);
		if (SignatureValid == false)
		{
			assert(false);
		}
		MBCrypto::ECDHEPrivatePublicKeyPair ClientKeypair;
		std::string ClientKeyExchange = p_GenerateClientECDHEKeyExchangeMessage(ConnectionParams, ClientKeypair,ServerKeyExchange);
		SocketTouse->SendData(ClientKeyExchange.data(), ClientKeyExchange.size());
		ConnectionParams.ClientSequenceNumber += 1;
		ConnectionParams.AllHandshakeMessages.push_back(ClientKeyExchange);
		p_ECDHECalculatePremasterSecret(ConnectionParams, ServerKeyExchange,ClientKeypair);
		SendChangeCipherMessage(SocketTouse);
	}
}
std::vector<TLS1_2::Extension> TLSHandler::p_GenerateDHEExtensions()
{
	std::vector<TLS1_2::Extension> ReturnValue = {};
	TLS1_2::Extension SupportedGroupsExtension;
	SupportedGroupsExtension.ExtensionType = TLS1_2::SupportedGroups;
	std::vector<MBCrypto::NamedElipticCurve> SupportedCurves = 
	{ MBCrypto::NamedElipticCurve::secp256r1};
	SupportedGroupsExtension.ExtensionData = {};
	assert(SupportedCurves.size() < 256);
	SupportedGroupsExtension.ExtensionData.push_back(0);
	SupportedGroupsExtension.ExtensionData.push_back((uint8_t)SupportedCurves.size()*2);
	for (size_t i = 0; i < SupportedCurves.size(); i++)
	{
		SupportedGroupsExtension.ExtensionData.push_back(uint16_t(SupportedCurves[i]) >> 8);
		SupportedGroupsExtension.ExtensionData.push_back(uint16_t(SupportedCurves[i]) % 256);
	}


	TLS1_2::Extension EC_PointFormats;
	EC_PointFormats.ExtensionType = TLS1_2::EC_Point_Formats;
	const char* EC_PointsFormatData = "\x01\x00";
	EC_PointFormats.ExtensionData = {};
	for (size_t i = 0; i < 2; i++)
	{
		EC_PointFormats.ExtensionData.push_back(EC_PointsFormatData[i]);
	}
	ReturnValue.push_back(SupportedGroupsExtension);
	ReturnValue.push_back(EC_PointFormats);
	return(ReturnValue);
}
MBError TLSHandler::InitiateHandShake(MBSockets::ConnectSocket* SocketToConnect,std::string const& HostName)
{
	MBError ReturnValue(true);
	ConnectionParameters.HostToConnectToDomainName = HostName;
	std::string ClientHelloData = GenerateTLS1_2ClientHello(SocketToConnect);
	SocketToConnect->SendData(ClientHelloData.c_str(), ClientHelloData.size());
	ConnectionParameters.ClientSequenceNumber += 1;
	ConnectionParameters.IsHost = false;
	std::vector<std::string> ServerHelloResponseData = p_GetServerHelloResponseRecords(SocketToConnect);

	std::string ServerHelloResponseDataFragment = ServerHelloResponseData[0].substr(5/*längd av record layern*/+4/*längd av handshake grejen innan datan*/);
	TLS1_2::TLS1_2ServerHelloStruct ServerHelloStruct;
	TLS1_2::ParseServerHelloToStruct(&ServerHelloStruct, ServerHelloResponseDataFragment);

	p_UpdateConnectionParametersAfterServerHello(ConnectionParameters, ServerHelloStruct);
	p_EstablishPreMasterSecret(ConnectionParameters, ServerHelloResponseData, SocketToConnect);
	GenerateMasterSecret(ConnectionParameters.PreMasterSecret);
	GenerateKeys();
	
	
	
	//std::ofstream DEBUG_Keyfile = std::ofstream("C:/Users/emanu/Desktop/Wireshark/LogFileFolder/KeyLogFileWireshark.txt");
	//DEBUG_Keyfile << "CLIENT_RANDOM " << MBUtility::ReplaceAll(MBUtility::HexEncodeString(std::string((char*)ConnectionParameters.client_random, 32)), " ", "") << " " << MBUtility::ReplaceAll(MBUtility::HexEncodeString(std::string((char*)ConnectionParameters.master_secret, 48)), " ", "");
	//DEBUG_Keyfile.flush();
	//DEBUG_Keyfile.close();
	//
	//
	//std::ofstream DEBUG_Keyfile2 = std::ofstream("C:/Users/emanu/Desktop/Wireshark/LogFileFolder/KeyLogFileWiresharkPremaster.txt");
	//DEBUG_Keyfile2 << "CLIENT_RANDOM " <<MBUtility::ReplaceAll(MBUtility::HexEncodeString(std::string((char*)ConnectionParameters.client_random, 32)), " ", "") << " " <<MBUtility::ReplaceAll(MBUtility::HexEncodeString(MBCrypto::I2OSP(ConnectionParameters.PreMasterSecret,48)), " ", "");
	//DEBUG_Keyfile2.flush();
	//DEBUG_Keyfile2.close();
	
	
	std::string VerifyDataMessage = GenerateVerifyDataMessage();
	SocketToConnect->SendData(VerifyDataMessage.data(), VerifyDataMessage.size());
	ConnectionParameters.HandshakeFinished = true;
	ConnectionParameters.ClientSequenceNumber+=1;
	ConnectionParameters.ServerSequenceNumber = 0;

	//std::cout << "Premaster secret: " << MBUtility::ReplaceAll(MBUtility::HexEncodeString(ConnectionParameters.PreMasterSecret), " ", "") << std::endl;
	std::vector<std::string> ServerVerifyDataResponse = GetNextPlaintextRecords(SocketToConnect,2,1000000);
	ConnectionParameters.ServerSequenceNumber = 0;
	assert(ServerVerifyDataResponse.size() > 1);
	assert(ServerVerifyDataResponse[1].size() >= 9);
	assert(VerifyFinishedMessage(ServerVerifyDataResponse[1].substr(9)));
	ConnectionParameters.ServerSequenceNumber = 1;
	IsConnected = true;
	return(ReturnValue);
}
MBError TLSHandler::EstablishHostTLSConnection(MBSockets::ConnectSocket* SocketToConnect)
{
	//förutsätter att vi redan connectat med TCP protokollet
	ConnectionParameters.IsHost = true;
	MBError ErrorToReturn(true);
	std::vector<std::string> ClientHello = GetNextPlaintextRecords(SocketToConnect,1,m_MaxBytesInMemory);
	ConnectionParameters.AllHandshakeMessages.push_back(ClientHello[0]);
	TLS1_2::TLS1_2HelloClientStruct ClientHelloStruct = ParseClientHelloStruct(ClientHello[0]);

	//ConnectionParameters.CipherSuiteInfo = p_GetCipherSuiteData(ClientHelloStruct.CipherSuites);

	//om clienten skicker ett session id så vill vi ta och använda den sesssion id:n värden
	if (ClientHelloStruct.SessionId.size() != 0)
	{
		std::string SessionID = std::string((char*)ClientHelloStruct.SessionId.data(), ClientHelloStruct.SessionId.size());
		if ( TLSHandler::SessionIsSaved(SessionID))
		{
			ErrorToReturn = ResumeSession(std::string((char*)ClientHelloStruct.SessionId.data(), ClientHelloStruct.SessionId.size()),ClientHelloStruct, SocketToConnect);
			return(ErrorToReturn);
		}
	}
	//assert(ClientHello.size() == 1);
	if (ClientHello.size() != 1)
	{
		ErrorToReturn.ErrorMessage = "invalid client hello message";
		return(ErrorToReturn);
	}
	std::string ServerHelloRecord = GenerateServerHello(ClientHelloStruct);
	SocketToConnect->SendData(ServerHelloRecord.data(), ServerHelloRecord.size());
	ConnectionParameters.AllHandshakeMessages.push_back(ServerHelloRecord);
	//test grej
	std::string ServerCertificateRecord = GenerateServerCertificateRecord(ConnectionParameters.DomainName);
	SocketToConnect->SendData(ServerCertificateRecord.c_str(),ServerCertificateRecord.size());
	ConnectionParameters.AllHandshakeMessages.push_back(ServerCertificateRecord);
	//server done record
	TLS_RecordGenerator ServerHelloDoneRecord;
	ServerHelloDoneRecord.SetContentType(TLS1_2::ContentType::handshake);
	ServerHelloDoneRecord.SetHandshakeType(TLS1_2::HandshakeType::server_hello_done);
	std::string ServerHelloDoneMessage = ServerHelloDoneRecord.GetHandShakeRecord();
	SocketToConnect->SendData(ServerHelloDoneMessage.c_str(), ServerHelloDoneMessage.size());
	ConnectionParameters.AllHandshakeMessages.push_back(ServerHelloDoneRecord.GetHandShakeRecord());
	std::vector<std::string> ClientResponse = GetNextPlaintextRecords(SocketToConnect,3,m_MaxBytesInMemory);
	if (ClientResponse.size() != 3 || this->m_HandshakeIsValid == false)
	{
		ErrorToReturn.ErrorMessage = "Error in establishing host tls connection";
		ErrorToReturn = false;
		return(ErrorToReturn);
	}
	//för så tar vi och decrypter premasterkeyn från dens första svar
	//debug grejer
	//std::cout << HexEncodeString(HMAC("123123", "123123")) << std::endl;
	ServerHandleKeyExchange(ClientResponse[0]);


	ConnectionParameters.AllHandshakeMessages.push_back(ClientResponse[0]);
	std::string ServerFinishedMessagePlaintext = p_DecryptRecord(ClientResponse[2]);
	if (VerifyFinishedMessage(ServerFinishedMessagePlaintext.substr(9)) == false)
	{
		ErrorToReturn = false;
		ErrorToReturn.ErrorMessage = "Failed to verify client finsihed message";
		return(ErrorToReturn);
	}
	//ConnectionParameters.AllHandshakeMessages.push_back(ClientResponse[2]);
	ConnectionParameters.AllHandshakeMessages.push_back(ServerFinishedMessagePlaintext);
	SendChangeCipherMessage(SocketToConnect);
	std::string ServerVerifyDataMessage = GenerateVerifyDataMessage();
	SocketToConnect->SendData(ServerVerifyDataMessage.c_str(), ServerVerifyDataMessage.size());

	ConnectionParameters.HandshakeFinished = true;
	ConnectionParameters.ServerSequenceNumber = 1;
	ConnectionParameters.ClientSequenceNumber = 1;
	IsConnected = true;
	CacheSession();
	return(ErrorToReturn);
}
void TLSHandler::GenerateMasterSecret(std::string const& PremasterSecret)
{
	std::string ClientRandom = std::string((char*)ConnectionParameters.client_random, 32);
	std::string ServerRandom = std::string((char*)ConnectionParameters.server_random, 32);
	std::string MasterSecretString = PRF(PremasterSecret, "master secret",  ClientRandom+ServerRandom , 48);
	for (size_t i = 0; i < 48; i++)
	{
		ConnectionParameters.master_secret[i] = MasterSecretString[i];
	}
}
void TLSHandler::ServerHandleKeyExchange(std::string const& ClientKeyExchangeRecord)
{
	RSADecryptInfo DecryptInfo = GetRSADecryptInfo(ConnectionParameters.DomainName);
	std::string PreMasterSecret = RSAES_PKCS1_v1_5_DecryptData(DecryptInfo, ClientKeyExchangeRecord.substr(11));
	GenerateMasterSecret(PreMasterSecret);
	GenerateKeys();
}
RSADecryptInfo TLSHandler::GetRSADecryptInfo(std::string const& DomainName)
{
	RSADecryptInfo ReturnValue;
	std::string KeyBinaryInfo = TLS1_2::PemToBinary(GetDomainResourcePath(DomainName)+"EncryptionResources/KeyfileRSA2096.key");
	ASN1Extracter Parser = ASN1Extracter((uint8_t*)KeyBinaryInfo.c_str());
	Parser.ExtractTagData();
	Parser.ExtractLengthOfType();
	Parser.SkipToNextField();
	//nu är vi vid public modolun
	Parser.ExtractTagData();
	unsigned int LengthOfModolu = Parser.ExtractLengthOfType();
	ReturnValue.PublicModulu.SetFromBigEndianArray(&KeyBinaryInfo[Parser.GetOffset()], LengthOfModolu);
	Parser.SetOffset(Parser.GetOffset() + LengthOfModolu);
	Parser.SkipToNextField();
	//inne i piravet exponent fieldet
	Parser.ExtractTagData();
	unsigned int LengthOfPrivateExponent = Parser.ExtractLengthOfType();
	ReturnValue.PrivateExponent.SetFromBigEndianArray(&KeyBinaryInfo[Parser.GetOffset()], LengthOfPrivateExponent);

	return(ReturnValue);
}

std::string TLSHandler::RSAES_PKCS1_v1_5_DecryptData(RSADecryptInfo const& DecryptInfo, std::string const& PublicKeyEncryptedData)
{
	//debug
	//MrBigInt CipherText;
	////uint8_t* DebugPointer =(uint8_t*) PublicKeyEncryptedData.c_str(); 
	//CipherText.SetFromBigEndianArray(PublicKeyEncryptedData.c_str(), PublicKeyEncryptedData.size());
	//MrBigInt PaddedPlaintextMessageRepresentative;
	//MrBigInt::PowM(CipherText, DecryptInfo.PrivateExponent, DecryptInfo.PublicModulu, PaddedPlaintextMessageRepresentative);
	std::clock_t Timer = clock();
	
	CryptoPP::Integer CipherText_CryptoPP((const CryptoPP::byte*)PublicKeyEncryptedData.c_str(),PublicKeyEncryptedData.size(),CryptoPP::Integer::UNSIGNED,CryptoPP::BIG_ENDIAN_ORDER);
	//DEBUG
	assert(MBGToCryptoPP(CryptoPPToMBG(CipherText_CryptoPP)) == CipherText_CryptoPP);

	CryptoPP::Integer PrivateExponent_CryptoPP = MBGToCryptoPP(DecryptInfo.PrivateExponent);
	CryptoPP::Integer PublicModolu_CryptoPP = MBGToCryptoPP(DecryptInfo.PublicModulu);
	//TODO Memory leak cryptoPP?
	CryptoPP::ModularArithmetic ma(PublicModolu_CryptoPP);
	CryptoPP::Integer PaddedPlaintextMessageRepresentative_CryptoPP = ma.Exponentiate(CipherText_CryptoPP,PrivateExponent_CryptoPP);
	


	MrBigInt PaddedPlaintextMessageRepresentative = CryptoPPToMBG(PaddedPlaintextMessageRepresentative_CryptoPP);
	

	std::string PaddedPlainTextMessage = I2OSP(PaddedPlaintextMessageRepresentative, 256);
	std::string PlaintextMessage = TLS1_2::Remove_PKCS1_v1_5_Padding(PaddedPlainTextMessage);

	assert(PlaintextMessage.size() == 48);
	if (PlaintextMessage.size() != 48)
	{
		std::cout << "error decrypting key exchange" << std::endl;
	}

	std::cout << "RSA Decryption Time: " << (clock() - Timer) / double(CLOCKS_PER_SEC) << "s";
	return(PlaintextMessage);
}
TLS1_2::TLS1_2HelloClientStruct TLSHandler::ParseClientHelloStruct(std::string const& ClientHelloData)
{
	TLS1_2::TLS1_2HelloClientStruct ReturnValue;
	TLS1_2::NetWorkDataHandler Parser((const uint8_t*)ClientHelloData.c_str());
	//vi börjar med att ta bort det som är på toppen, dvs record och handshake layer headern,total 5+4 = 9
	Parser.Extract32();
	Parser.Extract32();
	Parser.Extract8();
	//nu parser vi structen
	TLS1_2::ProtocolVersion ClientHelloProtocolVersion;
	ClientHelloProtocolVersion.Major = Parser.Extract8();
	ClientHelloProtocolVersion.Minor = Parser.Extract8();
	ReturnValue.ProtocolVers = ClientHelloProtocolVersion;
	
	std::string ClienHelloRandom = "";
	for (size_t i = 0; i < 32; i++)
	{
		ClienHelloRandom += Parser.Extract8();
		ConnectionParameters.client_random[i] = ClienHelloRandom[i];
		ReturnValue.RandomStruct.RandomBytes[i] = ClienHelloRandom[i];
	}

	uint8_t SessionIdLength = Parser.Extract8();
	for (size_t i = 0; i < SessionIdLength; i++)
	{
		ReturnValue.SessionId.push_back(Parser.Extract8());
	}

	unsigned int NumberOfCipherSuites = Parser.Extract16() / 2;
	for (size_t i = 0; i < NumberOfCipherSuites; i++)
	{
		TLS1_2::CipherSuite NewCipherSuite;
		NewCipherSuite.Del1 = Parser.Extract8();
		NewCipherSuite.Del2 = Parser.Extract8();
		ReturnValue.CipherSuites.push_back(NewCipherSuite);
	}

	unsigned int NumberOfCompressionMethods = Parser.Extract8();
	for (size_t i = 0; i < NumberOfCompressionMethods; i++)
	{
		ReturnValue.CompressionMethods.push_back(Parser.Extract8());
	}

	if (ClientHelloData.size() != Parser.GetPosition())
	{
		//det existerar extensions vi vill parsa
		unsigned int NumberOfExtensionBytes = Parser.Extract16();
		unsigned int ParsedBytes = 0;
		while (ParsedBytes != NumberOfExtensionBytes)
		{
			TLS1_2::Extension NewExtension;
			NewExtension.ExtensionType = (TLS1_2::ExtensionTypes)Parser.Extract16();
			ParsedBytes += 2;
			unsigned int NewExtensionsByteLength = Parser.Extract16();
			ParsedBytes += 2;
			for (size_t i = 0; i < NewExtensionsByteLength; i++)
			{
				NewExtension.ExtensionData.push_back(Parser.Extract8());
				ParsedBytes += 1;
			}
			ReturnValue.OptionalExtensions.push_back(NewExtension);
		}
	}
	return(ReturnValue);
}
std::vector<TLS1_2::SignatureAndHashAlgoritm> TLSHandler::GetSupportedSignatureAlgorithms(std::vector<TLS1_2::Extension> const& Extensions)
{
	std::vector<TLS1_2::SignatureAndHashAlgoritm> ReturnValue = {};
	TLS1_2::Extension SignatureExtension =  TLS1_2::GetExtension(Extensions, TLS1_2::ExtensionTypes::signature_algoritms);
	TLS1_2::NetWorkDataHandler Parser(&SignatureExtension.ExtensionData[0]);
	unsigned int ExtensionByteSize = Parser.Extract16();
	unsigned int ExtractedBytes = 0;
	while (ExtractedBytes != ExtensionByteSize)
	{
		TLS1_2::SignatureAndHashAlgoritm NewAlgorithm;
		NewAlgorithm.Hash = (TLS1_2::HashAlgorithm) Parser.Extract8();
		NewAlgorithm.Signature = (TLS1_2::SignatureAlgorithm) Parser.Extract8();
		ReturnValue.push_back(NewAlgorithm);
		ExtractedBytes += 2;
	}
	return(ReturnValue);
}
void TLSHandler::CacheSession()
{
	std::lock_guard<std::mutex> Lock(CachedSessionsMutex);
	CachedSessions[ConnectionParameters.SessionId] = ConnectionParameters;
}
bool TLSHandler::SessionIsSaved(std::string const& SessionID)
{
	std::lock_guard<std::mutex> Lock(CachedSessionsMutex);
	return(CachedSessions.find(SessionID) != CachedSessions.end());
}
TLS1_2::SecurityParameters TLSHandler::GetCachedSession(std::string const& SessionID)
{
	std::lock_guard<std::mutex> Lock(CachedSessionsMutex);
	return(CachedSessions[SessionID]);
}
MBError TLSHandler::ResumeSession(std::string const& SessionID,TLS1_2::TLS1_2HelloClientStruct const& ClientHello ,MBSockets::ConnectSocket* SocketToConnect)
{
	//std::cout<<HexEncodeString(SessionID)<<std::endl;
	MBError ErrorToReturn(true);
	std::string ServerHelloMessage = GenerateServerHello(ClientHello, SessionID);
	SocketToConnect->SendData(ServerHelloMessage.data(), ServerHelloMessage.size());
	std::vector<std::string> CurrentHandshakeRecords = ConnectionParameters.AllHandshakeMessages;
	CurrentHandshakeRecords.push_back(ServerHelloMessage);
	std::string NewClientRandom = std::string((char*)ConnectionParameters.client_random,32);
	std::string NewServerRandom = std::string((char*)ConnectionParameters.server_random, 32);

	this->ConnectionParameters = TLSHandler::GetCachedSession(SessionID);
	ConnectionParameters.AllHandshakeMessages = CurrentHandshakeRecords;
	for (size_t i = 0; i < 32; i++)
	{
		ConnectionParameters.client_random[i] = NewClientRandom[i];
		ConnectionParameters.server_random[i] = NewServerRandom[i];
	}
	ConnectionParameters.HandshakeFinished = false;
	GenerateKeys();
	SendChangeCipherMessage(SocketToConnect);
	//är ju frågan om det ska vara 0 eller 1 recored sent?
	std::string VerifyDataMessage = GenerateVerifyDataMessage();
	SocketToConnect->SendData(VerifyDataMessage.data(), VerifyDataMessage.size());

	std::vector<std::string> ClientVerifyDataMessage = GetNextPlaintextRecords(SocketToConnect,2,m_MaxBytesInMemory);
	while (ClientVerifyDataMessage.size() < 2)
	{
		ErrorToReturn = false;
		ErrorToReturn.ErrorMessage = "Error in recieving resuming session: Invalid Client Response";
		return(ErrorToReturn);
	}
	assert(ClientVerifyDataMessage.size() <= 2);
	ConnectionParameters.ServerSequenceNumber = 0;
	ConnectionParameters.ClientSequenceNumber = 0;
	std::string ServerFinishedMessagePlaintext = p_DecryptRecord(ClientVerifyDataMessage[1]);
	if (VerifyFinishedMessage(ServerFinishedMessagePlaintext.substr(9)) == false)
	{
		ErrorToReturn = false;
		ErrorToReturn.ErrorMessage = "Failed to verify client finsihed message";
		return(ErrorToReturn);
	}
	ConnectionParameters.HandshakeFinished = true;
	ConnectionParameters.ServerSequenceNumber = 1;
	ConnectionParameters.ClientSequenceNumber = 1;
	IsConnected = true;
	return(ErrorToReturn);
}
std::string TLSHandler::GenerateServerHello(TLS1_2::TLS1_2HelloClientStruct const& ClientHello,std::string const& SpecifiedSeessionID)
{
	TLS_RecordGenerator NewRecord;
	NewRecord.SetHandshakeType(TLS1_2::HandshakeType::server_hello);
	NewRecord.SetContentType(TLS1_2::ContentType::handshake);
	//server hellon ska innehålla vilken tls variation vi använder, vilket i vårt fall är 1.2
	NewRecord.AddUINT8(3);
	NewRecord.AddUINT8(3);
	//vi generar serverrandom
	std::string ServerRandom = MBRandom::GetRandomBytes(32);
	for (size_t i = 0; i < 32; i++)
	{
		ConnectionParameters.server_random[i] = ServerRandom[i];
	}
	NewRecord.AddOpaqueData(ServerRandom);
	//vi Generar session id
	std::string SessionId;
	if (SpecifiedSeessionID == "")
	{
		SessionId = TLSHandler::GenerateSessionId();
	}
	else
	{
		SessionId = SpecifiedSeessionID;
	}
	ConnectionParameters.SessionId = SessionId;
	NewRecord.AddOpaqueArray(SessionId,1);
	//0,0x3c enda cihpersuiten vi stödjer, egentligen ska vi ju ta och välja ut en beroende på etc
	std::vector<TLS1_2::SignatureAndHashAlgoritm> SupportedAlgorithms = GetSupportedSignatureAlgorithms(ClientHello.OptionalExtensions);
	//hardcodad af
	NewRecord.AddUINT8(0);
	NewRecord.AddUINT8(0x2f);

	ConnectionParameters.CipherSuiteInfo = p_GetCipherSuiteData({ 0,0x2f });
	ConnectionParameters.HashAlgorithm = MBCrypto::HashObject(MBCrypto::HashFunction::SHA1);
	//uppdatera compressions algorithmen
	ConnectionParameters.compression_algorithm = TLS1_2::nullCompressionMethod;
	NewRecord.AddUINT8(0);
	ConnectionParameters.DomainName = GetServerNameFromExtensions(ClientHello.OptionalExtensions);
	//std::cout <<"Sent server random:"<< HexEncodeString(ServerRandom) << std::endl;
	return(NewRecord.GetHandShakeRecord());
}
std::string TLSHandler::GetDomainResourcePath(std::string const& DomainName)
{
	std::string ActualDomainName;
	if (DomainName != "" && DomainName != "127.0.0.1")
	{
		ActualDomainName = DomainName.substr(DomainName.find_first_of("."));
		if (ActualDomainName[0] == '.')
		{
			ActualDomainName = DomainName;
		}
	}
	else
	{
		ActualDomainName = DefaultDomain;
	}
	//TODO detta är MEGA yikes, vi hardcodar en path som den här ska gå till... Borde kanske ha en socket factory eller att serversockets kräver mer data?
	std::string ReturnValue = "./MBWebsite/ServerResources/" + ActualDomainName + "/";
	if (!std::filesystem::exists(ReturnValue))
	{
		std::cout << "TLS domain resource path doesn't exist";
	}
	return(ReturnValue);
}
std::string TLSHandler::GetDefaultCertificate()
{
	//TODO MEGA yikes det här med
	return("./MBWebsite/ServerResources/" + DefaultDomain + "/" + "EncryptionResources/SignedCertificateRSA2096.der");
}
std::string TLSHandler::GenerateServerCertificateRecord(std::string const& DomainName)
{
	TLS_RecordGenerator NewRecord;
	std::string CertificatePath;
	if (DomainName != "")
	{
		CertificatePath = TLSHandler::GetDomainResourcePath(DomainName) + "EncryptionResources/SignedCertificateRSA2096.der";
	}
	else
	{
		CertificatePath = TLSHandler::GetDefaultCertificate();
	}
	std::ifstream t(CertificatePath,std::ifstream::in|std::ifstream::binary);
	size_t size = MBGetFileSize(CertificatePath);
	std::string CertificateDataBuffer(size, ' ');
	t.read(&CertificateDataBuffer[0], size);
	size_t ReadCharacters = t.gcount();
	std::string CertificateData(CertificateDataBuffer.c_str(), ReadCharacters);

	CertificateData = TLS_RecordGenerator::GetNetworkUINT24(CertificateData.size()) + CertificateData;
	NewRecord.SetContentType(TLS1_2::ContentType::handshake);
	NewRecord.SetHandshakeType(TLS1_2::HandshakeType::certificate);
	NewRecord.AddOpaqueArray(CertificateData, 3);
	return(NewRecord.GetHandShakeRecord());
}
std::string TLSHandler::GetServerNameFromExtensions(std::vector<TLS1_2::Extension> const& Extensions)
{
	TLS1_2::Extension NameExtension;
	for (size_t i = 0; i < Extensions.size(); i++)
	{
		if (Extensions[i].ExtensionType == TLS1_2::ExtensionTypes::server_name)
		{
			NameExtension = Extensions[i];
			break;
		}
	}
	if (NameExtension.ExtensionType == TLS1_2::ExtensionTypes::Null)
	{
		return("");
	}
	TLS1_2::NetWorkDataHandler Parser(&NameExtension.ExtensionData[0]);
	
	unsigned int NumberOfNames = Parser.Extract16();
	for (size_t i = 0; i < NumberOfNames; i++)
	{
		unsigned int NameType = Parser.Extract8();
		unsigned int NameLength = Parser.Extract16();
		std::string NameData = "";
		for (size_t i = 0; i < NameLength; i++)
		{
			NameData += Parser.Extract8();
		}
		if (NameType == TLS1_2::host_name)
		{
			return(NameData);
		}
	}
}
std::string TLSHandler::GenerateSessionId()
{
	//TODO Fixa så det faktist ger ett session id på ett bra sätt
	std::string NewSessionID = MBRandom::GetRandomBytes(32);
	std::lock_guard<std::mutex> Lock(CachedSessionsMutex);
	while (CachedSessions.find(NewSessionID) != CachedSessions.end())
	{
		NewSessionID = MBRandom::GetRandomBytes(32);
	}
	return(NewSessionID);
}
bool TLSHandler::VerifyFinishedMessage(std::string DataToVerify)
{
	std::string ConcattenatedHandshakeData = "";
	for (size_t i = 0; i < ConnectionParameters.AllHandshakeMessages.size(); i++)
	{
		ConcattenatedHandshakeData += ConnectionParameters.AllHandshakeMessages[i].substr(5);
	}
	std::string HashedHandshakeData = Sha256HashObject.Hash(ConcattenatedHandshakeData);
	std::string VerifyData;
	if (!ConnectionParameters.IsHost)
	{
		VerifyData = PRF(std::string(reinterpret_cast<char*>(ConnectionParameters.master_secret), 48), "server finished", HashedHandshakeData, 12);
	}
	else
	{
		VerifyData = PRF(std::string(reinterpret_cast<char*>(ConnectionParameters.master_secret), 48), "client finished", HashedHandshakeData, 12);
	}
	return(DataToVerify == VerifyData);
}
bool TLSHandler::VerifyMac(std::string Hash,TLS1_2::TLS1_2GenericRecord RecordToEncrypt)
{
	std::string SequenceNumber = std::string(8, 0);
	uint64_t TempSequenceNumber;
	std::string WriteMACKey;
	if (!ConnectionParameters.IsHost)
	{
		TempSequenceNumber = ConnectionParameters.ServerSequenceNumber;
		WriteMACKey = ConnectionParameters.server_write_MAC_Key;
	}
	else
	{
		TempSequenceNumber = ConnectionParameters.ClientSequenceNumber;
		WriteMACKey = ConnectionParameters.client_write_MAC_Key;
	}
	for (size_t i = 0; i < 8; i++)
	{
		SequenceNumber[7 - i] = TempSequenceNumber % 256;
		TempSequenceNumber = TempSequenceNumber >> 8;
	}
	std::string LengthString = std::string(2, 0);
	uint16_t TempLengthNumber = RecordToEncrypt.Length;
	for (int i = 0; i < 2; i++)
	{
		LengthString[1 - i] = TempLengthNumber % 256;
		TempLengthNumber = TempLengthNumber >> 8;
	}
	std::string LengthOfRecord = "";
	LengthOfRecord += RecordToEncrypt.Length >> 8;
	LengthOfRecord += RecordToEncrypt.Length % 256;
	//std::cout <<MBUtility::ReplaceAll(MBUtility::HexEncodeString(std::string((char*)ConnectionParameters.master_secret, 48))," ","") << std::endl;
	std::string MACStringInput = SequenceNumber + char(RecordToEncrypt.Type) + char(RecordToEncrypt.Protocol.Major) + char(RecordToEncrypt.Protocol.Minor) + LengthString + RecordToEncrypt.Data;
	std::string MAC = HMAC(WriteMACKey, MACStringInput);
	//std::cout << MBUtility::ReplaceAll(MBUtility::HexEncodeString(MAC)," ","") << std::endl;
	if (MAC != Hash)
	{
		std::cout << MBUtility::HexEncodeString(MAC) << std::endl;
		std::cout << MBUtility::HexEncodeString(Hash) << std::endl;
		std::cout << MBUtility::HexEncodeString(std::string((char*)ConnectionParameters.master_secret,48)) << std::endl;
		std::cout << "uh oh" << std::endl;
	}
	return(MAC == Hash);
}
std::string TLSHandler::p_Decrypt_AES_CBC_Record(std::string const& Data,bool* OutVerification)
{
	//beror egentligen på om vi är en server eller client men vi förutsätter att vi är en client
	std::string ReturnValue = "";
	std::string RecordHeader = Data.substr(0, 5);
	unsigned int HashOutputSize = ConnectionParameters.HashAlgorithm.GetDigestSize();
	unsigned char IV[16];
	if (Data[0] == TLS1_2::change_cipher_spec)
	{
		return(Data);
	}
	for (int i = 5; i < 21; i++)
	{
		IV[i - 5] = Data[i];
	}
	
	uint64_t DataToDecryptSize = Data.size()-21;
	std::string WriteKey;
	if (!ConnectionParameters.IsHost)
	{
		WriteKey = ConnectionParameters.server_write_Key;
	}
	else
	{
		WriteKey = ConnectionParameters.client_write_Key;
	}

	MBError DecryptionError = true;
	MBCrypto::BlockCipher_CBC_Handler Decryptor(MBCrypto::BlockCipher::AES);
	std::string ContentOctets = Decryptor.DecryptData(Data.c_str()+21, DataToDecryptSize, WriteKey.c_str(), WriteKey.size(), IV, 16, &DecryptionError);
	
	std::string MACOctets = ContentOctets.substr(ContentOctets.size()- HashOutputSize);
	ContentOctets.resize(ContentOctets.size() - HashOutputSize);

	RecordHeader[3] = ContentOctets.size() >> 8;
	RecordHeader[4] = ContentOctets.size() % 256;

	//vi verivierar att macen stämmer
	TLS1_2::TLS1_2GenericRecord RecordToEncrypt;
	RecordToEncrypt.Type = TLS1_2::ContentType(RecordHeader[0]);
	RecordToEncrypt.Protocol = { uint8_t(RecordHeader[1]),uint8_t(RecordHeader[2]) };
	RecordToEncrypt.Length = ContentOctets.size();
	RecordToEncrypt.Data = std::move(ContentOctets);
	bool VerificationResult = VerifyMac(MACOctets, RecordToEncrypt);
	assert(VerificationResult);
	*OutVerification = VerificationResult;
	//nu vet vi även att vi kan ta och öka record´sen vi fått
	ReturnValue = RecordHeader + std::move(RecordToEncrypt.Data);
	return(ReturnValue);
}
std::string TLSHandler::p_Decrypt_AES_GCM_Record(std::string const& Data,bool* OutVerification)
{
	std::string ReturnValue = "";
	std::string DecryptedData = "";
	std::string ImplicitNonce = "";
	if (!ConnectionParameters.IsHost)
	{
		ImplicitNonce = ConnectionParameters.server_write_IV;
	}
	else
	{
		ImplicitNonce = ConnectionParameters.client_write_IV;
	}
	std::string ExplicitNonce = Data.substr(5, ConnectionParameters.CipherSuiteInfo.NonceSize);
	std::string Nonce = ImplicitNonce+ExplicitNonce;
	std::string AdditionalData = p_GetAEADAdditionalData(ConnectionParameters, Data,false);
	size_t CipherOffset = 5 + ExplicitNonce.size();
	size_t TagSize = 16;
	bool VerificationResult = false;

	std::string WriteKey = "";

	if (!ConnectionParameters.IsHost)
	{
		WriteKey = ConnectionParameters.server_write_Key;
	}
	else
	{
		WriteKey = ConnectionParameters.client_write_Key;
	}
	std::string Mac = Data.substr(Data.size() - TagSize);
	//size_t EncryptedDataSize = Data.size() - (5 + ExplicitNonce.size()) - TagSize;
	size_t EncryptedDataSize = Data.size() - (5 + ExplicitNonce.size());
	const void* DataToDecrypt = Data.data() + CipherOffset;

	MBError EvaluationErrror = true;
	MBCrypto::BlockCipher_GCM_Handler Decryptor(MBCrypto::BlockCipher::AES);
	DecryptedData = Decryptor.DecryptData(DataToDecrypt, EncryptedDataSize, WriteKey.data(), WriteKey.size(), Nonce.data(), Nonce.size(), AdditionalData.data(), AdditionalData.size(), &EvaluationErrror);
	VerificationResult = EvaluationErrror;
	assert(VerificationResult);
	if (VerificationResult)
	{
		ReturnValue = Data.substr(0, 3);
		ReturnValue += char(DecryptedData.size() >> 8);
		ReturnValue += char(DecryptedData.size() % 256);
		ReturnValue += DecryptedData;
	}
	*OutVerification = VerificationResult;
	return(ReturnValue);
}
std::string TLSHandler::p_DecryptRecord(std::string const& Data)
{
	std::string ReturnValue = "";
	bool VerifcationResult = false;
	if (Data[0] == TLS1_2::change_cipher_spec)
	{
		return(Data);
	}
	if (ConnectionParameters.CipherSuiteInfo.CipherMode == TLS1_2::SymmetricCipherMode::GCM && ConnectionParameters.CipherSuiteInfo.EncryptionCipher ==TLS1_2::SymmetricEncryptionCipher::AES)
	{
		ReturnValue = p_Decrypt_AES_GCM_Record(Data,&VerifcationResult);
	}
	else if (ConnectionParameters.CipherSuiteInfo.CipherMode == TLS1_2::SymmetricCipherMode::CBC && ConnectionParameters.CipherSuiteInfo.EncryptionCipher == TLS1_2::SymmetricEncryptionCipher::AES)
	{
		ReturnValue = p_Decrypt_AES_CBC_Record(Data,&VerifcationResult);
	}
	if (Data[0] != TLS1_2::change_cipher_spec)
	{
		if (!ConnectionParameters.IsHost)
		{
			ConnectionParameters.ServerSequenceNumber += 1;
		}
		else
		{
			ConnectionParameters.ClientSequenceNumber += 1;
		}
	}
	return(ReturnValue);
}
void TLSHandler::SendDataAsRecord(const void* Data, size_t SizeOfData, MBSockets::ConnectSocket* AssociatedSocket)
{
	std::vector<std::string> RecordsData = std::vector<std::string>(0);
	size_t Offset = 0;
	size_t MacLengthOfRecord = 16384;
	while (Offset < SizeOfData)
	{
		RecordsData.push_back(std::string(&(((char*)Data)[Offset]),std::min(MacLengthOfRecord,SizeOfData-Offset)));
		Offset += MacLengthOfRecord;
	}
	for (int i = 0; i < RecordsData.size(); i++)
	{
		TLS1_2::TLS1_2GenericRecord RecordToSend;
		RecordToSend.Type = TLS1_2::application_data;
		RecordToSend.Protocol = ConnectionParameters.NegotiatedProtocolVersion;
		RecordToSend.Length = RecordsData[i].size();
		RecordToSend.Data = RecordsData[i];
		std::string DataToSend = GetEncryptedRecord(RecordToSend);
		AssociatedSocket->SendData(DataToSend.data(), DataToSend.size());
		if (!ConnectionParameters.IsHost)
		{
			ConnectionParameters.ClientSequenceNumber += 1;
		}
		else
		{
			ConnectionParameters.ServerSequenceNumber += 1;
		}
	}
}
void TLSHandler::SendDataAsRecord(std::string const& Data,MBSockets::ConnectSocket* AssociatedSocket)
{
	SendDataAsRecord(Data.c_str(), Data.size(), AssociatedSocket);
}
std::string TLSHandler::GetApplicationData(MBSockets::ConnectSocket* AssociatedSocket,int MaxNumberOfBytes)
{
	std::vector<std::string> ApplicationDataRecords = GetNextPlaintextRecords((MBSockets::ConnectSocket*)AssociatedSocket,MaxNumberOfBytes);
	//nu så bara klistrat vi ihop datan och skickar tillbaka den
	//OBS! Vi kan inte här se till att all data vi letar efter faktiskt är med, det får vi göra längre i call hierarkin
	std::string SentApplicationData = "";
	for (int i = 0; i < ApplicationDataRecords.size(); i++)
	{
		SentApplicationData += ApplicationDataRecords[i].substr(5);
	}
	return(SentApplicationData);
}
std::string TLSHandler::GetNextRawProtocol(MBSockets::ConnectSocket* SocketToConnect, int MaxBytesInMemory)
{
	if (MaxBytesInMemory < m_MaxRecordLength)
	{
		//orkar inte med edge cases, tar lite marginal
		MaxBytesInMemory = m_MaxRecordLength+200;
	}
	if (RecieveDataState.StoredRecords.size() > 0)
	{
		std::string StoredRecord = RecieveDataState.StoredRecords.front();
		RecieveDataState.StoredRecords.pop_front();
		return(StoredRecord);
	}
	std::string NewRecordData = "";
	std::swap(NewRecordData, RecieveDataState.CurrentRecordPreviousData);
	//newrecord data kan aldrig vara komplett record
	NewRecordData += SocketToConnect->RecieveData(MaxBytesInMemory - NewRecordData.size());
	if (NewRecordData.size() == 0)
	{
		return(NewRecordData);
	}
	if (NewRecordData.size() < 5)
	{
		//måste alltid kunna avgöra längden
		NewRecordData += SocketToConnect->RecieveData(MaxBytesInMemory - NewRecordData.size());
	}
	size_t RecievedDataOffset = 0;
	bool NewRecordExtracted = false;
	std::vector<std::string> NewRecords = {};
	while (RecievedDataOffset < NewRecordData.size())
	{
		if (!NewRecordExtracted)
		{
			//vi vill alltid ha minst 1 nytt record, garanterat minst 5 i size
			TLS1_2::NetWorkDataHandler LengthExtractor((const uint8_t*)&NewRecordData.c_str()[RecievedDataOffset]);
			LengthExtractor.SetPosition(LengthExtractor.GetPosition() + 3);
			int CurrentRecordSize = LengthExtractor.Extract16();
			if (NewRecordData.size()-5 < CurrentRecordSize)
			{
				NewRecordData += SocketToConnect->RecieveData(MaxBytesInMemory - NewRecordData.size());
			}
			if (NewRecordData.size() - 5 < CurrentRecordSize)
			{
				//socketen har slutat skicka eller något, nu kan vi inte få ett färdigt nytt record
				std::cout << "Error in recieving new record: Incomplete data" << std::endl;
				return("");
			}
			NewRecordExtracted = true;
		}
		if (NewRecordData.size() - RecievedDataOffset < 5)
		{
			RecieveDataState.CurrentRecordPreviousData = NewRecordData.substr(RecievedDataOffset);
			break;
		}
		else
		{
			TLS1_2::NetWorkDataHandler LengthExtractor((const uint8_t*)&NewRecordData.c_str()[RecievedDataOffset]);
			LengthExtractor.SetPosition(LengthExtractor.GetPosition() + 3);
			int CurrentRecordSize = LengthExtractor.Extract16();
			if (NewRecordData.size() - RecievedDataOffset >= 5 + CurrentRecordSize)
			{
				NewRecords.push_back(NewRecordData.substr(RecievedDataOffset, 5 + CurrentRecordSize));
				RecievedDataOffset += 5 + CurrentRecordSize;
			}
			else
			{
				RecieveDataState.CurrentRecordPreviousData = NewRecordData.substr(RecievedDataOffset);
				break;
			}
		}
	}
	//pushar alla records som var nya till vår stack av records som vi vill spara
	for (size_t i = 1; i < NewRecords.size(); i++)
	{
		RecieveDataState.StoredRecords.push_back(NewRecords[i]);
	}
	return(NewRecords[0]);
}
bool TLSHandler::HandleAlertMessage(MBSockets::ConnectSocket* SocketToConnect,std::string const& DecryptedAlertRecord)
{
	uint8_t AlertLevel = DecryptedAlertRecord[5];
	uint8_t ErrorMessage = DecryptedAlertRecord[6];
	//i detta fall vill vi printa meddelandet, annars kanske vi vill spara det i någon form av fel array
	std::cout << "Recieved alert message: " << TLS1_2::GetAlertErrorDescription(ErrorMessage) << std::endl;
	if (AlertLevel == TLS1_2::fatal)
	{
		IsConnected = false;
		m_HandshakeIsValid = false;
		if (ErrorMessage == TLS1_2::close_notify)
		{
			SendCloseNotfiy(SocketToConnect);
		}
		return(false);
	}
	else
	{
		//HandleNonFatalError();
		//eftersom vi fortfarande, om inte funktionen över säger så, ha kopplingen skickar vi ingen data, och går tillbaka till att recieva data
		return(true);
	}
}
std::vector<std::string> TLSHandler::GetNextPlaintextRecords(MBSockets::ConnectSocket* SocketToConnect, int NumberOfRecordsToGet, int MaxBytesInMemory)
{
	std::vector<std::string> ReturnValue = std::vector<std::string>(0);
	int NumberOfRecievedRecords = 0;
	while (NumberOfRecievedRecords < NumberOfRecordsToGet || NumberOfRecordsToGet == -1)
	{
		std::string NewRecord = GetNextRawProtocol(SocketToConnect, MaxBytesInMemory);
		if (NewRecord == "")
		{
			break;
		}
		if (ConnectionParameters.HandshakeFinished)
		{
			NewRecord = p_DecryptRecord(NewRecord);
		}
		if (NewRecord[0] == TLS1_2::alert)
		{
			bool IsValid = HandleAlertMessage(SocketToConnect,NewRecord);
			if (!IsValid)
			{
				break;
			}
		}
		else
		{
			ReturnValue.push_back(NewRecord);
			NumberOfRecievedRecords += 1;
		}
	}
	return(ReturnValue);
}
std::vector<std::string> TLSHandler::GetNextPlaintextRecords(MBSockets::ConnectSocket* SocketToConnect,int MaxNumberOfBytes)
{
	//när vi bara ska etablera en handskakning räcker det med att vi bara processar datan naivt, kan ju han en data som rent processar datan innan också
	std::vector<std::string> ReturnValue = std::vector<std::string>(0);
	//processadata med typ protokoll grejer antar jag
	while (true)
	{
		//max är egentligen 2<<14+2048+5, men varför inte ha lite marginal
		int MaxEncryptedRecordSize = (2 << 14) + 2048 + 100;
		if (MaxNumberOfBytes < MaxEncryptedRecordSize)
		{
			MaxNumberOfBytes = MaxEncryptedRecordSize;
		}
		std::string RawData = RecieveDataState.CurrentRecordPreviousData;
		int TotalRecievedData = RawData.size();
		std::string NewData = SocketToConnect->RecieveData(MaxNumberOfBytes-TotalRecievedData);
		if (NewData == "")
		{
			return(ReturnValue);
		}
		RawData += NewData;
		TotalRecievedData = RawData.size();
		std::vector<std::string> RawDataProtocols = std::vector<std::string>(0);
		uint64_t OffsetOfProcessedRecords = 0;
		//när vi vet att datan vi fått är krytperad vill vi innan vi går vidare med rseten av logiken decryptera den
		//Ny paradigm, vi går igenom vår data, kollar igenom och appendar alla hela records vi hittar, finns det kvar så slutar vi helt enkelt
		while (true)
		{
			while(RawData.size()-OffsetOfProcessedRecords < 5 && TotalRecievedData < MaxNumberOfBytes)
			{
				RawData += SocketToConnect->RecieveData(MaxNumberOfBytes-TotalRecievedData);
				TotalRecievedData = RawData.size();
			}

			if (TotalRecievedData >= MaxNumberOfBytes && RawData.size()-OffsetOfProcessedRecords < 5)
			{
				//avbryts mitt i, sparar nuvarande datan
				RecieveDataState.CurrentRecordPreviousData = RawData.substr(OffsetOfProcessedRecords);
				break;
			}

			TLS1_2::NetWorkDataHandler DataReader(reinterpret_cast<const uint8_t*> (&RawData.c_str()[OffsetOfProcessedRecords]));
			DataReader.Extract24();
			uint64_t LengthOfRecord = DataReader.Extract16();
			while(RawData.size()-OffsetOfProcessedRecords-5 < LengthOfRecord && TotalRecievedData < MaxNumberOfBytes)
			{
				RawData += SocketToConnect->RecieveData(MaxNumberOfBytes - TotalRecievedData);
				TotalRecievedData = RawData.size();
			}

			if (TotalRecievedData >= MaxNumberOfBytes && RawData.size() - OffsetOfProcessedRecords - 5 < LengthOfRecord)
			{
				//avbryts mitt i, sparar nuvarande datan
				RecieveDataState.CurrentRecordPreviousData = RawData.substr(OffsetOfProcessedRecords);
				break;
			}

			RawDataProtocols.push_back(RawData.substr(OffsetOfProcessedRecords,5 + LengthOfRecord));
			OffsetOfProcessedRecords += (5 + LengthOfRecord);
			if (OffsetOfProcessedRecords == RawData.size())
			{
				//avbröts på ett clean sett, reverta staten av tcp grejen
				RecieveDataState.CurrentRecordPreviousData = "";
				break;
			}
		}
		//vi har fått all krypterad data, nu okrypterar vi den
		if (ConnectionParameters.HandshakeFinished)
		{
			for (int i = 0; i < RawDataProtocols.size(); i++)
			{
				RawDataProtocols[i] = p_DecryptRecord(RawDataProtocols[i]);
				//std::string NewRawData = RawData.substr(0, 5);
				//NewRawData += DecryptBlockcipherRecord(RawData);
				//RawData = NewRawData;
			}
		}
		//ConnectionParameters.ServerSequenceNumber += RawDataProtocols.size();
		//Vi ska också ta att och processa datan så vi fpår plaintext
		//RawDataToTLSPlaintext
		for (size_t i = 0; i < RawDataProtocols.size(); i++)
		{
			std::string RecordPlaintextData = RawDataProtocols[i];
			if (RecordPlaintextData[0] == TLS1_2::application_data)
			{
				//HandleApplicationData
				//RawData.erase(0, 5);

				return(RawDataProtocols);
			}
			else if (RecordPlaintextData[0] == TLS1_2::alert)
			{
				uint8_t AlertLevel = RecordPlaintextData[5];
				uint8_t ErrorMessage = RecordPlaintextData[6];
				//i detta fall vill vi printa meddelandet, annars kanske vi vill spara det i någon form av fel array
				std::cout << "Recieved alert message: " << TLS1_2::GetAlertErrorDescription(ErrorMessage) << std::endl;
				if (AlertLevel == TLS1_2::fatal)
				{
					ReturnValue.push_back("FatalErrorOccured");
					IsConnected = false;
					if (ErrorMessage == TLS1_2::close_notify)
					{
						SendCloseNotfiy(SocketToConnect);
					}
					return(ReturnValue);
				}
				else
				{
					//HandleNonFatalError();
					//eftersom vi fortfarande, om inte funktionen över säger så, ha kopplingen skickar vi ingen data, och går tillbaka till att recieva data
				}
			}
			else if (RecordPlaintextData[0] == TLS1_2::handshake)
			{
				return(RawDataProtocols);
			}
			else if (RecordPlaintextData[0] == TLS1_2::change_cipher_spec)
			{
				return(RawDataProtocols);
			}
			else
			{
				//Unknown error, något har kajkat en del någonstans
				assert(false);
			}
		}
	}
	return(ReturnValue);
}
void TLSHandler::SendCloseNotfiy(MBSockets::ConnectSocket* SocketToUse)
{
	//då ska vi också ta skicka att vi stänger av
	std::string DataToSend = "";
	if (ConnectionParameters.HandshakeFinished)
	{
		TLS1_2::TLS1_2GenericRecord CloseNotifyRecord;
		CloseNotifyRecord.Protocol = { 3, 3 };
		CloseNotifyRecord.Type = TLS1_2::alert;
		std::string CloseNotifyData = "";
		CloseNotifyData += char(1);
		CloseNotifyData += char(0);
		CloseNotifyRecord.Length = 2;
		DataToSend = GetEncryptedRecord(CloseNotifyRecord);
	}
	else
	{
		DataToSend += char(TLS1_2::alert);
		DataToSend += char(3);
		DataToSend += char(3);
		DataToSend += char(0);
		DataToSend += char(2);
		DataToSend += char(1);
		DataToSend += char(0);
		IsConnected = false;
	}
	SocketToUse->SendData(DataToSend.data(), DataToSend.size());
}
TLSHandler::~TLSHandler()
{

}

/*Gammal logik för hur vi gettar protocol

		while (true)
		{
			TLS1_2::NetWorkDataHandler DataReader(reinterpret_cast<const uint8_t*> (&RawData.c_str()[OffsetOfProcessedRecords]));
			//vi vill bara komma åt längden
			DataReader.Extract24();
			uint16_t LengthOfRecord = DataReader.Extract16();
			if (LengthOfRecord+5+OffsetOfProcessedRecords > RawData.size())
			{
				//detta invalidatar datareader objektet
				RawData += SocketToConnect->GetNextRequestData();
			}
			RawDataProtocols.push_back(RawData.substr(OffsetOfProcessedRecords, 5 + LengthOfRecord));
			//vi behöver se till att all data kommer med, därför ser vi till att längden av alla records som skickas är längden av datan, annars väntar vi igen på mer data
			if (OffsetOfProcessedRecords+LengthOfRecord+5 < RawData.size())
			{
				OffsetOfProcessedRecords += 5 + uint64_t(LengthOfRecord);
			}
			else
			{
				break;
			}
		}
*/