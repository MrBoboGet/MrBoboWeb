#pragma once
#include <MBSearchEngine/MBISave.h>
#include <MBSearchEngine/MBSearchEngine.h>
namespace MBSearchEngine
{
	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, size_t& SourceInteger)
	{
		size_t IntegerToSave = SourceInteger;
		do
		{
			char ByteToWrite = IntegerToSave % 128;//sista 7 bitsen
			IntegerToSave >>= 7;
			if (IntegerToSave == 0)
			{
				ByteToWrite += 128;//sätter sista till 1
			}
			//assert(ByteToWrite != 0);
			FileToSaveTo.write(&ByteToWrite, 1);
		} while (IntegerToSave > 0);
	}
	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, int& SourceInteger)
	{
		size_t IntegerToSave = SourceInteger;
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, IntegerToSave);
	}
	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, std::string& SourceString)
	{
		size_t SourceStringSize = SourceString.size();
		MBI_SaveObjectToFile(FileToSaveTo, SourceStringSize);
		FileToSaveTo.write(&SourceString[0], SourceStringSize);
	}
	template<> size_t MBI_GetObjectDiskSize(const size_t& SourceInteger)
	{
		size_t ReturnValue = 1;
		size_t IntegerToEvaluate = SourceInteger;
		IntegerToEvaluate >>= 7;
		while (IntegerToEvaluate > 0)
		{
			ReturnValue += 1;
			IntegerToEvaluate >>= 7;
		}
		return(ReturnValue);
	}
	template<> size_t MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		size_t ReturnValue = 0;
		unsigned char IntegerData = 0;
		int NumberOfIterations = 0;
		do
		{
			FileToReadFrom.read((char*)&IntegerData, 1);
			assert(FileToReadFrom.good() && FileToReadFrom.is_open());
			//ReturnValue <<= 7;
			ReturnValue += (size_t((IntegerData & (~128)))<<(7*NumberOfIterations));
			NumberOfIterations += 1;
		} while ((IntegerData >> 7) == 0);
		return(ReturnValue);
	}
	template<> int MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		int ReturnValue = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> std::string MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		size_t StringSize = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		std::string ReturnValue = std::string(StringSize, 0);
		FileToReadFrom.read(&ReturnValue[0], StringSize);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, float& SourceFloat)
	{
		//TODO *extremt* bullshit sätt att sava floaten, fixa en faktiskt representation 
		unsigned int IntToSave = *((int*)&SourceFloat);
		for (size_t i = 0; i < 4; i++)
		{
			char CharToSave = IntToSave % 256;
			FileToSaveTo.write(&CharToSave,1);
			IntToSave >>= 8;
		}
	}
	template<> float MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		unsigned int ReturnValue = 0;
		for (size_t i = 0; i < 4; i++)
		{
			char NewChar = 0;
			FileToReadFrom.read(&NewChar, 1);
			ReturnValue += ((unsigned char)NewChar) << (8 * i);
		}
		return(*((float*)&ReturnValue));
	}
};

