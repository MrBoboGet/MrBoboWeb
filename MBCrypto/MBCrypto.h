#pragma once
#include <string>
#include <MrBigInt/MrBigInt.h>
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
	class HashObject
	{
	private:
		size_t m_BlockSize = 0;
		void* m_UnderlyingImplementation = nullptr;
	public:
		HashObject(HashFunction AssociatedFunction);
		HashObject(HashObject& ObjectToCopy);
		~HashObject();
		HashObject& operator=(HashObject const&);

		size_t GetBlockSize();
		void AddData(void* Data, size_t LengthOfData);
		void Finalize();
		std::string GetHash();
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
	HashObject GetHashObject(HashFunction AssociatedFunction);
	std::string HashData(std::string const& DataToHash, HashFunction FunctionToUse);
	std::string RSASSA_PKCS1_V1_5_SIGN(std::string const& DataToSign, std::string const& RSAPrivateKeyPath,HashFunction HashToUse);
	std::string EMSA_PKCS1_V1_5_ENCODE(std::string const& MessageData,size_t OutputLength,HashFunction HashFunctionToUse);
	std::string RSASP1(RSAPrivateKey const& PrivateKey,std::string const& BigEndianMessageRepresentative);
	std::string I2OSP(std::string const& BigEndianArray, size_t ResultLength);
	MrBigInt OS2IP(const char* Data, uint64_t LengthOfData);
}