#pragma once
#include <string>
#include <vector>
#include <MrPostOGet/Asn1Handlers.h>
#include <iostream>
#include <time.h>
#include <picosha2.h>
#include <plusaes.hpp>
enum class TLSVersions
{
	TLS1_2,
	TLS1_3,
	NullVersion
};
namespace TLS1_2
{
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
		signature_algoritms = 13
	};
	struct Extension
	{
		ExtensionTypes ExtensionType;
		std::vector<uint8_t> ExtensionData; //max 2^16-1 i längd
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
	enum TLS1_2AlertLevel : uint8_t { warning = 1, fatal = 2/*max en char i längd*/ };
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
		//(255) max en char i längd
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
		finished = 20// (255), max en char i längd
	};
	struct HandShake
	{
		HandshakeType MessageType;
		//uint24 data length, får tänka på hur jag ska göra den
		//sen så inhåller den datan från respektive handshake typ
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
		enum CompressionMethod { nullCompressionMethod = 0 };//char i längd
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
		uint8_t					PreMasterSecret[48];
		uint8_t					master_secret[48];
		uint8_t					client_random[32];
		uint8_t					server_random[32];
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
	};

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
		std::vector<Extension> OptionalExtensions; //kan tillskillnad från dem andra vara helt tom, max 2^16-1 bytes
	};
	struct TLS1_2ServerCertificate
	{
		std::vector<Certificate> CertificateList;
	};
	struct TLS1_2ClientKeyExchangeMessage
	{
		//innehåller beror helt och hållet på vilka algoritmer vi använder
	};
	struct TLS1_2RSAPremasterSecret
	{
		/*If RSA is being used for key agreement and authentication, the
		  client generates a 48-byte premaster secret, encrypts it using the
		  public key from the server's certificate, and sends the result in
		  an encrypted premaster secret message.  This structure is a
		  variant of the ClientKeyExchange message and is not a message in
		  itself.*/
		  //sjka tdydligen vara den variation man först föreslog av tls, inte den som blev?
		uint16_t ClientProtocolVersion;
		uint8_t RandomBytes[46];
	};
	struct FinishedMessage
	{
		std::vector<uint8_t> VerifyData; //verifydatalength i längd
		//verify data = PRF(master_secret, finished_label, Hash(handshake_messages))[0..verify_data_length - 1] ;
		//får kolla upp exakt i dokumentationen vad det innebär
	};
	struct TLS1_2PlainText
	{
		ContentType TypeOfContent;
		ProtocolVersion Protocol;
		uint16_t DataLength;
		std::vector<uint8_t> Data;
		//får max inehålla 2^14 antal chars
	};
	struct TLS1_2CompressedPlaintext
	{
		ContentType TypeOfContent;
		ProtocolVersion Protocol;
		uint16_t DataLength;
		std::vector<uint8_t> Data;
		//datan från en plaintext meddelandfe cryptearad
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
		std::vector<uint8_t> SessionId; //max 32 chars
		std::vector<CipherSuite> CipherSuites;//minst 2, max 2^16-2 bytes
		std::vector<uint8_t> CompressionMethods; // max 2^8-1 bytes
		std::vector<Extension> OptionalExtensions; //kan tillskillnad från dem andra vara helt tom, max 2^16-1 bytes
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
				//tar bort den största bitten
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
		//TODO: Ha en check som avgör om det är litel endian eller inte, så vi kan ändra beroende på 
		size_t ReturnValue = 0;
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
		std::vector<uint8_t> HostName;//beror egebtkugeb på typ, men existerar än så länge bara 1
	};
	struct TLS1_2GenericRecord
	{
		ContentType Type;
		ProtocolVersion Protocol;
		uint16_t Length;
		std::string Data;
	};
}
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
}
class TLSHandler
{
private:
	RSAPublicKey ExtractRSAPublicKeyFromBitString(std::string& BitString);
	MrBigInt OS2IP(const char* Data, uint64_t LengthOfData);
	MrBigInt RSAEP(TLSServerPublickeyInfo& RSAInfo, MrBigInt const& MessageRepresentative);
	std::string I2OSP(MrBigInt NumberToConvert, uint64_t LengthOfString);
	std::string RSAES_PKCS1_V1_5_ENCRYPT(TLSServerPublickeyInfo& RSAInfo, std::string& DataToEncrypt);
	void SendClientKeyExchange(TLSServerPublickeyInfo& Data, MBSockets::Socket* SocketToConnect);
	TLS1_2::SecurityParameters ConnectionParameters;
	std::string XORedString(std::string String1, std::string String2);
	std::string HMAC_SHA256(std::string Secret, std::string Seed);
	std::string P_Hash(std::string Secret, std::string Seed, uint64_t AmountOfData);
	std::string PRF(std::string Secret, std::string Label, std::string Seed, uint64_t AmountOfData);
public:
	TLSHandler();
	void EstablishTLSConnection(MBSockets::ConnectSocket* SocketToConnect);
	std::string GenerateTLS1_2ClientHello(MBSockets::ConnectSocket* SocketToConnect);
	std::vector<std::string> GetCertificateList(std::string& AllCertificateData);
	void SendClientChangeCipherMessage(MBSockets::ConnectSocket* SocketToConnect);
	void GenerateKeys();
	std::string GetEncryptedRecord(TLS1_2::TLS1_2GenericRecord& RecordToEncrypt);
	std::string GenerateVerifyDataMessage();
	void InitiateHandShake(MBSockets::ConnectSocket* SocketToConnect);
	bool VerifyFinishedMessage(std::string DataToVerify);
	bool VerifyMac(std::string Hash, TLS1_2::TLS1_2GenericRecord RecordToEncrypt);
	std::string DecryptBlockcipherRecord(std::string Data);
	void SendDataAsRecord(std::string& Data, MBSockets::Socket* AssociatedSocket);
	std::string GetApplicationData(MBSockets::Socket* AssociatedSocket);
	std::vector<std::string> GetNextPlaintextRecords(MBSockets::ConnectSocket* SocketToConnect);
	~TLSHandler();
};