//specifika för MBSearchEngine.h
namespace MBSearchEngine
{
	template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, DocID& SourceInteger)
	{
		size_t IntegerToSave = SourceInteger;
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, IntegerToSave);
	}
	template<> DocID MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		return(MBI_ReadObjectFromFile<size_t>(FileToReadFrom));
	}
	template<> void MBI_SaveObjectToFile<DocumentIndexData>(std::fstream& FileToSaveTo, DocumentIndexData& DocumentIndexDataToSave)
	{
		MBI_SaveObjectToFile<float>(FileToSaveTo, DocumentIndexDataToSave.DocumentLength);
	}
	template<> DocumentIndexData MBI_ReadObjectFromFile<DocumentIndexData>(std::fstream& FileToReadFrom)
	{
		DocumentIndexData ReturnValue;
		ReturnValue.DocumentLength = MBI_ReadObjectFromFile<float>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile<TokenDictionaryEntry>(std::fstream& FileToSaveTo, TokenDictionaryEntry& EntryToSave)
	{
		MBI_SaveObjectToFile(FileToSaveTo, EntryToSave.TokenData);
		size_t PostingIndex = EntryToSave.PostingIndex;
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, PostingIndex);
	}
	template<> TokenDictionaryEntry MBI_ReadObjectFromFile<TokenDictionaryEntry>(std::fstream& FileToReadFrom)
	{
		TokenDictionaryEntry ReturnValue;
		ReturnValue.TokenData = MBI_ReadObjectFromFile<std::string>(FileToReadFrom);
		ReturnValue.PostingIndex = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile<Posting>(std::fstream& FileToSaveTo, Posting& PostingToSave)
	{
		size_t DocumentReference = PostingToSave.DocumentReference;
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, DocumentReference);
		MBI_SaveObjectToFile<int>(FileToSaveTo, PostingToSave.NumberOfOccurances);
		MBI_SaveVectorToFile<int>(FileToSaveTo, PostingToSave.m_DocumentPositions);
	}
	template<> Posting MBI_ReadObjectFromFile<Posting>(std::fstream& FileToReadFrom)
	{
		Posting PostingToReturn;
		PostingToReturn.DocumentReference = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		PostingToReturn.NumberOfOccurances = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		PostingToReturn.m_DocumentPositions = MBI_ReadVectorFromFile<int>(FileToReadFrom);
		return(PostingToReturn);
	}
	template<> void MBI_SaveObjectToFile(std::fstream& OutFile, TokenDictionary& DictionaryToSave)
	{
		//MBI_SaveVectorToFile(OutFile, DictionaryToSave.m_TokenMapInMemory);
		size_t SizeOfMap = DictionaryToSave.m_TokenMapInMemory.size();
		MBI_SaveObjectToFile<size_t>(OutFile, SizeOfMap);
		for (TokenDictionaryEntry& DictionaryEntry : DictionaryToSave)
		{
			MBI_SaveObjectToFile<TokenDictionaryEntry>(OutFile, DictionaryEntry);
		}
	}
	template<> TokenDictionary MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		TokenDictionary ReturnValue;
		size_t NumberOfEntries = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		for (size_t i = 0; i < NumberOfEntries; i++)
		{
			TokenDictionaryEntry NewEntry = MBI_ReadObjectFromFile<TokenDictionaryEntry>(FileToReadFrom);
			ReturnValue.m_TokenMapInMemory[NewEntry.TokenData] = NewEntry;
		}
		//ReturnValue.m_TokenMapInMemory = MBI_ReadVectorFromFile<TokenDictionaryEntry>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile<PostingsList>(std::fstream& FileToSaveTo, PostingsList& PostingsListToSave)
	{
		//MBI_SaveVectorToFile(FileToSaveTo,PostingsListToSave.m_PostingsInMemory);
		size_t PostingSize = PostingsListToSave.size();
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, PostingSize);
		for (PostingClass Post : PostingsListToSave)
		{
			MBI_SaveObjectToFile<Posting>(FileToSaveTo, Post);
		}
		MBI_SaveVectorToFile<size_t>(FileToSaveTo, PostingsListToSave.m_SortedDocumentIDs);
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, PostingsListToSave.m_DocumentFrequency);
	}
	template<> PostingsList MBI_ReadObjectFromFile<PostingsList>(std::fstream& FileToReadFrom)
	{
		PostingsList ReturnValue;
		size_t NumberOfPostings = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		for (size_t i = 0; i < NumberOfPostings; i++)
		{
			Posting NewMember = MBI_ReadObjectFromFile<Posting>(FileToReadFrom);
			ReturnValue.m_PostingsInMemory[NewMember.DocumentReference] = NewMember;
		}
		//ReturnValue.m_PostingsInMemory = MBI_ReadVectorFromFile<PostingClass>(FileToReadFrom);
		ReturnValue.m_SortedDocumentIDs = MBI_ReadVectorFromFile<size_t>(FileToReadFrom);
		ReturnValue.m_DocumentFrequency = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		return(ReturnValue);
	}
};