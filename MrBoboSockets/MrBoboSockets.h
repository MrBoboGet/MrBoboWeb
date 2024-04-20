#define NOMINMAX
#define _CRT_RAND_S
#pragma once
#include <string>
#include <string.h>
#include <stdio.h>

#include <thread>
#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "TLSHandler.h"
#include <filesystem>
#include <cinttypes>

#include <MBUtility/MBInterfaces.h>
#include <MBMime/MBMime.h>

#include <map>
#if defined(WIN32) || defined(_WIN32)
typedef uintmax_t MB_OS_Socket;
#elif defined(__linux__)
typedef int MB_OS_Socket;
#endif

uint64_t MBGetFileSize(std::string const& PathToCheck);

class sockaddr;

struct FiledataIntervall
{
	uint64_t FirstByte = 0;
	uint64_t LastByte = 0;
};

namespace MBSockets
{
	typedef uint16_t OSPort;
	void Init();
	int MBCloseSocket(int SocketToClose);
	int MBSocketError();
	enum class MBSocketErrors :uint8_t
	{
		fatal,
		nonfatal
	};
	class Socket
	{
	protected:
	public:
		Socket& operator=(Socket const&) = delete;
		Socket(Socket const&) = delete;
		Socket() {};
		virtual bool IsValid() = 0;
		virtual void Close() = 0;
		virtual ~Socket()
		{

		};
	};
	class UDPSocket : public Socket
	{
	private:
		MB_OS_Socket m_UnderlyingHandle;
		int m_ErrorResult = 0;
		bool m_Invalid = false;
		bool m_SocketClosed = false;
		std::string m_LastErrorMessage = "";
		std::string p_GetLastError();
		void p_HandleError(std::string const& ErrorMessage, bool IsLethal);
	public:
		UDPSocket(std::string const& Adress, std::string const& Port);
		void UDPSendData(std::string const& DataToSend, std::string const& HostAdress, int PortNumber);
		virtual bool IsValid();
		virtual void Close();
		int Bind(int PortToAssociateWith);
		std::string UDPGetData();
		void UDPMakeSocketNonBlocking(float SecondsToWait = 0.5);
		void Listen(std::string const& PortNumber);
	};
	class ConnectSocket : public Socket
	{
	public:

		ConnectSocket() {};
		virtual bool IsConnected() = 0;
		virtual std::string RecieveData(size_t MaxNumberOfBytes = -1) = 0;
		virtual MBError SendData(const void* DataPointer, size_t DataLength) = 0;
		virtual MBError SendData(std::string const& DataToSend) = 0;
		virtual ~ConnectSocket()
		{

		};
	};
	class OSSocket
	{
	protected:
		MB_OS_Socket m_UnderlyingHandle;
		int m_ErrorResult = 0;
		bool m_Invalid = false;
		bool m_OSSocketClosed = false;
		std::string m_LastErrorMessage = "";
		void p_Swap(OSSocket& SocketToSwapWith);
		std::string p_GetLastError();
		void p_HandleError(std::string const& ErrorMessage, bool IsLethal);
		OSSocket();
		void p_SendAllData(MB_OS_Socket ConnectedSocket, const void* DataToSend, size_t DataSize);
		std::string p_RecieveData(MB_OS_Socket ConnectedSocket, size_t MaxNumberOfBytes = -1);
		void p_CloseOSSocket();
	public:
		OSSocket(OSSocket const&) = delete;
		~OSSocket();
	};
	class TCPClient : protected OSSocket, public ConnectSocket
	{
	private:
		bool m_IsConnected = false;
		bool m_Closed = false;

		size_t _m_ai_addrlen = 0;
		sockaddr* _m_addr = nullptr;
		void p_Swap(TCPClient& SocketToSwapWith);
	public:
		TCPClient();
		MBError Initialize(std::string const& Adress, std::string const& Port);
		TCPClient(std::string const& Adress, std::string const& Port);

		int Connect();
		
		//overrides
		virtual bool IsValid() override;
		virtual void Close() override;

		virtual bool IsConnected() override;
		virtual std::string RecieveData(size_t MaxNumberOfBytes = -1) override;
		virtual MBError SendData(const void* DataPointer, size_t DataLength) override;
		virtual MBError SendData(std::string const& DataToSend) override;
		virtual ~TCPClient();
	};
	class TCPServer : protected OSSocket, public ConnectSocket
	{
	private:
		bool m_IsConnected = false;
		bool m_ListenerClosed = true;
		size_t _m_ai_addrlen = 0;
		sockaddr* _m_addr = nullptr;

		void p_Swap(TCPServer& SocketToSwapWith);
		MB_OS_Socket m_ListenerSocket;
	public:
		TCPServer();
		MBError Initialize(std::string const& Port);
		TCPServer(std::string const& Port);
		int Bind();
		int Listen();
		int Accept();

		void TransferConnectedSocket(TCPServer* ServerToRecieve);
		//overrides
		virtual bool IsValid() override;
		virtual void Close() override;

