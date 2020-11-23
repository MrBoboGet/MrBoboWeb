//#include <gmpxx.h>
//#include <mpir>
#include <mpirxx.h>
#pragma once
class MrBigInt
{
private:
	mpz_class _UnderlyingImplentation = 0;
public:
	MrBigInt()
	{

	}
	MrBigInt(long IntToInitializeWith)
	{
		_UnderlyingImplentation = IntToInitializeWith;
	}
	MrBigInt(std::string StringToInitializeWith,int Base)
	{
		_UnderlyingImplentation.set_str(StringToInitializeWith.c_str(), Base);
	}

	void SetFromString(std::string StringToInitializeWith, int Base)
	{
		_UnderlyingImplentation.set_str(StringToInitializeWith.c_str(), Base);
	}
	std::string GetString()
	{
		return(_UnderlyingImplentation.get_str());
	}
	MrBigInt PowM(MrBigInt Exponent, MrBigInt Modolu)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass;
		mpz_powm(NewUnderlyingImplemenationClass.get_mpz_t(), _UnderlyingImplentation.get_mpz_t(), Exponent._UnderlyingImplentation.get_mpz_t(), Modolu._UnderlyingImplentation.get_mpz_t());
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt Pow(unsigned long int Exponent)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass;
		mpz_pow_ui(NewUnderlyingImplemenationClass.get_mpz_t(), _UnderlyingImplentation.get_mpz_t(),Exponent);
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}

	MrBigInt operator*(MrBigInt IntToMultiplyWith)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation*IntToMultiplyWith._UnderlyingImplentation;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt operator+(MrBigInt IntToMultiplyWith)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation + IntToMultiplyWith._UnderlyingImplentation;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt operator>>(int BitsToShift)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation>>BitsToShift;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt operator<<(int BitsToShift)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation << BitsToShift;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt operator%(MrBigInt Modolu)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation % Modolu._UnderlyingImplentation;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	int operator%(int Modolu)
	{
		int ReturnValue =  mpz_get_ui(_UnderlyingImplentation.get_mpz_t()) % Modolu;
		return(ReturnValue);
	}
	MrBigInt operator-(MrBigInt IntToMultiplyWith)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation - IntToMultiplyWith._UnderlyingImplentation;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
	MrBigInt operator/(MrBigInt IntToMultiplyWith)
	{
		MrBigInt ReturnValue = 0;
		mpz_class NewUnderlyingImplemenationClass = _UnderlyingImplentation / IntToMultiplyWith._UnderlyingImplentation;
		ReturnValue._UnderlyingImplentation = NewUnderlyingImplemenationClass;
		return(ReturnValue);
	}
};