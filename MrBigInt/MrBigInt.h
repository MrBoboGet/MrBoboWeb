//#include <gmpxx.h>
//#include <mpir>
#include <mpirxx.h>
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <time.h>
///*
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
	bool operator==(MrBigInt IntToCompare)
	{
		return(_UnderlyingImplentation==IntToCompare._UnderlyingImplentation);
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
//*/
/*
#define INT_BITS sizeof(unsigned int)*8
class MrBigInt
{
private:
	std::vector<unsigned int> InternalUnits = { 0 };
	void ChangeInternalUnitsNumber(unsigned int NewNumberOfInternalUnits)
	{
		//om det är mindre bara gör vi inget
		if (InternalUnits.size() <= NewNumberOfInternalUnits)
		{
			return;
		}
		unsigned int UnitDifference = NewNumberOfInternalUnits - InternalUnits.size();
		InternalUnits.reserve(NewNumberOfInternalUnits);
		if (!IsNegative())
		{
			for (size_t i = 0; i < UnitDifference; i++)
			{
				InternalUnits.push_back(0);
			}
		}
		else
		{
			NegateNumber();
			//eftersom att negeta att number kan öka mängden interna units måste vi göra om unit difference
			UnitDifference = NewNumberOfInternalUnits - InternalUnits.size();
			for (size_t i = 0; i < UnitDifference; i++)
			{
				InternalUnits.push_back(0);
			}
			NegateNumber();
		}
	}
	void NegateNumber()
	{
		//undantagsfallet om talet är positivt
		if (IsZero())
		{
			return;
		}
		//undantagsfallet om talet är negativt
		if (IsMostNegative())
		{
			InternalUnits.push_back(0);
			return;
		}
		for (size_t i = 0; i < InternalUnits.size(); i++)
		{
			InternalUnits[i] = ~InternalUnits[i];
		}
		(*this) += MrBigInt(1);
	}
	bool IsZero()
	{
		for (size_t i = 0; i < InternalUnits.size(); i++)
		{
			if (InternalUnits[i] != 0)
			{
				return(false);
			}
		}
		return(true);
	}
	bool IsMostNegative()
	{
		if (InternalUnits.back() != (1<<(INT_BITS-1)))
		{
			return(false);
		}
		for (size_t i = 0; i < InternalUnits.size()-1; i++)
		{
			if (InternalUnits[i] == 0)
			{
				return(false);
			}
		}
		return(true);
	}
public:
	bool IsNegative()
	{
		return(InternalUnits.back() >> (INT_BITS - 1) == 1);
	}
	//conversions
	MrBigInt(int& Number)
	{
		InternalUnits[0] = Number;
	}
	MrBigInt(int Number)
	{
		InternalUnits[0] = Number;
	}
	MrBigInt& operator*=(MrBigInt& RightInt)
	{

	}
	MrBigInt& operator+=(MrBigInt&& RightInt)
	{
		//temporärt värde så vi vet att vi inte behöver kopiera den
		bool WasNegative = false;
		unsigned int MaxComponents = std::max(this->InternalUnits.size(), RightInt.InternalUnits.size());
		//addition när båda har samma bit representation blir korrekt, om dem har olika tecken, men om det annars blir overflow så behöver vi utöka dem
		ChangeInternalUnitsNumber(MaxComponents);
		RightInt.ChangeInternalUnitsNumber(MaxComponents);
		if (IsNegative() == RightInt.IsNegative())
		{
			//kollar huruvida det blir overflow
			if (IsNegative())
			{
				WasNegative = true;
				NegateNumber();
				RightInt.NegateNumber();
				MaxComponents = std::max(this->InternalUnits.size(), RightInt.InternalUnits.size());
				ChangeInternalUnitsNumber(MaxComponents);
				RightInt.ChangeInternalUnitsNumber(MaxComponents);
			}
			//konservativ algoritm som extendar om det skulle bli problem om talen var negativa, eftersom talen aldrig har sin mest signifikanta bit ifylld blir det aldrig overflow
			unsigned int MostSignificantSum = RightInt.InternalUnits.back() + InternalUnits.back() + 1;
			if (MostSignificantSum >= (1<<(INT_BITS-1)))
			{
				ChangeInternalUnitsNumber(MaxComponents+1);
				RightInt.ChangeInternalUnitsNumber(MaxComponents+1);
			}
		}
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (size_t i = 0; i < MaxComponents; i++)
		{
			char NewCarry = 0;
			unsigned int HighestBit = 1 << (INT_BITS - 1);
			if((InternalUnits[i] >= HighestBit && RightInt.InternalUnits[i] >= HighestBit) || (InternalUnits[i]+RightInt.InternalUnits[i]+Carry == 0))
			{
				NewCarry = 1;
			}
			InternalUnits[i] += RightInt.InternalUnits[i] + Carry;
			Carry = NewCarry;
		}
		if (WasNegative)
		{
			NegateNumber();
		}

	}
	MrBigInt& operator-=(MrBigInt& RightInt)
	{

	}
	MrBigInt& operator/=(MrBigInt& RightInt)
	{

	}
	MrBigInt operator*(MrBigInt RightInt)
	{

	}
	MrBigInt operator+(MrBigInt RightInt)
	{

	}
	MrBigInt operator-(MrBigInt RightInt)
	{

	}
	MrBigInt operator/(MrBigInt RightInt)
	{

	}
	std::string ToString()
	{

	}
};
*/
typedef unsigned int UnitType;
#define UNIT_BITS (sizeof(UnitType)*8)
#define UNIT_MAX ((UnitType)(~0))
class MrBigInt2
{
private:
	bool IsNegative = false;
	std::vector<UnitType> InternalUnits = std::vector<UnitType>(1,0);
	void ChangeInternalUnitsNumber(unsigned int NewNumberOfInternalUnits)
	{
		//om det är mindre bara gör vi inget
		if (InternalUnits.size() >= NewNumberOfInternalUnits)
		{
			return;
		}
		unsigned int UnitDifference = NewNumberOfInternalUnits - InternalUnits.size();
		InternalUnits.reserve(NewNumberOfInternalUnits);
		for (size_t i = 0; i < UnitDifference; i++)
		{
			InternalUnits.push_back(0);
		}
	}
	void NegateNumber()
	{
		IsNegative = !IsNegative;
	}
	bool IsZero() const
	{
		for (size_t i = 0; i < InternalUnits.size(); i++)
		{
			if (InternalUnits[i] != 0)
			{
				return(false);
			}
		}
		return(true);
	}
	void AddUnsigned(MrBigInt2 const& NumberToAdd)
	{
		UnitType LeftSize = InternalUnits.size();
		UnitType RightSize = NumberToAdd.InternalUnits.size();
		unsigned int MaxComponents = std::max(LeftSize, RightSize);
		ChangeInternalUnitsNumber(MaxComponents);
		//NumberToAdd.ChangeInternalUnitsNumber(MaxComponents);
		
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (size_t i = 0; i < MaxComponents; i++)
		{
			char NewCarry = 0;
			UnitType HighestBit = 1 << (UNIT_BITS - 1);
			assert(i < InternalUnits.size());
			UnitType ThisValue = InternalUnits[i];
			UnitType RightValue = 0;
			if (i < RightSize)
			{
				RightValue = NumberToAdd.InternalUnits[i];
			}
			if ((UnitType)(ThisValue + RightValue+Carry) < ThisValue || (UnitType)(ThisValue + RightValue + Carry) < RightValue)
			{
				NewCarry = 1;
			}
			InternalUnits[i] += RightValue+ Carry;
			Carry = NewCarry;
		}
		if (Carry == 1)
		{
			InternalUnits.push_back(1);
		}
	}
	void SubtractUnsigned(MrBigInt2 const& NumberToSubtract)
	{
		//förutsätter att vänsterledet alltid är positivt och högerledet negativt
		unsigned int MaxComponents = std::max(this->InternalUnits.size(), NumberToSubtract.InternalUnits.size());
		ChangeInternalUnitsNumber(MaxComponents);
		UnitType RightSize = NumberToSubtract.InternalUnits.size();
		//NumberToSubtract.ChangeInternalUnitsNumber(MaxComponents);
		
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (size_t i = 0; i < MaxComponents; i++)
		{
			//först och främst så subtraherar vi med carry, är talet UNIT_MAX sätter vi nya carryn till 1 igen
			char NewCarry = 0;
			InternalUnits[i] -= Carry;
			if (InternalUnits[i] == UNIT_MAX)
			{
				NewCarry = 1;
			}
			UnitType ThisValue = InternalUnits[i];
			UnitType RightValue = 0;
			if (i < RightSize)
			{
				RightValue = NumberToSubtract.InternalUnits[i];
			}

			//subtraheringen
			if (i == MaxComponents-1)
			{
				//sista gången, specialfall
				if (ThisValue < RightValue)
				{
					InternalUnits[i] = RightValue - ThisValue;
					IsNegative = true;
				}
				else
				{
					InternalUnits[i] = ThisValue-RightValue;
					IsNegative = false;
				}
			}
			else if (ThisValue < RightValue)
			{
				//kommer overflowa
				NewCarry = 1;
				unsigned int NegativeNumber = RightValue - ThisValue;
				InternalUnits[i] = (UNIT_MAX - NegativeNumber) + 1;
			}
			else
			{
				InternalUnits[i] -= RightValue;
			} 
			Carry = NewCarry;
		}
		//TODO borde man ta bort leading nollor för att göra det aningen mer minnes effektivt och ha färre special fall?
		for (int i = InternalUnits.size()-1; i >= 1; i--)
		{
			if (InternalUnits[i] == 0)
			{
				InternalUnits.pop_back();
			}
		}
	}
	void NaiveMultiplication(MrBigInt2 const& LeftInt, MrBigInt2 const& RightInt, MrBigInt2& OutResult)
	{
		MrBigInt2 Iterator;
		OutResult = MrBigInt2(0);
		while (Iterator < RightInt)
		{
			OutResult += (*this);
			Iterator += MrBigInt2(1);
		}
	}
	void LongMultiplication(MrBigInt2 const& LeftInt, MrBigInt2 const& RightInt, MrBigInt2& OutResult)
	{
		OutResult = MrBigInt2(0);
		unsigned int RightSize = RightInt.InternalUnits.size();
		for (size_t i = 0; i < RightSize; i++)
		{
			UnitType CurrentUnit = RightInt.InternalUnits[i];
			for (size_t j = 0; j < UNIT_BITS; j++)
			{
				char CurrentBit = (CurrentUnit >> j)%2;
				if (CurrentBit == 1)
				{
					OutResult += LeftInt << (i* UNIT_BITS + j);
				}
			}
		}
	}
	void KaratsubaMultiplication(MrBigInt2 const& LeftInt, MrBigInt2 const& RightInt, MrBigInt2& OutResult)
	{
		MrBigInt2 B = MrBigInt2(1) << (UNIT_BITS);
		unsigned int MaxSize = std::max(LeftInt.InternalUnits.size(), RightInt.InternalUnits.size());
		unsigned int m = MaxSize / 2;
		if (MaxSize%2 == 1)
		{
			m += 1;
		}
		unsigned int LeftSize = LeftInt.InternalUnits.size();
		unsigned int RightSize = RightInt.InternalUnits.size();
		MrBigInt2 x0;
		MrBigInt2 x1;
		MrBigInt2 y0;
		MrBigInt2 y1;
		for (size_t i = 0; i < m; i++)
		{
			if (i < x0.InternalUnits.size())
			{
				x0.InternalUnits[i] = LeftInt.InternalUnits[i];
			}
			else
			{
				x0.InternalUnits.push_back(LeftInt.InternalUnits[i]);
			}

		}



		MrBigInt2 z2;
		MrBigInt2 z1;
		MrBigInt2 z0;
	}
	void UnsignedDivide(MrBigInt2 const& Divident, MrBigInt2& OutRemainder,MrBigInt2& OutQuotient) const
	{
		OutRemainder = MrBigInt2();
		OutQuotient = MrBigInt2();
		//unsigned int MaxComponents = std::max(this->InternalUnits.size(), Divident.InternalUnits.size());
		unsigned int ThisSize = InternalUnits.size();
		unsigned int MaxSize = std::max(InternalUnits.size(), Divident.InternalUnits.size());
		
		
		//ChangeInternalUnitsNumber(MaxComponents);
		//Divident.ChangeInternalUnitsNumber(MaxComponents);
 		for (int i = ThisSize-1; i >= 0; i--)
		{
			for (int j = UNIT_BITS-1; j >= 0; j--)
			{
				OutRemainder = OutRemainder << 1;
				OutRemainder += (InternalUnits[i]>>j)&1;
				if (Divident <= OutRemainder)
				{
					OutRemainder -= Divident;
					OutQuotient += MrBigInt2(1) << ((i * UNIT_BITS) + j);
				}
			}
		}
	}
public:
	static void PowM(MrBigInt2 const& Base, MrBigInt2 const& Exponent, MrBigInt2 Mod,MrBigInt2& OutResult)
	{
		OutResult = MrBigInt2(1);
		MrBigInt2 CurrentBase = MrBigInt2(Base);
		MrBigInt2 CurrentExponent = MrBigInt2(Exponent);
		MrBigInt2 Zero = MrBigInt2(0);
		MrBigInt2 One = MrBigInt2(1);
		MrBigInt2 OutMultiple = MrBigInt2(1);
		unsigned int ExponentDivisorInt = 16;
		MrBigInt2 ExponentDivisor = MrBigInt2(ExponentDivisorInt);
		while (Zero < CurrentExponent)
		{
			unsigned int TimesBeforDivisible = (CurrentExponent % MrBigInt2(ExponentDivisor)).GetLowestTerm();
			for (size_t i = 0; i < TimesBeforDivisible; i++)
			{
				OutMultiple *= CurrentBase;
				CurrentExponent -= One;
			}
 			if (CurrentExponent == MrBigInt2(0))
			{
				CurrentBase %= Mod;
				OutMultiple %= Mod;
				break;
			}
			CurrentExponent /= ExponentDivisor;
			MrBigInt2 BaseCopy = MrBigInt2(CurrentBase);
			for (size_t i = 0; i < ExponentDivisorInt-1; i++)
			{
				CurrentBase *= BaseCopy;
			}
			CurrentBase %= Mod;
			OutMultiple %= Mod;
		}
		OutResult = OutMultiple;
	}
	MrBigInt2 PowM(MrBigInt2 const& Exponent, MrBigInt2 const& Mod)
	{
		MrBigInt2 ReturnValue;
		MrBigInt2::PowM(*this, Exponent, Mod, ReturnValue);
		return(ReturnValue);
	}
	MrBigInt2 PowM(unsigned int Exponent)
	{
		MrBigInt2 ReturnValue = MrBigInt2(1);
		for (size_t i = 0; i < Exponent; i++)
		{
			ReturnValue *= *this;
		}
		return(ReturnValue);
	}
	unsigned int GetNumberOfUnits()
	{
		return(InternalUnits.size());
	}
	//conversions
	MrBigInt2(int& Number)
	{
		InternalUnits[0] = Number;
		if (Number < 0)
		{
			IsNegative = true;
		}
	}
	MrBigInt2(int Number)
	{
		InternalUnits[0] = Number;
		if (Number < 0)
		{
			IsNegative = true;
		}
	}
	MrBigInt2()
	{
	}
	MrBigInt2 operator<<(unsigned int BitsToShift) const
	{
		MrBigInt2 NewInt;
		//räknar ut hur stor den måste vara
		unsigned int NewUnitsCount = BitsToShift / UNIT_BITS;
		unsigned int BitToShiftCheck = UNIT_BITS - (BitsToShift % UNIT_BITS);
		if (BitToShiftCheck > UNIT_BITS-1)
		{
			BitToShiftCheck = UNIT_BITS-1;
		}
		unsigned int FirstNonZeroUnit = 0;
		for (int i = InternalUnits.size()-1; i >= 0; i--)
		{
			if (InternalUnits[i] != 0)
			{
				FirstNonZeroUnit = InternalUnits[i];
				break;
			}
		}
		if(FirstNonZeroUnit>>(BitToShiftCheck) != 0)
		{
			NewUnitsCount += 1;
		}
		unsigned int NormalizedNumberOfUnits = InternalUnits.size();
		for (int i = NormalizedNumberOfUnits-1; i >= 0; i--)
		{
			if (InternalUnits[i] == 0 && i != 0)
			{
				NormalizedNumberOfUnits -= 1;
			}
			else
			{
				break;
			}
		}
		NewInt.InternalUnits = std::vector<unsigned int>(NewUnitsCount + NormalizedNumberOfUnits, 0);
		//sätter automatiskt det 
		unsigned int DebugMod = BitsToShift % UNIT_BITS;
		unsigned int DebugShift = 1 << DebugMod;
		NewInt.InternalUnits[BitsToShift / UNIT_BITS] = InternalUnits[0] << (BitsToShift % UNIT_BITS);
		char CurrentUnitBits = BitsToShift%UNIT_BITS;
		unsigned int CurrentUnitIndex = 0;
		if (CurrentUnitBits == 0)
		{
			CurrentUnitIndex += 1;
			CurrentUnitBits = UNIT_BITS;
		}
		for (size_t i = (BitsToShift / UNIT_BITS)+1; i < NewInt.InternalUnits.size(); i++)
		{
			unsigned char CurrentNewUnitProgress = 0;
			if (CurrentUnitIndex == InternalUnits.size())
			{
				break;
			}
			while (CurrentNewUnitProgress != UNIT_BITS)
			{
				unsigned char BitsToTransfer = CurrentUnitBits;
				unsigned int NewBits = (InternalUnits[CurrentUnitIndex] >> (UNIT_BITS - BitsToTransfer));
				if (BitsToTransfer+CurrentNewUnitProgress > UNIT_BITS)
				{ 
					BitsToTransfer = UNIT_BITS - CurrentNewUnitProgress;

					char NewBitsMask;
					if (CurrentUnitBits != UNIT_BITS)
					{
						NewBitsMask = (((unsigned int)~0) << (UNIT_BITS - CurrentUnitBits)) & ((((unsigned int)~0) >> CurrentUnitBits) << BitsToTransfer);
					}
					else
					{
						NewBitsMask = ((unsigned int)~0) >> CurrentNewUnitProgress;
					}
					NewBits = NewBits & NewBitsMask;
				}
				NewInt.InternalUnits[i] += NewBits<<CurrentNewUnitProgress;
				CurrentNewUnitProgress += BitsToTransfer;
				CurrentUnitBits -= BitsToTransfer;
				if (CurrentUnitBits == 0)
				{
					CurrentUnitIndex += 1;
					CurrentUnitBits = UNIT_BITS;
					if (CurrentUnitIndex == InternalUnits.size())
					{
						break;
					}
				}
			}
		}
		return(NewInt);
		/*
		unsigned int PreviousBits = 0;
		for (size_t i = 0; i < NewInt.InternalUnits.size(); i++)
		{
			unsigned int NewBits = (NewInt.InternalUnits[i] & (~(UNIT_MAX >> BitsToShift)))>>(INT_BITS-BitsToShift);
			if (NewBits != 0)
			{
				int Hej = 2;
			}
			NewInt.InternalUnits[i] = (NewInt.InternalUnits[i] << BitsToShift)+PreviousBits;
			PreviousBits = NewBits;
		}
		if (PreviousBits != 0)
		{
			InternalUnits.push_back(PreviousBits);
		}
		return(NewInt);
		*/
	}
	MrBigInt2& operator%=(MrBigInt2 const& RightInt)
	{
		MrBigInt2 Remainder;
		MrBigInt2 Quotient;
		UnsignedDivide(RightInt, Remainder, Quotient);
		IsNegative = false;
		std::swap(Remainder.InternalUnits, InternalUnits);
		return(*this);
	}
	MrBigInt2& operator*=(const MrBigInt2& RightInt)
	{
		MrBigInt2 Result;
		LongMultiplication(*this, RightInt, Result);
		std::swap(Result.InternalUnits, InternalUnits);
		if (IsNegative == RightInt.IsNegative)
		{
			IsNegative = false;
		}
		else
		{
			IsNegative = true;
		}
		return(*this);
	}
	MrBigInt2& operator/=(MrBigInt2 const& RightInt)
	{
		//långsam division, lång division
		MrBigInt2 Remainder;
		MrBigInt2 Quotient;
		UnsignedDivide(RightInt, Remainder, Quotient);
		std::swap(InternalUnits, Quotient.InternalUnits);
		if (IsNegative == RightInt.IsNegative)
		{
			IsNegative = false;
		}
		else
		{
			IsNegative = true;
		}
		return(*this);
	}
	MrBigInt2& operator+=(const MrBigInt2& RightInt)
	{
		if (IsNegative == RightInt.IsNegative)
		{
			AddUnsigned(RightInt);
		}
		else if (IsNegative && RightInt.IsNegative == false)
		{
			//TODO kan optimseras genom att specifia vilket led som är höger och vänster i funktionen
			MrBigInt2 RightCopy = MrBigInt2(RightInt);
			RightCopy.SubtractUnsigned(*this);
			std::swap(InternalUnits, RightCopy.InternalUnits);
			IsNegative = RightCopy.IsNegative;
		}
		if (IsNegative == false && RightInt.IsNegative == true)
		{
			SubtractUnsigned(RightInt);
		}
		return(*this);
	}
	MrBigInt2& operator-=(MrBigInt2 const& RightInt)
	{
		if (!IsNegative && !RightInt.IsNegative)
		{
			SubtractUnsigned(RightInt);
		}
		if (IsNegative && RightInt.IsNegative)
		{
			//vänsterlaedet negativt, högleredet negativt, samma sak som -> VL-HL = HL-abs(VL)
			MrBigInt2 RightIntCopy = MrBigInt2(RightInt);
			RightIntCopy.NegateNumber();
			RightIntCopy.SubtractUnsigned(*this);
			std::swap(RightIntCopy.InternalUnits, InternalUnits);
			IsNegative = RightIntCopy.IsNegative;
		}
		if (!IsNegative && RightInt.IsNegative)
		{
			AddUnsigned(RightInt);
		}
		if (IsNegative && !RightInt.IsNegative)
		{
			AddUnsigned(RightInt);
		}
		//MrBigInt2 RightIntCopy = MrBigInt2(RightInt);
		//RightIntCopy.NegateNumber();
		//(*this) += RightIntCopy;
		return(*this);
	}
	/*
	MrBigInt2& operator+=(MrBigInt2&& RightInt)
	{
		if (IsNegative == RightInt.IsNegative)
		{
			AddUnsigned(RightInt);
		}
		else if (IsNegative && RightInt.IsNegative == false)
		{
			RightInt.SubtractUnsigned(*this);
			std::swap(InternalUnits, RightInt.InternalUnits);
		}
		if (IsNegative == false && RightInt.IsNegative == true)
		{
			SubtractUnsigned(RightInt);
		}
		return(*this);
	}
	*/
	bool operator<=(MrBigInt2 const& OtherInt) const
	{
		if ((*this)<OtherInt)
		{
			return(true);
		}
		if ((*this) == OtherInt)
		{
			return(true);
		}
		return(false);
	}
	bool operator==(MrBigInt2 const& OtherInt) const
	{
		unsigned int OtherIntSize = OtherInt.InternalUnits.size();
		unsigned int ThisSize = InternalUnits.size();
		unsigned int MaxSize = std::max(OtherIntSize, ThisSize);
		for (size_t i = 0; i < MaxSize; i++)
		{
			unsigned int ThisValue = 0;
			unsigned int OtherValue = 0;
			if (i < ThisSize)
			{
				ThisValue = InternalUnits[i];
			}
			if (i < OtherIntSize)
			{
				OtherValue = OtherInt.InternalUnits[i];
			}
			if (ThisValue != OtherValue)
			{
				return(false);
			}
		}
		return(true);
	}
	bool operator!=(MrBigInt2 const& OtherInt) const
	{
		return(!(*this == OtherInt));
	}
	bool operator<(MrBigInt2 const& OtherInt) const
	{
		if (!IsNegative && OtherInt.IsNegative)
		{
			return(false);
		}
		else if (IsNegative && !OtherInt.IsNegative)
		{
			return(true);
		}
		unsigned int MinComponents = std::min(this->InternalUnits.size(), OtherInt.InternalUnits.size());
		unsigned int MaxComponents = std::max(this->InternalUnits.size(), OtherInt.InternalUnits.size());
		if (InternalUnits.size() > MinComponents && (InternalUnits.back() != 0 && OtherInt.InternalUnits.back() != 0))
		{
			return(IsNegative);
		}
		if (InternalUnits.size() < MinComponents && (InternalUnits.back() != 0 && OtherInt.InternalUnits.back() != 0))
		{
			return(!IsNegative);
		}
		for (int i = MaxComponents-1; i >=0; i--)
		{
			unsigned int ThisValue = 0;
			unsigned int OtherValue = 0;
			if (i < InternalUnits.size())
			{
				ThisValue = InternalUnits[i];
			}
			if (i < OtherInt.InternalUnits.size())
			{
				OtherValue = OtherInt.InternalUnits[i];
			}
			if (ThisValue < OtherValue)
			{
				return(!IsNegative);
			}
			if (ThisValue > OtherValue)
			{
				return(IsNegative);
			}
		}
		return(false);
	}
	bool operator>(MrBigInt2 const& RightInt) const
	{
		return(!(*this <= RightInt));
	}
	bool operator>=(MrBigInt2 const& RightInt) const
	{
		return(!(*this < RightInt));
	}
	unsigned int GetLowestTerm() const
	{
		return(InternalUnits.front());
	}
	MrBigInt2 operator%(MrBigInt2 const& RightInt) const
	{
		MrBigInt2 ThisCopy(*this);
		ThisCopy %= RightInt;
		return(ThisCopy);
	}
	MrBigInt2 operator*(MrBigInt2 const& RightInt) const
	{
		//TODO icke naiv implementation
		MrBigInt2 ThisCopy(*this);
		ThisCopy *= RightInt;
		return(ThisCopy);
	}
	MrBigInt2 operator+(MrBigInt2 const& RightInt) const
	{
		MrBigInt2 ThisCopy(*this);
		ThisCopy += RightInt;
		return(ThisCopy);
	}
	MrBigInt2 operator-(MrBigInt2 const& RightInt) const
	{
		MrBigInt2 ThisCopy(*this);
		ThisCopy -= RightInt;
		return(ThisCopy);
	}
	MrBigInt2 operator/(MrBigInt2 const& RightInt) const
	{
		MrBigInt2 ThisCopy(*this);
		ThisCopy /= RightInt;
		return(ThisCopy);
	}
	std::string GetString() const
	{
		MrBigInt2 TenthPotens = 1;
		std::string Temp = "";
		//clock_t TotalTimeTimer = clock();
		double TotalDivideTime = 0;
		while (TenthPotens < (*this))
		{
			MrBigInt2 Quotient;
			MrBigInt2 Remainder;
			//clock_t DivideTimer = clock();
			UnsignedDivide(TenthPotens, *&Remainder, *&Quotient);
			//TotalDivideTime += (clock() - DivideTimer) / (double)CLOCKS_PER_SEC;
			unsigned int ModTen = 0;
			unsigned int ModMultiplier = 1;
			for (size_t i = 0; i < Quotient.InternalUnits.size(); i++)
			{
				unsigned int DebugModTen = ModTen;
				ModTen += (Quotient.InternalUnits[i] % 10)*ModMultiplier;
				ModMultiplier = (ModMultiplier * 6) % 10;
				assert(ModTen >= DebugModTen);
			}
			Temp += std::to_string(ModTen%10);
			TenthPotens = TenthPotens * 10;
		}
		std::string ReturnValue = "";
		for (size_t i = 0; i < Temp.size(); i++)
		{
			ReturnValue += Temp[Temp.size() - 1 - i];
		}
		//double TotalTime = (clock() - TotalTimeTimer) / (double)CLOCKS_PER_SEC;
		//std::cout << "Total time is: " << TotalTime << std::endl << "Total divide Time: " << TotalDivideTime << std::endl << "Percentege divie: " << TotalDivideTime / TotalTime << std::endl;
		return(ReturnValue);
	}
};