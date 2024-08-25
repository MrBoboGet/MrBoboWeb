#include "MBSystem.h"
#include <MBUtility/MBCompileDefinitions.h>
#ifdef MBWindows
#include<cstdlib>
#include <stdio.h>
#else // 
#include<cstdlib>
#endif
namespace MBSystem
{
    std::filesystem::path GetUserHomeDirectory()
    {
        std::filesystem::path ReturnValue; 
#ifdef MBWindows
        ReturnValue = std::getenv("USERPROFILE");
#elif defined(MBPosix)
        //https://linux.die.net/man/3/getpwuid
        //Apperently reading HOME is prefered to using the posix API,
        //I'm not entirely convinced as environemntal variables feels flimsy,
        const char* HomeString = std::getenv("HOME");
        if(HomeString == nullptr)
        {
            throw std::runtime_error("Can't determine home directory for user: HOME environment variable doesn't exist");   
        }
        ReturnValue = HomeString;
#endif
        return(ReturnValue);
    }
    bool System(std::string const& Program,std::vector<std::string> const& Args)
    {
        bool ReturnValue = true;
        std::string StringToExecute = Program+ " ";
        for(auto const& Arg : Args)
        {
            StringToExecute += Arg;
            StringToExecute += " ";
        }
        return std::system(StringToExecute.c_str()) == 0;
    }
	SystemErrorCode TranslateErrorCode(uint64_t ErrorCodeToTranslate)
	{
		SystemErrorCode ReturnValue = SystemErrorCode::OK;

		return(ReturnValue);
	}
	//borde egentligen byta plats på dem här, tänkte inte på att det var en C-string och skulle behöva kopieras med det
	SystemErrorCode SetEnvironmentVariable(const void* VariableNameData, size_t VariableNameSize, const void* VariableValueData, size_t VariableValueSize)
	{
		SystemErrorCode ReturnValue = SystemErrorCode::OK;
		std::string VariableName = std::string((const char*)VariableNameData, VariableNameSize);
		std::string VariableValue = std::string((const char*)VariableValueData, VariableValueSize);

#ifdef MBWindows
		errno_t ErrorCode = _putenv_s(VariableName.data(), VariableValue.data());
#elif defined(MBPosix)
		int ErrorCode = setenv(VariableName.data(), VariableValue.data(), true);
#else 
        static_assert(false,"Only windows and posix systems supported");
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
		FILE* OSHandle = nullptr;
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
#ifdef MBWindows
		if (Read)
		{
			m_AssociatedHandler->OSHandle = _popen(CommandToExecute.data(), "rb");
		}
		else
		{
			m_AssociatedHandler->OSHandle = _popen(CommandToExecute.data(), "wb");
		}
#elif defined(MBPosix) 
		if (Read)
		{
			m_AssociatedHandler->OSHandle = popen(CommandToExecute.data(), "r");
		}
		else
		{
			m_AssociatedHandler->OSHandle = popen(CommandToExecute.data(), "w");
		}
#else 
        static_assert(false,"Only windows and posix supported");
#endif
	}
	SystemErrorCode UniDirectionalSubProcess::SendData(const void* Data, size_t DataSize)
	{
#ifdef MBWindows
		int ErrorCode = fwrite(Data, 1, DataSize, m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#elif defined(MBPosix) // 
		int ErrorCode = fwrite(Data, 1, DataSize, m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
		return(m_LastError);
	}
	SystemErrorCode UniDirectionalSubProcess::SendData(std::string const& DataToSend)
	{				
		return(SendData(DataToSend.data(), DataToSend.size()));
	}
	size_t UniDirectionalSubProcess::Read(void* Buffer, size_t MaxBytesToRead) 
	{
		size_t ReadBytes = 0;
#ifdef MBWindows
		ReadBytes = fread(Buffer, 1, MaxBytesToRead, m_AssociatedHandler->OSHandle);
		if (ReadBytes != MaxBytesToRead)
		{
			m_IsValid = false;
		}
#elif defined(MBPosix)
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
		size_t ReadBytes = Read(ReturnValue.data(), MaxBytesToRead);
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
#ifdef MBWindows
		int ErrorCode = fflush(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#elif defined(MBPosix) // 
		int ErrorCode = fflush(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
	}
	void UniDirectionalSubProcess::close()
	{
		if (m_Closed)
		{
			return;
		}
#ifdef MBWindows
		int ErrorCode = _pclose(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#elif defined(MBPosix)
		int ErrorCode = pclose(m_AssociatedHandler->OSHandle);
		m_LastError = TranslateErrorCode(ErrorCode);
#endif
		m_Closed = true;
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
