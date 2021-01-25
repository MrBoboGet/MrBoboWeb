#include <cstdint>
#include <vector>
#include <assert.h>
#include <MrBigInt/MrBigInt.h>
#include <math.h>
#pragma once
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
                             -- If present, version MUST be v2 or v3
        subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
                             -- If present, version MUST be v2 or v3
        extensions      [3]  EXPLICIT Extensions OPTIONAL
                             -- If present, version MUST be v3
        }

    //definitioner av typerna
       Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }

       CertificateSerialNumber  ::=  INTEGER

       Validity ::= SEQUENCE 
       {
            notBefore      Time,
            notAfter       Time 
       }

       Time ::= CHOICE 
       {
            utcTime        UTCTime,
            generalTime    GeneralizedTime 
       }

       UniqueIdentifier  ::=  BIT STRING

       SubjectPublicKeyInfo  ::=  SEQUENCE  
       {
            algorithm            AlgorithmIdentifier,
            subjectPublicKey     BIT STRING  
       }

       Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension

       Extension  ::=  SEQUENCE  
       {
            extnID      OBJECT IDENTIFIER,
            critical    BOOLEAN DEFAULT FALSE,
            extnValue   OCTET STRING
                        -- contains the DER encoding of an ASN.1 value
                        -- corresponding to the extension type identified
                        -- by extnID
      }

        AlgorithmIdentifier  ::=  SEQUENCE  
        {
            algorithm               OBJECT IDENTIFIER,
            parameters              ANY DEFINED BY algorithm OPTIONAL  
        }
*/
class ASN1Type
{
private:

public:
    virtual void ParseStructFrom(uint8_t* DataToParseFrom)
    {

    }
    virtual bool TryParse(uint8_t* DataToParseFrom)
    {

    }
    virtual void InsertData(std::vector<uint8_t> ArrayToInserDataInto)
    {

    }
    ASN1Type()
    {

    }
    ~ASN1Type()
    {

    }
};
class ASN1BitString : ASN1Type
{
private:
public:
    ASN1BitString()
    {

    }
    ~ASN1BitString()
    {

    }
    void ParseStructFrom(uint8_t* DataToParseFrom) override
    {

    }
    bool TryParse(uint8_t* DataToParseFrom) override
    {

    }
    void InsertData(std::vector<uint8_t> ArrayToInserDataInto) override
    {

    }
};
class ASN1Integer : ASN1Type
{
private:
public:
    ASN1Integer()
    {

    }
    ~ASN1Integer()
    {

    }
    void ParseStructFrom(uint8_t* DataToParseFrom) override
    {

    }
    bool TryParse(uint8_t* DataToParseFrom) override
    {

    }
    void InsertData(std::vector<uint8_t> ArrayToInserDataInto) override
    {

    }
};
class ASN1Bool : ASN1Type
{
private:
public:
    ASN1Bool()
    {

    }
    ~ASN1Bool()
    {

    }
};
class ASN1Any : ASN1Type
{
private:
public:
    ASN1Any()
    {

    }
    ~ASN1Any()
    {

    }
};
class ASN1OctetString : ASN1Type
{
private:
public:
    ASN1OctetString()
    {

    }
    ~ASN1OctetString()
    {

    }
};
class ASN1ObjectIdentifier : ASN1Type
{
public:
    ASN1ObjectIdentifier()
    {

    }
    ~ASN1ObjectIdentifier()
    {

    }

private:

};
enum class X509Version : uint8_t
{
    v1 = 0,
    v2 = 1,
    v3 = 3
};
struct X509SignatureAlgoritm
{

};
struct X509CertificateSerialNumber
{
    ASN1Integer Integer;
};
struct X509AlgorithmIdentifier
{
   // AlgorithmIdentifier  :: = SEQUENCE{
    // algorithm               OBJECT IDENTIFIER,
    // parameters              ANY DEFINED BY algorithm OPTIONAL }
    ASN1ObjectIdentifier Algoritm;
    ASN1Any Parameters;
};
struct X509Name
{

};
struct X509Time
{
    //Time :: = CHOICE
    //{
    //     utcTime        UTCTime,
    //     generalTime    GeneralizedTime
    //}
};
struct X509Validity
{
    X509Time NotBefore;
    X509Time NotAfter;
};
struct X509SubjectPublicKeyInfo
{
    X509AlgorithmIdentifier Algorithm;
    ASN1BitString SubjectPublicKey;
};
struct X509UniqueIdentifier
{
    ASN1BitString BitString;
};

