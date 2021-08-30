#pragma once
#include <iostream>
#include <string>
#include <memory>
namespace MBSystem
{
	enum class SystemErrorCode
	{
		OK,
		VariableNotDefine,
		Null
	};
	SystemErrorCode TranslateErrorCode(uint64_t ErrorCodeToTranslate);
	SystemErrorCode SetEnvironmentVariable(const void* VariableNameData, size_t VariableNameSize, const void* VariableValueData, size_t VariableValueSize);
	SystemErrorCode SetEnvironmentVariable(std::string const& VariableName, std::string const& VariableValue);
	std::string GetEnvironmentVariable(std::string const& VariableName, SystemErrorCode* OutError = nullptr);

	struct MBProcessHandle;
	void MBProcessHandleDeleter(MBProcessHandle* HandleToDelete);
	class UniDirectionalSubProcess
	{
	private:
		SystemErrorCode m_LastError = SystemErrorCode::Null;
		std::unique_ptr<MBProcessHandle> m_AssociatedHandler;
		bool m_IsValid = true;
		bool m_Closed = false;
	public:
		UniDirectionalSubProcess();

		UniDirectionalSubProcess(UniDirectionalSubProcess const&) = delete;
		UniDirectionalSubProcess& operator=(UniDirectionalSubProcess const&) = delete;

		UniDirectionalSubProcess(std::string const& CommandToExecute,bool Read);
		SystemErrorCode SendData(const void* Data, size_t DataSize);
		SystemErrorCode SendData(std::string const& DataToSend);
		size_t RecieveDataToBuffer(void* Buffer, size_t MaxBytesToRead);
		std::string RecieveData(size_t MaxBytesToRead = 2048);
		bool Finished();
		SystemErrorCode GetLastError();

		void flush();
		void close();
		bool good();
		bool open();
		~UniDirectionalSubProcess();
	};

};
