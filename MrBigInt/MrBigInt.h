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
	void ChangeInternalUnitsNumber(size_t NewNumberOfInternalUnits)
	{
		//om det är mindre bara gör vi inget
		if (InternalUnits.size() >= NewNumberOfInternalUnits)
		{
			return;
		}
		size_t UnitDifference = NewNumberOfInternalUnits - InternalUnits.size();
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
		if (NumberToAdd.InternalUnits.size() == 1 && NumberToAdd.InternalUnits[0] == 0)
		{
			return;
		}
		UnitType LeftSize = InternalUnits.size();
		UnitType RightSize = NumberToAdd.InternalUnits.size();
		int MaxComponents = std::max(LeftSize, RightSize);
		//ChangeInternalUnitsNumber(MaxComponents);
		//NumberToAdd.ChangeInternalUnitsNumber(MaxComponents);
		InternalUnits.reserve(MaxComponents);
		for (int i = 0; i <MaxComponents-int(LeftSize); i++)
		{
			InternalUnits.push_back(0);
		}
		LeftSize = InternalUnits.size();
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (size_t i = 0; i < RightSize; i++)
		{
			char NewCarry = 0;
			//assert(i < InternalUnits.size());
			UnitType ThisValue = InternalUnits[i];
			UnitType RightValue = NumberToAdd.InternalUnits[i];
			if ((UnitType)(ThisValue + RightValue+Carry) < ThisValue || (UnitType)(ThisValue + RightValue + Carry) < RightValue)
			{
				NewCarry = 1;
			}
			InternalUnits[i] += RightValue+ Carry;
			Carry = NewCarry;
		}
		if (Carry == 1)
		{
			//fortsätter addera tills carryn inte är 0
			size_t ThisUnitIndex = RightSize;
			if (ThisUnitIndex >= InternalUnits.size())
			{
				InternalUnits.push_back(1);
			}
			else
			{
				while (ThisUnitIndex < LeftSize && InternalUnits[ThisUnitIndex] == UNIT_MAX)
				{
					InternalUnits[ThisUnitIndex] = 0;
					ThisUnitIndex += 1;
				}
				if (ThisUnitIndex == LeftSize)
				{
					InternalUnits.push_back(1);
				}
				else
				{
					InternalUnits[ThisUnitIndex] += 1;
				}
			}
		}
	}
	void SubtractUnsigned(MrBigInt const& NumberToSubtract)
	{
		//förutsätter att vänsterledet alltid är positivt och högerledet negativt
		size_t MaxComponents = std::max(this->InternalUnits.size(), NumberToSubtract.InternalUnits.size());
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
				UnitType NegativeNumber = RightValue - ThisValue;
				InternalUnits[i] = (UNIT_MAX - NegativeNumber) + 1;
			}
			else
			{
				InternalUnits[i] -= RightValue;
			} 
			Carry = NewCarry;
		}
		//TODO borde man ta bort leading nollor för att göra det aningen mer minnes effektivt och ha färre special fall?
		Normalize();
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
	static void SubtractWithOffset(MrBigInt& LeftInt,MrBigInt const& NumberToSubtract,unsigned int IndexToStop,unsigned int Offset)
	{
		//förutsätter att vänsterledet alltid är positivt och högerledet negativt
		IndexToStop = std::min(IndexToStop, (unsigned int)LeftInt.InternalUnits.size());
		unsigned int MaxComponents = std::max(LeftInt.InternalUnits.size(), NumberToSubtract.InternalUnits.size());
		LeftInt.ChangeInternalUnitsNumber(MaxComponents);
		//NumberToSubtract.ChangeInternalUnitsNumber(MaxComponents);
		int RightSize = NumberToSubtract.InternalUnits.size();
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (int i = Offset; i < IndexToStop; i++)
		{
			//först och främst så subtraherar vi med carry, är talet UNIT_MAX sätter vi nya carryn till 1 igen
			char NewCarry = 0;
			LeftInt.InternalUnits[i] -= Carry;
			if (LeftInt.InternalUnits[i] == UNIT_MAX)
			{
				NewCarry = 1;
			}
			UnitType ThisValue = LeftInt.InternalUnits[i];
			UnitType RightValue = 0;
			if (i-Offset < RightSize)
			{
				RightValue = NumberToSubtract.InternalUnits[i - Offset];
			}
			//subtraheringen
			if (i == MaxComponents - 1)
			{
				//sista gången, specialfall
				if (ThisValue < RightValue)
				{
					LeftInt.InternalUnits[i] = RightValue - ThisValue;
					LeftInt.IsNegative = true;
				}
				else
				{
					LeftInt.InternalUnits[i] = ThisValue - RightValue;
					LeftInt.IsNegative = false;
				}
			}
			else if (ThisValue < RightValue)
			{
				//kommer overflowa
				NewCarry = 1;
				UnitType NegativeNumber = RightValue - ThisValue;
				LeftInt.InternalUnits[i] = (UNIT_MAX - NegativeNumber) + 1;
			}
			else
			{
				LeftInt.InternalUnits[i] -= RightValue;
			}
			Carry = NewCarry;
		}
		//TODO borde man ta bort leading nollor för att göra det aningen mer minnes effektivt och ha färre special fall?
		LeftInt.Normalize();
	}
	static void AddWithOffset(MrBigInt& LeftInt, MrBigInt const& NumberToAdd,int IndexToStop, unsigned int Offset)
	{
		//int IndexToStop = Offset + NumberToAdd.InternalUnits.size();
		if (NumberToAdd.InternalUnits.size() == 1 && NumberToAdd.InternalUnits[0] == 0)
		{
			return;
		}
		IndexToStop = std::min(IndexToStop, (int)NumberToAdd.InternalUnits.size());
		UnitType LeftSize = LeftInt.InternalUnits.size();
		UnitType RightSize = NumberToAdd.InternalUnits.size();
		int MaxComponents = std::max(LeftSize, RightSize);
		//ChangeInternalUnitsNumber(MaxComponents);
		//NumberToAdd.ChangeInternalUnitsNumber(MaxComponents);
		LeftInt.InternalUnits.reserve(MaxComponents);
		for (int i = 0; i < std::max((int)IndexToStop-(int)LeftInt.InternalUnits.size(),0); i++)
		{
			LeftInt.InternalUnits.push_back(0);
		}
		LeftSize = LeftInt.InternalUnits.size();
		//nu tar vi och faktiskt adderar dem som vanliga unsigned ints
		char Carry = 0;
		for (size_t i = Offset; i < IndexToStop; i++)
		{
			char NewCarry = 0;
			//assert(i < InternalUnits.size());
			UnitType ThisValue = LeftInt.InternalUnits[i];
			UnitType RightValue = NumberToAdd.InternalUnits[i-Offset];
			if ((UnitType)(ThisValue + RightValue + Carry) < ThisValue || (UnitType)(ThisValue + RightValue + Carry) < RightValue)
			{
				NewCarry = 1;
			}
			LeftInt.InternalUnits[i] += RightValue + Carry;
			Carry = NewCarry;
		}
		if (Carry == 1 && IndexToStop == LeftInt.InternalUnits.size())
		{
			//fortsätter addera tills carryn inte är 0
			size_t ThisUnitIndex = RightSize;
			if (ThisUnitIndex >= LeftInt.InternalUnits.size())
			{
				LeftInt.InternalUnits.push_back(1);
			}
			else
			{
				while (ThisUnitIndex < LeftSize && LeftInt.InternalUnits[ThisUnitIndex] == UNIT_MAX)
				{
					LeftInt.InternalUnits[ThisUnitIndex] = 0;
					ThisUnitIndex += 1;
				}
				if (ThisUnitIndex == LeftSize)
				{
					LeftInt.InternalUnits.push_back(1);
				}
				else
				{
					LeftInt.InternalUnits[ThisUnitIndex] += 1;
				}
			}
		}
	}
	static bool UnsignedGreaterThan(MrBigInt const& LeftInt, MrBigInt const& RightInt,unsigned int Offset)
	{
		unsigned int IndexToStop = Offset + RightInt.InternalUnits.size();
		//antar att dem är normaliserade, och att uniten till 
		if (IndexToStop > LeftInt.InternalUnits.size())
		{
			return(false);
		}
		for (int i = IndexToStop; i >= Offset; i--)
		{
			if (LeftInt.InternalUnits[i] > RightInt.InternalUnits[i - Offset])
			{
				return(true);
			}
		}
		return(false);
	}
	static bool UnsignedLesserThan(MrBigInt const& LeftInt, MrBigInt const& RightInt,int IndexToStop, int Offset)
	{
		//antar att dem är normaliserade, och att uniten till 
		unsigned int LeadingZeros = 0;
		for (int i = LeftInt.InternalUnits.size()-1; i >= 1; i--)
		{
			if (LeftInt.InternalUnits[i] == 0)
			{
				LeadingZeros += 1;
			}
			else
			{
				break;
			}
		}
		int RightSize = RightInt.InternalUnits.size();
		UnitType LeftValue;
		UnitType RightValue;
		for (int i = IndexToStop; i >= Offset; i--)
		{
			LeftValue = LeftInt.InternalUnits[i- LeadingZeros];
			RightValue = 0;
			if (i-Offset < RightSize)
			{
				RightValue = RightInt.InternalUnits[i - Offset];
			}
			if (LeftValue < RightValue)
			{
				return(true);
			}
			else
			{
				return(false);
			}
		}
		return(false);
	}	
	static UnitType ReciproalUnit32(UnitType UnitToInvert)
	{
		//un
		UnitType d0 = UnitToInvert & 1;
		UnitType d10 = UnitToInvert>>22;
		UnitType d21 = (UnitToInvert>>11)+1;
		UnitType d31 = UnitToInvert>>1;
		UnitType v0 = ((1 << 24) - (1 << 14) + (1 << 9)) / d10;
		MrBigInt Temp = v0;
		Temp *= v0;
		Temp *= d21;
		assert(Temp.InternalUnits.size() == 2);
		UnitType TempResult = Temp.InternalUnits.back();
		UnitType v1 = (v0 << 4) - TempResult - 1;
		MrBigInt eTemp = 1;
		eTemp <<= 48;
		Temp = (v1 >> 1);
		Temp *= d0;
		eTemp += Temp;
		Temp = v1;
		Temp *= d31;
		eTemp -= Temp;//står "umullho" på denna rad
		assert(eTemp.InternalUnits.size() == 1);
		UnitType e = eTemp.InternalUnits[0];
		Temp = v1;
		Temp <<= 15;
		MrBigInt Tempv2 = v1;
		Tempv2 *= e;
		Temp += Tempv2.InternalUnits[1] >> 1;
		//assert(Tempv2.InternalUnits.size() == 1);
		MrBigInt v2 = Temp;
		
		//Temp = 1;
		//Temp <<= 32;
		Temp += 1;
		//Temp += v2;
		
		Temp *= UnitToInvert;
		//assert(v2.InternalUnits[0] > Temp.InternalUnits[1]);
		UnitType DebugPreviousV2 = v2.InternalUnits[0];
		v2 -= Temp.InternalUnits[1];
		//assert(Temp.InternalUnits.size() <= 2);
		//Tempv2 -= Temp.InternalUnits[0];
		UnitType v3 = v2.InternalUnits[0];
		//assert(DebugPreviousV2 == v3 - 1 || DebugPreviousV2 == v3 || DebugPreviousV2 == v3 + 1);
		//DEBUG GREJER
		//MrBigInt DebugInt = 1;
		//DebugInt <<= 32;
		//DebugInt += v2.InternalUnits[0];
		//DebugInt *= UnitToInvert;
		//std::cout << DebugInt.GetBinaryString() << std::endl;
		return(v3);
	}
	// förutsätter att ordet är normaliserat, high bit set
	static void UnsignedDivide_AOP(MrBigInt& IntToDivide,MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient)
	{
		//inte destruktiv, så vi tar och kopierar dividenten och denna
		//MrBigInt ThisCopy(*this);
		//MrBigInt DividentCopy(Divident);
		UnitType m = IntToDivide.InternalUnits.size() - Divident.InternalUnits.size();
		//antar att dem är normaliserade
		if (IntToDivide.InternalUnits.size() < Divident.InternalUnits.size())
		{
			std::swap(OutRemainder.InternalUnits, IntToDivide.InternalUnits);
			OutQuotient = MrBigInt(0);
			return;
		}
		if (Divident.InternalUnits.size() == 1)
		{
			SingleLimbDivision(&IntToDivide.InternalUnits[0], IntToDivide.InternalUnits.size(), Divident.InternalUnits[0], OutQuotient, OutRemainder.InternalUnits[0]);
			OutRemainder.InternalUnits.resize(1);
			return;
		}
		//normaliserar dem
		size_t PreviousSize = IntToDivide.InternalUnits.size();
		MrBigInt d = MrBigInt(UNIT_MAX / Divident.InternalUnits.back());
		IntToDivide *= d;
		char WasAppended = 0;
		if (PreviousSize == IntToDivide.InternalUnits.size())
		{
			IntToDivide.InternalUnits.push_back(0);
			WasAppended += 1;
		}
		UnitType n = Divident.InternalUnits.size();
		MrBigInt q;
		q.InternalUnits = std::vector<UnitType>(2, 0);
		MrBigInt r;
		MrBigInt b(1);
		b <<= UNIT_BITS;
		MrBigInt& u = IntToDivide;
		MrBigInt const* DividentCopy = &Divident;
		MrBigInt* DivTemp = nullptr;
		if (d != 1)
		{
			DivTemp = new MrBigInt(Divident);
			*DivTemp *= d;
			DividentCopy = DivTemp;
		}
		MrBigInt const& v = *DividentCopy;
		MrBigInt DivideInt = MrBigInt(0);
		DivideInt.InternalUnits.push_back(1);
		MrBigInt TempDivident = 0;
		MrBigInt TempR;
		TempR.InternalUnits = std::vector<UnitType>(2, 0);
		MrBigInt Temp3;
		MrBigInt Temp2;
		MrBigInt TempOutQuotient;
		TempOutQuotient.InternalUnits = std::vector<UnitType>(m + 1, 0);
		
		//OutQuotient.InternalUnits = std::vector<UnitType>(m + 1, 0);
		//DEBUG GREJER
		//MrBigInt DebugRemainder;
		//MrBigInt DebugQuotient;
		//UnsignedDivide_MB(Divident, DebugRemainder, DebugQuotient);
		

		UnitType VDivisor = v.InternalUnits.back();
		UnitType VReciprocal = ReciproalUnit32(v.InternalUnits.back());
		for (int j = m; j >= 0; j--)
		{
			//assert(ThisCopy.InternalUnits.size() >= this->InternalUnits.size());
			//räkna ut på något sätt
			//DoubleUnitDivide(u.InternalUnits[j + n], u.InternalUnits[j + n - 1], q, r);

			//DivideInt.InternalUnits[1] = u.InternalUnits[j + n];
			//DivideInt.InternalUnits[0] = u.InternalUnits[j + n - 1];
			//TempDivident.InternalUnits[0] = v.InternalUnits.back();
			//DivideInt.UnsignedDivide_MB(TempDivident, r, q);
			if (r.InternalUnits.size() > 1)
			{
				r.InternalUnits.pop_back();
			}
			DoubleUnitDivide(u.InternalUnits[j + n], u.InternalUnits[j + n - 1], VDivisor, &q, &r.InternalUnits[0], &VReciprocal);

			//Temp = (r << UNIT_BITS)+ u.InternalUnits[j + n - 2];
			TempR.InternalUnits[1] = r.InternalUnits.front();
			TempR.InternalUnits[0] = u.InternalUnits[j + n - 2];
			Temp2 = q * v.InternalUnits[n - 2];
			//if (q == b || Temp2 > Temp)
			if (q == b || Temp2 > TempR)
			{
				//gör grejer
				q -= 1;
				r += v.InternalUnits.back();
				if (r < b)
				{
					TempR.InternalUnits[1] = r.InternalUnits.front();
					TempR.InternalUnits[0] = u.InternalUnits[j + n - 2];
					if (q == b || (Temp2) > (TempR))
					{
						q -= 1;
						r += v.InternalUnits.back();
					}
				}
			}
			//multiply and subtract
			//assert(r.InternalUnits.size() == 1);
			bool WasNegative = false;
			if (UnsignedLesserThan(u, q, j + n, j))
			{
				WasNegative = true;
			}
			//assert(UnsignedLesserThan(u, q, j+n, j) == (u < q << (UNIT_BITS * j)));
			TempOutQuotient.InternalUnits[j] = q.InternalUnits[0];
			if (WasNegative == false)
			{
				//addback utgår jag ifrån bara resettar den, så vi subtraherar bara om vi vet att den behöva
				//MrBigInt UCopy(u);
				Temp3 = q * v;
				SubtractWithOffset(u, Temp3, j + n + 1, j);
				//asserta grejer för debug
				//MrBigInt DebugResult = UCopy - (Temp << (UNIT_BITS * j));
				//std::cout << u.GetHexEncodedString() << std::endl;
				//std::cout << DebugResult.GetHexEncodedString() << std::endl;
				//assert(DebugResult == u);
			}
			else
			{
				TempOutQuotient.InternalUnits[j] -= 1;
			}
			//if (j < 448)
			//{
			//	assert(OutQuotient.InternalUnits[j] == DebugQuotient.InternalUnits[j]);
			//}
		}
		//assert(d == 1);
		if (DivTemp != nullptr)
		{
			delete DivTemp;
		}
		TempOutQuotient.Normalize();
		std::swap(TempOutQuotient, OutQuotient);
		SingleLimbDivision(u.InternalUnits.data(), n, d.InternalUnits.back(), OutRemainder, Temp3.InternalUnits[0]);
		//DEBUG GREJER
		//if (DebugQuotient != OutQuotient)
		//{
		//	std::cout << "Fel i quotient" << std::endl;
		//}
		//if (DebugRemainder != OutRemainder)
		//{
		//	std::cout << "Fel i remainder" << std::endl;
		//}
		//unnormalize
	}
	void UnsignedDivide_AOP(MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient) const
	{
		//inte destruktiv, så vi tar och kopierar dividenten och denna
		MrBigInt ThisCopy(*this);
		//MrBigInt DividentCopy(Divident);
		UnitType m = ThisCopy.InternalUnits.size() - Divident.InternalUnits.size();
		//antar att dem är normaliserade
		if (ThisCopy.InternalUnits.size() < Divident.InternalUnits.size())
		{
			std::swap(OutRemainder.InternalUnits, ThisCopy.InternalUnits);
			OutQuotient = MrBigInt(0);
			return;
		}
		//normaliserar dem
		size_t PreviousSize =  ThisCopy.InternalUnits.size();
		MrBigInt d = MrBigInt(UNIT_MAX / Divident.InternalUnits.back());
		ThisCopy *= d;
		char WasAppended = 0;
		if (PreviousSize == ThisCopy.InternalUnits.size())
		{
			ThisCopy.InternalUnits.push_back(0);
			WasAppended += 1;
		}
		UnitType n = Divident.InternalUnits.size();
		MrBigInt q;
		q.InternalUnits = std::vector<UnitType>(2, 0);
		MrBigInt r;
		MrBigInt b(1);
		b <<= UNIT_BITS;
		MrBigInt& u = ThisCopy;
		MrBigInt const* DividentCopy = &Divident;
		MrBigInt* DivTemp = nullptr;
		if (d != 1)
		{
			DivTemp = new MrBigInt(Divident);
			*DivTemp *= d;
			DividentCopy = DivTemp;
		}
		MrBigInt const& v = *DividentCopy;
		MrBigInt DivideInt = MrBigInt(0);
		DivideInt.InternalUnits.push_back(1);
		MrBigInt TempDivident = 0;
		MrBigInt TempR;
		TempR.InternalUnits = std::vector<UnitType>(2, 0);
		MrBigInt Temp3;
		MrBigInt Temp2;
		OutQuotient.InternalUnits = std::vector<UnitType>(m+1,0);
		//DEBUG GREJER
		//MrBigInt DebugRemainder;
		//MrBigInt DebugQuotient;
		//UnsignedDivide_MB(Divident, DebugRemainder, DebugQuotient);
		
		
		UnitType VDivisor = v.InternalUnits.back();
		UnitType VReciprocal = ReciproalUnit32(v.InternalUnits.back());
		for (int j = m; j >= 0; j--)
		{
			//assert(ThisCopy.InternalUnits.size() >= this->InternalUnits.size());
			//räkna ut på något sätt
			//DoubleUnitDivide(u.InternalUnits[j + n], u.InternalUnits[j + n - 1], q, r);
			
			//DivideInt.InternalUnits[1] = u.InternalUnits[j + n];
			//DivideInt.InternalUnits[0] = u.InternalUnits[j + n - 1];
			//TempDivident.InternalUnits[0] = v.InternalUnits.back();
			//DivideInt.UnsignedDivide_MB(TempDivident, r, q);
			if (r.InternalUnits.size() > 1)
			{
				r.InternalUnits.pop_back();
			}
			DoubleUnitDivide(u.InternalUnits[j + n], u.InternalUnits[j + n - 1], VDivisor,&q,&r.InternalUnits[0],&VReciprocal);
			
			//Temp = (r << UNIT_BITS)+ u.InternalUnits[j + n - 2];
			TempR.InternalUnits[1] = r.InternalUnits.front();
			TempR.InternalUnits[0] = u.InternalUnits[j + n - 2];
			Temp2 = q * v.InternalUnits[n - 2];
			//if (q == b || Temp2 > Temp)
			if (q == b || Temp2 > TempR)
			{
				//gör grejer
				q -= 1;
				r += v.InternalUnits.back();
				if(r < b)
				{
					TempR.InternalUnits[1] = r.InternalUnits.front();
					TempR.InternalUnits[0] = u.InternalUnits[j + n - 2];
					if (q == b || (Temp2) > (TempR))
					{
						q -= 1;
						r += v.InternalUnits.back();
					}
				}
			}
			//multiply and subtract
			//assert(r.InternalUnits.size() == 1);
			bool WasNegative = false;
			if (UnsignedLesserThan(u,q, j + n,j))
			{
				WasNegative = true;
			}
			//assert(UnsignedLesserThan(u, q, j+n, j) == (u < q << (UNIT_BITS * j)));
			OutQuotient.InternalUnits[j] = q.InternalUnits[0];
			if (WasNegative == false)
			{
				//addback utgår jag ifrån bara resettar den, så vi subtraherar bara om vi vet att den behöva
				//MrBigInt UCopy(u);
				Temp3 = q * v;
				SubtractWithOffset(u, Temp3, j + n+1, j);
				//asserta grejer för debug
				//MrBigInt DebugResult = UCopy - (Temp << (UNIT_BITS * j));
				//std::cout << u.GetHexEncodedString() << std::endl;
				//std::cout << DebugResult.GetHexEncodedString() << std::endl;
				//assert(DebugResult == u);
			}
			else
			{
				OutQuotient.InternalUnits[j] -= 1;
			}
			//if (j < 448)
			//{
			//	assert(OutQuotient.InternalUnits[j] == DebugQuotient.InternalUnits[j]);
			//}
		}
		//assert(d == 1);
		if (DivTemp != nullptr)
		{
			delete DivTemp;
		}
		OutQuotient.Normalize();
		SingleLimbDivision(u.InternalUnits.data(), n, d.InternalUnits.back(), OutRemainder, Temp3.InternalUnits[0]);
		//DEBUG GREJER
		//if (DebugQuotient != OutQuotient)
		//{
		//	std::cout << "Fel i quotient" << std::endl;
		//}
		//if (DebugRemainder != OutRemainder)
		//{
		//	std::cout << "Fel i remainder" << std::endl;
		//}
		//unnormalize
	}
	void UnsignedDivide_MB(MrBigInt const& Divident, MrBigInt& OutRemainder,MrBigInt& OutQuotient) const
	{
		OutRemainder = MrBigInt();
		OutQuotient = MrBigInt();
		//unsigned int MaxComponents = std::max(this->InternalUnits.size(), Divident.InternalUnits.size());
		size_t ThisSize = InternalUnits.size();
		size_t MaxSize = std::max(InternalUnits.size(), Divident.InternalUnits.size());
		//DEBUG GREJER	
		//size_t OriginalSize = Divident.InternalUnits.size();
		//ChangeInternalUnitsNumber(MaxComponents);
		//Divident.ChangeInternalUnitsNumber(MaxComponents);
 		for (long long i = ThisSize-1; i >= 0; i--)
		{
			for (long long j = UNIT_BITS-1; j >= 0; j--)
			{
				//MrBigInt DebugCopy(OutRemainder);
				OutRemainder <<= 1;
				//if (DebugCopy*2 != OutRemainder)
				//{
				//	MrBigInt DebugCopy2 = DebugCopy * 2;
				//	MrBigInt DebugCopy3 = DebugCopy << 1;
				//	assert(false);
				//}
				OutRemainder.SetBitByIndex(0,(InternalUnits[i]>>j)&1);
				if (Divident <= OutRemainder)
				{
					//Divident <= DebugCopy;
					//MrBigInt DebugDividentCopy(OutRemainder);
					OutRemainder -= Divident;
					//assert(OutRemainder + Divident == DebugDividentCopy);
					//OutQuotient += MrBigInt(1) << ((i * UNIT_BITS) + j);
					OutQuotient.SetBitByIndex((i * UNIT_BITS) + j,1);
				}
				//assert(OutRemainder.InternalUnits.size() <= OriginalSize);
			}
		}
	}
	void UnsignedDivide(MrBigInt const& Divident, MrBigInt& OutRemainder, MrBigInt& OutQuotient) const
	{
		if (Divident.InternalUnits.size() != 1)
		{
			UnsignedDivide_AOP(Divident, OutRemainder, OutQuotient);
		}
		else
		{
			SingleLimbDivision(&InternalUnits[0], InternalUnits.size(), Divident.InternalUnits[0], OutQuotient, OutRemainder.InternalUnits[0]);
			OutRemainder.InternalUnits.resize(1);
		}
	}
	void Normalize()
	{
		//tar bort alla trailing nollor
		size_t ThisSize = InternalUnits.size();
		for (int i = ThisSize-1; i >=1 ; i--)
		{
			if (InternalUnits[i] == 0)
			{
				InternalUnits.erase(InternalUnits.begin() + i);
			}
			else
			{
				break;
			}
		}
	}
