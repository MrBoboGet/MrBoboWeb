#include "MBCrypto.h"
#include <assert.h>
#include <fstream>
#include <filesystem>
//TODO Vi vill helst bli av med denna depenadncy, exempelvis genom att optimera min klass
#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>
#include <cryptopp/algebra.h>
#include <cryptopp/hashfwd.h>
#include <cryptopp/xed25519.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/cbcmac.h>

//DEBUG GREJER
#include <MBUtility/MBStrings.h>
//
#include "Asn1Handlers.h"
namespace MBCrypto
{
	//crypto pp grejer
	CryptoPP::Integer h_CryptoPPFromBigEndianArray(std::string const& BigEndianArray)
	{
		return(CryptoPP::Integer((const CryptoPP::byte*)BigEndianArray.c_str(), BigEndianArray.size(), CryptoPP::Integer::UNSIGNED, CryptoPP::BIG_ENDIAN_ORDER));
	}
	std::string h_CryptoPPToString(CryptoPP::Integer const& IntegerToConvert)
	{
		//std::cout << IntegerToConvert.ByteCount() << std::endl;
		std::string ReturnValue = std::string(IntegerToConvert.MinEncodedSize(),0);
		IntegerToConvert.Encode((CryptoPP::byte*)ReturnValue.c_str(), IntegerToConvert.MinEncodedSize());
		//assert(IntegerToConvert == h_CryptoPPFromBigEndianArray(ReturnValue));
		return(ReturnValue);
	}
	
	//END Cryptopp grejer



