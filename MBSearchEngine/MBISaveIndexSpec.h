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
};

//specifika för MBSearchEngine.h
namespace MBSearchEngine
{
	template<> void MBI_SaveObjectToFile(std::fstream& OutFile, TokenDictionary& DictionaryToSave)
	{
		MBI_SaveVectorToFile(OutFile, DictionaryToSave.m_TokenMapInMemory);
	}
	template<> TokenDictionary MBI_ReadObjectFromFile(std::fstream& FileToReadFrom)
	{
		TokenDictionary ReturnValue;
		ReturnValue.m_TokenMapInMemory = MBI_ReadVectorFromFile<TokenDictionaryEntry>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile<TokenDictionaryEntry>(std::fstream& FileToSaveTo, TokenDictionaryEntry& EntryToSave)
	{
		MBI_SaveObjectToFile(FileToSaveTo, EntryToSave.TokenData);
		size_t DocumentFrequency = EntryToSave.DocumentFrequency;
		size_t PostingIndex = EntryToSave.PostingIndex;
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, DocumentFrequency);
		MBI_SaveObjectToFile<size_t>(FileToSaveTo, PostingIndex);
	}
	template<> TokenDictionaryEntry MBI_ReadObjectFromFile<TokenDictionaryEntry>(std::fstream& FileToReadFrom)
	{
		TokenDictionaryEntry ReturnValue;
		ReturnValue.TokenData = MBI_ReadObjectFromFile<std::string>(FileToReadFrom);
		ReturnValue.DocumentFrequency = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		ReturnValue.PostingIndex = MBI_ReadObjectFromFile<size_t>(FileToReadFrom);
		return(ReturnValue);
	}
	template<> void MBI_SaveObjectToFile<PostingsList>(std::fstream& FileToSaveTo, PostingsList& PostingsListToSave)
	{
		MBI_SaveVectorToFile(FileToSaveTo,PostingsListToSave.m_PostingsInMemory);
	}
	template<> PostingsList MBI_ReadObjectFromFile<PostingsList>(std::fstream& FileToReadFrom)
	{
		PostingsList ReturnValue;
		ReturnValue.m_PostingsInMemory = MBI_ReadVectorFromFile<PostingClass>(FileToReadFrom);
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
};