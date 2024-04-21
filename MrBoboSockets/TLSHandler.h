#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <time.h>
//#include <plusaes.hpp>
#include <MBUtility/MBErrorHandling.h>
#include <filesystem>
#include <deque>

#include <MBCrypto/MBCrypto.h>
#include <MBCrypto/Asn1Handlers.h>
#include <fstream>
#include <MBUtility/MBStrings.h>
enum class TLSVersions
{
	TLS1_2,
	TLS1_3,
	NullVersion
};
class TLSHashObject
{
private:
	std::string(*HashFunction)(std::string const& StringToHash);
	unsigned int HashOutputSize = 0;
	unsigned int HashBlockSize = 0;
public:
	std::string Hash(std::string const& StringToHash) { return(HashFunction(StringToHash)); };
	TLSHashObject(std::string(*NewHashFunction)(std::string const& StringToHash), unsigned int NewHashOutputSize, unsigned int NewHashBlockSize)
	{
		HashFunction = NewHashFunction;
		HashOutputSize = NewHashOutputSize;
		HashBlockSize = NewHashBlockSize;
	}
	unsigned int GetHashOutputSize()
	{
		return(HashOutputSize);
	}
	unsigned int GetHashBlockSize()
	{
		return(HashBlockSize);
	}
};
namespace TLS1_2
{

	enum class KeyExchangeMethod
	{
		RSA,
		DHE,
		ECDHE,
		Null,
	};
	enum class CertificateAuthenticationMethod
	{
		RSA,
		ECDHE,
		DHE,
		Null,
	};
	enum class SymmetricEncryptionCipher
	{
		AES,
		Null
	};
	enum class SymmetricCipherMode
	{
		CBC,
		GCM,
		Null,
	};
	struct CipherSuiteData
	{
		KeyExchangeMethod ExchangeMethod = KeyExchangeMethod::Null;
		CertificateAuthenticationMethod AuthenticationMethod = CertificateAuthenticationMethod::Null;
		size_t CipherKeySize = 0;
		size_t NonceSize = 0;
		size_t FixedIVLength = 0;
		SymmetricEncryptionCipher EncryptionCipher = SymmetricEncryptionCipher::Null;
		SymmetricCipherMode CipherMode = SymmetricCipherMode::Null;
		MBCrypto::HashFunction HashFunction = MBCrypto::HashFunction::Null;
	};