	//BEGIN InternalClasses
	template<typename T> 
	class i_CryptoPP_BlockCipher_GCM : public Generic_BlockCipher_GCM
	{
	private:
		//TODO bruh bruh bruh har ingen aning om varför det här behövs eller hur det fungerar
		//typename CryptoPP::GCM<T>::Encryption m_CryptoPPEncryptor;
		//typename CryptoPP::GCM<T>::Decryption m_CryptoPPDecryptor;
		MBError m_LastError = true;
	public:
		i_CryptoPP_BlockCipher_GCM()
		{

		}
		std::unique_ptr<Generic_BlockCipher_GCM> Clone() const override
		{
			i_CryptoPP_BlockCipher_GCM<T>* NewObject = new i_CryptoPP_BlockCipher_GCM<T>();
			std::unique_ptr<Generic_BlockCipher_GCM> ReturnValue = std::unique_ptr<Generic_BlockCipher_GCM>(NewObject);
			//NewObject->m_CryptoPPEncryptor = this->m_CryptoPPEncryptor;
			//NewObject->m_CryptoPPDecryptor = this->m_CryptoPPDecryptor;
			return(ReturnValue);
		}
		std::string DecryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* Nonce, size_t NonceSize,
			const void* AdditonalData, size_t AdditionalDataSize, MBError* OutError = nullptr) override
		{
			std::string ReturnValue = "";
			bool VerificationResult = true;
			if (m_LastError)
			{
				typename CryptoPP::GCM<T>::Decryption Decryptor;
				size_t TagSize = Decryptor.DigestSize();
				std::string Mac = std::string(((const char*)DataToDecrypt) + (DataSize - TagSize),TagSize);
				Decryptor.SetKeyWithIV((CryptoPP::byte*)WriteKey, WriteKeySize, (CryptoPP::byte*) Nonce, NonceSize);
				try
				{
					//std::string Mac = Data.substr(Data.size() - TagSize);
					CryptoPP::AuthenticatedDecryptionFilter df(Decryptor, nullptr, CryptoPP::AuthenticatedDecryptionFilter::MAC_AT_BEGIN | CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION, TagSize);

					// The order of the following calls are important
					df.ChannelPut(CryptoPP::DEFAULT_CHANNEL, (CryptoPP::byte*) Mac.data(), Mac.size());
					df.ChannelPut(CryptoPP::AAD_CHANNEL, (CryptoPP::byte*) AdditonalData, AdditionalDataSize);
					//size_t EncryptedDataSize = Data.size() - (5 + ExplicitNonce.size()) - TagSize;
					//std::string DEBUG_ENCData = std::string(&Data.data()[CipherOffset], EncryptedDataSize);
					df.ChannelPut(CryptoPP::DEFAULT_CHANNEL, (CryptoPP::byte*) DataToDecrypt, DataSize-TagSize);

					// If the object throws, it will most likely occur
					//   during ChannelMessageEnd()
					df.ChannelMessageEnd(CryptoPP::AAD_CHANNEL);
					df.ChannelMessageEnd(CryptoPP::DEFAULT_CHANNEL);

					// If the object does not throw, here's the only
					//  opportunity to check the data's integrity
					bool b = false;
					b = df.GetLastResult();
					assert(true == b);
					VerificationResult = b;

					df.SetRetrievalChannel(CryptoPP::DEFAULT_CHANNEL);
					size_t NumberOfCharacters = (size_t)df.MaxRetrievable();
					ReturnValue.resize(NumberOfCharacters);
					if (NumberOfCharacters > 0)
					{
						df.Get((CryptoPP::byte*)ReturnValue.data(), ReturnValue.size());
					}
				}
				catch (const CryptoPP::Exception& e)
				{
					ReturnValue = "";
					VerificationResult = false;
					m_LastError = false;
					m_LastError.ErrorMessage = "Error in decrypting GCM data: " +e.GetWhat();
				}
			}
			if (!VerificationResult)
			{
				if (OutError != nullptr)
				{
					*OutError = false;
					OutError->ErrorMessage = "Verification failed";
				}
			}
			if (!m_LastError)
			{
				if (OutError != nullptr)
				{
					*OutError = m_LastError;
				}
			}
			return(ReturnValue);
		}
		std::string EncryptData(const void* DataToEncrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* Nonce, size_t NonceSize,
			const void* AdditonalData, size_t AdditionalDataSize, MBError* OutError = nullptr) override
		{
			std::string ReturnValue = "";
			typename CryptoPP::GCM<T>::Encryption CryptoPPEncryptor;

			CryptoPPEncryptor.SetKeyWithIV((const CryptoPP::byte*)WriteKey, WriteKeySize, (const CryptoPP::byte*)Nonce, NonceSize);
			
			CryptoPP::AuthenticatedEncryptionFilter AEADFilter(CryptoPPEncryptor, new CryptoPP::StringSink(ReturnValue), false,CryptoPPEncryptor.DigestSize());
			AEADFilter.ChannelPut(CryptoPP::AAD_CHANNEL, (const CryptoPP::byte*) AdditonalData, AdditionalDataSize);
			AEADFilter.ChannelMessageEnd(CryptoPP::AAD_CHANNEL);
			AEADFilter.ChannelPut(CryptoPP::DEFAULT_CHANNEL, (const CryptoPP::byte*) DataToEncrypt, DataSize);
			AEADFilter.ChannelMessageEnd(CryptoPP::DEFAULT_CHANNEL);

			return(ReturnValue);
		}
		~i_CryptoPP_BlockCipher_GCM() override
		{

		}
	};

	template<typename T>
	class i_CryptoPP_BlockCipher_CBC : public Generic_BlockCipher_CBC
	{
	private:
		//typename CryptoPP::CBC_Mode<T>::Encryption m_CryptoPPEncryptor;
		//typename CryptoPP::CBC_Mode<T>::Decryption m_CryptoPPDecryptor;
	public:
		i_CryptoPP_BlockCipher_CBC()
		{

		}
		std::unique_ptr<Generic_BlockCipher_CBC> Clone() const override
		{
			i_CryptoPP_BlockCipher_CBC<T>* NewObject = new i_CryptoPP_BlockCipher_CBC<T>();
			std::unique_ptr<i_CryptoPP_BlockCipher_CBC> ReturnValue = std::unique_ptr<i_CryptoPP_BlockCipher_CBC>(NewObject);
			//NewObject->m_CryptoPPEncryptor = this->m_CryptoPPEncryptor;
			//NewObject->m_CryptoPPDecryptor = this->m_CryptoPPDecryptor;
			return(ReturnValue);
		}
		std::string DecryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* IV, size_t IVSize, MBError* OutError) override
		{
			//m_CryptoPPDecryptor.setKeyWithIv();
			std::string ReturnValue = "";
			if (m_LastError)
			{
				try
				{
					typename CryptoPP::CBC_Mode<T>::Decryption CryptoPPDecryptor;
					CryptoPPDecryptor.SetKeyWithIV((const CryptoPP::byte*)WriteKey, WriteKeySize, (const CryptoPP::byte*)IV,IVSize);
					CryptoPP::StringSource ss((const CryptoPP::byte*)DataToDecrypt, DataSize, true, new CryptoPP::StreamTransformationFilter(CryptoPPDecryptor, new CryptoPP::StringSink(ReturnValue),
						CryptoPP::StreamTransformationFilter::NO_PADDING));
				}
				catch (const CryptoPP::Exception& e)
				{
					m_LastError = false;
					m_LastError.ErrorMessage = e.what();
				}
			}
			if (!m_LastError)
			{
				if (OutError != nullptr)
				{
					*OutError = m_LastError;
				}
			}
			return(ReturnValue);
		}
		std::string EncryptData(const void* DataToEncrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* IV, size_t IVSize, MBError* OutError) override
		{
			std::string ReturnValue = "";
			if (m_LastError)
			{
				try
				{
					typename CryptoPP::CBC_Mode<T>::Encryption CryptoPPEncryptor;
					CryptoPPEncryptor.SetKeyWithIV((const CryptoPP::byte*)WriteKey, WriteKeySize, (const CryptoPP::byte*)IV);
					CryptoPP::StringSource ss((const CryptoPP::byte*)DataToEncrypt, DataSize, true, new CryptoPP::StreamTransformationFilter(CryptoPPEncryptor, new CryptoPP::StringSink(ReturnValue),
						CryptoPP::StreamTransformationFilter::NO_PADDING));
				}
				catch (const CryptoPP::Exception& e)
				{
					m_LastError = false;
					m_LastError.ErrorMessage = e.what();
				}
			}
			if (!m_LastError)
			{
				if (OutError != nullptr)
				{
					*OutError = m_LastError;
				}
			}
			return(ReturnValue);
		}
		~i_CryptoPP_BlockCipher_CBC() override
		{

		}
	};
	//END InternalClasses




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
		else if(AssociatedFunction == HashFunction::SHA1)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA1();
			CryptoPP::SHA1* ShaPointer = (CryptoPP::SHA1*) m_UnderlyingImplementation;
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
		else if (m_UnderlyingFunction == MBCrypto::HashFunction::SHA1)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA1();
			*(CryptoPP::SHA1*)m_UnderlyingImplementation = *(CryptoPP::SHA1*)ObjectToCopy.m_UnderlyingImplementation;
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
		else if (m_UnderlyingFunction == MBCrypto::HashFunction::SHA1)
		{
			m_UnderlyingImplementation = new CryptoPP::SHA1();
			*(CryptoPP::SHA1*)m_UnderlyingImplementation = *(CryptoPP::SHA1*)ObjectToCopy.m_UnderlyingImplementation;
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
		else if (m_UnderlyingFunction == HashFunction::SHA1)
		{
			CryptoPP::SHA1* Sha256Pointer = (CryptoPP::SHA1*)m_UnderlyingImplementation;
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
		else if (m_UnderlyingFunction == HashFunction::SHA1)
		{
			CryptoPP::SHA1* Sha256Pointer = (CryptoPP::SHA1*)m_UnderlyingImplementation;
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
		else if (m_UnderlyingFunction == HashFunction::SHA1)
		{
			CryptoPP::SHA1* Sha256Pointer = (CryptoPP::SHA1*)m_UnderlyingImplementation;
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

	//BEGIN BlockCipher_CBC_Handler
	void swap(BlockCipher_CBC_Handler& Left, BlockCipher_CBC_Handler& Right) noexcept
	{
		std::swap(Left.m_InternalImplementation, Right.m_InternalImplementation);
		MBError TempError = Right.m_LastError;
		Right.m_LastError = Left.m_LastError;
		Left.m_LastError = TempError;
	}
	BlockCipher_CBC_Handler::BlockCipher_CBC_Handler(BlockCipher BlockCipherToUse,CBC_PaddingScheme SchemeToUse)
	{
		m_PaddingScheme = SchemeToUse;
		if (BlockCipherToUse == BlockCipher::AES)
		{
			m_BlockSize = 16;
			m_InternalImplementation = std::unique_ptr<Generic_BlockCipher_CBC>(new i_CryptoPP_BlockCipher_CBC<CryptoPP::AES>());
		}
		else
		{
			m_LastError = false;
			m_LastError.ErrorMessage = "Invalid block cipher";
			assert(false);
		}
	}
	BlockCipher_CBC_Handler::BlockCipher_CBC_Handler(BlockCipher_CBC_Handler&& HandlerToSteal) noexcept
	{
		std::swap(*this, HandlerToSteal);
	}
	BlockCipher_CBC_Handler::BlockCipher_CBC_Handler(BlockCipher_CBC_Handler const& HandlerToCopy)
	{
		m_InternalImplementation = HandlerToCopy.m_InternalImplementation->Clone();
		m_LastError = HandlerToCopy.m_LastError;
	}
	BlockCipher_CBC_Handler& BlockCipher_CBC_Handler::operator=(BlockCipher_CBC_Handler HandlerToSteal)
	{
		std::swap(*this,HandlerToSteal);
		return(*this);
	}

	bool BlockCipher_CBC_Handler::IsValid()
	{
		return(m_LastError);
	}
	MBError BlockCipher_CBC_Handler::GetLastError()
	{
		return(m_LastError);
	}

	std::string BlockCipher_CBC_Handler::DecryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* IV, size_t IVSize, MBError* OutError)
	{
		std::string ReturnValue = m_InternalImplementation->DecryptData(DataToDecrypt, DataSize, WriteKey, WriteKeySize, IV, IVSize, OutError);
		if (m_PaddingScheme == CBC_PaddingScheme::TLS1_2)
		{
			ReturnValue.resize(ReturnValue.size() - (ReturnValue.back() + 1));
		}
		else if(m_PaddingScheme  == CBC_PaddingScheme::Null)
		{
			
		}
		else
		{
			assert(false);
		}
		return(ReturnValue);
	}
	std::string BlockCipher_CBC_Handler::EncryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* IV, size_t IVSize, MBError* OutError)
	{
		//TODO optimisera så att inte all data kopieras varje gång...
		if (m_PaddingScheme == CBC_PaddingScheme::TLS1_2)
		{
			assert(false);
			//ANTAGANDE för att inte behöva kopiera all data så utnyttjar vi att MBcrypto gör det ändå, och helt enkelt modifierar den så den blir rätt
			//size_t PaddingNeeded = (m_BlockSize - (DataSize % m_BlockSize))%m_BlockSize;
			//if (PaddingNeeded == 0)
			//{
			//	ReturnValue += std::string(m_BlockSize, m_BlockSize - 1);
			//}
			//else
			//{
			//	ReturnValue += std::string(PaddingNeeded, PaddingNeeded - 1);
			//}
		}
		else if (m_PaddingScheme == CBC_PaddingScheme::Null)
		{

		}
		else
		{
			assert(false);
		}
		std::string ReturnValue = m_InternalImplementation->EncryptData(DataToDecrypt, DataSize, WriteKey, WriteKeySize, IV, IVSize, OutError);
		return(ReturnValue);
	}
	//END BlockCipher_CBC_Handler

	//BEGIN BlockCipher_GCM_Handler
	void swap(BlockCipher_GCM_Handler& Left, BlockCipher_GCM_Handler& Right) noexcept
	{
		std::swap(Left.m_InternalImplementation, Right.m_InternalImplementation);
		MBError TempError = Right.m_LastError;
		Right.m_LastError = Left.m_LastError;
		Left.m_LastError = TempError;
	}
	BlockCipher_GCM_Handler::BlockCipher_GCM_Handler(BlockCipher BlockCipherToUse)
	{
		if (BlockCipherToUse == BlockCipher::AES)
		{
			m_InternalImplementation = std::unique_ptr<Generic_BlockCipher_GCM>(new i_CryptoPP_BlockCipher_GCM<CryptoPP::AES>());
		}
		else
		{
			m_LastError = false;
			m_LastError.ErrorMessage = "Invalid block cipher";
			assert(false);
		}
	}
	BlockCipher_GCM_Handler::BlockCipher_GCM_Handler(BlockCipher_GCM_Handler&& HandlerToSteal) noexcept
	{
		swap(*this, HandlerToSteal);
	}
	BlockCipher_GCM_Handler::BlockCipher_GCM_Handler(BlockCipher_GCM_Handler const& HandlerToCopy)
	{
		m_InternalImplementation = HandlerToCopy.m_InternalImplementation->Clone();
		m_LastError = HandlerToCopy.m_LastError;
	}
	BlockCipher_GCM_Handler& BlockCipher_GCM_Handler::operator=(BlockCipher_GCM_Handler HandlerToSteal)
	{
		swap(*this, HandlerToSteal);
		return(*this);
	}

	bool BlockCipher_GCM_Handler::IsValid()
	{
		return(m_LastError);
	}
	MBError BlockCipher_GCM_Handler::GetLastError()
	{
		return(m_LastError);
	}

	std::string BlockCipher_GCM_Handler::DecryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* Nonce, size_t NonceSize,
		const void* AdditonalData, size_t AdditionalDataSize, MBError* OutError)
	{
		return(m_InternalImplementation->DecryptData(DataToDecrypt, DataSize, WriteKey, WriteKeySize, Nonce, NonceSize, AdditonalData, AdditionalDataSize, OutError));
	}
	std::string BlockCipher_GCM_Handler::EncryptData(const void* DataToDecrypt, size_t DataSize, const void* WriteKey, size_t WriteKeySize, const void* Nonce, size_t NonceSize,
		const void* AdditonalData, size_t AdditionalDataSize, MBError* OutError)
	{
		return(m_InternalImplementation->EncryptData(DataToDecrypt, DataSize, WriteKey, WriteKeySize, Nonce, NonceSize, AdditonalData, AdditionalDataSize, OutError));
	}
	//END BlockCipher_GCM_Handler


	//BEGIN ElipticCurve
	ElipticCurvePoint DecodeECP_ANS1x92Encoding(std::string const& ANS1x92EncodedPoint)
	{
		ElipticCurvePoint ReturnValue;
		size_t CoordinateSize = (ANS1x92EncodedPoint.size() - 1) / 2;
		ReturnValue.BigEndianXCoordinate = ANS1x92EncodedPoint.substr(1, CoordinateSize);
		ReturnValue.BigEndianYCoordinate = ANS1x92EncodedPoint.substr(1 + CoordinateSize, CoordinateSize);
		return(ReturnValue);
	}
	ElipticCurve::ElipticCurve(NamedElipticCurve CurveParameters)
	{
		if (CurveParameters == NamedElipticCurve::secp256r1)
		{
			m_Group.Initialize(CryptoPP::ASN1::secp256r1());
		}
		else
		{
			assert(false);
		}
	}
	ECDHEPrivatePublicKeyPair ElipticCurve::GetPrivatePublicKeypair()
	{
		ECDHEPrivatePublicKeyPair ReturnValue;
		CryptoPP::AutoSeededRandomPool prng;
		CryptoPP::Integer PrivateKey(prng, CryptoPP::Integer::One(), m_Group.GetMaxExponent());
		CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>::Element PublicPoint = m_Group.ExponentiateBase(PrivateKey);
		ReturnValue.PrivateInteger = h_CryptoPPToString(PrivateKey);
		ReturnValue.PublicPoint.BigEndianXCoordinate = h_CryptoPPToString(PublicPoint.x);
		ReturnValue.PublicPoint.BigEndianYCoordinate = h_CryptoPPToString(PublicPoint.y);
		return(ReturnValue);
	}
	std::string ElipticCurve::CalculateSharedKey(ElipticCurvePoint const& PublicPoint, std::string const& PrivateInteger)
	{
		CryptoPP::Integer XCoordinate = h_CryptoPPFromBigEndianArray(PublicPoint.BigEndianXCoordinate);
		CryptoPP::Integer YCoordinate = h_CryptoPPFromBigEndianArray(PublicPoint.BigEndianYCoordinate);
		CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>::Element PointToUse(XCoordinate,YCoordinate);
		CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>::Element ResultPoint = m_Group.GetCurve().Multiply(h_CryptoPPFromBigEndianArray(PrivateInteger), PointToUse);
		//DEBUG TEST
		std::string ReturnValue = h_CryptoPPToString(ResultPoint.x);
		CryptoPP::ECDH<CryptoPP::ECP>::Domain dhA(CryptoPP::ASN1::secp256r1());
		std::string DEBUG_TestValue = std::string(ReturnValue.size(), 0);
		dhA.Agree((CryptoPP::byte*)DEBUG_TestValue.data(),(CryptoPP::byte*) PrivateInteger.data(),(CryptoPP::byte*) EncodeECP_ANS1x92Encoding(PublicPoint).data(), true);
		std::string ReturnValue2 = h_CryptoPPToString(m_Group.ExponentiateElement(PointToUse, h_CryptoPPFromBigEndianArray(PrivateInteger)).x);
		assert(DEBUG_TestValue == ReturnValue);
		assert(ReturnValue == ReturnValue2);
		return(h_CryptoPPToString(ResultPoint.x));
	}
	std::string ElipticCurve::EncodeECP_ANS1x92Encoding(ElipticCurvePoint const& PointToEncode)
	{
		std::string ReturnValue = "\x04";
		size_t CoordinateByteLength =  m_Group.GetCurve().GetField().MaxElementByteLength();
		ReturnValue += I2OSP(PointToEncode.BigEndianXCoordinate, CoordinateByteLength);
		ReturnValue += I2OSP(PointToEncode.BigEndianYCoordinate, CoordinateByteLength);
		return(ReturnValue);
	}

	//END ElipticCurve

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
	std::string GetFileHash(std::string const& FileToHashPath,HashFunction HashFunctionToUse)
	{
		HashObject HashObjectToUse = HashObject(HashFunctionToUse);
		std::ifstream FileToRead = std::ifstream(FileToHashPath, std::ios::in | std::ios::binary);
		const size_t ChunkSize = 4096;
		char Buffer[4096];
		while (true)
		{
			FileToRead.read(Buffer, ChunkSize);
			size_t ReadBytes = FileToRead.gcount();
			HashObjectToUse.AddData(Buffer, ReadBytes);
			if (ReadBytes < ChunkSize)
			{
				break;
			}
		}
		std::string ReturnValue = HashObjectToUse.Finalize();
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
		PemFile = MBUtility::ReplaceAll(PemFile, "\n", "");
		PemFile = MBUtility::ReplaceAll(PemFile, "\r", "");
		//nu måste vi konvertera från detta till binär data
		size_t DataBegin = PemFile.find("-----", 6) + 5;
		size_t DataEnd = PemFile.find("-----", DataBegin);
		std::string Base64Data = PemFile.substr(DataBegin, DataEnd-DataBegin);
		std::string BinaryData = Base64ToBinary(Base64Data);
		return(BinaryData);
	}
	bool RSASSA_PKCS1_V1_5_VERIFY(std::string const& DataToVerify, std::string const& CorrectHash, std::string const& RSAPublicKeyPath, HashFunction HashToUse)
	{
		return(true);
	}
	bool RSASSA_PKCS1_V1_5_VERIFY(std::string const& DataToVerify, std::string const& CorrectHash, RSAPublicKey const& AssociatedPublicKey, HashFunction HashToUse)
	{
		return(true);
	}
	RSAPublicKey ParsePublicKeyDEREncodedData(std::string const& DataToParse)
	{
		RSAPublicKey ReturnValue;
		//parsa primen från datan, först kollar vi att paddingen är 0
		if (DataToParse[0] != 0)
		{
			std::cout << "Fel formatterad public key" << std::endl;
			assert(false);
			return ReturnValue;
		}
		ASN1Extracter Parser(reinterpret_cast<const uint8_t*>(DataToParse.c_str()));
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
			ReturnValue.BigEndianPublicModolu = DataToParse.substr(Parser.GetOffset() + 1, LengthOfPrime - 1);
			for (size_t i = 0; i < LengthOfPrime; i++)
			{
				Parser.ExtractByte();
			}
		}
		//std::cout << PublicKeyPrime.get_str() << std::endl;
		//nu räknar vi ut exponenten
		Parser.ExtractTagData();
		uint64_t LengthOfExponent = Parser.ExtractLengthOfType();
		ReturnValue.BigEndianPublicExponent = DataToParse.substr(Parser.GetOffset(), LengthOfExponent);
		return(ReturnValue);
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
		if (FunctionToUse == HashFunction::SHA1)
		{
			CryptoPP::SHA1 Hasher;
			CryptoPP::byte digest[CryptoPP::SHA1::DIGESTSIZE];
			Hasher.Update((CryptoPP::byte*)DataToHash.data(), DataToHash.size());
			Hasher.Final(digest);
			return(std::string((char*)digest, CryptoPP::SHA1::DIGESTSIZE));
		}
	}
}