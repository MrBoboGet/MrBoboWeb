#pragma once
#include <string>
#include <vector>
#include <memory>
#include <MrBoboSockets/MrBoboSockets.h>
namespace MBTorrent
{
	struct BitTorrent_Peer
	{
		std::string ID = "";
		std::string Adress = "";
		uint32_t Port = 0;
	};
	struct ConnectTuple
	{
		std::string Adress = "";
		std::string Port = "";
	};
	struct BitTorrent_TrackerResponse
	{
		uintmax_t ResendInterval = 0;
		std::vector<BitTorrent_Peer> Peers = {};
	};
	struct BitTorrent_FileInfo
	{
		uintmax_t FileSize = 0;
		std::vector<std::string> SubDirectories = { };
	};
	struct BitTorrent_Torrent
	{
		std::string TrackerURL = "";
		std::vector<std::vector<std::string>> TrackerLists = {};
		std::string TorrentInfoHash = "";
		std::string OutputName = "";
		size_t PieceLength = 0;
		uintmax_t FileLength = 0;
		std::vector<std::string> PieceHashes = {};
		std::vector<BitTorrent_FileInfo> Files = {};
	};
	enum class BitTorrent_TrackerEvent
	{
		Started,
		Completed,
		Stopped,
		Null,
	};
	struct BitTorrent_TrackerParameters
	{
		std::string InfoHash = "";
		std::string PeerID = std::string(20,0);
		std::string IP = "";
		std::string Port = "6881";
		uintmax_t BytesUploaded = 0;
		uintmax_t BytesDownloaded = 0;
		uintmax_t BytesLeft = 0;
		BitTorrent_TrackerEvent Event = BitTorrent_TrackerEvent::Null;
	};
	struct BitTorrentConnectionState
	{
		std::atomic<bool> Chocked{ true };
		std::atomic<bool> Interested{ false };
		std::vector<bool> OwnedPieces = {};
	};
	struct BitTorrent_PieceDownloadState
	{
		bool Completed = false;
		size_t PieceIndex = 0;
		uintmax_t PieceSize = 0;
		uintmax_t CurrentOffset = 0;
	};
	struct BitTorrent_TorrentDownloadState
	{
		//antagande, varje piece laddas ner sekventiellt
		std::string OutputDirectory = "";
		std::vector<BitTorrent_PieceDownloadState> SavedPieceData = {};
	};
	struct BitTorrent_PieceDataInfo
	{
		size_t PieceIndex = -1;
		size_t PieceBegin = 0;
		size_t PieceLength = 0;
		std::string PieceData = "";
	};
	enum class BitTorrent_MessageType
	{
		choke = 0,
		unchoke = 1,
		interested = 2,
		not_interested = 3,
		have = 4,
		bitfield = 5,
		request = 6,
		piece = 7,
		cancel = 8,
	};
	uintmax_t ParseBigEndianInteger(const void* DataToParse,uint8_t IntegerSize, size_t InOffset, size_t* OutOffset);
	void InsertBigEndianInteger(void* Buffer, uint8_t IntegerSize, uintmax_t IntegerToInsert, size_t InOffset, size_t* OutOffset);
	class MBBitTorrentHandler;
	struct MBBitTorrent_PeerConnectionRetrievalState
	{
		size_t NumberOfPendingRequests = 0;
		size_t CurrentPiece = 0;
		bool AssignedPiecesFinished = false;
		std::deque<std::string> RecievedPieceMessages = {};
		std::vector<BitTorrent_PieceDownloadState> AssignedPieces = {};
		std::vector<BitTorrent_PieceDownloadState> DownloadState = {};
		std::vector<std::string> PieceData = {};
	};
	class MBBitTorrentPeerConnection
	{
	private:
		friend MBBitTorrentHandler;
		std::mutex m_InternalsMutex;
		std::mutex m_SendMutex;
		std::condition_variable m_RecieveConditional;

		MBBitTorrentHandler* m_AssociatedDownloadHandler{ nullptr };
		std::atomic<MBSockets::ConnectSocket*> m_AssociatedSocket{ nullptr };
		//skulle eventuellt kunna ta bort recieve tråden genom att låte dem inviduella trådarna kalla den, och gå ur när dem får ett meddalnde dem ska hantera
		std::unique_ptr<std::thread> m_RecieveThread = nullptr;
		std::unique_ptr<std::thread> m_PeerServiceThread{ nullptr };
		std::unique_ptr<std::thread> m_LocalRequestsThread{ nullptr };


		BitTorrentConnectionState m_ClientState;
		BitTorrentConnectionState m_PeerState;
		std::deque<std::string> m_RecievedPeerServiceMessages = {};
		MBBitTorrent_PeerConnectionRetrievalState m_RetrievalState;

