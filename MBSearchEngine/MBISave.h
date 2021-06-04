#pragma once
//#include <MBSearchEngine/MBSearchEngine.h>
#include <vector>
#include <string>
#include <assert.h>
namespace MBSearchEngine
{
	template<typename T> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, T& IntegerToSave)
	{
		return;
	}
	template<typename T> size_t MBI_GetObjectDiskSize(const T& IntegerToEvalaute)
	{
		return(0);
	}
	template<typename T> void MBI_SaveVectorToFile(std::fstream& FileToSaveTo, std::vector<T>& VectorToSave)
	{
		//size_t TotalSize = 0;
		//for (size_t i = 0; i < VectorToSave.size(); i++)
		//{
		//	TotalSize += MBI_GetObjectDiskSize(VectorToSave[i]);
		//}
		//skriver headern
		//size_t TotalSizeOnDisk = TotalSize + MBI_GetObjectDiskSize(VectorToSave.size());
		size_t NumberOfElements = VectorToSave.size();
		//MBI_SaveObjectToFile<size_t>(FileToSaveTo, TotalSizeOnDisk);
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, NumberOfElements);
		for (size_t i = 0; i < VectorToSave.size(); i++)
		{
			MBI_SaveObjectToFile<T>(FileToSaveTo, VectorToSave[i]);
		}
	}
	//variable byte encoding
	template<typename T> T MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		return(T());
	}
	template<typename T> std::vector<T> MBI_ReadVectorFromFile(std::fstream& FileToReadFrom)
	{
		std::vector<T> ReturnValue = {};
		//size_t TotalSize = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		size_t NumberOfElements = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		ReturnValue.reserve(NumberOfElements);
		for (size_t i = 0; i < NumberOfElements; i++)
		{
			ReturnValue.push_back(MBI_ReadObjectFromFile<T>(FileToReadFrom));
		}
		return(ReturnValue);
	}

	template<typename T> void MBI_SkipObject(std::fstream& StreamToRead, T*)
	{
		assert(false);
	}
	template<typename T> void MBI_SkipVector(std::fstream& StreamToRead)
	{
		size_t ElementsToSkip = MBI_ReadObjectFromFile<size_t>(StreamToRead);
		for (size_t i = 0; i < ElementsToSkip; i++)
		{
			MBI_SkipObject<T>(StreamToRead,nullptr);
		}
	}
};


//specifika för MBSearchEngine.h

//namespace MBSearchEngine
//{
//	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, size_t& SourceInteger)
//	{
//		size_t IntegerToSave = SourceInteger;
//		while (IntegerToSave > 0)
//		{
//			char ByteToWrite = IntegerToSave % 128;//sista 7 bitsen
//			IntegerToSave >>= 7;
//			if (IntegerToSave == 0)
//			{
//				ByteToWrite += 128;//sätter sista till 1
//			}
//			FileToSaveTo.write(&ByteToWrite, 1);
//		}
//	}
//	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, std::string& SourceString)
//	{
//		size_t SourceStringSize = SourceString.size();
//		MBI_SaveObjectToFile(FileToSaveTo, SourceStringSize);
//		FileToSaveTo.write(&SourceString[0], SourceStringSize);
//	}
//	template<> size_t MBI_GetObjectDiskSize(const size_t& SourceInteger)
//	{
//		size_t ReturnValue = 1;
//		size_t IntegerToEvaluate = SourceInteger;
//		IntegerToEvaluate >>= 7;
//		while (IntegerToEvaluate > 0)
//		{
//			ReturnValue += 1;
//			IntegerToEvaluate >>= 7;
//		}
//		return(ReturnValue);
//	}
//	template<> size_t MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
//	{
//		size_t ReturnValue = 0;
//		unsigned char IntegerData = 0;
//		int NumberOfIterations = 0;
//		do
//		{
//			FileToReadFrom.read((char*)&IntegerData, 1);
//			ReturnValue <<= 7;
//			ReturnValue += (IntegerData & 128);
//		} while ((IntegerData >> 7) == 0);
//		return(ReturnValue);
//	}
//	template<> std::string MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
//	{
//		size_t StringSize = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
//		std::string ReturnValue = std::string(StringSize, 0);
//		FileToReadFrom.read(&ReturnValue[0], StringSize);
//		return(ReturnValue);
//	}
//};

//specifika för MBSearchEngine.h
//namespace MBSearchEngine
//{
//	template<> void MBI_SaveObjectToFile(std::fstream& OutFile, TokenDictionary& DictionaryToSave)
//	{
//		MBI_SaveVectorToFile(OutFile, DictionaryToSave.m_TokenMapInMemory);
//	}
//	template<> TokenDictionary MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
//	{
//		TokenDictionary ReturnValue;
//		ReturnValue.m_TokenMapInMemory = MBI_ReadVectorFromFile<TokenDictionaryEntry>(FileToReadFrom);
//		return(ReturnValue);
//	}
//	template<> void MBI_SaveObjectToFile<TokenDictionaryEntry>(std::fstream& FileToSaveTo, TokenDictionaryEntry& EntryToSave)
//	{
//		MBI_SaveObjectToFile(FileToSaveTo, EntryToSave.TokenData);
//		MBI_SaveObjectToFile(FileToSaveTo, EntryToSave.DocumentFrequency);
//		MBI_SaveObjectToFile(FileToSaveTo, EntryToSave.PostingIndex);
//	}
//	template<> TokenDictionaryEntry MBI_ReadObjectFromFile<TokenDictionaryEntry>(std::fstream& FileToReadFrom)
//	{
//		TokenDictionaryEntry ReturnValue;
//		ReturnValue.TokenData = MBI_ReadObjectFromFile<std::string>(FileToReadFrom);
//		ReturnValue.DocumentFrequency = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
//		ReturnValue.PostingIndex = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
//		return(ReturnValue);
//	}
//	friend void MBI_SaveObjectToFile<PostingsList>(std::fstream& FileToSaveTo, PostingsList& PostingsListToSave)
//	{
//		MBI_SaveVectorToFile(FileToSaveTo, PostingsListToSave.m_PostingsInMemory);
//	}
//	friend PostingsList MBI_ReadObjectFromFile<PostingsList>(std::fstream& FileToReadFrom)
//	{
//		PostingsList ReturnValue;
//		ReturnValue.m_PostingsInMemory = MBI_ReadVectorFromFile<PostingClass>(FileToReadFrom);
//		return(ReturnValue);
//	}
//	friend void MBI_SaveObjectToFile<Posting>(std::fstream& FileToSaveTo, Posting& PostingToSave)
//	{
//		MBI_SaveObjectToFile(FileToSaveTo, PostingToSave.DocumentReference);
//		MBI_SaveObjectToFile(FileToSaveTo, PostingToSave.NumberOfOccurances);
//		MBI_SaveVectorToFile(FileToSaveTo, PostingToSave.m_DocumentPositions);
//	}
//	friend Posting MBI_ReadObjectFromFile<Posting>(std::fstream& FileToReadFrom)
//	{
//
//	}
//};