	struct Random
	{
		//unsigned int GMT_UNIX_TIME;
		uint8_t RandomBytes[32];
	};
	struct CipherSuite
	{
		uint8_t Del1;
		uint8_t Del2;
	};
	enum ExtensionTypes : uint16_t
	{
		server_name = 0,
		signature_algoritms = 13,
		SupportedGroups = 10,
		EC_Point_Formats = 11,
		Null
	};
	struct Extension
	{
		ExtensionTypes ExtensionType = ExtensionTypes::Null;
		std::vector<uint8_t> ExtensionData = {}; //max 2^16-1 i l�ngd
	};
	struct Certificate
	{
		std::vector<uint8_t> CertificateData;
	};
	struct ProtocolVersion
	{
		uint8_t Major;
		uint8_t Minor;
	};
	enum ContentType : uint8_t
	{
		change_cipher_spec = 20,
		alert = 21,
		handshake = 22,
		application_data = 23
		//ska bara ta en char i uttrymme
	};
	enum TLS1_2AlertLevel : uint8_t { warning = 1, fatal = 2/*max en char i l�ngd*/ };
	enum TLS1_2AlertDescription : uint8_t
	{
		close_notify = 0,
		unexpected_message = 10,
		bad_record_mac = 20,
		decryption_failed_RESERVED = 21,
		record_overflow = 22,
		decompression_failure = 30,
		handshake_failure = 40,
		no_certificate_RESERVED = 41,
		bad_certificate = 42,
		unsupported_certificate = 43,
		certificate_revoked = 44,
		certificate_expired = 45,
		certificate_unknown = 46,
		illegal_parameter = 47,
		unknown_ca = 48,
		access_denied = 49,
		decode_error = 50,
		decrypt_error = 51,
		export_restriction_RESERVED = 60,
		protocol_version = 70,
		insufficient_security = 71,
		internal_error = 80,
		user_canceled = 90,
		no_renegotiation = 100,
		unsupported_extension = 110
		//(255) max en char i l�ngd
	};
	enum HandshakeType : uint8_t {
		hello_request = 0,
		client_hello = 1,
		server_hello = 2,
		certificate = 11,
		server_key_exchange = 12,
		certificate_request = 13,
		server_hello_done = 14,
		certificate_verify = 15,
		client_key_exchange = 16,
		finished = 20// (255), max en char i l�ngd
	};
	struct HandShake
	{
		HandshakeType MessageType;
		//uint24 data length, f�r t�nka p� hur jag ska g�ra den
		//sen s� inh�ller den datan fr�n respektive handshake typ
	};
	enum HashAlgorithm : uint8_t
	{
		none = 0,
		md5 = 1,
		sha1 = 2,
		sha224 = 3,
		sha256 = 4,
		sha384 = 5,
		sha512 = 6
		//max en char
	};
	enum SignatureAlgorithm : uint8_t
	{
		anonymous = 0,
		rsa = 1,
		dsa = 2,
		ecdsa = 3
	};
	struct SignatureAndHashAlgoritm
	{
		HashAlgorithm Hash;
		SignatureAlgorithm Signature;
	};
	enum CompressionMethod { nullCompressionMethod = 0 };//char i l�ngd
	struct SecurityParameters
	{
		enum ConnectionEnd { server, client };
		enum PRFAlgorithm { tls_prf_sha256 };
		enum BulkCipherAlgorithm { nullbulkcipher, rc4, des3, aes };
		enum CipherType { stream, block, aead };
		enum MACAlgorithm {
			nullMaxAlgorithm, hmac_md5, hmac_sha1, hmac_sha256,
			hmac_sha384, hmac_sha512
		};
		ConnectionEnd			entity;
		PRFAlgorithm			prf_algorithm;
		BulkCipherAlgorithm		bulk_cipher_algorithm;
		CipherType				cipher_type;
		uint8_t					enc_key_length;
		uint8_t					block_length;
		uint8_t					fixed_iv_length;
		uint8_t					record_iv_length;
		MACAlgorithm			mac_algorithm;
		uint8_t                 mac_length;
		uint8_t					mac_key_length;
		CompressionMethod		compression_algorithm;
		std::string				PreMasterSecret = std::string(48,0);
		uint8_t					master_secret[48];
		uint8_t					client_random[32];
		uint8_t					server_random[32];
		MBCrypto::HashObject HashAlgorithm = MBCrypto::HashObject();
		std::string client_write_MAC_Key = "";
		std::string server_write_MAC_Key = "";
		std::string client_write_Key = "";
		std::string server_write_Key = "";
		std::string client_write_IV = "";
		std::string server_write_IV = "";
		uint64_t ClientSequenceNumber = 0;
		uint64_t ServerSequenceNumber = 0;
		ProtocolVersion NegotiatedProtocolVersion = { 3,3 };//standard
		std::vector<std::string> AllHandshakeMessages = std::vector<std::string>(0);
		bool HandshakeFinished = false;
		std::string SessionId = "";
		bool IsHost = false;
		std::string DomainName = "";
		std::string HostToConnectToDomainName = "";
		CipherSuiteData CipherSuiteInfo;
	};
	enum class ECCurveType : uint8_t
	{
		explicit_prime = 1,
		explicit_char2 = 2,
		named_curve = 3,
		Null
	};
	struct ECDHEServerKeyExchange
	{
		ECCurveType CurveType = ECCurveType::Null;
		MBCrypto::NamedElipticCurve NamedCurve = MBCrypto::NamedElipticCurve::Null;
		std::string ECPoint = ""; //0-32 bytes
		SignatureAndHashAlgoritm SignatureAlgorithm;
		std::string ServerSignedData = "";
		std::string ClientHashedServerECDHEParams = "";
	};
	struct ECDHECClientKeyExchange
	{
		std::string ECPoint = ""; //0-32 bytes
	};
	ECDHEServerKeyExchange GetServerECDHEKey(SecurityParameters& ConnectionParams, std::string const& CompleteRecordData);
	struct TLS1_2Alert {
		TLS1_2AlertLevel level;
		TLS1_2AlertDescription description;
	};


