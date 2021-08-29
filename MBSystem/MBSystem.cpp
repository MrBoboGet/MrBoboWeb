#include "MBSystem.h"

#ifdef _WIN32
#include<cstdlib>
#include <stdio.h>
#else // 
#include<cstdlib>
#endif
namespace MBSystem
{
	SystemErrorCode TranslateErrorCode(uint64_t ErrorCodeToTranslate)
	{
		SystemErrorCode ReturnValue = SystemErrorCode::OK;

		return(ReturnValue);
	}
	//borde egentligen byta plats p� dem h�r, t�nkte inte p� att det var en C-string och skulle beh�va kopieras med det
	SystemErrorCode SetEnvironmentVariable(const void* VariableNameData, size_t VariableNameSize, const void* VariableValueData, size_t VariableValueSize)
	{
		SystemErrorCode ReturnValue = SystemErrorCode::OK;
		std::string VariableName = std::string((const char*)VariableNameData, VariableNameSize);
		std::string VariableValue = std::string((const char*)VariableValueData, VariableValueSize);

#ifdef _WIN32
		errno_t ErrorCode = _putenv_s(VariableName.data(), VariableValue.data());
#else // 
		int ErrorCode = setenv(VariableName.data(), VariableValue.data(), true);
#endif
		return(ReturnValue);
	}
	SystemErrorCode SetEnvironmentVariable(std::string const& VariableName, std::string const& VariableValue)
	{
		return(SetEnvironmentVariable(VariableName.data(), VariableName.size(), VariableValue.data(), VariableValue.size()));
	}
	std::string GetEnvironmentVariable(std::string const& VariableName, SystemErrorCode* OutError)
	{
		SystemErrorCode SystemError = SystemErrorCode::OK;
		std::string ReturnValue = "";
		const char* EnvironmentValue = std::getenv(VariableName.data());
		if (EnvironmentValue == nullptr)
		{
			SystemError = SystemErrorCode::VariableNotDefine;
		}
		else
		{
			ReturnValue = std::string(EnvironmentValue);
		}
		if (OutError != nullptr)
		{
			*OutError = SystemError;
		}
		return(ReturnValue);
	}

	//BEGIN SubProcess
	struct MBProcessHandle
	{
#ifdef _WIN32
		FILE* OSHandle = nullptr;
#else // 
		FILE* OSHandle = nullptr;
#endif
	};
	UniDirectionalSubProcess::UniDirectionalSubProcess()
	{
		m_AssociatedHandler = std::unique_ptr<MBProcessHandle>(new MBProcessHandle());
	}
//	SubProcess::SubProcess(const char* CommandToExecuteData)
//	{
//		m_AssociatedHandler = std::unique_ptr<MBProcessHandle>(new MBProcessHandle());
//#ifdef _WIN32
//		m_AssociatedHandler->OSHandle = _popen(CommandToExecuteData, "rb");
//#else // 
//		m_AssociatedHandler->OSHandle = popen(CommandToExecuteData,"r");
//#endif
//	}
	UniDirectionalSubProcess::UniDirectionalSubProcess(std::string const& CommandToExecute,bool Read)
	{
		m_AssociatedHandler = std::unique_ptr<MBProcessHandle>(new MBProcessHandle());
#ifdef _WIN32
		if (Read)
		{
			m_AssociatedHandler->OSHandle = _popen(CommandToExecute.data(), "rb");
		}
		else
		{
			m_AssociatedHandler->OSHandle = _popen(CommandToExecute.data(), "wb");
		}
#else // 
		if (Read)
		{
			m_AssociatedHandler->OSHandle = popen(CommandToExecute.data(), "r");
		}
		else
		{
			m_AssociatedHandler->OSHandle = popen(CommandToExecute.data(), "w");
		}
#endif
	}
	SystemErrorCode UniDirectionalSubProcess::SendData(const void* Data, size_t DataSize)
	{
#ifdef _WIN32
		int ErrorCode = fwrite(Data, 1, DataSize, m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#else // 
		int ErrorCode = fwrite(Data, 1, DataSize, m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
		return(m_LastError);
	}
	SystemErrorCode UniDirectionalSubProcess::SendData(std::string const& DataToSend)
	{				
		return(SendData(DataToSend.data(), DataToSend.size()));
	}
	size_t UniDirectionalSubProcess::RecieveDataToBuffer(void* Buffer, size_t MaxBytesToRead) 
	{
		size_t ReadBytes = 0;
#ifdef _WIN32
		ReadBytes = fread(Buffer, 1, MaxBytesToRead, m_AssociatedHandler->OSHandle);
		if (ReadBytes != MaxBytesToRead)
		{
			m_IsValid = false;
		}
#else // 
		ReadBytes = fread(Buffer, 1, MaxBytesToRead, m_AssociatedHandler->OSHandle);
		if (ReadBytes != MaxBytesToRead)
		{
			m_IsValid = false;
		}
#endif
		if (ReadBytes != MaxBytesToRead)
		{
			m_IsValid = false;
		}
		return ReadBytes;
	}
	std::string UniDirectionalSubProcess::RecieveData(size_t MaxBytesToRead)
	{
		std::string ReturnValue = std::string(MaxBytesToRead, 0);
		size_t ReadBytes = RecieveDataToBuffer(ReturnValue.data(), MaxBytesToRead);
		ReturnValue.resize(ReadBytes);
		return(ReturnValue);
	}
	bool UniDirectionalSubProcess::Finished()
	{
		if (m_IsValid)
		{
			return(false);
		}
		return(true);
	}
	SystemErrorCode UniDirectionalSubProcess::GetLastError()
	{
		return(m_LastError);
	}

	void UniDirectionalSubProcess::flush()
	{
#ifdef _WIN32
		int ErrorCode = fflush(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#else // 
		int ErrorCode = fflush(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
	}
	void UniDirectionalSubProcess::close()
	{
#ifdef _WIN32
		int ErrorCode = _pclose(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#else // 
		int ErrorCode = pclose(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
	}
	bool UniDirectionalSubProcess::good()
	{
		return(m_IsValid);
	}
	bool UniDirectionalSubProcess::open()
	{
		return(m_IsValid);
	}
	UniDirectionalSubProcess::~UniDirectionalSubProcess()
	{
		flush();
		close();
	}
	//END SubProces	s
};