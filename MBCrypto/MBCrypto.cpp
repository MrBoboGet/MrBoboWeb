#include "MBCrypto.h"
#include <assert.h>
#include <fstream>
#include <filesystem>
//TODO Vi vill helst bli av med denna depenadncy, exempelvis genom att optimera min klass
#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>
#include <cryptopp/algebra.h>
#include <cryptopp/hashfwd.h>



//DEBUG GREJER
#include <MBStrings.h>
//
#include <MrPostOGet/Asn1Handlers.h>
namespace MBCrypto
{
	//crypto pp grejer
	CryptoPP::Integer h_CryptoPPFromBigEndianArray(std::string const& BigEndianArray)
	{
		return(CryptoPP::Integer((const CryptoPP::byte*)BigEndianArray.c_str(), BigEndianArray.size(), CryptoPP::Integer::UNSIGNED, CryptoPP::BIG_ENDIAN_ORDER));
	}
	std::string h_CryptoPPToString(CryptoPP::Integer const& IntegerToConvert)
	{
		std::string ReturnValue = std::string(IntegerToConvert.MinEncodedSize(),0);
		IntegerToConvert.Encode((CryptoPP::byte*)ReturnValue.c_str(), IntegerToConvert.MinEncodedSize());
		//assert(IntegerToConvert == h_CryptoPPFromBigEndianArray(ReturnValue));
		return(ReturnValue);
	}
	