	struct TLS1_2ServerHelloStruct
	{
		ProtocolVersion Protocol;
		Random RandomStruct;
		std::vector<uint8_t> SessionId; //max 32 chars
		CipherSuite CipherSuiteToUse;
		uint8_t CompressionToUse;
		std::vector<Extension> OptionalExtensions; //kan tillskillnad fr�n dem andra vara helt tom, max 2^16-1 bytes
	};
	struct TLS1_2ServerCertificate
	{
		std::vector<Certificate> CertificateList;
	};
	struct TLS1_2ClientKeyExchangeMessage
	{
		//inneh�ller beror helt och h�llet p� vilka algoritmer vi anv�nder
	};
	struct TLS1_2RSAPremasterSecret
	{
		/*If RSA is being used for key agreement and authentication, the
		  client generates a 48-byte premaster secret, encrypts it using the
		  public key from the server's certificate, and sends the result in
		  an encrypted premaster secret message.  This structure is a
		  variant of the ClientKeyExchange message and is not a message in
		  itself.*/
		  //sjka tdydligen vara den variation man f�rst f�reslog av tls, inte den som blev?
		uint16_t ClientProtocolVersion;
		uint8_t RandomBytes[46];
	};
	struct FinishedMessage
	{
		std::vector<uint8_t> VerifyData; //verifydatalength i l�ngd
		//verify data = PRF(master_secret, finished_label, Hash(handshake_messages))[0..verify_data_length - 1] ;
		//f�r kolla upp exakt i dokumentationen vad det inneb�r
	};
	struct TLS1_2PlainText
	{
		ContentType TypeOfContent;
		ProtocolVersion Protocol;
		uint16_t DataLength;
		std::vector<uint8_t> Data;
		//f�r max ineh�lla 2^14 antal chars
	};
	struct TLS1_2CompressedPlaintext
	{
		ContentType TypeOfContent;
		ProtocolVersion Protocol;
		uint16_t DataLength;
		std::vector<uint8_t> Data;
		//datan fr�n en plaintext meddelandfe cryptearad
	};
	struct TLS1_2CipherText
	{
		ContentType TypeOfContent;
		ProtocolVersion Protocol;
		uint16_t DataLength;
		std::vector<uint8_t> Data;
		//den crypterade compressade datan med MAC
	};
	struct TLS1_2HelloClientStruct
	{
		ProtocolVersion ProtocolVers;
		Random RandomStruct;
		std::vector<uint8_t> SessionId = {}; //max 32 chars
		std::vector<CipherSuite> CipherSuites = {};//minst 2, max 2^16-2 bytes
		std::vector<uint8_t> CompressionMethods = {}; // max 2^8-1 bytes
		std::vector<Extension> OptionalExtensions = {}; //kan tillskillnad fr�n dem andra vara helt tom, max 2^16-1 bytes
	};
	class uint24
	{
	private:
		uint32_t UnderlyingInt = 0;
	public:
		uint24(uint32_t IntToConvert)
		{
			if (IntToConvert > pow(2, 24) - 1)
			{
				//tar bort den st�rsta bitten
				IntToConvert = (IntToConvert << 8) >> 8;
			}
			UnderlyingInt = IntToConvert;
		}
		~uint24()
		{

		}
		operator uint32_t()
		{
			return(UnderlyingInt);
		}
	};
	inline 	uint32_t LocalToInternetByteOrder(uint32_t NumberToConvert)
	{
		//TODO: Ha en check som avg�r om det �r litel endian eller inte, s� vi kan �ndra beroende p� 
		uint32_t ReturnValue = 0;
		for (size_t i = 0; i < 4; i++)
		{
			ReturnValue += ((NumberToConvert >> (24 - i * 8)) % 256) << i * 8;
		}
		return(ReturnValue);
	}
	class NetWorkDataHandler
	{
	private:
		uint8_t* ArrayToFill = nullptr;
		const uint8_t* ArrayToReadFrom = nullptr;
		int Offset = 0;
		bool LocalIsLittleEndian = true;
	public:
		NetWorkDataHandler(uint8_t* Array)
		{
			ArrayToFill = Array;
			ArrayToReadFrom = Array;
		}
		NetWorkDataHandler(const uint8_t* Array)
		{
			ArrayToReadFrom = Array;
		}
		~NetWorkDataHandler()
		{

		}
		void operator<<(uint32_t Number)
		{
			uint32_t DataAsBigEndian = LocalToInternetByteOrder(Number);
			for (int i = 0; i < 4; i++)
			{
				ArrayToFill[Offset] = static_cast<uint8_t>((DataAsBigEndian >> (8 * i)) % 256);
				Offset += 1;
			}
		}
		void operator<<(uint24 Number)
		{
			uint32_t DataAsBigEndian = LocalToInternetByteOrder(Number) >> 8;
			for (int i = 0; i < 3; i++)
			{
				ArrayToFill[Offset] = static_cast<uint8_t>((DataAsBigEndian >> (8 * i)) % 256);
				Offset += 1;
			}
		}
		void operator<<(uint16_t Number)
		{
			uint32_t DataAsBigEndian = LocalToInternetByteOrder(Number) >> 16;
			for (int i = 0; i < 2; i++)
			{
				ArrayToFill[Offset] = static_cast<uint8_t>((DataAsBigEndian >> (8 * i)) % 256);
				Offset += 1;
			}
		}
		void operator<<(uint8_t Number)
		{
			ArrayToFill[Offset] = Number;
			Offset += 1;
		}
		void operator>>(uint32_t& Number)
		{
			if (LocalIsLittleEndian)
			{
				uint32_t ValueToChangeTo = 0;
				for (int i = 0; i < 4; i++)
				{
					ValueToChangeTo += uint16_t(ArrayToReadFrom[Offset]) << (3 - i) * 8;
					Offset += 1;
				}
				Number = ValueToChangeTo;
			}
		}
		void operator>>(uint24& Number)
		{
			if (LocalIsLittleEndian)
			{
				uint32_t ValueToChangeTo = 0;
				for (int i = 0; i < 3; i++)
				{
					ValueToChangeTo += uint16_t(ArrayToReadFrom[Offset]) << (2 - i) * 8;
					Offset += 1;
				}
				Number = uint24(ValueToChangeTo);
			}
		}
		void operator>>(uint16_t& Number)
		{
			if (LocalIsLittleEndian)
			{
				uint16_t ValueToChangeTo = 0;
				for (int i = 0; i < 2; i++)
				{
					uint8_t ValueOfArrayAtOffset = ArrayToReadFrom[Offset];
					ValueToChangeTo += uint16_t(ArrayToReadFrom[Offset]) << (1 - i) * 8;
					Offset += 1;
				}
				Number = ValueToChangeTo;
			}
		}
		void operator>>(uint8_t& Number)
		{
			Number = ArrayToReadFrom[Offset];
			Offset += 1;
		}
		uint32_t Extract32()
		{
			uint32_t NumberToReturn;
			*this >> NumberToReturn;
			return(NumberToReturn);
		}
		uint24 Extract24()
		{
			uint24 NumberToReturn = 0;
			*this >> NumberToReturn;
			return(NumberToReturn);
		}
		uint16_t Extract16()
		{
			uint16_t NumberToReturn;
			*this >> NumberToReturn;
			return(NumberToReturn);
		}
		uint8_t Extract8()
		{
			uint8_t NumberToReturn = 0;
			*this >> NumberToReturn;
			return(NumberToReturn);
		}
		uint32_t GetPosition()
		{
			return(Offset);
		}
		void SetPosition(uint32_t Position)
		{
			Offset = Position;
		}
	};
	enum ServerNameNameTypes : uint8_t
	{
		host_name = 0
	};
	struct ServerName
	{
		ServerNameNameTypes NameType;
		std::vector<uint8_t> HostName;//beror egebtkugeb p� typ, men existerar �n s� l�nge bara 1
	};
	struct TLS1_2GenericRecord
	{
		ContentType Type;
		ProtocolVersion Protocol;
		uint16_t Length;
		std::string Data;
	};
	inline Extension GetExtension(std::vector<Extension> const& Extensions,ExtensionTypes TypeToGet)
	{
		TLS1_2::Extension ExtensionToReturn;
		for (size_t i = 0; i < Extensions.size(); i++)
		{
			if (Extensions[i].ExtensionType == TypeToGet)
			{
				ExtensionToReturn = Extensions[i];
				break;
			}
		}
		return(ExtensionToReturn);
	}
	inline unsigned char Base64CharToBinary(unsigned char CharToDecode)
	{
		if (CharToDecode >= 65 && CharToDecode <= 90)
		{
			return(CharToDecode - 65);
		}
		if (CharToDecode >= 97 && CharToDecode <= 122)
		{
			return(CharToDecode - 71);
		}
		if (CharToDecode >= 48 && CharToDecode <= 57)
		{
			return(CharToDecode + 4);
		}
		if (CharToDecode == '+')
		{
			return(62);
		}
		if (CharToDecode == '/')
		{
			return(63);
		}
		assert(false);
        return(0);
	}
	inline std::string Base64ToBinary(std::string const& Base64Data)
	{
		std::string ReturnValue = "";
		unsigned int Base64DataSize = Base64Data.size();
		unsigned int Offset = 0;
		while (Offset < Base64DataSize)
		{
			uint32_t NewBytes = 0;
			char CharactersProcessed = 0;
			for (size_t i = 0; i < 4; i++)
			{
				NewBytes +=  uint32_t(Base64CharToBinary(Base64Data[Offset])) << (18 - 6 * i);
				Offset += 1;
				CharactersProcessed += 1;
				if (Offset == Base64DataSize)
				{
					break;
				}
				if (Base64Data[Offset] == '=')
				{
					break;
				}
			}
			for (size_t i = 0; i < CharactersProcessed-1; i++)
			{
				ReturnValue += char(NewBytes >> (16 - (i * 8)));
			}
			if (CharactersProcessed < 4)
			{
				break;
			}
		}
		return(ReturnValue);
	}
	inline std::string PemToBinary(std::string PemFile)	
	{
		PemFile = MBUtility::ReplaceAll(PemFile, "\n", "");
		PemFile = MBUtility::ReplaceAll(PemFile, "\r", "");
		//nu m�ste vi konvertera fr�n detta till bin�r data
		std::string BeginTag = "-----BEGIN RSA PRIVATE KEY-----";
		std::string EndTag = "-----END RSA PRIVATE KEY-----";
		std::string Base64Data = PemFile.substr(BeginTag.size(), PemFile.size() - BeginTag.size() - EndTag.size());
		std::string BinaryData = Base64ToBinary(Base64Data);
		return(BinaryData);
	}
	inline std::string Remove_PKCS1_v1_5_Padding(std::string const& PaddedString)
	{
		//TODO hardcoda inte modolun
		assert(PaddedString[0] == 0x00 || PaddedString[1] == 0x02);
		unsigned int OffsetToRealMessage = 1;
		unsigned int PaddedStringSize = PaddedString.size();
		for (size_t i = 2; i <PaddedStringSize; i++)
		{
			if (PaddedString[i] == 0)
			{
				if (OffsetToRealMessage == 2)
				{
					assert(false);
				}
				OffsetToRealMessage += 1;
				break;
			}
			OffsetToRealMessage += 1;
		}
		return(PaddedString.substr(OffsetToRealMessage+1));
	}
	struct TLS1_2HanshakeMessage
	{
		HandshakeType Type;
		std::string MessageData = "";
	};
	std::string GetRecordString(TLS1_2GenericRecord const& RecordToEncode);
	std::string GetRecordString(TLS1_2HanshakeMessage const& MessageToEncode);
}
//Hashalgoritmer vi beh�ver
std::string MBSha256(std::string const& StringToHash);
//std::string MBSha1(std::string const& StringToHash);
class TLS_RecordGenerator
{
private:
	std::string BodyData = "";
	uint8_t MajorVersion = 3;
	uint8_t MinorVersion = 3;
	TLS1_2::HandshakeType CurrentHandshakeType = TLS1_2::HandshakeType::hello_request;
	TLS1_2::ContentType CurrentContentType = TLS1_2::ContentType::handshake;
public:
	void AddUINT8(uint8_t ValueToAdd)
	{
		BodyData += char(ValueToAdd);
	}
	void AddUINT16(uint16_t ValueToAdd)
	{
		BodyData += char(ValueToAdd >> 8);
		BodyData += char(ValueToAdd);
	}
	void AddUINT24(uint32_t ValueToAdd)
	{
		BodyData += char(ValueToAdd >> 16);
		BodyData += char(ValueToAdd >> 8);
		BodyData += char(ValueToAdd);
	}
	void AddUINT32(uint32_t ValueToAdd)
	{
		BodyData += char(ValueToAdd >> 24);
		BodyData += char(ValueToAdd>>16);
		BodyData += char(ValueToAdd>>8);
		BodyData += char(ValueToAdd);
	}
	static std::string GetNetworkUINT8(uint8_t IntToConvert)
	{
		std::string ReturnValue = "";
		ReturnValue += char(IntToConvert);
		return(ReturnValue);
	}
	static std::string GetNetworkUINT16(uint16_t IntToConvert)
	{
		std::string ReturnValue = "";
		ReturnValue += char(IntToConvert >> 8);
		ReturnValue += char(IntToConvert);
		return(ReturnValue);
	}
	static std::string GetNetworkUINT24(uint32_t IntToConvert)
	{
		std::string ReturnValue = "";
		ReturnValue += char(IntToConvert >> 16);
		ReturnValue += char(IntToConvert >> 8);
		ReturnValue += char(IntToConvert);
		return(ReturnValue);
	}
	static std::string GetNetworkUINT32(uint32_t IntToConvert)
	{
		std::string ReturnValue = "";
		ReturnValue += char(IntToConvert >> 24);
		ReturnValue += char(IntToConvert >> 16);
		ReturnValue += char(IntToConvert >> 8);
		ReturnValue += char(IntToConvert);
		return(ReturnValue);
	}
	void AddOpaqueArray(std::string OpaqueData, unsigned int MaxSize = 2)
	{
		uint32_t ArraySize = OpaqueData.size();
		if (MaxSize == 1)
		{
			AddUINT8(ArraySize);
		}
		else if (MaxSize == 2)
		{
			AddUINT16(ArraySize);
		}
		else if (MaxSize == 3)
		{
			AddUINT24(ArraySize);
		}
		else if (MaxSize == 4)
		{
			AddUINT32(ArraySize);
		}
		else
		{
			assert(false);
		}
		BodyData += OpaqueData;
	}
	void AddOpaqueData(std::string NewData)
	{
		BodyData += NewData;
	}
	void SetHandshakeType(TLS1_2::HandshakeType NewHandshakeType)
	{
		CurrentHandshakeType = NewHandshakeType;
	}
	void SetContentType(TLS1_2::ContentType NewContentType)
	{
		CurrentContentType = NewContentType;
	}
	std::string GetRecordData()
	{
		std::string CompleteRecordData = "";
		CompleteRecordData += char(CurrentContentType);
		CompleteRecordData += char(MajorVersion);
		CompleteRecordData += char(MinorVersion);
		CompleteRecordData += TLS_RecordGenerator::GetNetworkUINT16(BodyData.size());
		CompleteRecordData += BodyData;
		return(CompleteRecordData);
	}
	std::string GetHandShakeRecord()
	{
		std::string CompleteRecordData = "";
		CompleteRecordData += char(CurrentContentType);
		CompleteRecordData += char(MajorVersion);
		CompleteRecordData += char(MinorVersion);
		CompleteRecordData += TLS_RecordGenerator::GetNetworkUINT16(BodyData.size()+4);
		CompleteRecordData += char(CurrentHandshakeType);
		CompleteRecordData += TLS_RecordGenerator::GetNetworkUINT24(BodyData.size());
		CompleteRecordData += BodyData;
		return(CompleteRecordData);
	}
	std::string GetBodyData()
	{
		return(BodyData);
	}
};
struct TLSServerPublickeyInfo
{
	//associated algoritm i guess
	std::string ServerKeyData;
};

