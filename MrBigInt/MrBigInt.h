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
#include <MinaStringOperations.h>
//bara för debug
/*
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
*/
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
///*
typedef unsigned int UnitType;
#define UNIT_BITS (sizeof(UnitType)*8)
#define UNIT_MAX ((UnitType)(~0))
class MrBigInt
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
	void AddUnsigned(MrBigInt const& NumberToAdd)
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
	void SubtractUnsigned(MrBigInt const& NumberToSubtract)
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
	void NaiveMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		MrBigInt Iterator;
		OutResult = MrBigInt(0);
		while (Iterator < RightInt)
		{
			OutResult += (*this);
			Iterator += MrBigInt(1);
		}
	}
	void LongMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		OutResult = MrBigInt(0);
		unsigned int RightSize = RightInt.InternalUnits.size();
		for (size_t i = 0; i < RightSize; i++)
		{
			UnitType CurrentUnit = RightInt.InternalUnits[i];
			for (size_t j = 0; j < UNIT_BITS; j++)
			{
				char CurrentBit = (CurrentUnit >> j)&1;
				if (CurrentBit == 1)
				{
					OutResult += LeftInt << ((i* UNIT_BITS) + j);
				}
			}
		}
	}
	void KaratsubaMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		MrBigInt B = MrBigInt(1) << (UNIT_BITS);
		unsigned int MaxSize = std::max(LeftInt.InternalUnits.size(), RightInt.InternalUnits.size());
		unsigned int m = MaxSize / 2;
		if (MaxSize%2 == 1)
		{
			m += 1;
		}
		unsigned int LeftSize = LeftInt.InternalUnits.size();
		unsigned int RightSize = RightInt.InternalUnits.size();
		MrBigInt x0;
		MrBigInt x1;
		MrBigInt y0;
		MrBigInt y1;
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



		MrBigInt z2;
		MrBigInt z1;
		MrBigInt z0;
	}
	void UnsignedDivide(MrBigInt const& Divident, MrBigInt& OutRemainder,MrBigInt& OutQuotient) const
	{
		OutRemainder = MrBigInt();
		OutQuotient = MrBigInt();
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
					OutQuotient += MrBigInt(1) << ((i * UNIT_BITS) + j);
				}
			}
		}
	}