//Extensions  :: = SEQUENCE SIZE(1..MAX) OF Extension
//Extension  :: = SEQUENCE
//{
//     extnID      OBJECT IDENTIFIER,
//     critical    BOOLEAN DEFAULT FALSE,
//     extnValue   OCTET STRING
//                 -- contains the DER encoding of an ASN.1 value
//                 -- corresponding to the extension type identified
//                 -- by extnID
//}
struct X509SExtension
{
    ASN1ObjectIdentifier ExtnId;
    ASN1Bool Critical;
    ASN1OctetString ExtnValue;
};
struct X509ListOfExtensions
{
    std::vector<X509SExtension> Extensions; //(1...MAX)
};
struct X509TBSCertificate
{
    //version[0]  EXPLICIT Version DEFAULT v1,
    X509Version Version = X509Version::v1;
    X509CertificateSerialNumber CertificateSerialNumber;
    X509AlgorithmIdentifier AlgorithmIdentifier;
    X509Name Issuer;
    X509Validity Validity;
    X509Name Subject;
    X509SubjectPublicKeyInfo SubjectPublicKeyInfo;
    //issuerUniqueID[1]  IMPLICIT UniqueIdentifier OPTIONAL,
    //    --If present, version MUST be v2 or v3
    //subjectUniqueID[2]  IMPLICIT UniqueIdentifier OPTIONAL,
    //    --If present, version MUST be v2 or v3
    //extensions[3]  EXPLICIT Extensions OPTIONAL
    //    -- If present, version MUST be v3
    X509UniqueIdentifier IssuerUniqueId;
    X509UniqueIdentifier SubjeUniqueId;
    X509ListOfExtensions Extensions;
};
struct X509Certificate
{
    X509TBSCertificate Certificate;
    X509SignatureAlgoritm SignatureAlgoritm;
    ASN1BitString BitString;
};

enum class ASN1TypeTag : uint8_t
{
    Universal,
    Application,
    ContextSpecific,
    Private
};
enum class ASN1PrimitiveTagNumber : uint8_t
{
    Integer = 2,
    BitString = 3,
    OctetString = 4,
    Null = 5,
    ObjectIdentifier = 6,
    Sequence = 16,
    SequenceOf = 16,
    Set = 17,
    SetOf = 17,
    PrintableString = 19,
    T61String = 20,
    IA5String = 22,
    UTCTime = 23,
    NonStandardTag = uint8_t(255)>>3//00011111 alla dem sista är ettor
};
enum class ASN1TagClass : uint8_t
{
    Universal = 0,
    Application = 1,
    ContextSpecific = 1 << 1, //2
    Private = (1 << 1) + 1 //3
};
/*

Class	Bit 8	Bit 7
universal	        0	0
application	        0	1
context-specific	1	0
private	            1	1

*/
struct ASN1TagValue
{
    ASN1TagClass Class;
    bool IsConstructed = false;
    ASN1PrimitiveTagNumber TagType;
    int AlternateTagValue = -1;
};
class ASN1Extracter
{
private:
    uint64_t Offset = 0;
    uint8_t* DataToModify = nullptr;
    const uint8_t* DataToReadFrom = nullptr;
public:
    ASN1Extracter(uint8_t* ModifiyableData)
    {
        DataToModify = ModifiyableData;
        DataToReadFrom = ModifiyableData;
    }
    ASN1Extracter(const uint8_t* DataToRead)
    {
        DataToReadFrom = DataToRead;
    }
    ASN1TagValue ExtractTagData()
    {
        ASN1TagValue ReturnValue;
        uint8_t FirstTagByte = DataToReadFrom[Offset];
        Offset += 1;
        ReturnValue.Class =static_cast<ASN1TagClass>(FirstTagByte >> 6);
        if((FirstTagByte & (1<<6)) == 1)
        {
            ReturnValue.IsConstructed = true;
        }
        ReturnValue.TagType =static_cast<ASN1PrimitiveTagNumber>(FirstTagByte&(uint8_t(255)>>3));
        // om tagtypen inte är nonstandard slutar vi här
        if (ReturnValue.Class == ASN1TagClass::ContextSpecific)
        {
            ReturnValue.AlternateTagValue = uint8_t(ReturnValue.TagType);
        }
        if (ReturnValue.TagType == ASN1PrimitiveTagNumber::NonStandardTag)
        {
            //vill ej deala med det här just nu, så vi bara assertar false om det händer och blir ledsen
            assert(false);
        }
        return(ReturnValue);
    }
    uint64_t ExtractLengthOfType()
    {
        uint64_t ReturnValue = 0;
        uint8_t FirstLengthByte = DataToReadFrom[Offset];
        Offset += 1;
        //kollar huruvida det är short eller lång form
        if (FirstLengthByte>>7 == 0)
        {
            //short form, resten av bitarna är det som faktiskt avgör värdet
            ReturnValue = FirstLengthByte;//tar bort den stösrta ibten
        }
        else
        {
            //första bitten avgör hur många bits som följer 
            uint8_t NumberOfLengthBits = FirstLengthByte - 128;
            //tar inte i åtanke att den kan overflowa
            for (int i = NumberOfLengthBits-1; i >= 0; i--)
            {
                //big endian, så den första biten är den största
                ReturnValue += DataToReadFrom[Offset] * pow(2, i * 8);
                Offset += 1;
            }
        }
        return(ReturnValue);
    }
    uint64_t GetOffset()
    {
        return(Offset);
    }
    void SetOffset(uint64_t OffsetToSet)
    {
        Offset = OffsetToSet;
    }
    void SkipToNextField()
    {
        ExtractTagData();
        uint64_t LengthOfDataToSkip = ExtractLengthOfType();
        Offset += LengthOfDataToSkip;
    }
    uint8_t ExtractByte()
    {
        uint8_t ReturnValue = DataToReadFrom[Offset];
        Offset += 1;
        return(ReturnValue);
    }
    ~ASN1Extracter()
    {

    }
};
class Asn1Handler
{
private:

public:
    static void AppendIdentifierOctets(std::vector<uint8_t>& Data,ASN1TypeTag Type, uint64_t TagValue = 0)
    {

    }
	Asn1Handler()
	{

	}
	~Asn1Handler()
	{

	}
};