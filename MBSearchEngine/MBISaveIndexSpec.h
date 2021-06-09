#pragma once
#include <MBSearchEngine/MBISave.h>
#include <MBSearchEngine/MBSearchEngine.h>
namespace MBSearchEngine
{
	template<> void MBI_SkipObject(std::fstream& FileToReadFrom, size_t*)
	{
		unsigned char IntegerData = 0;
		int NumberOfIterations = 0;
		do
		{
			FileToReadFrom.read((char*)&IntegerData, 1);
			assert(FileToReadFrom.good() && FileToReadFrom.is_open());
			NumberOfIterations += 1;
		} while ((IntegerData >> 7) == 0);
	}
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
//fixed width integer grejer

namespace MBSearchEngine
{
	inline uintmax_t MBI_ReadFixedSizeInteger(std::fstream& FileToReadFrom, char IntegerWidth)
	{
		uintmax_t ReturnValue = 0;
		char ReadBuffer[sizeof(uintmax_t) / 8];
		FileToReadFrom.read(ReadBuffer, IntegerWidth);
		for (size_t i = 0; i < IntegerWidth; i++)
		{
			ReturnValue += (((unsigned char)ReadBuffer[i]) << (i * 8));
		}
		return(ReturnValue);
	}
	inline void MBI_SaveFixedSizeInteger(std::fstream& FileToSaveTo, uintmax_t IntegerToSave, char NumberOfBytes)
	{
		char ArrayToWrite[sizeof(uintmax_t) / 8];
		for (size_t i = 0; i < NumberOfBytes; i++)
		{
			ArrayToWrite[i] = IntegerToSave & (255);
			IntegerToSave >>= 8;
		}
		FileToSaveTo.write(ArrayToWrite, NumberOfBytes);
	}
	inline void MBI_ReadFixedSizeIntegerArray(std::fstream& FileToReadFrom, void* Buffer, size_t NumberOfElements, char IntegerWidth)
	{
		size_t BufferOffset = 0;
		for (size_t i = 0; i < NumberOfElements; i++)
		{
			uintmax_t CurrentInteger = MBI_ReadFixedSizeInteger(FileToReadFrom, IntegerWidth);
			memcpy(&(static_cast<char*>(Buffer)[BufferOffset]), &CurrentInteger, IntegerWidth);
			BufferOffset += IntegerWidth;
		}
	}
	inline void MBI_ReadFixedSizeIntegerArray(std::fstream& FileToReadFrom, void* Buffer, char IntegerWidth)
	{
		size_t NumberOfElements = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		MBI_ReadFixedSizeIntegerArray(FileToReadFrom, Buffer, NumberOfElements, IntegerWidth);
	}
};




//specifika för MBSearchEngine.h
namespace MBSearchEngine
{
	//template<> void MBI_SaveObjectToFile(std::fstream& FileToSaveTo, DocID& SourceInteger)
	//{
	//	size_t IntegerToSave = SourceInteger;
	//	MBI_SaveObjectToFile<size_t>(FileToSaveTo, IntegerToSave);
	//}
	//template<> DocID MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	//{
	//	return(MBI_ReadObjectFromFile<size_t>(FileToReadFrom));
	//}
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
	template<> void MBI_SkipObject<Posting>(std::fstream& FileToReadFrom, Posting*)
	{
		MBI_SkipObject<size_t>(FileToReadFrom,nullptr);
		MBI_SkipObject<size_t>(FileToReadFrom, nullptr);
		MBI_SkipVector<size_t>(FileToReadFrom);
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
	template<> void MBI_SkipObject(std::fstream& FileToReadFrom,PostingsList*)
	{
		size_t NumberOfPostings = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		for (size_t i = 0; i < NumberOfPostings; i++)
		{
			MBI_SkipObject<Posting>(FileToReadFrom,nullptr);
		}
		MBI_SkipVector<size_t>(FileToReadFrom);
		MBI_SkipObject<size_t>(FileToReadFrom,nullptr);
	}
	template<> PostingsList MBI_ReadObjectFromFile<PostingsList>(std::fstream& FileToReadFrom)
	{
		PostingsList ReturnValue;
		size_t NumberOfPostings = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		for (size_t i = 0; i < NumberOfPostings; i++)
		{
			Posting NewMember = MBI_ReadObjectFromFile<Posting>(FileToReadFrom);
			//ReturnValue.m_PostingsInMemory[NewMember.DocumentReference] = NewMember;
			swap(ReturnValue.m_PostingsInMemory[NewMember.DocumentReference], NewMember);
		}
		//ReturnValue.m_PostingsInMemory = MBI_ReadVectorFromFile<PostingClass>(FileToReadFrom);
		ReturnValue.m_SortedDocumentIDs = MBI_ReadVectorFromFile<size_t>(FileToReadFrom);
		ReturnValue.m_DocumentFrequency = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		return(ReturnValue);
	}

	//PostinglistHandler
	template<> void MBI_SaveObjectToFile<PostingslistHandler>(std::fstream& FileToSaveTo, PostingslistHandler& HandlerToSave)
	{
		size_t NumberOfLists = HandlerToSave.NumberOfPostings();
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, NumberOfLists);
		HandlerToSave.p_MergeFilesAndCache(FileToSaveTo);
	}
	template<> PostingslistHandler MBI_ReadObjectFromFile<PostingslistHandler>(std::fstream& FileToReadFrom)
	{
		PostingslistHandler ReturnValue = PostingslistHandler();
		size_t NumberOfPostings = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		ReturnValue.m_NumberOfPostings = NumberOfPostings;
		PostingDiskDataInfo NewDiskDataInfo;
		NewDiskDataInfo.DiskDataFilepath = "";
		NewDiskDataInfo.IsTemporaryFile = false;
		for (size_t i = 0; i < NumberOfPostings; i++)
		{
			NewDiskDataInfo.PostinglistFilePositions[i] = MBI_ReadFixedSizeInteger(FileToReadFrom, 4);
		}
		for (size_t i = 0; i < NumberOfPostings; i++)
		{
			//NewDiskDataInfo.PostinglistFilePositions[i] = FileToReadFrom.tellg();
			if (i < PostingslistHandler::MaxPostingsToRead)
			{
				PostingsList* NewMemory = new PostingsList();
			    *NewMemory = MBI_ReadObjectFromFile<PostingsList>(FileToReadFrom);
				ReturnValue.m_PostingsInMemory[i] = std::shared_ptr<PostingsList>(NewMemory);
			}
			else
			{
				break;
			}
			//else
			//{
			//	MBI_SkipObject<PostingsList>(FileToReadFrom,nullptr);
			//}
		}
		ReturnValue.m_PostingsOnDisk.push_back(NewDiskDataInfo);
		return(ReturnValue);
	}
};