public:
	static void PowM(MrBigInt const& Base, MrBigInt const& Exponent, MrBigInt const& Mod,MrBigInt& OutResult)
	{
		OutResult = MrBigInt(1);
		MrBigInt CurrentBase = MrBigInt(Base);
		MrBigInt CurrentExponent = MrBigInt(Exponent);
		MrBigInt Zero = MrBigInt(0);
		MrBigInt One = MrBigInt(1);
		MrBigInt OutMultiple = MrBigInt(1);
		//en idealiserad och aningen avrundad bild av förhållandet visar att om vi betraktar exponentdivisor som x och exponenten som den andra variabeln
		//har vi att exponentdivisor som optimalast är konstant för olika värden på expoenenten givet ett förhållande mellan multiplikationen och divisionen
		//2 har givit bäst resualt än så länge, så vi har kvar den på 2. Kan en annan formula vara mer effektiv?
		//idealiserad tidskomplexitet som bara beror på exponenten och exponent divisor samt förhållandet:
		//Q*Log(ExpDiv,Exp)+ExpDiv*Log(ExpDiv,Exp)+(ExpDiv-Konstant)*Log(ExpDiv,Exp) Kosntanten ska vara en funjktiin och ska vara floor och ceil inblandat
		unsigned int ExponentDivisorInt = 2;
		MrBigInt ExponentDivisor = MrBigInt(ExponentDivisorInt);

		//unsigned int TimesInLoop = 0;
		//unsigned int MultiplicationCount = 0;
		//clock_t Timer = clock();
		//clock_t DeltaDivision = clock();
		//clock_t DeltaMultiplication = clock();
		//clock_t TotalDivisionTime = 0;
		//clock_t TotalMultiplicationTime = 0;
		//clock_t TotalTime;
		while (Zero < CurrentExponent)
		{
			//TimesInLoop++;
			//DeltaDivision = clock();
			unsigned int TimesBeforDivisible = (CurrentExponent % MrBigInt(ExponentDivisor)).GetLowestTerm();
			//TotalDivisionTime += clock() - DeltaDivision;
			for (size_t i = 0; i < TimesBeforDivisible; i++)
			{
				//DeltaMultiplication = clock();
				OutMultiple *= CurrentBase;
				CurrentExponent -= One;
				//MultiplicationCount += 1;
				//TotalMultiplicationTime += clock() - DeltaMultiplication;
			}
 			if (CurrentExponent == MrBigInt(0))
			{
				CurrentBase %= Mod;
				OutMultiple %= Mod;
				break;
			}
			//DeltaDivision = clock();
			CurrentExponent /= ExponentDivisor;
			//TotalDivisionTime += clock() - DeltaDivision;
			MrBigInt BaseCopy = MrBigInt(CurrentBase);
			for (size_t i = 0; i < ExponentDivisorInt-1; i++)
			{
				//DeltaMultiplication = clock();
				CurrentBase *= BaseCopy;
				//MultiplicationCount += 1;
				//TotalMultiplicationTime += clock() - DeltaMultiplication;
			}
			//DeltaDivision = clock();
			CurrentBase %= Mod;
			OutMultiple %= Mod;
			//TotalDivisionTime += clock()-DeltaDivision;
		}
		OutResult = OutMultiple;
		//TotalTime = (clock() - Timer);
		//std::cout << "PowM Tog " << TotalTime/(double)CLOCKS_PER_SEC << std::endl;
		//std::cout << "Times in loop " << TimesInLoop << std::endl;
		//std::cout << "MultiplicationCount " << MultiplicationCount << std::endl;
		//std::cout << "Exponent Number " << Exponent.GetString() << std::endl;
		//std::cout << "Division time " << TotalDivisionTime/(double)CLOCKS_PER_SEC << std::endl;
		//std::cout << "multiplication time " << TotalMultiplicationTime/(double)CLOCKS_PER_SEC << std::endl;
		//std::cout << "Division ratio " << TotalDivisionTime / (double)TotalTime << std::endl;
		//std::cout << "Multiplication Ratio " << TotalMultiplicationTime / (double)TotalTime << std::endl;
	}
	static void Pow(MrBigInt const& Base,unsigned int Exponent,MrBigInt& OutResult)
	{
		//auto Timer = std::chrono::steady_clock::now();
		OutResult = MrBigInt(1);
		///*
		unsigned int NumberOfRepititions = std::ceil(std::sqrt(Exponent));//Metod med konstant division, just nu snabbare med storleksordning 100ggr
		unsigned int NumberBeforeDivisible = Exponent % NumberOfRepititions;
		for (size_t i = 0; i < NumberBeforeDivisible; i++)
		{
			OutResult *= Base;
		}
		MrBigInt BaseCopy = MrBigInt(Base);
		for (size_t i = 1; i < NumberOfRepititions; i++)
		{
			BaseCopy *= Base;
		}
		Exponent /= NumberOfRepititions;
		for (size_t i = 0; i < Exponent; i++)
		{
			OutResult *= BaseCopy;
		}
		//*/
		//Metod med division för varje steg
		
		/*
		unsigned int ExpDiv = 2;
		MrBigInt BaseCopy(Base);
		while (Exponent > 0)
		{
			unsigned int TimesBeforeDivisible = Exponent % 2;
			for (size_t i = 0; i < TimesBeforeDivisible; i++)
			{
				OutResult*= BaseCopy;
				Exponent -= 1;
			}
			if (Exponent == 0)
			{
				break;
			}
			Exponent /= ExpDiv;
			MrBigInt BaseTemp(BaseCopy);
			for (size_t i = 0; i < ExpDiv; i++)
			{
				BaseCopy *= BaseTemp;
			}
		}
		*/

		//auto End = std::chrono::steady_clock::now();
		//std::chrono::nanoseconds Duration = End-Timer;	
		//std::cout << "Pow tog " << Duration.count() << std::endl;
	}
	MrBigInt PowM(MrBigInt const& Exponent, MrBigInt const& Mod)
	{
		MrBigInt ReturnValue;
		MrBigInt::PowM(*this, Exponent, Mod, ReturnValue);
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
	void SetFromBigEndianArray(const void* Data, unsigned int NumberOfBytes)
	{
		unsigned char BytesInUnit = (UNIT_BITS / 8);
		unsigned int NumberOfElements = NumberOfBytes / BytesInUnit;
		unsigned char* CharData = (unsigned char*)Data;
		if (NumberOfBytes %BytesInUnit != 0)
		{
			NumberOfElements += 1;
		}
		if (NumberOfElements == 0)
		{
			NumberOfElements = 1;
		}
		InternalUnits = std::vector<UnitType>(NumberOfElements, 0);
		int ByteIterator = NumberOfBytes-1;
		for (size_t i = 0; i < NumberOfElements; i++)
		{
			for (size_t j = 0; j < BytesInUnit; j++)
			{
				if(ByteIterator < 0)
				{
					break;
				}
				InternalUnits[i] +=(unsigned int)(((unsigned int)CharData[ByteIterator]) << (j * 8));
				ByteIterator -= 1;
			}
			if (ByteIterator < 0)
			{
				break;
			}
		}
		for (int i = NumberOfElements-1; i >= 1 ; i--)
		{
			if (InternalUnits[i] == 0)
			{
				InternalUnits.pop_back();
			}
			else
			{
				break;
			}
		}
	}
	void SetFromString(std::string StringToInitializeWith, int Base)
	{
		//TODO gör grejer här
		assert(false);
	}
	std::string GetBigEndianArray()
	{
		unsigned char UnitBytes = UNIT_BITS / 8;
		std::string ReturnValue = "";
		unsigned int UnitArraySize = InternalUnits.size();
		for (int i = UnitArraySize-1; i >= 0; i--)
		{
			for (size_t j = 0; j < UnitBytes; j++)
			{
				ReturnValue += (InternalUnits[i] >> (UNIT_BITS-((j+1)*8)))&255;
			}
		}
		return(ReturnValue);
	}

	//conversions
	MrBigInt(int& Number)
	{
		InternalUnits[0] = Number;
		if (Number < 0)
		{
			IsNegative = true;
		}
	}
	MrBigInt(int Number)
	{
		InternalUnits[0] = Number;
		if (Number < 0)
		{
			IsNegative = true;
		}
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



	MrBigInt operator<<(unsigned int BitsToShift) const
	{
		if (BitsToShift == 0)
		{
			return(*this);
		}
		MrBigInt NewInt;
		unsigned int NewZeroUnits = BitsToShift / UNIT_BITS;
		unsigned int SingleBitsToShift = BitsToShift % UNIT_BITS;
		NewInt.InternalUnits = std::vector<UnitType>(InternalUnits.size()+NewZeroUnits, 0);
		UnitType PreviousBits = 0;
		UnitType ThisSize = InternalUnits.size();
		for (size_t i = 0; i < ThisSize; i++)
		{
			NewInt.InternalUnits[i + NewZeroUnits] += PreviousBits;
			NewInt.InternalUnits[i + NewZeroUnits] += (UnitType)(InternalUnits[i] << SingleBitsToShift);
			if (SingleBitsToShift == 0)
			{
				PreviousBits = 0;
			}
			else
			{
				PreviousBits = InternalUnits[i] >> (UNIT_BITS - SingleBitsToShift);
			}
		}
		if (PreviousBits != 0)
		{
			NewInt.InternalUnits.push_back(PreviousBits);
		}
		return(NewInt);
	}
	MrBigInt operator>>(unsigned int BitsToShift)
	{
		//TODO gör grejer här
		assert(false);
		return(0);
	}
	MrBigInt& operator%=(MrBigInt const& RightInt)
	{
		MrBigInt Remainder;
		MrBigInt Quotient;
		UnsignedDivide(RightInt, Remainder, Quotient);
		IsNegative = false;
		std::swap(Remainder.InternalUnits, InternalUnits);
		return(*this);
	}
	MrBigInt& operator*=(const MrBigInt& RightInt)
	{
		MrBigInt Result;
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
	MrBigInt& operator/=(MrBigInt const& RightInt)
	{
		//långsam division, lång division
		MrBigInt Remainder;
		MrBigInt Quotient;
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
	MrBigInt& operator+=(const MrBigInt& RightInt)
	{
		if (IsNegative == RightInt.IsNegative)
		{
			AddUnsigned(RightInt);
		}
		else if (IsNegative && RightInt.IsNegative == false)
		{
			//TODO kan optimseras genom att specifia vilket led som är höger och vänster i funktionen
			MrBigInt RightCopy = MrBigInt(RightInt);
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
	MrBigInt& operator-=(MrBigInt const& RightInt)
	{
		if (!IsNegative && !RightInt.IsNegative)
		{
			SubtractUnsigned(RightInt);
		}
		if (IsNegative && RightInt.IsNegative)
		{
			//vänsterlaedet negativt, högleredet negativt, samma sak som -> VL-HL = HL-abs(VL)
			MrBigInt RightIntCopy = MrBigInt(RightInt);
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
		//MrBigInt RightIntCopy = MrBigInt(RightInt);
		//RightIntCopy.NegateNumber();
		//(*this) += RightIntCopy;
		return(*this);
	}
	bool operator<=(MrBigInt const& OtherInt) const
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
	bool operator==(MrBigInt const& OtherInt) const
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
	bool operator!=(MrBigInt const& OtherInt) const
	{
		return(!(*this == OtherInt));
	}
	bool operator<(MrBigInt const& OtherInt) const
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
	bool operator>(MrBigInt const& RightInt) const
	{
		return(!(*this <= RightInt));
	}
	bool operator>=(MrBigInt const& RightInt) const
	{
		return(!(*this < RightInt));
	}
	unsigned int GetLowestTerm() const
	{
		return(InternalUnits.front());
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
	std::string GetString() const
	{
		std::string Temp = "";
		//clock_t TotalTimeTimer = clock();
		double TotalDivideTime = 0;
		MrBigInt NumberToDivide = MrBigInt(*this)*10;
		MrBigInt Ten = MrBigInt(10);
		MrBigInt Quotient;
		MrBigInt Remainder;
		while (NumberToDivide >= Ten)
		{
			//clock_t DivideTimer = clock();
			NumberToDivide.UnsignedDivide(Ten, Remainder, Quotient);
			//TotalDivideTime += (clock() - DivideTimer) / (double)CLOCKS_PER_SEC;
			unsigned int ModTen = 0;
			unsigned int ModMultiplier = 1;
			for (size_t i = 0; i < Quotient.InternalUnits.size(); i++)
			{
				unsigned int DebugModTen = ModTen;
				ModTen += (Quotient.InternalUnits[i] % 10)*ModMultiplier;
				ModMultiplier = (ModMultiplier * 6) % 10;
				assert(ModTen >= DebugModTen);
				assert(ModMultiplier == 1 || ModMultiplier == 6);
			}
			Temp += std::to_string(ModTen%10);
			NumberToDivide = Quotient;
		}
		std::string ReturnValue = "";
		for (size_t i = 0; i < Temp.size(); i++)
		{
			ReturnValue += Temp[Temp.size() - 1 - i];
		}
		if (ReturnValue == "")
		{
			ReturnValue += std::to_string(InternalUnits.front());
		}
		//double TotalTime = (clock() - TotalTimeTimer) / (double)CLOCKS_PER_SEC;
		//std::cout << "Total time is: " << TotalTime << std::endl << "Total divide Time: " << TotalDivideTime << std::endl << "Percentege divie: " << TotalDivideTime / TotalTime << std::endl;
		return(ReturnValue);
	}
	std::string GetHexEncodedString()
	{
		std::string ReturnValue = "";
		for (int i = InternalUnits.size()-1; i >= 0; i--)
		{
			std::string HexEncodedUnit = "";
			for (size_t j = 0; j < UNIT_BITS/8; j++)
			{
				HexEncodedUnit += HexEncodeByte(InternalUnits[i] >> (UNIT_BITS - ((j + 1)*8)));
			}
			ReturnValue += HexEncodedUnit;
		}
		return(ReturnValue);
	}
};
//*/