struct RSAPublicKey
{
	MrBigInt Modolu;
	MrBigInt Exponent;
};
namespace MBSockets
{
	class Socket;
	class ConnectSocket;
	class ServerSocket;
}
struct RSADecryptInfo
{
	MrBigInt PrivateExponent = MrBigInt(0);
	MrBigInt PublicModulu = MrBigInt(0);
};
struct TLS_TCP_State
{
	int CurrentRecordSize = 0;
	int CurrentRecordParsed = 0;
	std::string CurrentRecordPreviousData = "";
	std::deque<std::string> StoredRecords = {};
};

class DomainHandler
{
public:
    virtual std::string GetCertificateData(std::string const& DomainName) = 0;
    virtual std::string GetKeyData(std::string const& DomainName) = 0;
    virtual ~DomainHandler(){};
};

class TLSHandler
{
private:
	TLSHashObject Sha256HashObject = TLSHashObject(MBSha256, 32, 64);
	bool IsConnected = false;
	bool m_HandshakeIsValid = true;
	int m_MaxRecordLength = 16384;
	int m_MaxBytesInMemory = 200000000;
	TLS_TCP_State RecieveDataState;
	TLS1_2::SecurityParameters ConnectionParameters;
	std::string DefaultDomain = "mrboboget.se";
	size_t m_NonceSequenceNumber = 0;
    std::unique_ptr<DomainHandler> m_CertificateRetriever;

