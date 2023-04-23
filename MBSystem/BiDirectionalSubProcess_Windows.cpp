#include "BiDirectionalSubProcess.h"

#ifdef MBWindows
#include <windows.h> 
#include <fileapi.h>

#include <iostream>

namespace MBSystem
{
    std::string h_GetErrorMessage()
    {
        auto ErrorCode = GetLastError(); 
        char Buffer[1024];
        int MessageSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            ErrorCode,
            0,
            Buffer,
            sizeof(Buffer),
            NULL);
        return("Error code "+std::to_string(ErrorCode)+": "+std::string(Buffer,Buffer+MessageSize));


    }
    struct i_WindowsPipeData
    {
        //used by close to ensure that multiple closes doesn't interfere
        bool Initialized = false;
        bool WriteValid = false;
        bool ReadValid = false;

        PROCESS_INFORMATION SubProcessInfo;
        HANDLE Stdin_R;
        HANDLE Stdin_W;
        HANDLE Stdout_R;
        HANDLE Stdout_W;
        HANDLE Stderr_W;
    };
    BiDirectionalSubProcess::BiDirectionalSubProcess(std::string const& ProgramName,std::vector<std::string> const& Args)
    {
        std::string CommandString = ProgramName + " "; 
        m_ImplementationData = std::unique_ptr<void,void(*)(void*)>(new i_WindowsPipeData(),[](void* Data) -> void
                {
                    delete (i_WindowsPipeData*)Data; 
                });
        i_WindowsPipeData& SubInfo = *(reinterpret_cast<i_WindowsPipeData*>(m_ImplementationData.get()));
        for(std::string const& String : Args)
        {
            CommandString += String;   
            CommandString += " ";
        }
        SECURITY_ATTRIBUTES saAttr; 
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
        saAttr.bInheritHandle = TRUE; 
        saAttr.lpSecurityDescriptor = NULL; 
        //Create In
        if(!CreatePipe(&SubInfo.Stdin_R,&SubInfo.Stdin_W,&saAttr,0))
        {
            throw std::runtime_error("Error createing stdin pipe");
        }
        if(!CreatePipe(&SubInfo.Stdout_R,&SubInfo.Stdout_W,&saAttr,0))
        {
            CloseHandle(SubInfo.Stdin_R);
            CloseHandle(SubInfo.Stdin_W);
            throw std::runtime_error("Error createing stdin pipe");
        }
        SubInfo.Stderr_W = CreateFileA("nul",GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
        if(SubInfo.Stderr_W == INVALID_HANDLE_VALUE)
        {
            CloseHandle(SubInfo.Stdout_R);
            CloseHandle(SubInfo.Stdout_W);
            CloseHandle(SubInfo.Stdin_R);
            CloseHandle(SubInfo.Stdin_W);
            throw std::runtime_error("Error creating stderr pipe");
        }
	  	//TCHAR szCmdline[]=TEXT("child");
		//PROCESS_INFORMATION piProcInfo; 
		STARTUPINFO siStartInfo;
		 
		// Set up members of the PROCESS_INFORMATION structure. 
		 
		ZeroMemory( &SubInfo.SubProcessInfo, sizeof(PROCESS_INFORMATION) );
		 
		// Set up members of the STARTUPINFO structure. 
		// This structure specifies the STDIN and STDOUT handles for redirection.
		 
		ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
		siStartInfo.cb = sizeof(STARTUPINFO); 
		siStartInfo.hStdError = SubInfo.Stderr_W;
		siStartInfo.hStdOutput = SubInfo.Stdout_W;
		siStartInfo.hStdInput = SubInfo.Stdin_R;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
		 
		// Create the child process. 
			
		bool Result = CreateProcess(NULL, 
			  CommandString.data(),     // command line 
			  NULL,          // process security attributes 
			  NULL,          // primary thread security attributes 
			  TRUE,          // handles are inherited 
			  CREATE_NO_WINDOW,             // creation flags 
			  NULL,          // use parent's environment 
			  NULL,          // use parent's current directory 
			  &siStartInfo,  // STARTUPINFO pointer 
			  &SubInfo.SubProcessInfo);  // receives PROCESS_INFORMATION  
        if(!Result)
        {
            //mega code duplication, but no good sentinel for invalid file descriptiors
            //make this mostly neccessary 
            CloseHandle(SubInfo.Stdin_W);
            CloseHandle(SubInfo.Stdin_R);
            CloseHandle(SubInfo.Stderr_W);
            CloseHandle(SubInfo.Stdout_W);
            CloseHandle(SubInfo.Stdout_R);
            throw std::runtime_error("Error creating sub-process");
        }
        Result = CloseHandle(SubInfo.Stdin_R);
        Result = CloseHandle(SubInfo.Stderr_W);
        Result = CloseHandle(SubInfo.Stdout_W);
        SubInfo.ReadValid = true;
        SubInfo.WriteValid = true;
        SubInfo.Initialized = true;
    }
    size_t BiDirectionalSubProcess::Write(void const* Data,size_t DataSize) 
    {
        i_WindowsPipeData& SubInfo = *(reinterpret_cast<i_WindowsPipeData*>(m_ImplementationData.get()));
        size_t ReturnValue = 0;
        if(!SubInfo.WriteValid)
        {
            return(ReturnValue);
        }
        DWORD WrittenBytes = 0;
        BOOL Result = WriteFile(SubInfo.Stdin_W,Data,DataSize,&WrittenBytes,NULL);
        if(!Result)
        {
            SubInfo.WriteValid = false;
            //std::cout<<h_GetErrorMessage()<<std::endl;
            return(0);
        }
        ReturnValue = WrittenBytes;
        return(ReturnValue);
    }
    size_t BiDirectionalSubProcess::ReadSome(void* Buffer,size_t BufferSize) 
    {
        if(BufferSize == 0)
        {
            return(0);   
        }
        i_WindowsPipeData& SubInfo = *(reinterpret_cast<i_WindowsPipeData*>(m_ImplementationData.get()));
        size_t ReturnValue = 0;
        if(!SubInfo.ReadValid)
        {
            return(ReturnValue);
        }
        DWORD ReadBytes = 0;
        //In order to ensure that it blocks until input is available
        BOOL Result = ReadFile(SubInfo.Stdout_R,Buffer,1,&ReadBytes,NULL);
        if(!Result)
        {
            //std::cout<<h_GetErrorMessage()<<std::endl;
            SubInfo.ReadValid = false;
            return(0);
        }
        if(ReadBytes == 0)
        {
            SubInfo.ReadValid = false;
            return(0);
        }
        if(Result && BufferSize == 1)
        {
            return(ReadBytes);   
        }
        DWORD PeekedBytes = 0;
        Result = PeekNamedPipe(SubInfo.Stdout_R,((char*)Buffer)+ReadBytes,BufferSize-1,&PeekedBytes,NULL,NULL);
        if(!Result)
        {
            //std::cout<<h_GetErrorMessage()<<std::endl;
            SubInfo.ReadValid = false;
            return(0);
        }
        Result = ReadFile(SubInfo.Stdout_R,((char*)Buffer)+ReadBytes,PeekedBytes,&PeekedBytes,NULL);
        if(!Result)
        {
            //std::cout<<h_GetErrorMessage()<<std::endl;
            SubInfo.ReadValid = false;
            return(0);
        }
        ReturnValue = ReadBytes+PeekedBytes;
        return(ReturnValue);
    }
    void BiDirectionalSubProcess::Flush()
    {
        i_WindowsPipeData& SubInfo = *(reinterpret_cast<i_WindowsPipeData*>(m_ImplementationData.get()));
        if(!SubInfo.WriteValid)
        {
            return;    
        }
        FlushFileBuffers(SubInfo.Stdin_R);
    }
    void BiDirectionalSubProcess::Close()
    {
        i_WindowsPipeData& SubInfo = *(reinterpret_cast<i_WindowsPipeData*>(m_ImplementationData.get()));
        if(!SubInfo.Initialized)
        {
            return;    
        }
        CancelIoEx(SubInfo.Stdin_W,NULL);
        CancelIoEx(SubInfo.Stdout_R,NULL);
        SubInfo.Initialized = false;
        //CloseHandle(SubInfo.Stdin_W);
        CloseHandle(SubInfo.Stdin_W);
        //CloseHandle(SubInfo.Stderr_W);
        //CloseHandle(SubInfo.Stdout_W);
        CloseHandle(SubInfo.Stdout_R);
        
        CloseHandle(SubInfo.SubProcessInfo.hProcess);
        CloseHandle(SubInfo.SubProcessInfo.hThread);

    }
}
#endif