	//BEGIN HashObject
	void swap(HashObject& LeftObject, HashObject& RightObject)
	{
		std::swap(LeftObject.m_UnderlyingFunction, RightObject.m_UnderlyingFunction);
		std::swap(LeftObject.m_BlockSize, RightObject.m_BlockSize);
		std::swap(LeftObject.m_DigestSize, RightObject.m_DigestSize);
		std::swap(LeftObject.m_UnderlyingImplementation, RightObject.m_UnderlyingImplementation);
	}
	HashObject::HashObject()
	{

	}
	HashObject::HashObject(HashFunction AssociatedFunction)
	{
		if (AssociatedFunction == HashFunction::SHA256)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA256();
			CryptoPP::SHA256* ShaPointer = (CryptoPP::SHA256 *) m_UnderlyingImplementation;
			m_BlockSize = ShaPointer->BlockSize();
			m_DigestSize = ShaPointer->DigestSize();
			m_UnderlyingFunction = AssociatedFunction;
		}
		else
		{
			assert(false);
		}
	}
	HashObject::HashObject(HashObject const& ObjectToCopy)
	{
		m_BlockSize = ObjectToCopy.m_BlockSize;
		m_UnderlyingFunction = ObjectToCopy.m_UnderlyingFunction;
		m_DigestSize = ObjectToCopy.m_DigestSize;
		if (m_UnderlyingFunction == MBCrypto::HashFunction::SHA256)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA256();
			*(CryptoPP::SHA256*)m_UnderlyingImplementation = *(CryptoPP::SHA256*)ObjectToCopy.m_UnderlyingImplementation;
		}
	}
	HashObject::HashObject(HashObject& ObjectToCopy)
	{
		m_BlockSize = ObjectToCopy.m_BlockSize;
		m_UnderlyingFunction = ObjectToCopy.m_UnderlyingFunction;
		m_DigestSize = ObjectToCopy.m_DigestSize;
		if (m_UnderlyingFunction == MBCrypto::HashFunction::SHA256)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA256();
			*(CryptoPP::SHA256*)m_UnderlyingImplementation = *(CryptoPP::SHA256*)ObjectToCopy.m_UnderlyingImplementation;
		}
	}
	HashObject::~HashObject()
	{
		delete m_UnderlyingImplementation;
	}
	HashObject& HashObject::operator=(HashObject ObjectToCopy)
	{
		swap(*this, ObjectToCopy);
		return(*this);
	}
	size_t HashObject::GetBlockSize()
	{
		return(m_BlockSize);
	}
	size_t HashObject::GetDigestSize()
	{
		return(m_DigestSize);
	}
	std::string HashObject::Hash(std::string const& DataToHash)
	{
		return(Hash(DataToHash.data(), DataToHash.size()));
	}
	std::string HashObject::Hash(const void* DataToHash, size_t NumberOfBytes)
	{
		Restart();
		AddData(DataToHash, NumberOfBytes);
		return(Finalize());
	}
	void HashObject::AddData(const void* Data, size_t LengthOfData)
	{
		if (m_UnderlyingFunction == HashFunction::SHA256)
		{
			CryptoPP::SHA256* Sha256Pointer = (CryptoPP::SHA256 *)m_UnderlyingImplementation;
			Sha256Pointer->Update((const CryptoPP::byte*)Data, LengthOfData);
		}
		else
		{
			assert(false);
		}
	}
	void HashObject::Restart()
	{
		if (m_UnderlyingFunction == HashFunction::SHA256)
		{
			CryptoPP::SHA256* Sha256Pointer = (CryptoPP::SHA256*)m_UnderlyingImplementation;
			Sha256Pointer->Restart();
		}
		else
		{
			assert(false);
		}
	}
	std::string HashObject::Finalize()
	{
		std::string ReturnValue = std::string(m_DigestSize,0);
		if (m_UnderlyingFunction == HashFunction::SHA256)
		{
			CryptoPP::SHA256* Sha256Pointer = (CryptoPP::SHA256*)m_UnderlyingImplementation;
			Sha256Pointer->Final((CryptoPP::byte*)ReturnValue.data());
			Sha256Pointer->Restart();
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}

	//END HashObject



	std::string I2OSP(std::string const& BigEndianArray, size_t ResultLength)
	{
		std::string ReturnValue = "";
		//std::cout << MaxValueOfInteger.GetString() << std::endl;
		if (BigEndianArray.size() > ResultLength)
		{
			assert(false);
			return("ERROR");
		}
		else
		{
			for (size_t i = 0; i < ResultLength - BigEndianArray.size(); i++)
			{
				ReturnValue += char(0);
			}
			ReturnValue += BigEndianArray;
			return(ReturnValue);
		}
	}
	MrBigInt OS2IP(const char* Data, uint64_t LengthOfData)
	{
		MrBigInt ReturnValueTest(0);
		ReturnValueTest.SetFromBigEndianArray(Data, LengthOfData);
		return(ReturnValueTest);
	}
	std::string EMSA_PKCS1_V1_5_ENCODE(std::string const& MessageData, size_t OutputLength, HashFunction HashFunctionToUse)
	{
		std::string HashedMessageData = HashData(MessageData, HashFunctionToUse);
		std::cout << "Hex encoded message data: " << std::endl;
		std::cout << MBUtility::ReplaceAll(MBUtility::HexEncodeString(HashedMessageData)," ", "") << std::endl;
		std::string DerEncodedObject = "";
		if (HashFunctionToUse == HashFunction::SHA256)
		{
			//SHA-256: (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20
			const char* FirstBytes = "\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20";
			DerEncodedObject = std::string(FirstBytes,19);
			DerEncodedObject += HashedMessageData;
		}
		else
		{
			assert(false);
		}
		std::string ReturnValue = "";
		ReturnValue += (char)0;
		ReturnValue += (char)1;
		//DerEncodedObject += (char)ASN1PrimitiveTagNumber::OctetString;
		//DerEncodedObject += (char)HashedMessageData.size();
		//DerEncodedObject += HashedMessageData;
		//std::string ReturnValue = "\x00\x01";
		assert(OutputLength - DerEncodedObject.size() - 3 >= 3);
		for (size_t i = 0; i < OutputLength- DerEncodedObject.size()-3; i++)
		{
			ReturnValue += "\xff";
		}
		ReturnValue += (char)0;
		ReturnValue += DerEncodedObject;
		//ReturnValue += DerEncodedObject;
		return(ReturnValue);
	}
	std::string LoadPEMBinaryData(std::string const& KeyPath)
	{
		std::string PemFilePath = KeyPath;
		std::ifstream t(PemFilePath, std::ifstream::in | std::ifstream::binary);
		size_t size = std::filesystem::file_size(PemFilePath);
		std::string PemFileBuffer(size, ' ');
		t.read(&PemFileBuffer[0], size);
		size_t ReadCharacters = t.gcount();
		std::string PemFile(PemFileBuffer.c_str(), ReadCharacters);
		PemFile = ReplaceAll(PemFile, "\n", "");
		PemFile = ReplaceAll(PemFile, "\r", "");
		//nu måste vi konvertera från detta till binär data
		size_t DataBegin = PemFile.find("-----", 6) + 5;
		size_t DataEnd = PemFile.find("-----", DataBegin);
		std::string Base64Data = PemFile.substr(DataBegin, DataEnd-DataBegin);
		std::string BinaryData = Base64ToBinary(Base64Data);
		return(BinaryData);
	}
	RSAPrivateKey RSALoadPEMPrivateKey(std::string const& KeyPath)
	{
		RSAPrivateKey ReturnValue;
		std::string KeyBinaryInfo = LoadPEMBinaryData(KeyPath);
		ASN1Extracter Parser = ASN1Extracter((uint8_t*)KeyBinaryInfo.c_str());
		Parser.ExtractTagData();
		Parser.ExtractLengthOfType();
		Parser.SkipToNextField();
		//nu är vi vid public modolun
		Parser.ExtractTagData();
		unsigned int LengthOfModolu = Parser.ExtractLengthOfType();
		ReturnValue.BigEndianPublicModolu = std::string(&KeyBinaryInfo[Parser.GetOffset()], LengthOfModolu);
		Parser.SetOffset(Parser.GetOffset() + LengthOfModolu);
		Parser.SkipToNextField();
		//inne i piravet exponent fieldet
		Parser.ExtractTagData();
		unsigned int LengthOfPrivateExponent = Parser.ExtractLengthOfType();
		ReturnValue.BigEndianPrivateExponent = std::string(&KeyBinaryInfo[Parser.GetOffset()], LengthOfPrivateExponent);
		return(ReturnValue);
	}
	std::string RSASSA_PKCS1_V1_5_SIGN(std::string const& DataToSign, std::string const& RSAPrivateKeyPath, HashFunction HashToUse)
	{
		size_t k_MessageSize = 256;
		RSAPrivateKey PrivateKey = RSALoadPEMPrivateKey(RSAPrivateKeyPath);
		std::cout << "Public Modolu hex encoded: " << std::endl;
		std::cout << MBUtility::ReplaceAll(MBUtility::HexEncodeString(PrivateKey.BigEndianPublicModolu), " ", "") << std::endl;
		std::cout << "Private exponent hex encoded: " << std::endl;
		std::cout << MBUtility::ReplaceAll(MBUtility::HexEncodeString(PrivateKey.BigEndianPrivateExponent), " ", "") << std::endl;
		std::string BigEndianMessageRepresentative = EMSA_PKCS1_V1_5_ENCODE(DataToSign, k_MessageSize, HashToUse);
		std::string SignedMessage = RSASP1(PrivateKey, BigEndianMessageRepresentative);
		std::string ReturnValue = I2OSP(SignedMessage, k_MessageSize);
		return(ReturnValue);
	}
	std::string RSASP1(RSAPrivateKey  const& PrivateKey, std::string const& BigEndianMessageRepresentative)
	{
		std::string ReturnValue = "";
		CryptoPP::Integer MessageRepresentative = h_CryptoPPFromBigEndianArray(BigEndianMessageRepresentative);
		CryptoPP::Integer PrivateExponent = h_CryptoPPFromBigEndianArray(PrivateKey.BigEndianPrivateExponent);
		CryptoPP::Integer PublicModulu = h_CryptoPPFromBigEndianArray(PrivateKey.BigEndianPublicModolu);
		CryptoPP::ModularArithmetic Modolu(PublicModulu);
		CryptoPP::Integer Result = Modolu.Exponentiate(MessageRepresentative, PrivateExponent);
		return(h_CryptoPPToString(Result));
	}
	std::string HashData(std::string const& DataToHash, HashFunction FunctionToUse)
	{
		if (FunctionToUse == HashFunction::SHA256)
		{
			CryptoPP::SHA256 Hasher;
			CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
			Hasher.Update((CryptoPP::byte*)DataToHash.data(), DataToHash.size());
			Hasher.Final(digest);
			return(std::string((char*)digest, CryptoPP::SHA256::DIGESTSIZE));
		}
	}
}