	RSAPublicKey ExtractRSAPublicKeyFromBitString(std::string& BitString);
	MrBigInt OS2IP(const char* Data, uint64_t LengthOfData);
	MrBigInt RSAEP(TLSServerPublickeyInfo& RSAInfo, MrBigInt const& MessageRepresentative);
	static std::string I2OSP(MrBigInt NumberToConvert, uint64_t LengthOfString);
	std::string RSAES_PKCS1_V1_5_ENCRYPT(TLSServerPublickeyInfo& RSAInfo, std::string& DataToEncrypt);
	void SendClientKeyExchange(TLSServerPublickeyInfo& Data, MBSockets::ConnectSocket* SocketToConnect);
	std::string XORedString(std::string String1, std::string String2);
	std::string HMAC(std::string Secret, std::string Seed);
	std::string P_Hash(std::string Secret, std::string Seed, uint64_t AmountOfData);
	std::string PRF(std::string Secret, std::string Label, std::string Seed, uint64_t AmountOfData);
	std::string GenerateServerHello(TLS1_2::TLS1_2HelloClientStruct const& ClientHello, std::string const& SpecifiedSeessionID = "");
	std::string GenerateServerCertificateRecord(std::string const& ServerName);
	TLS1_2::TLS1_2HelloClientStruct ParseClientHelloStruct(std::string const& ClientHelloData);
	std::string GetServerNameFromExtensions(std::vector<TLS1_2::Extension> const& Extensions);
	void GenerateMasterSecret(std::string const& PremasterSecret);
	static std::vector<TLS1_2::SignatureAndHashAlgoritm> GetSupportedSignatureAlgorithms(std::vector<TLS1_2::Extension> const& Extensions);
	static std::string GenerateSessionId();
	std::string GetDomainResourcePath(std::string const& DomainName);
	static std::string RSAES_PKCS1_v1_5_DecryptData(RSADecryptInfo const& DecryptInfo, std::string const& PublicKeyEncryptedData);
	std::string GetDefaultCertificate();
	void ServerHandleKeyExchange(std::string const& ClientKeyExchangeRecord);
	MBError ResumeSession(std::string const& SessionID,TLS1_2::TLS1_2HelloClientStruct const& StructToUse,MBSockets::ConnectSocket* SocketToConnect);
	void CacheSession();
	static bool SessionIsSaved(std::string const& SessionID);
	TLS1_2::SecurityParameters GetCachedSession(std::string const& SessiondID);
	void SendCloseNotfiy(MBSockets::ConnectSocket* SocketToUse);
	std::vector<std::string> GetNextPlaintextRecords(MBSockets::ConnectSocket* SocketToConnect, int NumberOfRecordsToGet, int MaxBytesInMemory);
	bool HandleAlertMessage(MBSockets::ConnectSocket* SocketToConnect,std::string const& DecryptedAlertRecord);
	std::string GetNextRawProtocol(MBSockets::ConnectSocket* SocketToConnect, int MaxBytesInMemory);
	static TLS1_2::CipherSuiteData p_GetCipherSuiteData(TLS1_2::CipherSuite);
	static std::vector<TLS1_2::CipherSuite> p_GetSupportedCipherSuites();
	static void p_UpdateConnectionParametersAfterServerHello(TLS1_2::SecurityParameters& ConnectionParameters,TLS1_2::TLS1_2ServerHelloStruct const& ServerHelloStruct);
	std::vector<TLS1_2::Extension> p_GetClientExtensions();
	std::vector<std::string> p_GetServerHelloResponseRecords(MBSockets::ConnectSocket* SocketToUse);
	void p_EstablishPreMasterSecret(TLS1_2::SecurityParameters& ConnectionParameters, std::vector<std::string> const& ServerHelloResponse, MBSockets::ConnectSocket* SocketTouse);
	static std::string p_GenerateClientECDHEKeyExchangeMessage(TLS1_2::SecurityParameters const& ConnectionParams,MBCrypto::ECDHEPrivatePublicKeyPair& OutKeypair, TLS1_2::ECDHEServerKeyExchange const& ServerExchange);
	static void p_ECDHECalculatePremasterSecret(TLS1_2::SecurityParameters& ConnectionParams,TLS1_2::ECDHEServerKeyExchange const& ServerExchange, MBCrypto::ECDHEPrivatePublicKeyPair const& ClientKeypair);
	static std::vector<TLS1_2::Extension> p_GenerateDHEExtensions();
	static std::string p_GetAEADAdditionalData(TLS1_2::SecurityParameters const& SecurityParameters, TLS1_2::TLS1_2GenericRecord const& AssociatedRecord,bool Encrypts);
	static std::string p_GetAEADAdditionalData(TLS1_2::SecurityParameters const& SecurityParameters, std::string const& RecordData,bool Encrypts);
	std::string p_GetCBCEncryptedRecord(TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt);
	std::string p_GetGCMEncryptedRecord(TLS1_2::SecurityParameters const& SecurityParams,TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt);
	std::string p_Get_AES_GCM_EncryptedRecord(TLS1_2::SecurityParameters const& SecurityParams, TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt);
	std::string p_GetExplicitNonce();