#define KARATSUBA_UNIT_COUNT 20
public:
	static void DoubleUnitDivide(UnitType Higher, UnitType Lower, UnitType Divisor, MrBigInt* OutQuotient, UnitType* OutRemainder, UnitType* DivisorReciprocalPointer = nullptr)
	{
		if (Higher == 0 && Lower < Divisor)
		{
			OutQuotient->InternalUnits[1] = 0;
			OutQuotient->InternalUnits[0] = 0;
			OutRemainder[0] = Lower;
			return;
		}
		UnitType DivisorReciprocal;
		if (DivisorReciprocalPointer != nullptr)
		{
			DivisorReciprocal = *DivisorReciprocalPointer;
		}
		else
		{
			DivisorReciprocal = ReciproalUnit32(Divisor);
		}
		//utifall att det inte går på det normaliserade sättet kör vi den andra metoden
		if (Higher >= Divisor)
		{
			UnitType NewHigher;
			MrBigInt HighQuotient;
			if (OutQuotient->InternalUnits.size() < 2)
			{
				OutQuotient->InternalUnits.push_back(0);
			}
			DoubleUnitDivide(0, Higher, Divisor, &HighQuotient, &NewHigher, &DivisorReciprocal);
			OutQuotient->InternalUnits[1] = HighQuotient.InternalUnits[0];
			UnitType ModResult;
			MrBigInt LowQuotient;
			DoubleUnitDivide(NewHigher, Lower, Divisor, &LowQuotient, &ModResult, &DivisorReciprocal);
			//OutQuotient->InternalUnits = std::vector<UnitType>(2,0);
			OutQuotient->InternalUnits[1] = HighQuotient.InternalUnits[0];
			OutQuotient->InternalUnits[0] = LowQuotient.InternalUnits[0];
			*OutRemainder = ModResult;
			//Debug grejer
			MrBigInt DebugInt(Higher);
			DebugInt <<= 32;
			DebugInt += Lower;
			//MrBigInt DebugRemainder;
			//MrBigInt DebugQuotient;
			//DebugInt.UnsignedDivide_MB(Divisor, DebugRemainder, DebugQuotient);
			//assert(OutRemainder[0] == DebugRemainder.InternalUnits[0]);
			//assert(DebugQuotient.InternalUnits[0] == OutQuotient->InternalUnits[0]);
			//assert(DebugQuotient.InternalUnits[1] == OutQuotient->InternalUnits[1]);
			return;
		}
		struct DoubleWord
		{
			UnitType High = 0;
			UnitType Low = 0;
		};
		MrBigInt u;
		u.InternalUnits = std::vector<UnitType>(2, 0);
		u.InternalUnits[1] = Higher;
		u.InternalUnits[0] = Lower;

		DoubleWord Temp;
		MrBigInt::GetHighLowUnitMultiplication(DivisorReciprocal, Higher, Temp.High, Temp.Low);
		//assert(Temp.High != 0);
		UnitType q = Temp.High + Higher;

		//MrBigInt Temp = DivisorReciprocal;
		//Temp *= Higher;
		//UnitType q = Temp.InternalUnits.back() + Higher;

		MrBigInt p = q;
		p *= Divisor;

		assert(u >= p);
		MrBigInt r = u - p;
		r.IsNegative = false;
		while (r.InternalUnits.size() > 1 || r.InternalUnits[0] > Divisor)
		{
			q += 1;
			r -= Divisor;
		}
		OutQuotient->InternalUnits[0] = q;
		assert(r.InternalUnits.size() == 1);
		*OutRemainder = r.InternalUnits[0];

		//Debug grejer
		//MrBigInt TestInt = Higher;
		//TestInt <<= 32;
		//TestInt += Lower;
		//MrBigInt TestRemainder;
		//MrBigInt TestQuotient;
		//TestInt.UnsignedDivide_MB(Divisor, TestRemainder, TestQuotient);
		//assert(OutRemainder[0] == TestRemainder.InternalUnits[0]);
		//assert(OutQuotient[0] == TestQuotient.InternalUnits[0]);
		return;
#ifdef MB_USE_NATIVE_DIV
		//Debug
		unsigned __int64 HIgherCopy = Higher;
		unsigned __int64 Modolu;
		unsigned __int64 HighQuotient = 0;
		//TODO optimera denna kod, suger röv, men har ingen aning om hur man ska få end 2xUnitype/1xUnitype att ge en 2xUnitType kvot
		if (Higher >= Divisor)
		{
			HighQuotient = _udiv128(0, Higher, Divisor, &Modolu);
			Higher = Modolu;
			OutQuotient[1] = HighQuotient;
		}
		unsigned __int64 LowerQuotient = _udiv128(Higher, Lower, Divisor, &Modolu);
		OutQuotient[0] = LowerQuotient;
		*OutRemainder = Modolu;


		//Debug grejer
		//MrBigInt DebugInt = HIgherCopy;
		//DebugInt <<= UNIT_BITS;
		//DebugInt += Lower;
		//MrBigInt DebugRemainder;
		//MrBigInt DebugQuotient;
		//DebugInt.UnsignedDivide_MB(Divisor, DebugRemainder, DebugQuotient);
		//assert(DebugRemainder.InternalUnits[0] == *OutRemainder);
		//assert(DebugQuotient.InternalUnits[0] == OutQuotient[0]);
		//if (DebugQuotient.InternalUnits.size() == 2)
		//{
		//	assert(DebugQuotient.InternalUnits[1] == OutQuotient[1]);
		//}


		//unsigned __int64 HighWord = 0;
		//while (Higher >= Divisor)
		//{
		//	unsigned __int64 IntToSubtractHigh = 0;
		//	unsigned __int64 IntToSubtractLow = _umul128(Quotient, Divisor,&IntToSubtractHigh);
		//	unsigned __int64 LowerCopy = Lower;
		//	Lower -= IntToSubtractLow;
		//	if (Lower > IntToSubtractLow)
		//	{
		//		Higher -= 1;
		//	}
		//	Higher -= IntToSubtractHigh;
		//	Quotient = _udiv128(Higher, Lower, Divisor, &Modolu);
		//	HighWord = Quotient;
		//}
		//OutQuotient[1] = HighWord;
#endif
	}
	static void SingleLimbDivision(UnitType const* IntToDivideUnits, unsigned int NumberOfUnits, UnitType Divident, MrBigInt& OutQuotient, UnitType& OutRemainder)
	{
		//MrBigInt Temp;
		//MrBigInt Temp2;
		//Temp.InternalUnits.assign(IntToDivideUnits, IntToDivideUnits+NumberOfUnits);
		//Temp.UnsignedDivide_MB(Divident, Temp2, OutQuotient);
		//OutRemainder = Temp2.InternalUnits[0];
		if (Divident == 1)
		{
			OutRemainder = 0;
			OutQuotient.InternalUnits = std::vector<UnitType>(IntToDivideUnits, IntToDivideUnits + NumberOfUnits);
			return;
		}
		int j = NumberOfUnits - 1;
		UnitType r = 0;
		MrBigInt TempResult;
		UnitType NormalizedDivisor = UNIT_MAX / Divident;
		//kopierar inten för att kunna normalisera den
		MrBigInt IntToDivide;
		IntToDivide.InternalUnits = std::vector<UnitType>(IntToDivideUnits, IntToDivideUnits + NumberOfUnits);
		IntToDivide *= NormalizedDivisor;
		Divident *= NormalizedDivisor;
		TempResult.InternalUnits = std::vector<UnitType>(2, 0);
		OutQuotient.InternalUnits = std::vector<UnitType>(j + 1, 0);
		//UnitType DividentReciprocal = ReciproalUnit32(NormalizedDivisor);
		UnitType DividentReciprocal = ReciproalUnit32(Divident);
		do
		{
			assert(r < IntToDivide.InternalUnits[j]);
			//DoubleUnitDivide(r, IntToDivide.InternalUnits[j], NormalizedDivisor, &TempResult, &r, &DividentReciprocal);
			DoubleUnitDivide(r, IntToDivide.InternalUnits[j], Divident, &TempResult, &r, &DividentReciprocal);
			//assert(TempResult.InternalUnits[1] == 1);
			OutQuotient.InternalUnits[j] = TempResult.InternalUnits[0];
			j -= 1;
		} while (j >= 0);
		OutQuotient.Normalize();
		OutRemainder = r;
		//deub grejer
		//MrBigInt DebugInt;
		//MrBigInt TempQuot;
		//MrBigInt TempRemainder;
		//DebugInt.InternalUnits.assign(IntToDivideUnits, IntToDivideUnits + NumberOfUnits);
		//DebugInt.UnsignedDivide_MB(Divident, TempRemainder, TempQuot);
		//assert(OutQuotient == TempQuot);
		//assert(OutRemainder == TempRemainder.InternalUnits[0]);

	}
	static void KaratsubaMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		//bas = 2^32
		//MrBigInt B = MrBigInt(1) << (UNIT_BITS);
		//MrBigInt B2(0);
		//B2.SetBitByIndex(UNIT_BITS * 2, 1);
		int LeftSize = LeftInt.InternalUnits.size();
		int RightSize = RightInt.InternalUnits.size();
		if ((LeftSize == 1 && LeftInt.InternalUnits[0] == 0) || (RightSize == 1 && RightInt.InternalUnits[0] == 0))
		{
			OutResult = MrBigInt(0);
			return;
		}
		//if (LeftSize+RightSize <=2* KARATSUBA_UNIT_COUNT)
		if (LeftInt.InternalUnits.size() < KARATSUBA_UNIT_COUNT && RightInt.InternalUnits.size() < KARATSUBA_UNIT_COUNT)
		{
			LongMultiplication(LeftInt, RightInt, OutResult);
			return;
		}
		size_t MaxSize = std::max(LeftInt.InternalUnits.size(), RightInt.InternalUnits.size());
		int m = (MaxSize) / 2;
		if (MaxSize % 2 == 1)
		{
			m += 1;
		}
		MrBigInt x0;
		MrBigInt x1;
		MrBigInt y0;
		MrBigInt y1;
		size_t LeftSmallSize = std::min(LeftSize, m);
		size_t RightSmallSize = std::min(RightSize, m);
		size_t LeftBigSize = std::max(LeftSize - m, 0);
		size_t RightBigSize = std::max(RightSize - m, 0);
		x0.InternalUnits = std::vector<UnitType>(LeftSmallSize,0);
		x1.InternalUnits = std::vector<UnitType>(LeftBigSize,0);
		y0.InternalUnits = std::vector<UnitType>(RightSmallSize,0);
		y1.InternalUnits = std::vector<UnitType>(RightBigSize,0);
		if (LeftBigSize == 0)
		{
			x1.InternalUnits.push_back(0);
		}
		if (RightBigSize == 0)
		{
			y1.InternalUnits.push_back(0);
		}
		for (size_t i = 0; i < LeftSmallSize; i++)
		{
			x0.InternalUnits[i] = LeftInt.InternalUnits[i];
		}
		for (size_t i = 0; i < RightSmallSize; i++)
		{
			y0.InternalUnits[i] = RightInt.InternalUnits[i];
		}
		for (size_t i = 0; i < LeftBigSize; i++)
		{
			x1.InternalUnits[i] = LeftInt.InternalUnits[i+m];
		}
		for (size_t i = 0; i < RightBigSize; i++)
		{
			y1.InternalUnits[i] = RightInt.InternalUnits[i+m];
		}
		x1.Normalize();
		x0.Normalize();
		y0.Normalize();
		y1.Normalize();
		MrBigInt z2;
		MrBigInt z1;
		MrBigInt z0;
		KaratsubaMultiplication(x1, y1, z2);
		KaratsubaMultiplication(x0, y0, z0);
		x1 += x0;
		y1 += y0;
		KaratsubaMultiplication(x1, y1, z1);
		z1 -= z2;
		z1 -= z0;
		//räkna resultat
		z2 <<= 2 *m*UNIT_BITS;
		z1 <<= m*UNIT_BITS;
		OutResult = MrBigInt(0);
		OutResult += z2;
		OutResult += z1;
		OutResult += z0;
		//assert(OutResult != 0);
	}
	static void GetHighLowUnitMultiplication(UnitType LeftUnit, UnitType RightUnit, UnitType& OutHighUnit, UnitType& OutLowUnit)
	{
		OutHighUnit = 0;
		OutLowUnit = 0;
		unsigned int HalfUnitBits = UNIT_BITS / 2;
		UnitType UnitLowerMask = (UnitType)(~0) >> HalfUnitBits;
		UnitType RightHigher = RightUnit >> HalfUnitBits;
		UnitType RightLower = RightUnit & UnitLowerMask;
		UnitType LeftHigher = LeftUnit >> HalfUnitBits;
		UnitType LeftLower = LeftUnit & UnitLowerMask;
		UnitType Temp = 0;
		UnitType Temp2 = 0;
		UnitType PartialMulResult = 0;
		//lowest part of outlowunit
		//PartialMulResult = LeftLower * RightLower;
		if (LeftHigher >= 1<<15 && LeftLower >= 1<<15 && RightHigher>= 1<<15 && RightLower >= 1<<15)
		{
			int hej = 2;
		}
		OutLowUnit += UnitType(LeftLower*RightLower);
		//higher part of outlowunit
		PartialMulResult = (RightLower * LeftHigher);
		Temp = OutLowUnit;
		Temp2 = UnitType(PartialMulResult << HalfUnitBits);
		OutLowUnit += Temp2;
		if (OutLowUnit < Temp || OutLowUnit < Temp2)
		{
			OutHighUnit += 1;
		}
		OutHighUnit += UnitType(PartialMulResult >> HalfUnitBits);
		//higherpart of lowunit, second muliplicant
		PartialMulResult = (RightHigher*LeftLower);
		Temp = OutLowUnit;
		Temp2 = UnitType(PartialMulResult << HalfUnitBits);
		OutLowUnit += Temp2;
		if (OutLowUnit < Temp || OutLowUnit < Temp2)
		{
			OutHighUnit += 1;
		}
		OutHighUnit += UnitType(PartialMulResult >> HalfUnitBits);
		//high part of both
		OutHighUnit += UnitType(RightHigher * LeftHigher);
		//debug grejer
		//MrBigInt DebugInt1(LeftUnit);
		//MrBigInt DebugInt2(RightUnit);
		//MrBigInt Debugint3;
		//MrBigInt::LongMultiplication_MB(DebugInt1, DebugInt2, Debugint3);
		//assert(Debugint3.InternalUnits[1] == OutHighUnit && Debugint3.InternalUnits[0] == OutLowUnit);
	}
	static void LongMultiplication_AOP(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		size_t NeededUnits = LeftInt.InternalUnits.size() + RightInt.InternalUnits.size();
		size_t LeftSize = LeftInt.InternalUnits.size();
		size_t RightSize = RightInt.InternalUnits.size();
		OutResult.InternalUnits = std::vector<UnitType>(NeededUnits, 0);
		UnitType k;
		UnitType t_Higer;
		UnitType t_Lower;
		UnitType Temp;
		for (size_t j = 0; j < RightSize; j++)
		{
			k = 0;
			for (size_t i = 0; i < LeftSize; i++)
			{
				//inner multiplication result
				GetHighLowUnitMultiplication(LeftInt.InternalUnits[i], RightInt.InternalUnits[j], t_Higer, t_Lower);
				Temp = t_Lower;
				t_Lower += OutResult.InternalUnits[j + i];
				if (t_Lower < Temp || t_Lower < OutResult.InternalUnits[j+i])
				{
					t_Higer += 1;
				}
				Temp = t_Lower;
				t_Lower += k;
				if (t_Lower < Temp || t_Lower < k)
				{
					t_Higer += 1;
				}
				OutResult.InternalUnits[j + i] = t_Lower;
				k = t_Higer;
			}
			OutResult.InternalUnits[j + LeftSize] = k;
		}
		OutResult.Normalize();
		//debug grejer
		//MrBigInt NormalResult;
		//LongMultiplication_MB(LeftInt, RightInt, NormalResult);
		//assert(NormalResult == OutResult);
	}
	static void LongMultiplication_MB(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		std::cout << "long multiplication har en bugg" << std::endl;
		//assert(false);
		size_t RightSize = RightInt.InternalUnits.size();
		MrBigInt LeftIntCopy(LeftInt);
		MrBigInt RightIntCopy;
		const MrBigInt* RightIntData = &RightInt;
		if (&RightInt == &OutResult)
		{
			RightIntCopy = RightInt;
			RightIntData = &RightIntCopy;
		}
		OutResult = MrBigInt(0);
		//reservar så den har plats med minst båda
		OutResult.InternalUnits.reserve(RightInt.InternalUnits.size() + LeftInt.InternalUnits.size());
		size_t LastBitshiftIndex = 0;
		for (size_t i = 0; i < RightSize; i++)
		{
			UnitType CurrentUnit = RightIntData->InternalUnits[i];
			for (size_t j = 0; j < UNIT_BITS; j++)
			{
				char CurrentBit = (CurrentUnit >> j) & 1;
				if (CurrentBit == 1)
				{
					assert(LeftIntCopy.IsNegative == false);
					//OutResult += LeftInt << ((i* UNIT_BITS) + j);
					//MrBigInt LeftCopyCopy(LeftIntCopy);
					//unsigned int PreviousShift = (((i * UNIT_BITS) + j) - LastBitshiftIndex);
					//MrBigInt DebugInt = LeftIntCopy << (((i * UNIT_BITS) + j) - LastBitshiftIndex);
					//assert(OutResult.InternalUnits.back() != 0 || OutResult.InternalUnits.size() == 1);
					//std::cout << "Bits to shift: " << (((i * UNIT_BITS) + j) - LastBitshiftIndex) << std::endl;
					//std::cout << "leftintcopy innan shift: " << LeftIntCopy.GetHexEncodedString() << std::endl;
					LeftIntCopy <<= (((i * UNIT_BITS) + j) - LastBitshiftIndex);
					//std::cout << "leftintcopy efter shift: " << LeftIntCopy.GetHexEncodedString() << std::endl;
					//if (DebugInt != LeftIntCopy)
					//{
					//	LeftCopyCopy <<= PreviousShift;
					//	DebugInt == LeftIntCopy;
					//}
					LastBitshiftIndex = ((i * UNIT_BITS) + j);
					//std::cout << LeftIntCopy.GetString() << std::endl;
					MrBigInt OutResultCopy(OutResult);
					OutResult += LeftIntCopy;
					MrBigInt IntToCompare = (OutResult - LeftIntCopy);
					assert(OutResultCopy == IntToCompare);
					//assert(OutResult.InternalUnits.back() != 0 || OutResult.InternalUnits.size() == 1);
				}
			}
		}
		//assert(OutResult.InternalUnits.back() != 0 || OutResult.InternalUnits.size() == 1);
	}
	static void LongMultiplication(MrBigInt const& LeftInt, MrBigInt const& RightInt, MrBigInt& OutResult)
	{
		LongMultiplication_AOP(LeftInt, RightInt, OutResult);
	}
	void SetBitByIndex(size_t Index,char BitValue)
	{
		size_t UnitIndex = Index / UNIT_BITS;
		size_t BitIndex = Index % UNIT_BITS;
		if (UnitIndex >= InternalUnits.size())
		{
			size_t NewUnitsNeeded = UnitIndex+1 - InternalUnits.size();
			for (size_t i = 0; i < NewUnitsNeeded; i++)
			{
				InternalUnits.push_back(0);
			}
		}
		InternalUnits[UnitIndex] &= ~UnitType(UnitType(BitValue&1) << BitIndex);
		InternalUnits[UnitIndex] += UnitType(UnitType(BitValue & 1) << BitIndex);
	}
	unsigned int GetBitByIndex(unsigned int Index) const
	{
		unsigned int UnitIndex = Index / UNIT_BITS;
		unsigned int UnitBitIndex = Index % UNIT_BITS;
		if (UnitIndex > InternalUnits.size() - 1)
		{
			return(0);
		}
		return((InternalUnits[UnitIndex] >> (UnitBitIndex)) & 1);
	}
	unsigned int GetBitLength() const
	{
		//funktionen antar att talet är normaliserat och inte har leading nollor
		unsigned int BitLength = InternalUnits.size() * UNIT_BITS;
		UnitType BackUnit = InternalUnits.back();
		for (size_t i = 0; i <UNIT_BITS; i++)
		{
			if (BackUnit>>(UNIT_BITS-1-i)&1 == 0)
			{
				BitLength -= 1;
			}
			else
			{
				break;
			}
		}
		return(BitLength);
	}
	static void PowM_SlidinWindow(MrBigInt const& Base, MrBigInt const& Exponent, MrBigInt const& Mod, MrBigInt& OutResult)
	{
		//DEBUG GREJER
		double TotalDivisionTime = 0;
		double TotalMultiplicationTime = 0;
		clock_t TempTimer = clock();

		clock_t TotalTimeTimer = clock();
		unsigned int K = 10;
		unsigned int NumberOfIterations = (1<<(K - 1)) - 1;
		unsigned int t_ExponentBitLength = Exponent.GetBitLength();
		std::vector<MrBigInt> GList = std::vector<MrBigInt>(NumberOfIterations*2+2);
		//MrBigInt const& g = Base;
		//MrBigInt const& g1 = Base;
		//MrBigInt const& g2 = (g1 * g1)%Mod;
		GList[1] = Base;
		GList[2] = (GList[1]*GList[1])%Mod;
		for (size_t i = 1; i <= NumberOfIterations; i+=1)
		{
			GList[i * 2 + 1] = (GList[2] * GList[i * 2 - 1]) % Mod;
		}
		OutResult =  MrBigInt(1);
		int i = t_ExponentBitLength;
		std::cout << "Klar med precomputation: " << (clock() - TotalTimeTimer) / double(CLOCKS_PER_SEC) << std::endl;
		while (i >= 0)
		{
			if (Exponent.GetBitByIndex(i) == 0)
			{
				TempTimer = clock();
				OutResult *= OutResult;
				TotalMultiplicationTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				TempTimer = clock();
				OutResult %= Mod;
				TotalDivisionTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				i -= 1;
			}
			else
			{
				//kalkylerar längden på bitstringen
				int L = i-K+1;
				while ((i-1-L) <= K && L < i)
				{
					//L += 1;
					if (Exponent.GetBitByIndex(L) == 1)
					{
						break;
					}
					L += 1;
				}
				//räknar ut indexen i GListan som vi vill ha
				UnitType TempIndex = L;
				UnitType j = 0;
				UnitType GListIndex = 0;
				while (TempIndex <= i)
				{
					GListIndex += Exponent.GetBitByIndex(TempIndex) << j;
					j += 1;
					TempIndex += 1;
				}
				UnitType NumberOfSquare = i - L + 1;
				for (size_t i = 0; i < NumberOfSquare; i++)
				{
					TempTimer = clock();
					OutResult *= OutResult;
					TotalMultiplicationTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
					TempTimer = clock();
					OutResult %= Mod;
					TotalDivisionTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				}
				assert(GListIndex % 2 == 1);
				TempTimer = clock();
				OutResult *= GList[GListIndex];
				TotalMultiplicationTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				TempTimer = clock();
				OutResult %= Mod;
				TotalDivisionTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				assert(OutResult != 0);
				i = L - 1;
			}
		}
		double TotalTime = (clock() - TotalTimeTimer) / double(CLOCKS_PER_SEC);
		double TotalOtherTime = TotalTime - TotalMultiplicationTime - TotalDivisionTime;
		std::cout << "Total time SlidinWindow: " << TotalTime << std::endl;
		std::cout << "Total Multiplication Time: " << TotalMultiplicationTime<<" Percent: "<<TotalMultiplicationTime/TotalTime << std::endl;
		std::cout << "Total Division Time: " << TotalDivisionTime <<" Percent: "<<TotalDivisionTime/TotalTime <<std::endl;
		std::cout << "Total other time: " << TotalOtherTime <<" Percent: "<< TotalOtherTime /TotalTime <<std::endl;
	}
	static void PowM(MrBigInt const& Base, MrBigInt const& Exponent, MrBigInt const& Mod,MrBigInt& OutResult)
	{
		//itererar över exponentsens bits
		MrBigInt::PowM_SlidinWindow(Base, Exponent, Mod, OutResult);
		return;
		unsigned int ExponentBitSize = UNIT_BITS * Exponent.InternalUnits.size();
		OutResult = MrBigInt(1);
		MrBigInt BaseCopy(Base);
		MrBigInt ExponentCopy(Exponent);
		clock_t TotaltTimeTimer = clock();
		clock_t TempTimer = 0;
		double TotalDivisionTime = 0;
		double TotalMultiplicationTime = 0;
		if (ExponentCopy.GetBitByIndex(0) == 1)
		{
			OutResult = Base;
		}
		for (size_t i = 1; i < ExponentBitSize; i++)
		{
			TempTimer = clock();
			BaseCopy *= BaseCopy;
			TotalMultiplicationTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
			TempTimer = clock();
			BaseCopy %= Mod;
			TotalDivisionTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
			if (ExponentCopy.GetBitByIndex(i) == 1)
			{
				TempTimer = clock();
				OutResult *= BaseCopy;
				TotalMultiplicationTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
				TempTimer = clock();
				OutResult %= Mod;
				TotalDivisionTime += (clock() - TempTimer) / double(CLOCKS_PER_SEC);
			}
		}
		double TotalTime = (clock() - TotaltTimeTimer) / double(CLOCKS_PER_SEC);
		std::cout << "Total time: " << TotalTime <<std::endl;
		std::cout << "Total multiplication time: " << TotalMultiplicationTime << " Percentage: "<<TotalMultiplicationTime/TotalTime << std::endl;;
		std::cout << "Total division time: "<< TotalDivisionTime << " Percentage: " << TotalDivisionTime / TotalTime << std::endl;;
	}
	static void Pow(MrBigInt const& Base,unsigned int Exponent,MrBigInt& OutResult)
	{
		//auto Timer = std::chrono::steady_clock::now();
		OutResult = MrBigInt(1);
		///*
		size_t NumberOfRepititions = std::ceil(std::sqrt(Exponent));//Metod med konstant division, just nu snabbare med storleksordning 100ggr
		size_t NumberBeforeDivisible = Exponent % NumberOfRepititions;
		for (size_t i = 0; i < NumberBeforeDivisible; i++)
		{
			OutResult *= Base;
		}
		MrBigInt BaseCopy = MrBigInt(Base);
		for (size_t i = 1; i < NumberOfRepititions; i++)
		{
			assert(BaseCopy.InternalUnits.back() != 0);
			BaseCopy *= Base;
		}
		Exponent /= NumberOfRepititions;
		for (size_t i = 0; i < Exponent; i++)
		{
			assert(OutResult.InternalUnits.back() != 0);
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
				InternalUnits[i] +=(UnitType)(((UnitType)CharData[ByteIterator]) << (j * 8));
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


	MrBigInt& operator<<=(unsigned int BitsToShift)
	{
		if (BitsToShift == 0)
		{
			return(*this);
		}
		if (InternalUnits.size() == 1 && InternalUnits[0] == 0)
		{
			return(*this);
		}
		//MrBigInt NewInt;
		unsigned int NewZeroUnits = BitsToShift / UNIT_BITS;
		unsigned int SingleBitsToShift = BitsToShift % UNIT_BITS;
		//unsigned int OldIntSize = InternalUnits.size();
		unsigned int OverFlowBytes = 0;
		unsigned int StartIndexOffset = 0;
		//kollar huruvida den enskilda biten på slutet av den första kommer overflowa, då behöver vi en till unit
		///*
		OverFlowBytes = InternalUnits.back() >> (UNIT_BITS - SingleBitsToShift);
		for (size_t i = 0; i < NewZeroUnits; i++)
		{
			InternalUnits.push_back(0);
		}
		if (SingleBitsToShift != 0)
		{
			//unsigned int DebugGrejer = InternalUnits.back();
			//unsigned int DebugGrejer2 = UNIT_BITS - SingleBitsToShift;
			//unsigned int DebugGrejer3 = DebugGrejer >> DebugGrejer2;
			if (OverFlowBytes != 0)
			{
				InternalUnits.push_back(OverFlowBytes);
				StartIndexOffset += 1;
			}
		}
		//*/
		unsigned int NewIntSize = InternalUnits.size();
		//int SizeDifference = NewIntSize - OldIntSize;
		UnitType PreviousUnit = 0;
		if (SingleBitsToShift == 0)
		{
			for (int i = NewIntSize - 1 - StartIndexOffset; i >= NewZeroUnits; i--)
			{
				//UnitType NewPreviousUnit = InternalUnits[i - NewZeroUnits];
				InternalUnits[i] = InternalUnits[i - NewZeroUnits];
			}
		}
		else
		{
			for (int i = NewIntSize-1-StartIndexOffset; i >= NewZeroUnits+1; i--)
			{
				UnitType FirstUnit = InternalUnits[i - NewZeroUnits];
				UnitType SecondUnit = InternalUnits[i - NewZeroUnits - 1];
				InternalUnits[i] = FirstUnit << SingleBitsToShift;
				InternalUnits[i] += SecondUnit >> (UNIT_BITS - SingleBitsToShift);
			}
			InternalUnits[NewZeroUnits] = InternalUnits[0] << SingleBitsToShift;
		}
		for (size_t i = 0; i < NewZeroUnits; i++)
		{
			InternalUnits[i] = 0;
		}
		//går baklänges och lägger till grejer
		/*
		UnitType PreviousOriginalUnitValue = InternalUnits.back();
		if (SingleBitsToShift == 0)
		{
			for (int i = NewIntSize-1; i >= SizeDifference; i--) 
			{
				InternalUnits[i] = InternalUnits[i - SizeDifference];
			}
			if (Overflowed == 1)
			{
				InternalUnits[SizeDifference - 1] = InternalUnits[SizeDifference - 1] << SingleBitsToShift;
			}
			for (size_t i = 0; i < SizeDifference-Overflowed; i++)
			{
				InternalUnits[i] = 0;
			}
		}
		else
		{
			if (SizeDifference != 0)
			{
				for (int i = NewIntSize - 1; i >= SizeDifference; i--)
				{
					UnitType CurrentUnit = InternalUnits[i - SizeDifference];
					InternalUnits[i] = CurrentUnit >> (UNIT_BITS - SingleBitsToShift);
					InternalUnits[i] += PreviousOriginalUnitValue << SingleBitsToShift;
					PreviousOriginalUnitValue = CurrentUnit;
				}
				if (Overflowed == 1)
				{
					InternalUnits[SizeDifference - 1] = InternalUnits[SizeDifference - 1] << SingleBitsToShift;
				}
				for (size_t i = 0; i < SizeDifference-Overflowed; i++)
				{
					InternalUnits[i] = 0;
				}
			}
			else
			{
				UnitType PreviousBits = 0;
				for (size_t i = 0; i < NewIntSize; i++)
				{
					UnitType CurrentUnit = InternalUnits[i];
					InternalUnits[i] <<= SingleBitsToShift;
					InternalUnits[i] += PreviousBits;
					PreviousBits = CurrentUnit >> (UNIT_BITS - SingleBitsToShift);
				}
			}
		}
		*/
		//NewInt.InternalUnits = std::vector<UnitType>(InternalUnits.size() + NewZeroUnits, 0);
		//assert(InternalUnits.back() != 0 || InternalUnits.size() == 1);
		return(*this);
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
		MrBigInt::UnsignedDivide_AOP(*this, RightInt, Remainder, Quotient);
		IsNegative = false;
		std::swap(Remainder.InternalUnits, InternalUnits);
		return(*this);
	}
	MrBigInt& operator*=(const MrBigInt& RightInt)
	{
		//MrBigInt Result;
		//MrBigInt TestCopy(RightInt);
		//MrBigInt TestCopyThis(*this);
		//MrBigInt KaratsubaResult;
		//MrBigInt LongResult;
		//KaratsubaMultiplication(TestCopyThis, TestCopy, KaratsubaResult);
		//LongMultiplication(TestCopyThis, TestCopy, LongResult);
		//if (LongResult != KaratsubaResult)
		//{
		//	LongMultiplication(TestCopyThis, TestCopy, LongResult);
		//	std::cout << "hej" << std::endl;
		//}
		if (RightInt.InternalUnits.size() > 70 || InternalUnits.size() > 70)
		{
			KaratsubaMultiplication(*this, RightInt, *this);
		}
		else
		{
			MrBigInt Result;
			LongMultiplication(*this, RightInt, Result);
			std::swap(this->InternalUnits, Result.InternalUnits);
		}
		//std::swap(Result.InternalUnits, InternalUnits);
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
		MrBigInt::UnsignedDivide_AOP(*this, RightInt, Remainder, Quotient);
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
		//(*this) += RightIntCopy;'
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
			UnitType ThisValue = 0;
			UnitType OtherValue = 0;
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
	std::string GetHexEncodedString() const
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
			ReturnValue += " ";
		}
		return(ReturnValue);
	}
	std::string GetBinaryString() const
	{
		std::string ReturnValue = "";
		size_t AmountOfBits = InternalUnits.size() * UNIT_BITS;
		for (int i = AmountOfBits-1; i >= 0; i--)
		{
			ReturnValue += std::to_string(GetBitByIndex(i));
			if (i % UNIT_BITS == 0 && i != 0)
			{
				ReturnValue += " ";
			}
		}
		return(ReturnValue);
	}
};
//*/