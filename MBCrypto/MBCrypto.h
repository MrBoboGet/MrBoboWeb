#pragma once
#include <string>
#include <MrBigInt/MrBigInt.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
namespace MBCrypto
{
	enum class HashFunction
	{
		SHA1,
		SHA256,
		Null
	};
	struct RSAPrivateKey
	{
		std::string BigEndianPrivateExponent = "";
		std::string BigEndianPublicModolu = "";
	};
	struct RSAPublicKey
	{
		std::string BigEndianPublicExponent = "";
		std::string BigEndianPublicModolu = "";
	};
	class HashObject
	{
	private:
		friend void swap(HashObject& LeftObject, HashObject& RightObject);
		size_t m_BlockSize = 0;
		size_t m_DigestSize = 0;
		HashFunction m_UnderlyingFunction = HashFunction::Null;
		void* m_UnderlyingImplementation = nullptr;
	public:
		HashObject();
		HashObject(HashFunction AssociatedFunction);
		HashObject(HashObject& ObjectToCopy);
		HashObject(HashObject const& ObjectToCopy);
		~HashObject();
		HashObject& operator=(HashObject ObjectToCopy);
			
		size_t GetBlockSize();
		size_t GetDigestSize();
		void Restart();
		std::string Hash(std::string const& DataToHash);
		std::string Hash(const void* DataToHash, size_t NumberOfBytes);
		void AddData(const void* Data, size_t LengthOfData);
		std::string Finalize();
	};
	enum class NamedElipticCurve : uint16_t
	{
		sect163k1 = 1,
		sect163r1 = 2, sect163r2 = 3,
		sect193r1 = 4, sect193r2 = 5, sect233k1 = 6,
		sect233r1 = 7, sect239k1 = 8, sect283k1 = 9,
		sect283r1 = 10, sect409k1 = 11, sect409r1 = 12,
		sect571k1 = 13, sect571r1 = 14, secp160k1 = 15,
		secp160r1 = 16, secp160r2 = 17, secp192k1 = 18,
		secp192r1 = 19, secp224k1 = 20, secp224r1 = 21,
		secp256k1 = 22, secp256r1 = 23, secp384r1 = 24,
		secp521r1 = 25, x25519 = 0x1d,
		Null
	};
	struct ElipticCurvePoint
	{
		std::string BigEndianXCoordinate = "";
		std::string BigEndianYCoordinate = "";
	};
	struct ECDHEPrivatePublicKeyPair
	{
		ElipticCurvePoint PublicPoint;
		std::string PrivateInteger = "";
	};
	ElipticCurvePoint DecodeECP_ANS1x92Encoding(std::string const& ANS1x92EncodedPoint);
	class ElipticCurve
	{
	private:
		CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> m_Group;
	public:
		ElipticCurve(NamedElipticCurve);
		ECDHEPrivatePublicKeyPair GetPrivatePublicKeypair();
		std::string CalculateSharedKey(ElipticCurvePoint const& PublicPoint, std::string const& PrivateInteger);
		std::string EncodeECP_ANS1x92Encoding(ElipticCurvePoint const& PointToEncode);
	};
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
	}
	inline std::string Base64ToBinary(std::string const& Base64Data)
	{
		std::string ReturnValue = "";
		unsigned int Base64DataSize = Base64Data.size();
		unsigned int Offset = 0;
		while (Offset < Base64DataSize)
		{
			unsigned int NewBytes = 0;
			char CharactersProcessed = 0;
			for (size_t i = 0; i < 4; i++)
			{
				NewBytes += Base64CharToBinary(Base64Data[Offset]) << (18 - 6 * i);
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
			for (size_t i = 0; i < CharactersProcessed - 1; i++)
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
	std::string LoadPEMBinaryData(std::string const& KeyPath);
	RSAPrivateKey RSALoadPEMPrivateKey(std::string const& KeyPath);
	//HashObject GetHashObject(HashFunction AssociatedFunction);
	std::string HashData(std::string const& DataToHash, HashFunction FunctionToUse);

	std::string RSASSA_PKCS1_V1_5_SIGN(std::string const& DataToSign, std::string const& RSAPrivateKeyPath,HashFunction HashToUse);
	
	RSAPublicKey ParsePublicKeyDEREncodedData(std::string const& DataToParse);
	bool RSASSA_PKCS1_V1_5_VERIFY(std::string const& DataToVerify,std::string const& CorrectHash, std::string const& RSAPublicKeyPath,HashFunction HashToUse);
	bool RSASSA_PKCS1_V1_5_VERIFY(std::string const& DataToVerify,std::string const& CorrectHash, RSAPublicKey const& AssociatedPublicKey,HashFunction HashToUse);
	std::string EMSA_PKCS1_V1_5_ENCODE(std::string const& MessageData,size_t OutputLength,HashFunction HashFunctionToUse);
	std::string RSASP1(RSAPrivateKey const& PrivateKey,std::string const& BigEndianMessageRepresentative);
	std::string I2OSP(std::string const& BigEndianArray, size_t ResultLength);
	MrBigInt OS2IP(const char* Data, uint64_t LengthOfData);
}