		std::atomic<bool> m_ShouldStop{ false };
		void p_InitiateHandshake();
		void p_SendBitfield();
		void p_HandleMessage(std::string& MessageToHandle);
		void p_RecievePeerMessagesHandler();
		void p_RetrievePiecesHandler();
		void p_SendClientUninterestedMessage();
		static std::string sp_GeneratePieceRequestMessage(size_t PieceIndex, size_t PieceBegin, size_t Length);
		static void sp_UpdatePieceDownloadState(MBBitTorrent_PeerConnectionRetrievalState& StateToUpdate, std::string const& PieceDataMessage);
		void p_SendPieceDataRequests();
		bool p_IsStopping();
		void p_SendDataToPeer(std::string const& DataToSend);
		MBSockets::ConnectSocket* p_GetAssociatedSocket();
		std::string p_RecieveNextPeerData();
		static bool sp_PieceIsFinished(MBBitTorrent_PeerConnectionRetrievalState const& StateToCheck, size_t PieceIndex);
		MBBitTorrentPeerConnection() 
		{
			
		};
		MBBitTorrentPeerConnection(MBBitTorrentHandler* TorrentHandler)
		{
			m_AssociatedDownloadHandler = TorrentHandler;
		}
	public:
		void InitiateConnection(std::string const& IPAdressToConnectTo,std::string const& PortToUse);
		bool IsConnected();
		bool PieceIsFinished(size_t PieceIndex);
		void AssignPieces(std::vector<BitTorrent_PieceDownloadState> const& DownloadState);
		bool AssignedPiecesAreFinished();
		bool CloseConnection();
		bool PauseDownload();
		void ResetDownloadState();
		void TransferPiecedata(std::vector<std::string>& TargetDestination);
		void TransferAssignedPieces(std::vector<BitTorrent_PieceDownloadState>& TargetDestination);
		void TransferDownloadState(std::vector<BitTorrent_PieceDownloadState>& TargetDestination);
		uintmax_t BytesLeft();
		clock_t TimeSinceDownloadStart();
		//bool PeerIsChocked();
		//bool ClientIsChocked();
		//bool PeerIsIntersted();
		//bool ClientIsInterested();
	};
	struct SavePieceDataInfo
	{
		std::string Filepath = "";
		size_t SaveOffset = 0;
		size_t NumberOfBytesToSave = 0;
	};
	class MBBitTorrentHandler
	{
	private:
		friend MBBitTorrentPeerConnection;
		std::string ID = "";
		std::atomic<bool> m_ShouldStop{ false };
		std::atomic<bool> m_DownloadFinished{false};
		std::mutex m_InternalsMutex;
		std::atomic<size_t> m_FinishedAssignmentsWaiting{ 0 };
		std::condition_variable m_AssignmentFinishedConditional;
		

		BitTorrent_Torrent m_AssociatedTorrent;
		BitTorrent_TrackerResponse m_CurrentTrackerResponse;
		BitTorrent_TrackerParameters m_TrackerState;
		BitTorrent_TorrentDownloadState m_DownloadState;

		std::vector<bool> m_PieceAssigned = {};

		std::vector<std::unique_ptr<MBBitTorrentPeerConnection>> m_PeerConnections = {};
		
		
		std::string p_GetLocalPieceData(size_t PieceIndex, size_t Offset, size_t Length);
		std::string p_GenerateTrackerGetParameters();

		ConnectTuple p_GetAnnounceURLConnectionTuple(std::string const& AnnounceURL);
		std::string p_EventToString(BitTorrent_TrackerEvent EventToConvert);
		BitTorrent_TrackerResponse p_ParseBitTorrent_TrackerResponse(std::map<std::string, std::string> const& DecodedResponse);
		BitTorrent_Torrent p_ParseBitTorrent_Torrent(std::string const& TorrentData);
		void p_UpdateTrackerResponse();
		void p_NotifyPieceDownloadFinished();
		std::string p_GetClientID();
		std::string p_GetTorrentHash();
		std::vector<bool> p_GetLocalPieces();

		void p_NotifyAssignmentFinished();
		bool p_DownloadFinished();
		bool p_ShouldStop();
		std::vector<BitTorrent_PieceDownloadState> p_GetPiecesToDownload();
		static std::vector<SavePieceDataInfo> sp_GetPieceSaveInfo(BitTorrent_Torrent const& AssociatedTorrent,
			BitTorrent_PieceDownloadState const& AssignedPieces, std::string const& PieceData);
		void p_SavePieceDataInfo(std::vector<SavePieceDataInfo> const& SaveData, std::string const& DataToSave);
		void p_SavePiecesAndState(std::vector<BitTorrent_PieceDownloadState> const& AssignedPieces, std::vector<std::string> const& PieceData);
		void p_DownloadLoop();
	public:
		void SaveDownloadState(std::string const& OutputFilepath);
		void LoadDownloadState(std::string const& OutputFilepath);
		void LoadTorrentInfo(std::string const& TorrentFilepath);
		void StartDownload();
		void StopDownload();
		bool DownloadFinished();
		uintmax_t GetTotalTorrentSize();
	};
}