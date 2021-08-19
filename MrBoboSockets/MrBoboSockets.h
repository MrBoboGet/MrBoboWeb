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
#include <MrPostOGet/TLSHandler.h>
#include <filesystem>
#include <cinttypes>
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
	void Init();
	int MBCloseSocket(int SocketToClose);
	int MBSocketError();
	enum class TraversalProtocol
	{
		TCP
	};
	enum class SocketType
	{
		Server,
		Client
	};
	enum class MBSocketErrors :uint8_t
	{
		fatal,
		nonfatal
	};
	enum class ApplicationProtocols
	{
		HTTP,
		HTTPS
	};
	enum class HTTPDocumentType
	{
		OctetString,
		HTML,
		png,
		jpg,
		json,
		ts,
		m3u8,
		mkv,
		javascript,
		css,
		mp4,
		PDF,
		GIF,
		mp3,
		Text,
		Wav,
		WebP,
		WebM,
		Opus,
		Null
	};
	enum class HTTPRequestStatus
	{
		OK = 200,
		PartialContent = 206,
		NotFound = 404,
		Conflict = 409,
	};
	enum class MediaType
	{
		Video,
		Audio,
		Image,
		Text,
		PDF,
		Null
	};
	struct HTTPDocument
	{
		HTTPDocumentType Type = HTTPDocumentType::Null;
		HTTPRequestStatus RequestStatus = HTTPRequestStatus::OK;
		std::vector<std::string> ExtraHeaders = {};
		std::vector<FiledataIntervall> IntervallsToRead = {};
		std::string DocumentData;
		std::string DocumentDataFileReference = "";
	};
	struct HTTPTypeTuple
	{
		HTTPDocumentType FileHTTPDocumentType = HTTPDocumentType::Null;
		MediaType FileMediaType = MediaType::Null;
		std::vector<std::string> FileExtensions = {};
		std::string MIMEMediaString = "";
	};
	class Socket
	{
	protected:
		MB_OS_Socket m_UnderlyingHandle;
		int m_ErrorResult = 0;
		bool m_Invalid = false;
		std::string m_LastErrorMessage = "";
		std::string p_GetLastError();
		void p_HandleError(std::string const& ErrorMessage, bool IsLethal);
		friend void swap(Socket& FirstSocket, Socket& SecondSocket);
	public:
		Socket& operator=(Socket const&) = delete;
		Socket& operator=(Socket&&);
		Socket(Socket const&) = delete;
		Socket(Socket&&);
		bool IsValid();
		virtual void Close();
		//int SendData(const char* DataPointer, int DataLength);
		//int RecieveData(char* Buffer, int BufferSize);
		//std::string RecieveData();
		//std::string RecieveData(int MaxNumberOfBytes);
		Socket();
		~Socket();
	};
	class UDPSocket : public Socket
	{
	public:
		UDPSocket(std::string const& Adress, std::string const& Port);
		void UDPSendData(std::string const& DataToSend, std::string const& HostAdress, int PortNumber);
		int Bind(int PortToAssociateWith);
		std::string UDPGetData();
		void UDPMakeSocketNonBlocking(float SecondsToWait = 0.5);
		void Listen(std::string const& PortNumber);
	};
	class ConnectSocket : public Socket
	{
	protected:
		bool m_IsConnected = false;
		bool m_TLSConnectionEstablished = false;
		size_t _m_ai_addrlen = 0;
		sockaddr* _m_addr = nullptr;
		TLSHandler m_TLSHandler = TLSHandler();
	public:
		std::string HostName;
		int Connect();
		bool IsConnected();
		std::string GetIpOfConnectedSocket();
		int SendRawData(const void* DataPointer, size_t DataLength);
		virtual int SendData(const void* DataPointer, size_t DataLength);
		virtual int SendData(std::string const& DataToSend);
		std::string RecieveRawData(size_t MaxNumberOfBytes = -1);
		virtual std::string RecieveData(size_t MaxNumberOfBytes = -1);
		virtual ConnectSocket& operator<<(std::string const& DataToSend);
		virtual ConnectSocket& operator>>(std::string& DataBuffer);
		virtual MBError EstablishTLSConnection();
		ConnectSocket() {};
		ConnectSocket(ConnectSocket&&)
		{

		}
		~ConnectSocket();
	};
	class ClientSocket : public ConnectSocket
	{
	private:
	public:
		int Connect();
		MBError EstablishTLSConnection() override;
		ClientSocket(std::string const& Adress, std::string const& Port);
		ClientSocket(ClientSocket&&)
		{

		}
	};
	class ServerSocket : public ConnectSocket
	{
	private:
		MB_OS_Socket m_ListenerSocket;
	protected:
		TLSHandler SocketTlsHandler = TLSHandler();
		friend void swap(ServerSocket& FirstSocket, ServerSocket& SecondSocket);
		//bool SecureConnectionEstablished = false;
	public:
		MBError EstablishTLSConnection() override;
		virtual int Bind();
		virtual int Listen();
		void TransferConnectedSocket(ServerSocket& OtherSocket);
		virtual int Accept();
		ServerSocket();
		ServerSocket(std::string const& Port);
		~ServerSocket();
	};
	std::string GetHeaderValue(std::string Header, const std::string& HeaderContent);
	std::vector<std::string> GetHeaderValues(std::string const& HeaderTag, std::string const& HeaderContent);
	int HexToDec(std::string NumberToConvert);
	class HTTPConnectSocket : public ClientSocket
	{
	private:
		std::string URl;
		bool UsingHTTPS = false;
		TLSHandler TLSConnectionHandler = TLSHandler();

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
	public:
		//int HTTPSendData(std::string const& DataToSend);
		bool DataIsAvailable();
		void ResetRequestRecieveState();
		void UpdateAndDechunkData(std::string& DataToDechunk, size_t Offset);
		std::string HTTPGetData();
		int Get(std::string Resource = "");
		int Head(std::string Resource = "");
		std::string GetDataFromRequest(const std::string& RequestType, std::string Resource);
		HTTPConnectSocket(std::string const& URL, std::string const& Port);
		~HTTPConnectSocket();
	};
	class HTTPServerSocket : public ServerSocket
	{
	private:
		bool ChunksRemaining = false;
		bool RequestIsChunked = false;
		int CurrentChunkSize = 0;
		int CurrentChunkParsed = 0;
		int CurrentContentLength = 0;
		int ParsedContentData = 0;
		int p_GetNextChunkSize(int ChunkHeaderPosition, std::string const& Data, int& OutChunkDataBeginning);
		std::string p_UpdateAndDeChunkData(std::string const& ChunkedData);
	public:
		bool DataIsAvailable();
		HTTPServerSocket(std::string const& Port);
		std::string GetHTTPRequest();
		void SendDataAsHTTPBody(const std::string& Data);
		void SendHTTPBody(const std::string& Data);
		void SendHTTPDocument(HTTPDocument const& DocumentToSend);
		std::string GetNextChunkData();
	};
	std::string GetRequestType(const std::string& RequestData);
	std::string GetReqestResource(const std::string& RequestData);
	class HTTPTypesConnector
	{
	private:
		std::vector<HTTPTypeTuple> SuppportedTupples =
		{
			{HTTPDocumentType::OctetString,MediaType::Null,{},"application/octet-stream"},
			{HTTPDocumentType::png,	MediaType::Image,{"png"},"image/png"},
			{HTTPDocumentType::jpg,MediaType::Image,{"jpg","jpeg"},"image/jpeg"},
			{HTTPDocumentType::json,MediaType::Text,{"json"},"application/json"},
			{HTTPDocumentType::ts,MediaType::Video,{"ts"},"video/MP2T"},
			{HTTPDocumentType::m3u8,MediaType::Text,{"m3u8"},"application/x-mpegURL"},
			{HTTPDocumentType::mkv,MediaType::Video,{"mkv"},"video/x-matroska"},
			{HTTPDocumentType::javascript,MediaType::Text,{"js"},"text/javascript"},
			{HTTPDocumentType::css,MediaType::Text,{"css"},"text/css"},
			{HTTPDocumentType::mp4,MediaType::Video,{"mp4"},"video/mp4"},
			{HTTPDocumentType::HTML,MediaType::Text,{"html","htm"},"text/html"},
			{HTTPDocumentType::PDF,MediaType::PDF,{"pdf"},"application/pdf"},
			{HTTPDocumentType::GIF,MediaType::Image,{"gif"},"image/gif"},
			{HTTPDocumentType::mp3,MediaType::Audio,{"mp3"},"audio/mpeg"},
			{HTTPDocumentType::Text,MediaType::Text,{"txt"},"text/plain"},
			{HTTPDocumentType::Wav,MediaType::Audio,{"wav"},"audio/wav"},
			{HTTPDocumentType::WebP,MediaType::Image,{"webp"},"image/webp"},
			{HTTPDocumentType::WebM,MediaType::Video,{"webm"},"video/webm"},
			{HTTPDocumentType::Opus,MediaType::Audio,{"opus"},"audio/opus"},
		};
		HTTPTypeTuple NullTupple = { HTTPDocumentType::Null,MediaType::Null,{},"" };
	public:
		HTTPTypeTuple GetTupleFromExtension(std::string const& Extension);
		HTTPTypeTuple GetTupleFromDocumentType(HTTPDocumentType DocumentType);
	};
	HTTPDocumentType DocumentTypeFromFileExtension(std::string const& FileExtension);
	MediaType GetMediaTypeFromExtension(std::string const& FileExtension);
	std::string GetMIMEFromDocumentType(HTTPDocumentType TypeToConvert);
	std::string HTTPRequestStatusToString(HTTPRequestStatus StatusToConvert);
	std::string GenerateRequest(HTTPDocument const& DocumentToSend);
	std::string GenerateRequest(const std::string& HTMLBody);

	class FileIntervallExtracter
	{
	private:
		std::ifstream FileToRead;
		std::vector<FiledataIntervall> IntervallsToRead = {};
		uint64_t FileSize = 0;
		uint64_t IntervallIndex = 0;
		uint64_t MaxDataInMemory = 10000000;
		uint64_t TotalDataRead = 0;
	public:
		FileIntervallExtracter(std::string const& FilePath, std::vector<FiledataIntervall> const& Intervalls, size_t MaxDataInMemory);
		std::string GetNextIntervall();
		bool IsDone();
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