		virtual bool IsConnected() override;
		virtual std::string RecieveData(size_t MaxNumberOfBytes = -1) override;
		virtual MBError SendData(const void* DataPointer, size_t DataLength) override;
		virtual MBError SendData(std::string const& DataToSend) override;
		virtual ~TCPServer();
	};
	class TLSConnectSocket : public ConnectSocket
	{
		TLSHandler m_TLSHandler = TLSHandler();
		std::string m_HostName = "";
		std::unique_ptr<ConnectSocket> m_UnderlyingSocket;

		void p_Initialize(std::unique_ptr<ConnectSocket> NewSocket);
	public:
		TLSConnectSocket(std::unique_ptr<ConnectSocket> NewSocket);

		MBError EstablishClientTLSConnection(std::string const& RemoteHostName);
		MBError EstablishHostTLSConnection(std::unique_ptr<DomainHandler> Retriever);
		virtual bool IsValid() override;
		virtual void Close() override;

		virtual bool IsConnected() override;
		virtual std::string RecieveData(size_t MaxNumberOfBytes = -1) override;
		virtual MBError SendData(const void* DataPointer, size_t DataLength) override;
		virtual MBError SendData(std::string const& DataToSend) override;
	};
	class HTTPClientSocket
	{
	private:
		std::string URl;
		//bool UsingHTTPS = false;
		
		std::unique_ptr<TLSConnectSocket> m_UnderlyingSocket;
		std::string m_RemoteHost = "";

		//recieve data status
		int MaxBytesInMemory = 100000000;//100 MB, helt godtyckligt
		int CurrentContentLength = -1;
		int RecievedContentData = 0;
		bool HeadRecieved = false;
		bool IsChunked = false;
		bool RequestFinished = true;

		int CurrentChunkLength = -1;
		int CurrentRecievedChunkData = 0;
		size_t ChunkParseOffset = 0;


		std::string p_GetHeaderValue(std::string const& Header,std::string const& HeaderContent);
		void UpdateAndDechunkData(std::string& DataToDechunk, size_t Offset);
		void ResetRequestRecieveState();
		int Get(std::string Resource = "");
		int Head(std::string Resource = "");

		std::vector<std::pair<std::string, std::string>> p_GetDefaultHeaders();
		void p_SendRequest(std::string const& RequestType,std::string const& ResourceToGet, std::vector<std::pair<std::string, std::string>> const& HeadersToSend);
		
	public:
		//int HTTPSendData(std::string const& DataToSend);
		std::string HTTPGetData();
		bool DataIsAvailable();
		MBError EstablishTLSConnection();
		std::string GetDataFromRequest(const std::string& RequestType, std::string Resource);
		std::string GetDataFromRequest(const std::string& RequestType, std::string const& Resource,std::vector<std::pair<std::string,std::string>> const& ExtraHeaders);
		MBError Connect(std::string const& Host, std::string const& Port);
		HTTPClientSocket() {};
		HTTPClientSocket(std::string const& URL, std::string const& Port);
		HTTPClientSocket(std::unique_ptr<TLSConnectSocket> ConnectedSocket,std::string const& Host);

		bool IsConnected();
		bool IsValid();
		//overrides
		~HTTPClientSocket();
	};



	class ThreadPool;
	class Worker;
	void WorkerThreadFunction(Worker*);
	class Worker
	{
		friend class ThreadPool;
		friend void WorkerThreadFunction(Worker*);
	private:
		std::mutex ObjectValuesMutex;
		std::deque<std::function<void()>> TaskQueue = {};
		std::condition_variable CanExecuteConditional;
		std::thread WorkingThread;
		ThreadPool* ConnectedThreadPool = nullptr;
		bool AllTasksFinished = true;
		bool Running = false;
		bool ShouldStop = false;
		int WorkerID = -1;

