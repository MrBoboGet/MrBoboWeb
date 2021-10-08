//#include <gmpxx.h>
//#include <mpir>
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <time.h>
#include <chrono>
#include <cmath>

#include <assert.h>
#include <MBUtility/MBStrings.h>
//#define __arm__
//#ifdef __arm__
//#include<arm_neon.h>
//#include<arm_acle.h>
#ifdef _MSC_VER
#include <intrin.h>
#elif __GNUC__
#endif
typedef unsigned int UnitType;
#define UNIT_BITS (sizeof(UnitType)*8)
#define UNIT_MAX ((UnitType)(~0))
class MrBigInt
{
private:
	bool IsNegative = false;
	std::vector<UnitType> InternalUnits = std::vector<UnitType>(1,0);


	void ChangeInternalUnitsNumber(size_t NewNumberOfInternalUnits);
	void NegateNumber();
	bool IsZero() const;
	void AddUnsigned(MrBigInt const& NumberToAdd);
	void SubtractUnsigned(MrBigInt const& NumberToSubtract);
	void NaiveMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult);
	static void SubtractWithOffset(MrBigInt& LeftInt, MrBigInt const& NumberToSubtract, unsigned int IndexToStop, unsigned int Offset);
	static void AddWithOffset(MrBigInt& LeftInt, MrBigInt const& NumberToAdd, int IndexToStop, unsigned int Offset);
	static bool UnsignedGreaterThan(MrBigInt const& LeftInt, MrBigInt const& RightInt, unsigned int Offset);
	static bool UnsignedLesserThan(MrBigInt const& LeftInt, MrBigInt const& RightInt, int IndexToStop, int Offset);
	static UnitType ReciproalUnit32(UnitType UnitToInvert);
	// förutsätter att ordet är normaliserat, high bit set
	static void UnsignedDivide_AOP(MrBigInt& IntToDivide, MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient);
	void UnsignedDivide_AOP(MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient) const;
	void UnsignedDivide_MB(MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient) const;
	void UnsignedDivide(MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient) const;
	void Normalize();
#define KARATSUBA_UNIT_COUNT 20
public:
	std::vector<UnitType> GetInternalUnits() const
	{
		return(InternalUnits);
	}
	static void DoubleUnitDivide(UnitType Higher, UnitType Lower, UnitType Divisor, MrBigInt* OutQuotient, UnitType* OutRemainder, UnitType* DivisorReciprocalPointer = nullptr);
	static void SingleLimbDivision(UnitType const* IntToDivideUnits, unsigned int NumberOfUnits, UnitType Divident, MrBigInt& OutQuotient, UnitType& OutRemainder);
	static void KaratsubaMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult);
	static void GetHighLowUnitMultiplication(UnitType LeftUnit, UnitType RightUnit, UnitType& OutHighUnit, UnitType& OutLowUnit);
	static void LongMultiplication_AOP(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult);
	static void LongMultiplication_MB(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult);
	static void LongMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		LongMultiplication_AOP(LeftInt, RightInt, OutResult);
	}
	void SetBitByIndex(size_t Index, char BitValue);
	unsigned int GetBitByIndex(unsigned int Index) const;
	unsigned int GetBitLength() const;
	static void PowM_SlidinWindow(MrBigInt const& Base, MrBigInt const& Exponent, MrBigInt const& Mod, MrBigInt& OutResult);
	static void PowM(MrBigInt const& Base, MrBigInt const& Exponent, MrBigInt const& Mod, MrBigInt& OutResult);
	static void Pow(MrBigInt const& Base, unsigned int Exponent, MrBigInt& OutResult);
	MrBigInt PowM(MrBigInt const& Exponent, MrBigInt const& Mod)
	{
		MrBigInt ReturnValue;
		MrBigInt::PowM_SlidinWindow(*this, Exponent, Mod, ReturnValue);
		return(ReturnValue);
	}
	MrBigInt Pow(unsigned int Exponent)
	{
		MrBigInt ReturnValue;
		Pow(*this, Exponent, ReturnValue);
		return(ReturnValue);
	}
	unsigned int GetNumberOfUnits()
	{
		return(InternalUnits.size());
	}
	void SetFromBigEndianArray(const void* Data, unsigned int NumberOfBytes);
	void SetFromLittleEndianArray(const void* Data, unsigned int NumberOfBytes);
	void SetFromString(std::string StringToInitializeWith, int Base)
	{
		//TODO gör grejer här
		assert(false);
	}
	std::string GetBigEndianArray();

	//conversions
	
	unsigned int GetLowestTerm() const
	{
		return(InternalUnits.front());
	}
	MrBigInt(UnitType Number)
	{
		InternalUnits[0] = Number;
	}
	//implicit conversion
	explicit operator int() const
	{
		return(GetLowestTerm());
	}
	explicit operator char() const
	{
		return(GetLowestTerm());
	}
	MrBigInt()
	{
	}
	MrBigInt(std::string StringToInitializeWith, int Base)
	{
		SetFromString(StringToInitializeWith, 16);
	}


	MrBigInt& operator<<=(unsigned int BitsToShift);
	MrBigInt operator<<(unsigned int BitsToShift) const;
	MrBigInt operator>>(unsigned int BitsToShift);
	MrBigInt& operator%=(MrBigInt const& RightInt);
	MrBigInt& operator*=(const MrBigInt& RightInt);
	MrBigInt& operator/=(MrBigInt const& RightInt);
	MrBigInt& operator+=(const MrBigInt& RightInt);
	MrBigInt& operator-=(MrBigInt const& RightInt);
	bool operator<=(MrBigInt const& OtherInt) const;
	bool operator==(MrBigInt const& OtherInt) const;
	bool operator!=(MrBigInt const& OtherInt) const
	{
		return(!(*this == OtherInt));
	}
	bool operator<(MrBigInt const& OtherInt) const;
	bool operator>(MrBigInt const& RightInt) const
	{
		return(!(*this <= RightInt));
	}
	bool operator>=(MrBigInt const& RightInt) const
	{
		return(!(*this < RightInt));
	}
	MrBigInt operator%(MrBigInt const& RightInt) const
	{
		MrBigInt ThisCopy(*this);
		ThisCopy %= RightInt;
		return(ThisCopy);
	}
	MrBigInt operator*(MrBigInt const& RightInt) const
	{
		//TODO icke naiv implementation
		MrBigInt ThisCopy(*this);
		ThisCopy *= RightInt;
		return(ThisCopy);
	}
	MrBigInt operator+(MrBigInt const& RightInt) const
	{
		MrBigInt ThisCopy(*this);
		ThisCopy += RightInt;
		return(ThisCopy);
	}
	MrBigInt operator-(MrBigInt const& RightInt) const
	{
		MrBigInt ThisCopy(*this);
		ThisCopy -= RightInt;
		return(ThisCopy);
	}
	MrBigInt operator/(MrBigInt const& RightInt) const
	{
		MrBigInt ThisCopy(*this);
		ThisCopy /= RightInt;
		return(ThisCopy);
	}
	std::string GetString() const;
	std::string GetHexEncodedString() const;
	std::string GetBinaryString() const;
	std::string GetLittleEndianString() const;
};
//*/