	std::string p_Decrypt_AES_CBC_Record(std::string const& Data, bool* OutVerification);
	std::string p_Decrypt_AES_GCM_Record(std::string const& Data, bool* OutVerification);
	std::string p_DecryptRecord(std::string const& Data);
public:
	TLSHandler();
	bool ConnectionIsActive() { return(IsConnected); };
	RSADecryptInfo GetRSADecryptInfo(std::string const& DomainName);
	MBError EstablishTLSConnection(MBSockets::ConnectSocket* SocketToConnect,std::string const& HostName);
	MBError EstablishHostTLSConnection(MBSockets::ConnectSocket* SocketToConnect,std::unique_ptr<DomainHandler> );
	std::string GenerateTLS1_2ClientHello(MBSockets::ConnectSocket* SocketToConnect);
	std::vector<std::string> GetCertificateList(std::string& AllCertificateData);
	void SendChangeCipherMessage(MBSockets::ConnectSocket* SocketToConnect);
	void GenerateKeys();
	std::string GetEncryptedRecord(TLS1_2::TLS1_2GenericRecord const& RecordToEncrypt);
	//std::string TestEncryptRecord(void* IV,uint64_t RecordNumber ,std::string ClientWriteMacKey, std::string ClientWriteKey, TLS1_2::TLS1_2GenericRecord& RecordToEncrypt)
	//{
	//	std::string PreviousWriteMacKey = ConnectionParameters.client_write_MAC_Key;
	//	std::string PreviousWriteKey = ConnectionParameters.client_write_Key;
	//	uint64_t PreviousRecordNumber = ConnectionParameters.ClientSequenceNumber;
	//	
	//	ConnectionParameters.ClientSequenceNumber = RecordNumber;
	//	ConnectionParameters.client_write_MAC_Key = ClientWriteMacKey;
	//	ConnectionParameters.client_write_Key = ClientWriteKey;
	//	std::string EncryptedRecord = GetEncryptedRecord(RecordToEncrypt,IV);
	//
	//	ConnectionParameters.client_write_MAC_Key = PreviousWriteMacKey;
	//	ConnectionParameters.client_write_Key = PreviousWriteKey;
	//	ConnectionParameters.ClientSequenceNumber = PreviousRecordNumber;
	//	return(EncryptedRecord);
	//}
	bool EstablishedSecureConnection() { return(ConnectionParameters.HandshakeFinished); };
	std::string GenerateVerifyDataMessage();
	MBError InitiateHandShake(MBSockets::ConnectSocket* SocketToConnect,std::string const& HostName);
	bool VerifyFinishedMessage(std::string DataToVerify);
	bool VerifyMac(std::string Hash, TLS1_2::TLS1_2GenericRecord RecordToEncrypt);
	void SendDataAsRecord(std::string const& Data, MBSockets::ConnectSocket* AssociatedSocket);
	void SendDataAsRecord(const void* Data, size_t SizeOfData, MBSockets::ConnectSocket* AssociatedSocket);
	std::string GetApplicationData(MBSockets::ConnectSocket* AssociatedSocket,int MaxNumberOfBytes = 1000000);
	//std::vector<std::string> GetNextPlaintextRecords(MBSockets::ConnectSocket* SocketToConnect,int MaxNumberOfBytes = 1000000);
	~TLSHandler();
};
