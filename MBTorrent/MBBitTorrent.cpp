#include "MBBitTorrent.h"
#include <MBCrypto/MBCrypto.h>
#include <MrBoboSockets/MrBoboSockets.h>
#include <MBStrings.h>
namespace MBTorrent
{
	//BEGIN MBBitTorrentPeerConnection
	void MBBitTorrentPeerConnection::p_InitiateHandshake()
	{
	
		//if (PeerHandshake.size() < DataToSend.size())
		//{
		//	PeerHandshake += p_RecieveNextPeerData();
		//}
		std::string DataToSend = "\x13""BitTorrent protocol";
		DataToSend += std::string(8, 0);
		std::string TorrentHash = "";
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			TorrentHash = m_AssociatedDownloadHandler->p_GetTorrentHash();
			DataToSend += TorrentHash;
			DataToSend += m_AssociatedDownloadHandler->p_GetClientID();
		}
		p_SendDataToPeer(DataToSend);
		//p_SendDataToPeer(sp_GeneratePieceRequestMessage(0, 0, 5000));
		std::string PeerHandshake = p_RecieveNextPeerData();
		if (PeerHandshake.substr(20 + 8, 20) != TorrentHash)
		{
			m_AssociatedSocket.load()->Close();
		}
	}
	void MBBitTorrentPeerConnection::InitiateConnection(std::string const& IPAdressToConnectTo, std::string const& PortToUse)
	{
		MBSockets::ClientSocket* SocketToUse = new MBSockets::ClientSocket(IPAdressToConnectTo, PortToUse);
		SocketToUse->Connect();
		m_AssociatedSocket.store(SocketToUse);
		if (SocketToUse->IsConnected())
		{
			p_InitiateHandshake();
			if (m_AssociatedSocket.load()->IsConnected())
			{
				m_RecieveThread = std::unique_ptr<std::thread>(new std::thread(&MBBitTorrentPeerConnection::p_RecievePeerMessagesHandler, this));
				m_LocalRequestsThread = std::unique_ptr<std::thread>(new std::thread(&MBBitTorrentPeerConnection::p_RetrievePiecesHandler, this));
			}
		}
	}
	std::string h_BoolVectorToOctets(std::vector<bool> const& ArrayToConvert)
	{
		std::string ReturnValue = "";
		uint8_t CurrentOctet = 0;
		char CurrentBitIndex = 7;
		for (size_t i = 0;i < ArrayToConvert.size();i++)
		{
			CurrentOctet += (ArrayToConvert[i] << CurrentBitIndex);
			if (CurrentBitIndex == 0)
			{
				ReturnValue += CurrentOctet;
				CurrentBitIndex = 7;
				CurrentOctet = 0;
			}
			else
			{
				CurrentBitIndex -= 1;
			}
		}
		if (CurrentOctet != 0)
		{
			ReturnValue += CurrentOctet;
		}
		return(ReturnValue);
	}
	std::vector<bool> h_BitmapToBoolVector(const void* Data, size_t DataSize)
	{
		std::vector<bool> ReturnValue = {};
		uint8_t* ByteData = (uint8_t*)Data;
		for (size_t i = 0; i < DataSize; i++)
		{
			uint8_t CurrentByte = *ByteData;
			for (size_t j = 0; j < 8; j++)
			{
				bool CurrentBit = (((CurrentByte >> (7 - i))&1) == 1);
				ReturnValue.push_back(CurrentBit);
			}
		}
		return(ReturnValue);
	}
	void MBBitTorrentPeerConnection::p_SendBitfield()
	{
		std::vector<bool> LocalPieces;
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			LocalPieces = m_AssociatedDownloadHandler->p_GetLocalPieces();
		}
		std::string DataToSend = "";
		DataToSend += char(BitTorrent_MessageType::bitfield);
		DataToSend.resize(5);
		DataToSend += h_BoolVectorToOctets(LocalPieces);
		InsertBigEndianInteger(DataToSend.data(), 4, DataToSend.size(), 1, nullptr);
		p_SendDataToPeer(DataToSend);
	}
	void MBBitTorrentPeerConnection::p_HandleMessage(std::string& MessageToHandle)
	{
		BitTorrent_MessageType MessageType = (BitTorrent_MessageType)MessageToHandle[5];
		if (MessageType == BitTorrent_MessageType::interested)
		{
			m_PeerState.Interested = true;
		}
		else if (MessageType == BitTorrent_MessageType::not_interested)
		{
			m_PeerState.Interested = false;
		}
		else if (MessageType == BitTorrent_MessageType::choke)
		{
			m_PeerState.Chocked = true;
		}
		else if (MessageType == BitTorrent_MessageType::unchoke)
		{
			m_PeerState.Chocked = false;
		}
		else if (MessageType == BitTorrent_MessageType::piece)
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			m_RetrievalState.RecievedPieceMessages.push_back(std::move(MessageToHandle));
		}
		else if (MessageType == BitTorrent_MessageType::request)
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			m_RecievedPeerServiceMessages.push_back(std::move(MessageToHandle));
		}
		else if (MessageType == BitTorrent_MessageType::cancel)
		{
			// inte helt säker på vad som ska hända här
		}
		else if (MessageType == BitTorrent_MessageType::bitfield)
		{
			//inte helt säker på vad som ska hända här
			std::vector<bool> MessageBits = h_BitmapToBoolVector(MessageToHandle.data() + 5, MessageToHandle.size() - 5);
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			m_PeerState.OwnedPieces = MessageBits;
		}
		else if (MessageType == BitTorrent_MessageType::have)
		{
			//inte helt säker på vad som ska hända här
			size_t HavePiece = ParseBigEndianInteger(MessageToHandle.data(), 4, 5, nullptr);
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			if (m_PeerState.OwnedPieces.size() < HavePiece+1)
			{
				size_t SizeDifference = m_PeerState.OwnedPieces.size() - 1 - HavePiece;
				for (size_t i = 0; i < SizeDifference; i++)
				{
					m_PeerState.OwnedPieces.push_back(false);
				}
			}
			m_PeerState.OwnedPieces[HavePiece] = true;
		}
	}
	uintmax_t ParseBigEndianInteger(const void* DataToParse, uint8_t IntegerSize, size_t InOffset, size_t* OutOffset)
	{
		uintmax_t ReturnValue = 0;
		uint8_t* ByteData =(uint8_t*) DataToParse;
		for (size_t i = 0; i < IntegerSize; i++)
		{
			ReturnValue <<= 8;
			ReturnValue += ByteData[InOffset + i];
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = InOffset + IntegerSize;
		}
		return(ReturnValue);
	}
	void InsertBigEndianInteger(void* Buffer, uint8_t IntegerSize, uintmax_t IntegerToInsert, size_t InOffset, size_t* OutOffset)
	{
		uint8_t* BytePointer = (uint8_t*)Buffer;
		for (size_t i = 0; i < IntegerSize; i++)
		{
			uint8_t ByteToInsert = IntegerToInsert >> ((IntegerSize * 8) - ((1 + i) * 8));
			BytePointer[InOffset + i] = ByteToInsert;
		}
		if (OutOffset != nullptr)
		{
			*OutOffset = InOffset + IntegerSize;
		}
	}
	MBSockets::ConnectSocket* MBBitTorrentPeerConnection::p_GetAssociatedSocket()
	{
		MBSockets::ConnectSocket* ReturnValue = m_AssociatedSocket.load();
		return(ReturnValue);
	}
	std::string MBBitTorrentPeerConnection::p_RecieveNextPeerData()
	{
		MBSockets::ConnectSocket* SocketToUse = p_GetAssociatedSocket();
		return(SocketToUse->RecieveData());
	}
	void MBBitTorrentPeerConnection::p_RecievePeerMessagesHandler()
	{
		//MBSockets::ConnectSocket* SocketToUse = p_GetAssociatedSocket();
		size_t CurrentMessageSize = -1;
		std::string CurrentMessage = "";
		std::string NewMessageData = "";
		while (IsConnected())
		{
			NewMessageData += p_RecieveNextPeerData();
			if (CurrentMessageSize == -1)
			{
				while(NewMessageData.size() < 4)
				{
					NewMessageData += p_RecieveNextPeerData();
				}
				CurrentMessageSize = ParseBigEndianInteger(NewMessageData.data(), 4, 0, nullptr);
				CurrentMessage = NewMessageData.substr(0, CurrentMessageSize+4);
				NewMessageData = NewMessageData.substr(std::min(NewMessageData.size(), CurrentMessageSize + 4));
			}
			else
			{
				size_t BytesForCurrentMessage = std::min(NewMessageData.size(), CurrentMessageSize + 4 - CurrentMessage.size());
				CurrentMessage += NewMessageData.substr(0, BytesForCurrentMessage);
				NewMessageData = NewMessageData.substr(BytesForCurrentMessage);
			}
			if (CurrentMessage.size() != CurrentMessageSize + 4)
			{
				continue;
			}
			CurrentMessageSize = -1;
			p_HandleMessage(CurrentMessage);
			CurrentMessage = "";
		}
	}
	void MBBitTorrentPeerConnection::sp_UpdatePieceDownloadState(MBBitTorrent_PeerConnectionRetrievalState& StateToUpdate, std::string const& PieceDataMessage)
	{
		BitTorrent_PieceDataInfo PieceInfo;
		size_t ParseOffset = 5;
		PieceInfo.PieceIndex = ParseBigEndianInteger(PieceDataMessage.data(), 4, ParseOffset, &ParseOffset);
		PieceInfo.PieceBegin = ParseBigEndianInteger(PieceDataMessage.data(), 4, ParseOffset, &ParseOffset);
		PieceInfo.PieceLength = PieceDataMessage.size() - ParseOffset;
		for (size_t i = 0; i < StateToUpdate.DownloadState.size(); i++)
		{
			BitTorrent_PieceDownloadState& DownloadStateToCompare = StateToUpdate.DownloadState[i];
			if (DownloadStateToCompare.PieceIndex == PieceInfo.PieceIndex)
			{
				assert(DownloadStateToCompare.CurrentOffset == PieceInfo.PieceBegin);
				StateToUpdate.PieceData[i] += PieceDataMessage.substr(ParseOffset);
				DownloadStateToCompare.CurrentOffset += PieceInfo.PieceLength;
				if (DownloadStateToCompare.CurrentOffset == DownloadStateToCompare.PieceSize)
				{
					DownloadStateToCompare.Completed = true;
				}
				break;
			}
		}
		bool IsFinished = true;
		for (size_t i = 0; i < StateToUpdate.DownloadState.size(); i++)
		{
			if (!StateToUpdate.DownloadState[i].Completed)
			{
				IsFinished = false;
				break;
			}
		}
		StateToUpdate.AssignedPiecesFinished = IsFinished;
	}
	std::string MBBitTorrentPeerConnection::sp_GeneratePieceRequestMessage(size_t PieceIndex, size_t PieceBegin, size_t Length)
	{
		std::string ReturnValue = std::string(4, 0);
		ReturnValue += char(BitTorrent_MessageType::request);
		ReturnValue += std::string(4, 0);
		ReturnValue += std::string(4, 0);
		ReturnValue += std::string(4, 0);
		size_t ParseOffset = 5;
		InsertBigEndianInteger(ReturnValue.data(), 4, PieceIndex, ParseOffset, &ParseOffset);
		InsertBigEndianInteger(ReturnValue.data(), 4, PieceBegin, ParseOffset, &ParseOffset);
		InsertBigEndianInteger(ReturnValue.data(), 4, Length, ParseOffset, &ParseOffset);
		InsertBigEndianInteger(ReturnValue.data(), 4, ReturnValue.size()-4, 0, nullptr);
		return(ReturnValue);
	}
	void MBBitTorrentPeerConnection::p_SendDataToPeer(std::string const& DataToSend)
	{
		MBSockets::ConnectSocket* SocketToUse = p_GetAssociatedSocket();
		std::lock_guard<std::mutex> Lock(m_SendMutex);
		SocketToUse->SendData(DataToSend);
	}
	void MBBitTorrentPeerConnection::p_SendPieceDataRequests()
	{
		size_t MaxSimultaneousRequests = 10;
		uintmax_t MaxRequestBytes = 1 << 14;
		std::string ConcattenatedMessageData = "";
		size_t NumberOfRequests = 0;
		size_t CurrentPieceRequestedData = 0;
		while (NumberOfRequests < MaxSimultaneousRequests && m_RetrievalState.CurrentPiece < m_RetrievalState.DownloadState.size())
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			BitTorrent_PieceDownloadState& CurrentDownloadState = m_RetrievalState.DownloadState[m_RetrievalState.CurrentPiece];
			size_t PieceIndex = CurrentDownloadState.PieceIndex;
			size_t PieceOffset = CurrentDownloadState.CurrentOffset+CurrentPieceRequestedData;
			size_t PieceLength = std::min(CurrentDownloadState.PieceSize - CurrentPieceRequestedData,MaxRequestBytes);
			CurrentPieceRequestedData += PieceLength;
			if (CurrentPieceRequestedData + CurrentDownloadState.CurrentOffset == CurrentDownloadState.PieceSize)
			{
				m_RetrievalState.CurrentPiece += 1;
			}
			ConcattenatedMessageData += sp_GeneratePieceRequestMessage(PieceIndex, PieceOffset, PieceLength);
		}
		p_SendDataToPeer(ConcattenatedMessageData);
	}
	void MBBitTorrentPeerConnection::p_SendClientUninterestedMessage()
	{
		MBSockets::ConnectSocket* SocketToUse = p_GetAssociatedSocket();
		std::string DataToSend = { 0,0,0,1,char(BitTorrent_MessageType::interested) };
		SocketToUse->SendData(DataToSend);
	}
	void MBBitTorrentPeerConnection::p_RetrievePiecesHandler()
	{
		while (IsConnected() && !p_IsStopping())
		{
			std::unique_lock<std::mutex> Lock(m_InternalsMutex);
			while (m_ClientState.Interested == false)
			{
				m_RecieveConditional.wait(Lock);
			}
			if (m_RetrievalState.NumberOfPendingRequests == 0)
			{
				p_SendPieceDataRequests();
			}
			while (m_RetrievalState.RecievedPieceMessages.empty() && !p_IsStopping())
			{
				m_RecieveConditional.wait(Lock);
			}
			size_t NewMessages = m_RetrievalState.RecievedPieceMessages.size();
			for (size_t i = 0; i < NewMessages; i++)
			{
				std::string MessageData;
				std::swap(MessageData, m_RetrievalState.RecievedPieceMessages.front());
				m_RetrievalState.RecievedPieceMessages.pop_front();
				sp_UpdatePieceDownloadState(m_RetrievalState,MessageData);
				m_RetrievalState.NumberOfPendingRequests -= 1;
				if (m_RetrievalState.AssignedPiecesFinished)
				{
					p_SendClientUninterestedMessage();
					m_AssociatedDownloadHandler->p_NotifyAssignmentFinished();
				}
			}
		}
	}
	void MBBitTorrentPeerConnection::ResetDownloadState()
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		m_RetrievalState = MBBitTorrent_PeerConnectionRetrievalState();
	}
	void MBBitTorrentPeerConnection::AssignPieces(std::vector<BitTorrent_PieceDownloadState> const& DownloadState)
	{
		ResetDownloadState();
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		m_RetrievalState.DownloadState = DownloadState;
		m_RetrievalState.AssignedPieces = DownloadState;
	}
	void MBBitTorrentPeerConnection::TransferAssignedPieces(std::vector<BitTorrent_PieceDownloadState>& TargetDestination)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		TargetDestination = std::move(m_RetrievalState.AssignedPieces);
		m_RetrievalState.PieceData = {};
	}
	void MBBitTorrentPeerConnection::TransferPiecedata(std::vector<std::string>& TargetDestination)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		TargetDestination = std::move(m_RetrievalState.PieceData);
		m_RetrievalState.PieceData = {};
	}
	void MBBitTorrentPeerConnection::TransferDownloadState(std::vector<BitTorrent_PieceDownloadState>& TargetDestination)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		TargetDestination = std::move(m_RetrievalState.DownloadState);
		m_RetrievalState.DownloadState = {};
	}
	bool MBBitTorrentPeerConnection::p_IsStopping()
	{
		return(m_ShouldStop.load());
	}
	bool MBBitTorrentPeerConnection::sp_PieceIsFinished(MBBitTorrent_PeerConnectionRetrievalState const& StateToCheck, size_t PieceIndex)
	{
		return(StateToCheck.DownloadState.at(PieceIndex).Completed);
	}
	bool MBBitTorrentPeerConnection::IsConnected()
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		bool ReturnValue = m_AssociatedSocket.load()->IsConnected();
		return(ReturnValue);
	}
	bool MBBitTorrentPeerConnection::PieceIsFinished(size_t PieceIndex)
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		return(sp_PieceIsFinished(m_RetrievalState, PieceIndex));
	}
	bool MBBitTorrentPeerConnection::AssignedPiecesAreFinished()
	{
		size_t NumberOfPieces = 0;
		{
			std::lock_guard<std::mutex> Lock(m_InternalsMutex);
			NumberOfPieces = m_RetrievalState.DownloadState.size();
		}
		bool AllAreFinished = true;
		for (size_t i = 0; i < NumberOfPieces; i++)
		{
			if (!PieceIsFinished(i))
			{
				AllAreFinished = false;
				break;
			}
		}
		return(true);
	}
	//BEGIN MBBitTorrentHandler
	std::string MBBitTorrentHandler::p_GetLocalPieceData(size_t PieceIndex, size_t Offset, size_t Length)
	{
		return("");
	}
	std::string h_ParseBencodedString(std::string const& DataToParse, size_t InParseOffset, size_t* OutParseOffset)
	{
		size_t ParseOffset = InParseOffset;
		size_t SizeEnd = DataToParse.find(':', ParseOffset);
		size_t StringSize = std::stoi(DataToParse.substr(InParseOffset, SizeEnd - ParseOffset));
		ParseOffset = SizeEnd + 1;
		std::string ReturnValue = DataToParse.substr(ParseOffset, StringSize);
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset + StringSize;
		}
		return(ReturnValue);
	}
	uintmax_t h_ParseBencodedInteger(std::string const& DataToParse, size_t InParseOffset, size_t* OutParseOffset)
	{
		size_t ParseOffset = InParseOffset;
		ParseOffset += 1;
		size_t IntegerEnd = DataToParse.find('e', ParseOffset);
		uintmax_t ReturnValue = std::stoi(DataToParse.substr(ParseOffset, IntegerEnd - ParseOffset));
		ParseOffset = IntegerEnd + 1;
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	std::string h_ExtractBencodedElementData(std::string const& DataToParse, size_t InParseOffset, size_t* OutParseOffset)
	{
		size_t ParseOffset = InParseOffset;
		std::string ReturnValue = "";
		if (DataToParse[ParseOffset] == 'i')
		{
			size_t ElementEnd = DataToParse.find('e', ParseOffset);
			ReturnValue = DataToParse.substr(ParseOffset, ElementEnd - ParseOffset);
			ReturnValue += 'e';
			ParseOffset = ElementEnd + 1;
		}
		else if (DataToParse[ParseOffset] == 'l')
		{
			ReturnValue = "l";
			ParseOffset += 1;
			while (DataToParse[ParseOffset] != 'e')
			{
				ReturnValue += h_ExtractBencodedElementData(DataToParse, ParseOffset, &ParseOffset);
			}
			ReturnValue += 'e';
			ParseOffset += 1;
		}
		else if (DataToParse[ParseOffset] == 'd')
		{
			ReturnValue = "d";
			ParseOffset += 1;
			while (DataToParse[ParseOffset] != 'e')
			{
				ReturnValue += h_ExtractBencodedElementData(DataToParse, ParseOffset, &ParseOffset);
				ReturnValue += h_ExtractBencodedElementData(DataToParse, ParseOffset, &ParseOffset);
			}
			ReturnValue += 'e';
			ParseOffset += 1;
		}
		else
		{
			//måste vara en string 
			size_t ColonPosition = DataToParse.find(':', ParseOffset);
			ReturnValue = DataToParse.substr(ParseOffset, ColonPosition - ParseOffset + 1);
			ReturnValue += h_ParseBencodedString(DataToParse, ParseOffset, &ParseOffset);
		}
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	std::map<std::string, std::string> h_ParseBencodedDictionary(std::string const& DataToParse, size_t InParseOffset, size_t* OutParseOffset)
	{
		std::map<std::string, std::string> ReturnValue = {};
		size_t ParseOffset = InParseOffset;
		if (DataToParse[ParseOffset] != 'd')
		{
			assert(false);
		}
		ParseOffset += 1;
		while (DataToParse[ParseOffset] != 'e')
		{
			std::string NewKeyName = h_ParseBencodedString(DataToParse,ParseOffset,&ParseOffset);
			ReturnValue[NewKeyName] = h_ExtractBencodedElementData(DataToParse, ParseOffset, &ParseOffset);
		}
		ParseOffset += 1;
		if (OutParseOffset != nullptr)
		{
			*OutParseOffset = ParseOffset;
		}
		return(ReturnValue);
	}
	std::string MBBitTorrentHandler::p_GetClientID()
	{
		std::lock_guard < std::mutex> Lock(m_InternalsMutex);
		return(m_TrackerState.PeerID);
	}
	std::vector<bool> MBBitTorrentHandler::p_GetLocalPieces()
	{
		std::vector<bool> ReturnValue = {};
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		for (size_t i = 0; i < m_DownloadState.SavedPieceData.size(); i++)
		{
			ReturnValue.push_back(m_DownloadState.SavedPieceData[i].Completed);
		}
		return(ReturnValue);
	}
	std::string MBBitTorrentHandler::p_GetTorrentHash()
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		return(m_AssociatedTorrent.TorrentInfoHash);
	}
	BitTorrent_Torrent MBBitTorrentHandler::p_ParseBitTorrent_Torrent(std::string const& TorrentData)
	{
		BitTorrent_Torrent ReturnValue;
		std::map<std::string, std::string> TopLevelTorrentDictionary = h_ParseBencodedDictionary(TorrentData, 0, nullptr);
		ReturnValue.TrackerURL = h_ParseBencodedString(TopLevelTorrentDictionary["announce"], 0, nullptr);
		ReturnValue.TorrentInfoHash = MBCrypto::HashData(TopLevelTorrentDictionary["info"], MBCrypto::HashFunction::SHA1);
		std::map<std::string, std::string> InfoDictionary = h_ParseBencodedDictionary(TopLevelTorrentDictionary["info"], 0,nullptr);
		ReturnValue.OutputName = h_ParseBencodedString(InfoDictionary["name"], 0, nullptr);
		ReturnValue.PieceLength = h_ParseBencodedInteger(InfoDictionary["piece length"], 0, nullptr);
		std::string PiecesString = h_ParseBencodedString(InfoDictionary["pieces"],0,nullptr);
		for (size_t i = 0; i < PiecesString.size()/20; i++)
		{
			ReturnValue.PieceHashes.push_back(PiecesString.substr(20 * i, 20));
		}
		if (TopLevelTorrentDictionary.find("announce-list") != TopLevelTorrentDictionary.end())
		{
			std::string& ListData = TopLevelTorrentDictionary["announce-list"];
			size_t ParseOffset = 1;
			while (ListData[ParseOffset] != 'e')
			{
				std::vector<std::string> NewTrackerTier = {};
				while (ListData[ParseOffset] != 'e')
				{
					ParseOffset += 1;
					NewTrackerTier.push_back(h_ParseBencodedString(ListData, ParseOffset, &ParseOffset));
				}
				ParseOffset += 1;
				ReturnValue.TrackerLists.push_back(NewTrackerTier);
			}
		}
		if (InfoDictionary.find("length") != InfoDictionary.end())
		{
			ReturnValue.FileLength = h_ParseBencodedInteger(InfoDictionary["length"], 0, 0);
		}
		else if (InfoDictionary.find("files") != InfoDictionary.end())
		{
			std::string& FileListData = InfoDictionary["files"];
			size_t FileListParseOffset = 1;
			while (FileListData[FileListParseOffset] != 'e')
			{
				BitTorrent_FileInfo NewFileInfo;
				FileListParseOffset += 1;//dictionary börjar med d
				h_ParseBencodedString(FileListData, FileListParseOffset, &FileListParseOffset);
				NewFileInfo.FileSize = h_ParseBencodedInteger(FileListData, FileListParseOffset, &FileListParseOffset);
				h_ParseBencodedString(FileListData, FileListParseOffset, &FileListParseOffset);
				FileListParseOffset += 1;
				while (FileListData[FileListParseOffset] != 'e')
				{
					NewFileInfo.SubDirectories.push_back(h_ParseBencodedString(FileListData, FileListParseOffset, &FileListParseOffset));
				}
				FileListParseOffset += 1;
				ReturnValue.Files.push_back(NewFileInfo);
				FileListParseOffset += 1;//slutar på ett e
			}
		}
		return(ReturnValue);
	}
	std::string MBBitTorrentHandler::p_GenerateTrackerGetParameters()
	{
		std::string ReturnValue = "?";
		ReturnValue += "peer_id=" + MBUtility::URLEncodeData(m_TrackerState.PeerID);
		ReturnValue += "&info_hash=" + MBUtility::URLEncodeData(m_AssociatedTorrent.TorrentInfoHash);
		ReturnValue += "&port=" + m_TrackerState.Port;
		ReturnValue += "&left=" + std::to_string(m_TrackerState.BytesLeft);
		ReturnValue += "&downloaded=" + std::to_string(m_TrackerState.BytesDownloaded);
		ReturnValue += "&uploaded=" + std::to_string(m_TrackerState.BytesUploaded);
		//ReturnValue += "&event=" + p_EventToString(m_TrackerState.Event);
		return(ReturnValue);
	}
	std::string MBBitTorrentHandler::p_EventToString(BitTorrent_TrackerEvent EventToConvert)
	{
		if (EventToConvert == BitTorrent_TrackerEvent::Completed)
		{
			return("completed");
		}
		else if (EventToConvert == BitTorrent_TrackerEvent::Started)
		{
			return("started");
		}
		else if (EventToConvert == BitTorrent_TrackerEvent::Stopped)
		{
			return("stopped");
		}
		else
		{
			return("");
		}
	}
	ConnectTuple MBBitTorrentHandler::p_GetAnnounceURLConnectionTuple(std::string const& AnnounceURL)
	{
		ConnectTuple ReturnValue;
		size_t PortBegin = AnnounceURL.find(':',AnnounceURL.find(":") + 1);
		size_t PortEnd = AnnounceURL.find('/', PortBegin);
		ReturnValue.Port = AnnounceURL.substr(PortBegin + 1, PortEnd - PortBegin - 1);
		ReturnValue.Adress = AnnounceURL.substr(0, PortBegin) + AnnounceURL.substr(PortEnd);
		return(ReturnValue);
	}
	BitTorrent_TrackerResponse MBBitTorrentHandler::p_ParseBitTorrent_TrackerResponse(std::map<std::string, std::string> const& DecodedDictionary)
	{
		BitTorrent_TrackerResponse ReturnValue;
		ReturnValue.ResendInterval = h_ParseBencodedInteger(DecodedDictionary.at("interval"),0,nullptr);
		std::string const& PeerData = DecodedDictionary.at("peers");
		if (PeerData[0] == 'l')
		{
			size_t ParseOffset = 1;
			while (PeerData[ParseOffset])
			{
				BitTorrent_Peer NewPeer;
				std::map<std::string, std::string> PeerDictionary = h_ParseBencodedDictionary(PeerData, ParseOffset, &ParseOffset);
				NewPeer.ID = h_ParseBencodedString(PeerDictionary["peer id"],0,nullptr);
				NewPeer.Adress = h_ParseBencodedString(PeerDictionary["ip"], 0, nullptr);
				NewPeer.Port = h_ParseBencodedInteger(PeerDictionary["port"], 0, nullptr);
				ReturnValue.Peers.push_back(NewPeer);
			}
		}
		else
		{
			//det är en compact string istället
			std::string TotalPeerData = h_ParseBencodedString(PeerData,0,nullptr);
			size_t NumberOfPeers = TotalPeerData.size() / 6;
			for (size_t i = 0; i < NumberOfPeers; i++)
			{
				BitTorrent_Peer NewPeer;
				std::string NewPeerData = TotalPeerData.substr(i * 6, 6);
				for (size_t j = 0; j < 4; j++)
				{
					NewPeer.Adress += std::to_string((unsigned char)NewPeerData[j]);
					if (j < 3)
					{
						NewPeer.Adress += '.';
					}
				}
				NewPeer.Port = uint8_t(NewPeerData[4]) << 8;
				NewPeer.Port += uint8_t(NewPeerData[5]);
				ReturnValue.Peers.push_back(NewPeer);
			}
		}
		return(ReturnValue);
	}
	void MBBitTorrentHandler::p_UpdateTrackerResponse()
	{
		std::string AnnounceURLToTest = m_AssociatedTorrent.TrackerURL;
		if (AnnounceURLToTest == "")
		{
			return;
		}
		size_t TierOffset = 0;
		size_t ListOffset = -1;
		while (true)
		{
			ConnectTuple ConnectionToTest = p_GetAnnounceURLConnectionTuple(AnnounceURLToTest);
			std::string Host = ConnectionToTest.Adress.substr(ConnectionToTest.Adress.find("//") + 2);
			Host = Host.substr(0, Host.find_last_of('/'));
			MBSockets::HTTPConnectSocket SocketToUse(Host, ConnectionToTest.Port);
			SocketToUse.Connect();
			if (ConnectionToTest.Adress.substr(0, 5) == "https")
			{
				SocketToUse.EstablishTLSConnection();
			}
			bool RequestFailed = false;
			std::string BencodedResponse = SocketToUse.GetDataFromRequest("GET",ConnectionToTest.Adress.substr(ConnectionToTest.Adress.find_last_of('/')+1)+p_GenerateTrackerGetParameters());
			BencodedResponse = BencodedResponse.substr(BencodedResponse.find("\r\n\r\n") + 4);
			std::map<std::string, std::string> DecodedResponse;
			if (BencodedResponse == "")
			{
				RequestFailed = true;
			}
			else
			{
				DecodedResponse = h_ParseBencodedDictionary(BencodedResponse, 0, nullptr);
			}
			if (DecodedResponse.find("failure reason") != DecodedResponse.end())
			{
				std::cout << DecodedResponse["failure reason"] << std::endl;
				RequestFailed = true;
			}
			if (!RequestFailed)
			{
				m_CurrentTrackerResponse = p_ParseBitTorrent_TrackerResponse(DecodedResponse);
				break;
			}
			else
			{
				if (TierOffset + 1 > m_AssociatedTorrent.TrackerLists.size())
				{
					break;
				}
				ListOffset += 1;
				if (ListOffset >= m_AssociatedTorrent.TrackerLists[TierOffset].size())
				{
					ListOffset = 0;
					TierOffset += 1;
				}
				if (TierOffset + 1 > m_AssociatedTorrent.TrackerLists.size())
				{
					break;
				}
				AnnounceURLToTest = m_AssociatedTorrent.TrackerLists[TierOffset][ListOffset];
			}
		}
	}

	void MBBitTorrentHandler::SaveDownloadState(std::string const& OutputFilepath)
	{

	}
	void MBBitTorrentHandler::LoadDownloadState(std::string const& OutputFilepath)
	{

	}
	void MBBitTorrentHandler::LoadTorrentInfo(std::string const& TorrentFilepath)
	{
		std::ifstream TorrentFile = std::ifstream(TorrentFilepath, std::ios::binary | std::ios::in);
		std::string TorrentData = std::string(MBGetFileSize(TorrentFilepath), 0);
		TorrentFile.read(TorrentData.data(), TorrentData.size());
		m_AssociatedTorrent = p_ParseBitTorrent_Torrent(TorrentData);
		for (size_t i = 0; i < m_AssociatedTorrent.PieceHashes.size(); i++)
		{
			m_DownloadState.SavedPieceData.push_back(BitTorrent_PieceDownloadState());
		}
	}
	void MBBitTorrentHandler::p_NotifyAssignmentFinished()
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		m_FinishedAssignmentsWaiting += 1;
		m_AssignmentFinishedConditional.notify_one();
	}
	bool MBBitTorrentHandler::p_DownloadFinished()
	{
		return(m_DownloadFinished);
	}
	bool MBBitTorrentHandler::p_ShouldStop()
	{
		return(m_ShouldStop);
	}
	void MBBitTorrentHandler::p_NotifyPieceDownloadFinished()
	{
		m_AssignmentFinishedConditional.notify_one();
	}
	std::vector<SavePieceDataInfo> MBBitTorrentHandler::sp_GetPieceSaveInfo(BitTorrent_Torrent const& AssociatedTorrent,
		BitTorrent_PieceDownloadState const& AssignedPiece, std::string const& PieceData)
	{
		std::vector<SavePieceDataInfo> ReturnValue = {};
		size_t PieceDataOffset = 0;
		while (PieceDataOffset < PieceData.size())
		{
			size_t FileIndex = 0;
			size_t TotalCurrentOffset = AssignedPiece.PieceIndex * AssociatedTorrent.PieceLength;
			TotalCurrentOffset += PieceDataOffset;
			//O(n) men palla optimera
			for (size_t i = 0; i < AssociatedTorrent.Files.size(); i++)
			{
				if (TotalCurrentOffset <= AssociatedTorrent.Files[i].FileSize)
				{
					TotalCurrentOffset -= AssociatedTorrent.Files[i].FileSize;
					FileIndex += 1;
				}
				else
				{
					break;
				}
			}
			SavePieceDataInfo InfoToAdd;
			BitTorrent_FileInfo const& CurrentFile = AssociatedTorrent.Files[FileIndex];
			InfoToAdd.Filepath = "";
			for (size_t i = 0; i < CurrentFile.SubDirectories.size(); i++)
			{
				InfoToAdd.Filepath += CurrentFile.SubDirectories[i];
				if (i+1 < CurrentFile.SubDirectories.size())
				{
					InfoToAdd.Filepath += "/";
				}
			}
			InfoToAdd.SaveOffset = TotalCurrentOffset;
			size_t NumberOfBytesToWrite = std::min((size_t)PieceData.size() - PieceDataOffset,(size_t) CurrentFile.FileSize - TotalCurrentOffset);
			InfoToAdd.NumberOfBytesToSave = NumberOfBytesToWrite;
			PieceDataOffset += NumberOfBytesToWrite;
		}
		return(ReturnValue);
	}
	void MBBitTorrentHandler::p_SavePieceDataInfo(std::vector<SavePieceDataInfo> const& SaveData, std::string const& DataToSave)
	{
		size_t DataOffset = 0;
		for (size_t i = 0; i < SaveData.size(); i++)
		{
			std::ofstream FileToSaveTo = std::ofstream(SaveData[i].Filepath,std::ios::out|std::ios::binary);
			FileToSaveTo.seekp(SaveData[i].SaveOffset);
			FileToSaveTo.write(DataToSave.data() + DataOffset, SaveData[i].NumberOfBytesToSave);
			FileToSaveTo.flush();
			FileToSaveTo.close();
			DataOffset += SaveData[i].NumberOfBytesToSave;
		}
	}
	void MBBitTorrentHandler::p_SavePiecesAndState(std::vector<BitTorrent_PieceDownloadState> const& AssignedPieces, std::vector<std::string> const& PieceData)
	{
		for (size_t i = 0; i < AssignedPieces.size(); i++)
		{
			std::vector<SavePieceDataInfo> PieceSaveInfo;
			//ANTAGANDE vi vet att längden av stringen i indatan repensterar hur långt på piecen den parsat
			//ANTAGANDE den assignade piecen vi får in är alltid lika med vårt nuvarande state
			{
				std::lock_guard<std::mutex> Lock(m_InternalsMutex);
				size_t CurrentPieceIndex = AssignedPieces[i].PieceIndex;
				assert(m_DownloadState.SavedPieceData[CurrentPieceIndex].CurrentOffset == AssignedPieces[i].CurrentOffset);
				m_DownloadState.SavedPieceData[CurrentPieceIndex] = AssignedPieces[i];
				m_PieceAssigned[CurrentPieceIndex] = false;
				PieceSaveInfo = sp_GetPieceSaveInfo(m_AssociatedTorrent, AssignedPieces[i], PieceData[i]);
			}
			p_SavePieceDataInfo(PieceSaveInfo, PieceData[i]);
		}
	}
	//Uses O(n) Algorithm for the amount of pieces, could maybe be optimized but should be negligable
	std::vector<BitTorrent_PieceDownloadState> MBBitTorrentHandler::p_GetPiecesToDownload()
	{
		std::lock_guard<std::mutex> Lock(m_InternalsMutex);
		std::vector<BitTorrent_PieceDownloadState> ReturnValue = {};
		for (size_t i = 0; i < m_DownloadState.SavedPieceData.size(); i++)
		{
			if (m_DownloadState.SavedPieceData[i].Completed == false && m_PieceAssigned[i] == false)
			{
				ReturnValue.push_back(m_DownloadState.SavedPieceData[i]);
				m_PieceAssigned[i] = true;
			}
		}
		return(ReturnValue);
	}
	void MBBitTorrentHandler::p_DownloadLoop()
	{
		while(!p_ShouldStop() && !p_DownloadFinished()) 
		{
			for (size_t i = 0; i < m_PeerConnections.size(); i++)
			{
				if (m_PeerConnections[i]->AssignedPiecesAreFinished())
				{
					std::vector<BitTorrent_PieceDownloadState> AssignedPieces;
					std::vector<BitTorrent_PieceDownloadState> CurrentState;
					std::vector<std::string> FinishedData;
					m_PeerConnections[i]->TransferAssignedPieces(AssignedPieces);
					m_PeerConnections[i]->TransferPiecedata(FinishedData);
					p_SavePiecesAndState(AssignedPieces, FinishedData);
					if (!p_DownloadFinished())
					{
						m_PeerConnections[i]->AssignPieces(p_GetPiecesToDownload());
					}
				}
			}
			std::unique_lock<std::mutex> Lock(m_InternalsMutex);
			while (m_FinishedAssignmentsWaiting == 0 && !p_ShouldStop() && !p_DownloadFinished())
			{
				m_AssignmentFinishedConditional.wait(Lock);
			}
		}
	}
	void MBBitTorrentHandler::StartDownload()
	{
		p_UpdateTrackerResponse();
		//ska väl egentligen skapa flera kopplingar men nu hard codar vi
		//m_PeerConnections.push_back(std::unique_ptr<MBBitTorrentPeerConnection>(new MBBitTorrentPeerConnection(this)));
		//m_PeerConnections.back()->InitiateConnection("158.140.162.224","59520");
		m_PeerConnections.push_back(std::unique_ptr<MBBitTorrentPeerConnection>(new MBBitTorrentPeerConnection(this)));
		for (size_t i = 5; i < m_CurrentTrackerResponse.Peers.size(); i++)
		{
			if (m_CurrentTrackerResponse.Peers[i].Adress != "188.151.142.88")
			{
				m_PeerConnections.back()->InitiateConnection(m_CurrentTrackerResponse.Peers[i].Adress,std::to_string(m_CurrentTrackerResponse.Peers[i].Port));
				if (m_PeerConnections.back()->IsConnected())
				{
					break;
				}
			}
		}
		p_DownloadLoop();
	}
	void MBBitTorrentHandler::StopDownload()
	{

	}
	bool MBBitTorrentHandler::DownloadFinished()
	{
		return(true);
	}
	uintmax_t MBBitTorrentHandler::GetTotalTorrentSize()
	{
		return(0);
	}
	//END MBBitTorrentHandler
};