		void StartWorking()
		{
			Running = true;
			WorkingThread = std::thread(WorkerThreadFunction, this);
		}
	public:
		Worker()
		{

		}
		Worker(ThreadPool* ThreadPoolToAttachTo)
		{
			ConnectedThreadPool = ThreadPoolToAttachTo;
		}
		~Worker()
		{
			Stop();
		}
		template<typename... Args>
		void AddTask(Args&&... args)
		{
			{
				std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
				TaskQueue.push_back(std::bind(std::forward<Args>(args)...));
			}
			CanExecuteConditional.notify_all();
		}
		void Start()
		{
			if (Running == true)
			{
				return;
			}
			else
			{
				StartWorking();
			}
		}
		void Stop()
		{
			{
				std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
				if (Running == false)
				{
					return;
				}
				ShouldStop = true;
				Running = false;
			}
			CanExecuteConditional.notify_all();
			WorkingThread.join();
			//resettar tr�den till ursrpungs tillst�ndet
			ShouldStop = false;
			AllTasksFinished = true;
		}
		bool InStandby()
		{
			std::lock_guard<std::mutex> Locken(ObjectValuesMutex);
			if (Running && AllTasksFinished)
			{
				return(true);
			}
			else
			{
				return(false);
			}
		}
	};
	class ThreadPool
	{
		friend 	void WorkerThreadFunction(Worker* WorkerAttachedTo);
	private:
		std::vector<Worker> AvailableWorkers;
		std::deque<std::function<void()>> TaskQueue = {};
		std::condition_variable WaitForThreadsToFinishConditional;
		std::deque<Worker*> ThreadsOnStandby = std::deque<Worker*>(0);
		std::mutex ThreadPoolMutex;
		bool Running = false;
		//std::deque<
	public:
		ThreadPool(int NumberOfThreads)
		{
			AvailableWorkers = std::vector<Worker>(NumberOfThreads);
		}
		template<typename... Args>
		void AddTask(Args&&... args)
		{
			Worker* WorkerToAssignWork = nullptr;
			{
				std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
				if (ThreadsOnStandby.size() > 0)
				{
					WorkerToAssignWork = ThreadsOnStandby[0];
					ThreadsOnStandby.pop_front();
				}
			}
			//om vi nu har en worker vi kan assigna arbete s� g�r vi det, annars s� l�gger vi till den i v�r queue av grejer som beh�ver g�ras
			if (WorkerToAssignWork != nullptr)
			{
				WorkerToAssignWork->AddTask(args...);
			}
			else
			{
				std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
				TaskQueue.push_back(std::bind(std::forward<Args>(args)...));
			}
		}
		void WaitForThreadsToFinish()
		{
			//vi har en wait variabel som vi v�ntar p� och kollar huruvida v�ra grejer p� standby �r lika med antalet, d� �r det ju klart
			std::unique_lock<std::mutex> Locken(ThreadPoolMutex);
			while (ThreadsOnStandby.size() != AvailableWorkers.size())
			{
				WaitForThreadsToFinishConditional.wait(Locken);
			}
			int i = 1;
		}
		void Start()
		{
			if (Running)
			{
				return;
			}
			else
			{
				Running = true;
				for (int i = 0; i < AvailableWorkers.size(); i++)
				{
					std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
					AvailableWorkers[i].ConnectedThreadPool = this;
					if (TaskQueue.size() > 0)
					{
						AvailableWorkers[i].AddTask(TaskQueue.front());
						TaskQueue.pop_front();
						AvailableWorkers[i].Start();
					}
					else
					{
						AvailableWorkers[i].Start();
					}
				}
			}
		}
		void Stop()
		{
			for (int i = 0; i < AvailableWorkers.size(); i++)
			{
				AvailableWorkers[i].Stop();
			}
		}
		int NumberOfActiveWorkers()
		{
			std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
			return(AvailableWorkers.size() - ThreadsOnStandby.size());
		}
		int NumberOfQueuedTasks()
		{
			std::lock_guard<std::mutex> Locken(ThreadPoolMutex);
			return(TaskQueue.size());
		}
		~ThreadPool()
		{
			Stop();
		}

	};
	inline void WorkerThreadFunction(Worker* WorkerAttachedTo)
	{
		while (true)
		{

			//vi poppar det l�ngst ner p� listan, executar, tar en ny
			std::function<void()> TaskToExecute;
			decltype(WorkerAttachedTo->TaskQueue) SeperateTaskQueue;
			{
				std::unique_lock<std::mutex> Locken(WorkerAttachedTo->ObjectValuesMutex);
				if (WorkerAttachedTo->TaskQueue.empty() && WorkerAttachedTo->ConnectedThreadPool != nullptr)
				{
					{
						std::lock_guard<std::mutex> TPLocken(WorkerAttachedTo->ConnectedThreadPool->ThreadPoolMutex);
						if (!WorkerAttachedTo->ConnectedThreadPool->TaskQueue.empty())
						{
							WorkerAttachedTo->TaskQueue.push_back(WorkerAttachedTo->ConnectedThreadPool->TaskQueue.front());
							WorkerAttachedTo->ConnectedThreadPool->TaskQueue.pop_front();
						}
						else
						{
							//l�gger till den i listan av grejer p� standby
							//assert(WorkerAttachedTo != nullptr);
							WorkerAttachedTo->ConnectedThreadPool->ThreadsOnStandby.push_back(WorkerAttachedTo);
						}
					}
					WorkerAttachedTo->ConnectedThreadPool->WaitForThreadsToFinishConditional.notify_all();

				}
				while (WorkerAttachedTo->TaskQueue.empty() && !WorkerAttachedTo->ShouldStop)
				{
					WorkerAttachedTo->AllTasksFinished = true;
					WorkerAttachedTo->CanExecuteConditional.wait(Locken);
				}
				if (WorkerAttachedTo->ShouldStop)
				{
					SeperateTaskQueue = WorkerAttachedTo->TaskQueue;
					WorkerAttachedTo->TaskQueue.clear();
				}
				else
				{
					WorkerAttachedTo->AllTasksFinished = false;
				}
			}
			if (WorkerAttachedTo->ShouldStop)
			{
				for (int i = 0; i < SeperateTaskQueue.size(); i++)
				{
					SeperateTaskQueue[i]();
				}
				return;
			}
			else
			{
				std::unique_lock<std::mutex> Locken(WorkerAttachedTo->ObjectValuesMutex);
				TaskToExecute = WorkerAttachedTo->TaskQueue.front();
				TaskToExecute();
				WorkerAttachedTo->TaskQueue.pop_front();
			}
		}
	}
};
