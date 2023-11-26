#define NOMINMAX
#include <MrPostOGet/MrPostOGet.h>
#include "MPGMemeSite.h"
#include <MBSearchEngine/MBSearchEngine.h>
#include <filesystem>
#include <MBUnicode/MBUnicode.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
#include <MBMime/MBMime.h>
#include <ctime>
#include <chrono>
#include <MBSystem/MBSystem.h>
#include <MBUtility/MBStrings.h>
#include <MBPacketManager/MBPacketManager.h>
#include <unordered_map>
#include <unordered_set>
//username cookie = 
//password cookie = 
namespace MBWebsite
{
	using ::ToJason;
	enum class DBPermissions
	{
		Read,
		Edit,
		Upload,
		Null,
	};
	//std::map<std::string, std::unique_ptr<MBDB_Object>> m_Attributes = {};
	//std::vector<std::unique_ptr<MBDB_Object>> m_ArrayObjects = {};
	//MBDBO_Type m_Type = MBDBO_Type::Null;
	//bool m_IsAtomic = false;
	//std::string m_AtomicValue = "";



	std::string h_ReadWholeInput(std::unique_ptr<MBUtility::MBSearchableInputStream>& StreamToRead)
	{
		std::string ReturnValue;
		const size_t ChunkReadSize = 4096;//godtycklig
		std::string ReadBuffer = std::string(ChunkReadSize, 0);
		while (true)
		{
			size_t ReadBytes = StreamToRead->Read(ReadBuffer.data(), ChunkReadSize);
			if (ReadBytes == -1)
			{
				break;
			}
			ReadBuffer.resize(ReadBytes);
			ReturnValue += ReadBuffer;
			if (ReadBytes != ChunkReadSize)
			{
				break;
			}
		}
		return(ReturnValue);
	}






	//BEGIN MBDB_Website_BaiscPasswordAuthenticator
	MBDB_Website_BaiscPasswordAuthenticator::MBDB_Website_BaiscPasswordAuthenticator(MBDB_Website* AssociatedServer)
	{
		m_AssociatedServer = AssociatedServer;
	}
	bool MBDB_Website_BaiscPasswordAuthenticator::VerifyUser(std::string const& Username, std::string const& Password)
	{
		std::string HashedPassword = MBCrypto::HashData(Password, MBCrypto::HashFunction::SHA256);
		std::vector<MBDB::MBDB_RowData> UserResult = m_AssociatedServer->m_GetUser(Username, MBUtility::ReplaceAll(MBUtility::HexEncodeString(HashedPassword), " ", ""));
		if (UserResult.size() == 0)
		{
			return(false);
		}
		else
		{
			return(true);
		}
	}
	//END MBDB_Website_BaiscPasswordAuthenticator

	//BEGIN MBDB_Website_GitHandler
	bool  MBDB_Website_GitHandler::p_VerifyAuthentication(MrPostOGet::HTTPClientRequest const& AssociatedRequest)
	{
		if (m_UserAuthenticator == nullptr)
		{
			return(false);
		}
		else
		{
			if (AssociatedRequest.Headers.find("authorization") == AssociatedRequest.Headers.end())
			{
				return(false);
			}
			std::string AuthenticationHeader = AssociatedRequest.Headers.at("authorization").front();
			size_t AuthenticationDataBegin = AuthenticationHeader.find(' ');
			if (AuthenticationDataBegin == AuthenticationHeader.npos)
			{
				return(false);
			}
			std::string AuthenticationData = MBParsing::BASE64Decode(AuthenticationHeader.substr(AuthenticationDataBegin + 1));
			size_t ColonPosition = AuthenticationData.find(':');
			if (ColonPosition == AuthenticationData.npos)
			{
				return(false);
			}
			std::string Username = AuthenticationData.substr(0, ColonPosition);
			std::string Password = AuthenticationData.substr(ColonPosition + 1);
			return(m_UserAuthenticator->VerifyUser(Username, Password));
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website_GitHandler::p_GetAuthenticationPrompt(MrPostOGet::HTTPClientRequest const& AssociatedRequest)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		//vi m�ste kolla vilken anv�ndare det �r
		std::string RepoUser = "";
		std::string TopLevelDirectoryPath = AssociatedRequest.RequestResource.substr(m_URLPrefix.size());
		size_t UserEnd = TopLevelDirectoryPath.find('/');
		if (UserEnd == TopLevelDirectoryPath.npos)
		{
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
		}
		else
		{
			std::string User = TopLevelDirectoryPath.substr(0, UserEnd);
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::Authenticate;
			ReturnValue.ExtraHeaders["WWW-Authenticate"].push_back("Basic realm=\"/" + User + "/\"");
		}
		ReturnValue.DocumentData = "<html><body></body></html>";
		return(ReturnValue);
	}
	void MBDB_Website_GitHandler::p_SetGCIVariables(MrPostOGet::HTTPClientRequest const& RequestToParse)
	{
		//ANTAGANDE fungerar bara givet att vi faktiskt servar den h�r requesten
		std::string REQUEST_METHOD = MrPostOGet::HTTPRequestTypeToString(RequestToParse.Type);
		std::string QUERY_STRING = "";
		std::string PATH_TRANSLATED = m_TopLevelDirectory + RequestToParse.RequestResource.substr(m_URLPrefix.size()); //
		size_t QueryBegin = PATH_TRANSLATED.find('?');
		if (QueryBegin != PATH_TRANSLATED.npos)
		{
			QUERY_STRING = PATH_TRANSLATED.substr(QueryBegin + 1);
			PATH_TRANSLATED = PATH_TRANSLATED.substr(0, QueryBegin);
		}

		std::string CONTENT_TYPE = "";
		if (RequestToParse.Headers.find("content-type") != RequestToParse.Headers.end())
		{
			CONTENT_TYPE = RequestToParse.Headers.at("content-type").front();
		}
		MBSystem::SetEnvironmentVariable("PATH_TRANSLATED", PATH_TRANSLATED);
		MBSystem::SetEnvironmentVariable("REQUEST_METHOD", REQUEST_METHOD);
		MBSystem::SetEnvironmentVariable("QUERY_STRING", QUERY_STRING);
		if (CONTENT_TYPE != "")
		{
			MBSystem::SetEnvironmentVariable("CONTENT_TYPE", CONTENT_TYPE);
		}
		//borde egentligen bar ag�ras en g�ng
		MBSystem::SetEnvironmentVariable("GIT_HTTP_EXPORT_ALL", "true");

		//DEBUG
		if (RequestToParse.Headers.find("authorization") != RequestToParse.Headers.end())
		{
			MBSystem::SetEnvironmentVariable("REMOTE_USER", "Test");
		}

		//DEBUG
		std::cout << "PATH_TRANSLATED=" << PATH_TRANSLATED << std::endl;
		std::cout << "QUERY_STRING=" << QUERY_STRING << std::endl;
		std::cout << "REQUEST_METHOD=" << REQUEST_METHOD << std::endl;
		std::cout << "CONTENT_TYPE=" << CONTENT_TYPE << std::endl;
	}
	MBDB_Website_GitHandler::MBDB_Website_GitHandler(std::string const& TopResourceDirectory, MBUtility::MBBasicUserAuthenticator* Authenticator)
	{
		m_UserAuthenticator = Authenticator;
		m_TopLevelDirectory = TopResourceDirectory;
	}
	void MBDB_Website_GitHandler::SetURLPrefix(std::string const& PathPrefix)
	{
		std::lock_guard<std::mutex> LockGuard(m_InternalsMutex);
		m_URLPrefix = PathPrefix;
	}
	void MBDB_Website_GitHandler::SetTopDirectory(std::string const& DirectoryToSet)
	{
		std::lock_guard<std::mutex> LockGuard(m_InternalsMutex);
		m_TopLevelDirectory = DirectoryToSet;
	}

	bool MBDB_Website_GitHandler::HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer)
	{
		return(RequestToHandle.RequestResource.substr(0, m_URLPrefix.size()) == m_URLPrefix);
	}
	MrPostOGet::HTTPDocument MBDB_Website_GitHandler::GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket* Socket, MrPostOGet::HTTPServer*)
	{
		//TODO fixa s� att man inte kan skriva och laddad ner samtidigt
		std::lock_guard<std::mutex> Lock(m_UploadMutex);
		MrPostOGet::HTTPDocument ReturnValue;
		if (true)
		{
			bool CommandNeedsAuthentication = false;
			bool RequestHasAuthentication = false;
			//TODO kolla faktiskt igenom protokollet f�r att se vilka URL:er som kr�ver authentication
			if (Request.SearchParameters.find("service") != Request.SearchParameters.end())
			{
				if (Request.SearchParameters.at("service") == "git-receive-pack" && Request.Headers.find("authorization") == Request.Headers.end())
				{
					//ReturnValue = p_GetAuthenticationPrompt(Request);
					CommandNeedsAuthentication = true;
				}
			}
			if (Request.RequestResource.find("git-receive-pack") != Request.RequestResource.npos)
			{
				CommandNeedsAuthentication = true;
			}
			if (Request.Headers.find("authorization") != Request.Headers.end())
			{
				RequestHasAuthentication = true;
			}
			if (CommandNeedsAuthentication && !RequestHasAuthentication)
			{
				ReturnValue = p_GetAuthenticationPrompt(Request);
			}
			else
			{
				bool AuthenticationResult = true;
				if (CommandNeedsAuthentication)
				{
					AuthenticationResult = p_VerifyAuthentication(Request);
				}
				if (AuthenticationResult)
				{
					//nu vet vi att det vi f�r �r authenticated
					p_SetGCIVariables(Request);
					std::string HTTPResponse = "";

					//ANTAGANDE hela bodyn som beh�vs f�r plats
					assert(Socket->DataIsAvailable() == false);
					if (Request.Type == MrPostOGet::HTTPRequestType::POST)
					{
						MBSystem::UniDirectionalSubProcess GCIProcess("git http-backend > __GIT_HTTP_BACKEND.temp", false);
						GCIProcess.SendData(Request.BodyData);
						GCIProcess.close();
						HTTPResponse = MrPostOGet::LoadWholeFile("__GIT_HTTP_BACKEND.temp");
					}
					else
					{
						MBSystem::UniDirectionalSubProcess GCIProcess("git http-backend", true);
						while (!GCIProcess.Finished())
						{
							HTTPResponse += GCIProcess.RecieveData();
						}
					}
					std::unordered_map<std::string, std::vector<std::string>> ResponseHeaders = MBMIME::ExtractMIMEHeaders(HTTPResponse, 0, 0);
					size_t BodyBegin = HTTPResponse.find("\r\n\r\n") + 4;
					ReturnValue.ExtraHeaders["Content-Type"].push_back(ResponseHeaders["content-type"].front());
					ReturnValue.DocumentData = HTTPResponse.substr(BodyBegin);
				}
				else
				{
					ReturnValue = p_GetAuthenticationPrompt(Request);
				}
			}
		}


		return(ReturnValue);
	}
	//END MBDB_Website_GitHandler

	//BEGIN MBDB_Website_MBPP_UploadIncorporator
	void MBDB_Website_MBPP_UploadIncorporator::IncorporatePacketChanges(std::string const& UpdatedPacket, MBPM::MBPP_ComputerInfo ComputerInfo, std::vector<std::string> const& RemovedObjects)
	{
		UpdatedPackets.push_back({ UpdatedPacket,ComputerInfo,RemovedObjects });
	}
	//END MBDB_Website_MBPP_UploadIncorporator



	//BEGIN MBDB_Website_MBPP_Handler
	MBDB_Website_MBPP_Handler::MBDB_Website_MBPP_Handler(std::string const& PacketDirectory, MBUtility::MBBasicUserAuthenticator* Authenticator)
	{
		m_PacketsDirectory = PacketDirectory;
		m_UserAuthenticator = Authenticator;
	}
	//void MBDB_Website_MBPP_Handler::AddPacketDirectory(std::string const& NewDirectory)
	//{
	//	m_PacketsDirectory = NewDirectory;
	//}
	bool MBDB_Website_MBPP_Handler::HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer)
	{
		return(RequestToHandle.RequestResource == "/MBPM" || RequestToHandle.RequestResource == "/MBPM/");
	}
	std::string MBDB_Website_MBPP_Handler::p_GetComputerDiffTopDirectory(std::string const& PacketName, MBPM::MBPP_ComputerInfo ComputerInfo)
	{
		std::string ReturnValue = "";
		std::string PacketTopDirectory = m_PacketsDirectory + PacketName + "/";
		std::string PacketComputerDiffInfoDirectory = PacketTopDirectory + ".mbpm/ComputerDiff/";
		if (std::filesystem::exists(PacketTopDirectory + ".mbpm/ComputerDiff/MBPM_ComputerDiffInfo.json"))
		{
			MBError ParsingError = true;
			MBPM::MBPP_ComputerDiffInfo ComputerDiffInfo;
			ParsingError = ComputerDiffInfo.ReadInfo(PacketTopDirectory + ".mbpm/ComputerDiff/MBPM_ComputerDiffInfo.json");
			if (ParsingError)
			{
				std::string ComputerDiffDirectory = ComputerDiffInfo.Match(ComputerInfo);
				if (ComputerDiffDirectory != "")
				{
					ReturnValue = PacketComputerDiffInfoDirectory + ComputerDiffDirectory + "/";
				}
			}
		}

		return(ReturnValue);
	}
	void MBDB_Website_MBPP_Handler::p_IncorporatePacketChanges(MBPM::MBPP_ComputerInfo ComputerInfo, std::string const& UpdatedPacket,std::vector<std::string> const& DeletedObjects)
	{
		if (std::filesystem::exists(m_PacketsDirectory + UpdatedPacket))
		{
			if (std::filesystem::exists(m_PacketsDirectory + UpdatedPacket + "/MBPM_UploadedChanges"))
			{
				bool UpdatedComputerDiff = false;
				std::string ComputerDiffTopDirectory = "";
				std::string PacketTopDirectory = m_PacketsDirectory + UpdatedPacket + "/";
				if (ComputerInfo.OSType != MBPM::MBPP_OSType::Null || ComputerInfo.ProcessorType != MBPM::MBPP_ProcessorType::Null)
				{
					UpdatedComputerDiff = true;
					ComputerDiffTopDirectory = p_GetComputerDiffTopDirectory(UpdatedPacket, ComputerInfo);
					if (ComputerDiffTopDirectory != "")
					{
						//om detta eje st�mmer borde vi egenttligen throwa error, kan ju korrumpera directoryn
						PacketTopDirectory = ComputerDiffTopDirectory + "/Data/";
					}
				}
				std::string UploadedChangesDirectory = m_PacketsDirectory + UpdatedPacket + "/MBPM_UploadedChanges/";
				std::filesystem::recursive_directory_iterator UploadedChangesIterator = std::filesystem::recursive_directory_iterator(UploadedChangesDirectory);
				//h�nder oavsett
				std::set<std::filesystem::path> UploadedFiles = {};
				for (auto const& Entry : UploadedChangesIterator)
				{
					if (Entry.is_regular_file())
					{
						std::string FilePath = MBUnicode::PathToUTF8(std::filesystem::relative(Entry.path(), UploadedChangesDirectory).generic_string());
						UploadedFiles.insert(std::filesystem::path("/" + FilePath).lexically_normal());
						std::filesystem::path NewFileDirectory = std::filesystem::path(PacketTopDirectory + FilePath).parent_path();
						if (!std::filesystem::exists(NewFileDirectory))
						{
							std::filesystem::create_directories(NewFileDirectory);
						}
						std::filesystem::rename(UploadedChangesDirectory + FilePath, PacketTopDirectory + FilePath);
					}
				}
				//deletar det som ska deletas
				//resettar directoryn
				std::vector<std::string> RemovedDefaultObjects = {};
				for (size_t i = 0; i < DeletedObjects.size(); i++)
				{
					if (MBPM::MBPP_PathIsValid(DeletedObjects[i]))
					{
						try
						{
							if (std::filesystem::exists(PacketTopDirectory + "/" + DeletedObjects[i]))
							{
								std::filesystem::remove_all(PacketTopDirectory + "/" + DeletedObjects[i]);
							}
							else
							{
								RemovedDefaultObjects.push_back(DeletedObjects[i]);
							}
						}
						catch (const std::exception& e)
						{
							std::cout << "Error deleting file from updated packet: " << e.what() << std::endl;
						}
					}
				}
				std::filesystem::remove_all(UploadedChangesDirectory);
				//skapar nytt index
				if (!UpdatedComputerDiff)
				{
					MBPM::CreatePacketFilesData(PacketTopDirectory);
				}
				else
				{
					//vi uppdaterat txt filen, samt fixar MBPM_FileInfon
					std::set<std::string> ComputerDiffRemovedObjects = {};
					std::ifstream DiffRemovedObjects = std::ifstream(ComputerDiffTopDirectory + "/MBPM_RemovedObjects.txt", std::ios::in);
					std::string CurrentLine = "";
					while (std::getline(DiffRemovedObjects, CurrentLine))
					{
						if (CurrentLine.size() > 0 && CurrentLine.back() == '\r')
						{
							CurrentLine.resize(CurrentLine.size() - 1);
						}
						ComputerDiffRemovedObjects.insert(CurrentLine);
					}
					DiffRemovedObjects.close();
					for (size_t i = 0; i < RemovedDefaultObjects.size(); i++)
					{
						ComputerDiffRemovedObjects.insert((RemovedDefaultObjects[i]));
					}
					std::vector<std::string> NewComputerDiffRemovedObjects = {};
					for (std::string const& RemovedObjects : ComputerDiffRemovedObjects)
					{
						if (UploadedFiles.find(std::filesystem::path(RemovedObjects).lexically_normal()) == UploadedFiles.end())
						{
							NewComputerDiffRemovedObjects.push_back(RemovedObjects);
						}
					}
					std::ofstream NewRemovedObjectsFile = std::ofstream(ComputerDiffTopDirectory + "/MBPM_RemovedObjects.txt", std::ios::out);
					for (size_t i = 0; i < NewComputerDiffRemovedObjects.size(); i++)
					{
						NewRemovedObjectsFile << NewComputerDiffRemovedObjects[i] << std::endl;
					}
					NewRemovedObjectsFile.flush();
					NewRemovedObjectsFile.close();
					//FileInfon
					std::ofstream NewFileInfo = std::ofstream(ComputerDiffTopDirectory + "/MBPM_UpdatedFileInfo", std::ios::out | std::ios::binary);
					MBUtility::MBFileOutputStream OutputStream = MBUtility::MBFileOutputStream(&NewFileInfo);
					MBPM::CreatePacketFilesData(ComputerDiffTopDirectory + "/Data/", &OutputStream);
					NewFileInfo.flush();
					NewFileInfo.close();
				}
			}
			else
			{
				//inga chagnges tillagda...
			}
		}
		else
		{
			//dafuq?
		}
	}
	MrPostOGet::HTTPDocument MBDB_Website_MBPP_Handler::GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket* Socket, MrPostOGet::HTTPServer*)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		std::lock_guard<std::mutex> Lock(m_WriteMutex);
		MBError GenerationError = true;
		ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::Conflict;
		try
		{
			MBPM::MBPP_Server ResponseGenerator(m_PacketsDirectory,&m_UploadIncorporator);
			ResponseGenerator.SetUserAuthenticator(m_UserAuthenticator);
			GenerationError = ResponseGenerator.InsertClientData(Request.BodyData);
			//h�r ska ocks� n�gra checks g�ras f�r att se huruvida datan �r av typen upload, och d�rmed kr�ver verifiering
			//alternativt s� ger vi den en Password verifierare
			//kan vi anta att en socket alltid �r valid samtidigt som den �r connectad? i s�nna fall kan vi nog skippa dem h�r redun
			while (!ResponseGenerator.ClientRequestFinished() && Socket->DataIsAvailable() && GenerationError && Socket->IsConnected() && Socket->IsValid())
			{
				GenerationError = ResponseGenerator.InsertClientData(Socket->GetNextChunkData());
			}
			if (ResponseGenerator.ClientRequestFinished())
			{
				MBPM::MBPP_ServerResponseIterator* ResponseIterator = ResponseGenerator.GetResponseIterator();
				std::string HTTPHeader = "HTTP/1.1 200 OK\r\n";
				HTTPHeader += "Content-Type: application/x-MBPP-record\r\n";
				HTTPHeader += "Content-Length: " + std::to_string(ResponseIterator->GetResponseSize()) + "\r\n\r\n";
				if (Socket->IsConnected() && Socket->IsValid())
				{
					Socket->SendRawData(HTTPHeader);
					while (!ResponseIterator->IsFinished() && Socket->IsConnected() && Socket->IsValid())
					{
						Socket->SendRawData(**ResponseIterator);
						ResponseIterator->Increment();
					}
				}
				ResponseGenerator.FreeResponseIterator(ResponseIterator);
				ReturnValue.DataSent = true;
				//det �r nu vi collar huruvida ett packet uppdaterats
				//if (ResponseGenerator.PacketUpdated())
				//{
				//	p_IncorporatePacketChanges(ResponseGenerator.GetUpdatedPacket(), ResponseGenerator.GetPacketRemovedFiles());
				//}
				for (size_t i = 0; i < m_UploadIncorporator.UpdatedPackets.size(); i++)
				{
					MBPP_UploadPacketTuple CurrentTuple = m_UploadIncorporator.UpdatedPackets.back();
					p_IncorporatePacketChanges(CurrentTuple.ComputerInfo, CurrentTuple.UpdatedPacket, CurrentTuple.RemovedObjects);
					m_UploadIncorporator.UpdatedPackets.pop_back();
				}

			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Error in MBPP handling: " << e.what() << std::endl;
		}

		return(ReturnValue);
	}
	//END MBDB_Website_MBPP_Handler


	//BEGIN MBDB_Website
	void MBDB_Website::m_InitDatabase()
	{
		std::fstream MBDBConfigFile("./MBDBConfigFile");
		std::string CurrentFileLine = "";
		std::string ResourceFolderConfig = "MBDBTopFolder=";
		if (MBDBConfigFile.is_open())
		{
			while (std::getline(MBDBConfigFile, CurrentFileLine))
			{
				if (CurrentFileLine.substr(0, ResourceFolderConfig.size()) == ResourceFolderConfig)
				{
					__MBTopResourceFolder = CurrentFileLine.substr(ResourceFolderConfig.size());
				}
			}
		}
		std::cout << __MBTopResourceFolder << std::endl;
		//if (m_ReadonlyDatabase == nullptr)
		//{
		//	m_ReadonlyDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/TestDatabas", 0);
		//}
		//if (WritableDatabase == nullptr)
		//{
		//	WritableDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/TestDatabas", 1);
		//}
		if (m_LoginDatabase == nullptr)
		{
			m_LoginDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/MBGLoginDatabase", MBDB::DBOpenOptions::ReadOnly);
		}
		//l�ser in mbdb config filen och initaliserar directoryn med r�tt
		__DBIndexMap = new std::unordered_map<std::string, MBSearchEngine::MBIndex>();
		m_MBDBResourceFolder = __MBTopResourceFolder + "/MBDBResources/";
		std::filesystem::directory_iterator IndexIterator(GetResourceFolderPath() + "Indexes");
		for (auto& DirectoryEntry : IndexIterator)
		{
			if (DirectoryEntry.is_regular_file())
			{
				(*__DBIndexMap)[DirectoryEntry.path().filename().generic_string()] = MBSearchEngine::MBIndex(DirectoryEntry.path().generic_string());
			}
		}
	}
	MBDB_Website::~MBDB_Website()
	{
		//delete m_ReadonlyDatabase;
		//delete WritableDatabase;
		delete m_LoginDatabase;
		delete __DBIndexMap;
	}

	MBDB_Website::MBDB_Website()
	{
		m_InitDatabase();
		__LegacyRequestHandlers = {
			{ &MBDB_Website::DBLogin_Predicate,				&MBDB_Website::DBLogin_ResponseGenerator },
			//{ &MBDB_Website::DBSite_Predicate,				&MBDB_Website::DBSite_ResponseGenerator },
			{ &MBDB_Website::UploadFile_Predicate,			&MBDB_Website::UploadFile_ResponseGenerator },
			{ &MBDB_Website::UploadSite_Predicate,			&MBDB_Website::UploadSite_ResponseGenerator },
			{ &MBDB_Website::DBGet_Predicate,				&MBDB_Website::DBGet_ResponseGenerator },
			{ &MBDB_Website::DBView_Predicate,				&MBDB_Website::DBView_ResponseGenerator },
			{ &MBDB_Website::DBViewEmbedd_Predicate,		&MBDB_Website::DBViewEmbedd_ResponseGenerator },
			{ &MBDB_Website::DBAdd_Predicate,				&MBDB_Website::DBAdd_ResponseGenerator },
			{ &MBDB_Website::DBGeneralAPI_Predicate,		&MBDB_Website::DBGeneralAPI_ResponseGenerator },
			{ &MBDB_Website::DBUpdate_Predicate,			&MBDB_Website::DBUpdate_ResponseGenerator } };
		__NumberOfHandlers.store(__LegacyRequestHandlers.size());
		__HandlersData.store(__LegacyRequestHandlers.data());

		__ModernRequestHandlers = { {&MBDB_Website::p_Edit_Predicate,&MBDB_Website::p_Edit_ResponseGenerator} };
		__ModernHandlersCount.store(__ModernRequestHandlers.size());
		__ModernHandlersData.store(__ModernRequestHandlers.data());

		m_BasicPasswordAuthenticator = std::unique_ptr<MBDB_Website_BaiscPasswordAuthenticator>(new MBDB_Website_BaiscPasswordAuthenticator(this));
		m_GitHandler = std::unique_ptr<MBDB_Website_GitHandler>(new MBDB_Website_GitHandler("/git/", m_BasicPasswordAuthenticator.get()));
		m_GitHandler->SetURLPrefix("/git/");
		char* ServerPacketPathData = std::getenv("MBPM_SERVER_PACKETS_DIRECTORY");
		std::string ServerPacketPath = "./";
		if (ServerPacketPathData != nullptr)
		{
			ServerPacketPath = ServerPacketPathData;
		}
		std::cout << "Server packets path: " << ServerPacketPath << std::endl;
		m_MPPHandler = std::unique_ptr<MBDB_Website_MBPP_Handler>(new MBDB_Website_MBPP_Handler(ServerPacketPath, m_BasicPasswordAuthenticator.get())); // ta och �ndra



		AddPlugin(std::unique_ptr<MBSite_MBEmbedder>(new MBSite_MBEmbedder()));
		AddPlugin(std::unique_ptr<MBSite_DBPlugin>(new MBSite_DBPlugin()));
		AddPlugin(std::unique_ptr<MBSitePlugin>(new MBSite_FilesystemAPIPlugin()));

		//MBDB_Website_GitHandler* InternalGitHandler = new MBDB_Website_GitHandler("", m_BasicPasswordAuthenticator.get());
		//__InternalHandlers = { std::unique_ptr< MBDB_Website_GitHandler>(InternalGitHandler)};
		//__InternalHandlersCount.store(__InternalHandlers.size());
		//__InternalHandlersData.store(__InternalHandlers.data());

	}
	bool MBDB_Website::HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer)
	{
		if (!m_GlobalResourceFolderLoaded.load())
		{
			std::lock_guard<std::mutex> Lock(m_GlobalResourceFolderMutex);
			m_GlobalResourceFolderLoaded.store(true);
			m_GlobalResourceFolder = AssociatedServer->GetResourcePath("mrboboget.se");
		}
		LegacyRequestHandler* HandlersData = __HandlersData.load();
		for (size_t i = 0; i < __NumberOfHandlers.load(); i++)
		{
			if ((this->*(HandlersData[i].Predicate))(RequestToHandle.RawRequestData))
			{
				return(true);
			}
		}
		ModernRequestHandler* ModernData = __ModernHandlersData.load();
		for (size_t i = 0; i < __ModernHandlersCount.load(); i++)
		{
			if ((this->*(ModernData[i].Predicate))(RequestToHandle, ConnectionState, AssociatedServer))
			{
				return(true);
			}
		}
		if (m_GitHandler->HandlesRequest(RequestToHandle, ConnectionState, AssociatedServer))
		{
			return(true);
		}
		if (m_MPPHandler->HandlesRequest(RequestToHandle, ConnectionState, AssociatedServer))
		{
			return(true);
		}
		//
		{
			std::string TopDirectory = RequestToHandle.RequestResource.substr(1);
			if (TopDirectory.size() > 0 && TopDirectory.find('/') != TopDirectory.npos)
			{
				TopDirectory = TopDirectory.substr(0, TopDirectory.find('/'));
			}
			std::lock_guard<std::mutex> Lock(m_HTTPHandlersMutex);
			for (auto const& Handlers : m_PluginHTTPHandlers)
			{
				if (Handlers.first == TopDirectory)
				{
					return(true);
				}
			}
		}
		
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const& ConnectionState
		, MrPostOGet::HTTPServerSocket* Connection, MrPostOGet::HTTPServer* Server)
	{
        MrPostOGet::HTTPDocument ReturnValue;
		LegacyRequestHandler* HandlersData = __HandlersData.load();
		for (size_t i = 0; i < __NumberOfHandlers.load(); i++)
		{
			if ((this->*(HandlersData[i].Predicate))(Request.RawRequestData))
			{
				return((this->*(HandlersData[i].Generator))(Request.RawRequestData, Server, Connection));
			}
		}
		ModernRequestHandler* ModernData = __ModernHandlersData.load();
		for (size_t i = 0; i < __ModernHandlersCount.load(); i++)
		{
			if ((this->*(ModernData[i].Predicate))(Request, ConnectionState, Server))
			{
				return((this->*(ModernData[i].Generator))(Request, ConnectionState, Connection, Server));
			}
		}
		if (m_GitHandler->HandlesRequest(Request, ConnectionState, Server))
		{
			m_GitHandler->SetTopDirectory(this->GetResourceFolderPath() + "/Users/");
			//m_GitHandler->ur(this->GetResourceFolderPath()+"/Users/");
			return(m_GitHandler->GenerateResponse(Request, ConnectionState, Connection, Server));
		}
		if (m_MPPHandler->HandlesRequest(Request, ConnectionState, Server))
		{
			return(m_MPPHandler->GenerateResponse(Request, ConnectionState, Connection, Server));
		}
		MBSite_HTTPHandler* MBSiteHTTPHandlerToUse = nullptr;
		{
			std::string TopDirectory = Request.RequestResource.substr(1);
			if (TopDirectory.size() > 0 && TopDirectory.find('/') != TopDirectory.npos)
			{
				TopDirectory = TopDirectory.substr(0, TopDirectory.find('/'));
			}
			std::lock_guard<std::mutex> HTTPLock(m_HTTPHandlersMutex);			
			MBSiteHTTPHandlerToUse = m_PluginHTTPHandlers[TopDirectory];
		}
		if (MBSiteHTTPHandlerToUse != nullptr)
		{
            DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(Request.RawRequestData);
            MBSiteUser NewUser;
            NewUser.m_Username = ConnectionPermissions.AssociatedUser;
            NewUser.m_GeneralPermissions |= (ConnectionPermissions.Read) * uint64_t(GeneralPermissions::View);
            NewUser.m_GeneralPermissions |= ConnectionPermissions.Upload ? uint64_t(GeneralPermissions::Upload) : 0;
            NewUser.m_GeneralPermissions |= ConnectionPermissions.Edit ? uint64_t(GeneralPermissions::Edit) : 0;
            return(MBSiteHTTPHandlerToUse->GenerateResponse(Request, NewUser, Connection));
		}
        return(ReturnValue);
	}

	std::string MBDB_Website::GetResourceFolderPath()
	{
		std::lock_guard<std::mutex> Lock(__MBTopResourceFolderMutex);
		return(m_MBDBResourceFolder);
		//return(__MBTopResourceFolder + "/MBDBResources/");
	}
	void InitDatabase()
	{
		//utg�r fr�n den foldern som programmet k�rs i
	}
	int MBGWebsiteMain()
	{
#ifndef NDEBUG
		std::cout << "Is Debug" << std::endl;
#endif // DEBUG
		MBSockets::Init();

		MrPostOGet::HTTPServer TestServer("./MBWebsite/ServerResources/mrboboget.se/HTMLResources/", 443);
		//MrPostOGet::HTTPServer TestServer("./MBWebsite/ServerResources/mrboboget.se/HTMLResources/", 1337);
		//TestServer.UseTLS(false);
		//MrPostOGet::HTTPServer TestServer("", 443);
		TestServer.LoadDomainResourcePaths("MPGDomainResourcePaths.txt");
		//TestServer.UseTLS(false);
		TestServer.AddRequestHandler(new MBDB_Website());
		TestServer.StartListening();
		return(0);
	}
	std::vector<MBDB::MBDB_RowData> MBDB_Website::m_GetUser(std::string const& UserName, std::string const& PasswordHash)
	{
		std::string SQLStatement = "SELECT * FROM Users WHERE UserName=? AND PasswordHash=?;";
		std::vector<MBDB::MBDB_RowData> QuerryResult = {};
		{
			std::lock_guard<std::mutex> Lock(m_LoginDatabaseMutex);
			MBDB::SQLStatement NewStatement = m_LoginDatabase->GetSQLStatement(SQLStatement);
			NewStatement.BindString(UserName, 1);
			NewStatement.BindString(PasswordHash, 2);
			QuerryResult = m_LoginDatabase->GetAllRows(NewStatement);
		}
		return(QuerryResult);
	}
	DBPermissionsList MBDB_Website::m_GetConnectionPermissions(std::string const& RequestData)
	{
		DBPermissionsList ReturnValue;
		std::vector<MrPostOGet::Cookie> Cookies = MrPostOGet::GetCookiesFromRequest(RequestData);
		std::string UserName = "";
		std::string PasswordHash = "";
		for (size_t i = 0; i < Cookies.size(); i++)
		{
			if (Cookies[i].Name == "DBUsername")
			{
				UserName = Cookies[i].Value;
			}
			if (Cookies[i].Name == "DBPassword")
			{
				PasswordHash = Cookies[i].Value;
			}
		}
		ReturnValue.AssociatedUser = UserName;
		std::string SQLStatement = "SELECT * FROM Users WHERE UserName=?;";
		std::vector<MBDB::MBDB_RowData> QuerryResult = m_GetUser(UserName, PasswordHash);
		if (QuerryResult.size() != 0)
		{
			std::tuple<MBDB::IntType, std::string, std::string, MBDB::IntType, MBDB::IntType, MBDB::IntType> UserInfo = QuerryResult[0].GetTuple<MBDB::IntType, std::string, std::string, MBDB::IntType, MBDB::IntType, MBDB::IntType>();
			std::string RequestPassword = std::get<2>(UserInfo);
			if (RequestPassword == PasswordHash)
			{
				ReturnValue.IsNull = false;
				if (std::get<3>(UserInfo))
				{
					ReturnValue.Read = true;
				}
				if (std::get<4>(UserInfo))
				{
					ReturnValue.Edit = true;
				}
				if (std::get<5>(UserInfo))
				{
					ReturnValue.Upload = true;
				}
			}
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::m_GetUsername(std::string const& RequestData)
	{
		std::string ReturnValue = "";
		std::vector<MrPostOGet::Cookie> Cookies = MrPostOGet::GetCookiesFromRequest(RequestData);
		for (size_t i = 0; i < Cookies.size(); i++)
		{
			if (Cookies[i].Name == "DBUsername")
			{
				ReturnValue = Cookies[i].Value;
				break;
			}
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::sp_GetPassword(std::string const& RequestData)
	{
		std::string ReturnValue = "";
		std::vector<MrPostOGet::Cookie> Cookies = MrPostOGet::GetCookiesFromRequest(RequestData);
		for (size_t i = 0; i < Cookies.size(); i++)
		{
			if (Cookies[i].Name == "DBPassword")
			{
				ReturnValue = Cookies[i].Value;
				break;
			}
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetTimestamp()
	{
		std::string ReturnValue = "";

		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);
		// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
		// for more information about date/time format
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

		ReturnValue = MBUtility::ReplaceAll(std::string(buf), ":", "_");

		return(ReturnValue);
	}
	//yikes det var mycket att trassla ut...
	bool h_StringIsPath(std::string const& StringToCheck)
	{
		bool ReturnValue = true;
		bool ContainsNewline = StringToCheck.find('\n') != StringToCheck.npos;
		bool ContainsSpace = StringToCheck.find(' ') != StringToCheck.npos;
		bool ContainsSlash = StringToCheck.find('/') != StringToCheck.npos;
		if (ContainsNewline || ContainsSpace || !ContainsSlash)
		{
			ReturnValue = false;
		}
		return(ReturnValue);
	}

	bool MBDB_Website::p_StringIsPath(std::string const& StringToCheck)
	{
		return(h_StringIsPath(StringToCheck));
	}
	bool MBDB_Website::DBLogin_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBLogin")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		std::string ServerResources = AssociatedServer->GetResourcePath("mrboboget.se");
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::unordered_map<std::string, std::string> FileVariables = {};

		std::string RequestUsername = m_GetUsername(RequestData);
		std::string RequestPassword = sp_GetPassword(RequestData);
		FileVariables["LoginValue"] = RequestUsername;
		if (m_GetUser(RequestUsername, RequestPassword).size() != 0)
		{
			FileVariables["LoginValue"] = "Currently logged in as: " + FileVariables["LoginValue"];
		}
		ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(ServerResources + "DBLogin.html", ServerResources), FileVariables);
		return(ReturnValue);
	}

	//bool MBDB_Website::DBSite_Predicate(std::string const& RequestData)
	//{
	//	std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
	//	std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
	//	if (Directorys.size() >= 1)
	//	{
	//		if (Directorys[0] == "DBSite")
	//		{
	//			return(true);
	//		}
	//	}
	//	return(false);
	//}

	//MrPostOGet::HTTPDocument MBDB_Website::DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	//{
	//	std::string RequestType = MrPostOGet::GetRequestType(RequestData);
	//	MrPostOGet::HTTPDocument NewDocument;
	//	NewDocument.Type = MBMIME::MIMEType::HTML;
	//	NewDocument.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBSite.html", AssociatedServer->GetResourcePath("mrboboget.se"));
	//	std::unordered_map<std::string, std::string> MapKeys = {};
	//
	//	std::string QuerryString = "";
	//	std::string RequestURL = MrPostOGet::GetRequestResource(RequestData);
	//	size_t QuerryStringPosition = RequestURL.find("?SQLQuerry=");
	//	if (QuerryStringPosition != RequestURL.npos)
	//	{
	//		QuerryString = RequestURL.substr(QuerryStringPosition + 11);
	//	}
	//	if (QuerryString == "")
	//	{
	//		MapKeys = { {"SQLResult",""} };
	//	}
	//	else
	//	{
	//		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
	//		if (!ConnectionPermissions.Read)
	//		{
	//			MapKeys = { {"SQLResult","<p style=\"color:red; text-align:center\" class=\"center\">Error in SQL Querry: Require permissions to read</p>"} };
	//		}
	//		else
	//		{
	//			std::string SQLCommand = QuerryString;
	//			bool CommandSuccesfull = true;
	//			MBError SQLError(true);
	//			std::vector<MBDB::MBDB_RowData> SQLResult = {};
	//			{
	//				std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
	//				SQLResult = m_ReadonlyDatabase->GetAllRows(SQLCommand, &SQLError);
	//				if (!SQLError)
	//				{
	//					CommandSuccesfull = false;
	//				}
	//			}
	//			if (CommandSuccesfull)
	//			{
	//				std::string TotalRowData = "";
	//				for (size_t i = 0; i < SQLResult.size(); i++)
	//				{
	//					MrPostOGet::HTMLNode NewRow = MrPostOGet::HTMLNode::CreateElement("tr");
	//					for (size_t j = 0; j < SQLResult[0].GetNumberOfColumns(); j++)
	//					{
	//						MrPostOGet::HTMLNode NewColumn = MrPostOGet::HTMLNode::CreateElement("td");
	//						NewColumn["style"] = "height: 100%";
	//						std::string ElementData = SQLResult[i].ColumnToString(j);
	//						if (p_StringIsPath(ElementData))
	//						{
	//							ElementData = p_GetEmbeddedResource(ElementData, AssociatedServer->GetResourcePath("mrboboget.se"), ConnectionPermissions);
	//						}
	//						NewColumn.AppendChild(MrPostOGet::HTMLNode(ElementData));
	//						NewRow.AppendChild(std::move(NewColumn));
	//					}
	//					TotalRowData += NewRow.ToString();
	//				}
	//				MapKeys["SQLResult"] = std::move(TotalRowData);
	//			}
	//			else
	//			{
	//				MapKeys = { {"SQLResult","<p style=\"color:red; text-align:center\" class=\"center\">Error in SQL Querry: " + SQLError.ErrorMessage + "</p>"} };
	//			}
	//		}
	//	}
	//	NewDocument.DocumentData = MrPostOGet::ReplaceMPGVariables(NewDocument.DocumentData, MapKeys);
	//	return(NewDocument);
	//}

    bool MBDB_Website::UploadSite_Predicate(std::string const& RequestData)
    {
        bool ReturnValue = false;
        std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
        std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
        if (Directorys.size() >= 1)
        {
            if (Directorys[0] == "DBUploadFile.html")
            {
                return(true);
            }
        }
        return(false);
        return(ReturnValue);
    }
    MrPostOGet::HTTPDocument MBDB_Website::UploadSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
    {
        MrPostOGet::HTTPDocument ReturnValue;
		std::string ResourcePath = MrPostOGet::GetRequestResource(RequestData);
		std::string DBResourcesPath = GetResourceFolderPath();
        std::string HTMLResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
        DBPermissionsList Permissions = m_GetConnectionPermissions(RequestData);
        if(Permissions.AssociatedUser == "")
        {
            ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "InvalidPermissions.html", HTMLResourcePath);
        }
        else
        {
            ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "DBUploadFile.html", HTMLResourcePath);
        }
        return(ReturnValue);
    }
	bool MBDB_Website::UploadFile_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "UploadFile" && MrPostOGet::GetRequestType(RequestData) == "POST")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::UploadFile_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		//Content-Type: multipart/form-data; boundary=---------------------------226143532618736951363968904467
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		MrPostOGet::HTTPDocument NewDocument;
		NewDocument.Type = MBMIME::MIMEType::json;
		std::string Boundary = "";
		std::vector<std::string> ContentTypes = MrPostOGet::GetHeaderValues("Content-Type", RequestData);
		std::string FormType = "multipart/form-data";
		std::string BoundaryHeader = "; boundary=";
		if (!ConnectionPermissions.Upload)
		{
			NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permission to upload\"}";
			NewDocument.RequestStatus = MrPostOGet::HTTPRequestStatus::Conflict;
			AssociatedConnection->Close();
			return(NewDocument);
		}
		for (size_t i = 0; i < ContentTypes.size(); i++)
		{
			if (ContentTypes[i].substr(0, FormType.size()) == FormType)
			{
				Boundary = ContentTypes[i].substr(FormType.size() + BoundaryHeader.size(), ContentTypes[i].size() - FormType.size() - BoundaryHeader.size() - 1);
				break;
			}
		}
		if (Boundary == "")
		{
			assert(false);
		}
		size_t FirstBoundaryLocation = RequestData.find(Boundary);
		size_t FirstFormParameterLocation = RequestData.find(Boundary, FirstBoundaryLocation + Boundary.size()) + Boundary.size() + 2;
		size_t EndOfFirstParameters = RequestData.find("\r\n", FirstFormParameterLocation);
		std::string FieldParameters = RequestData.substr(FirstFormParameterLocation, EndOfFirstParameters - FirstFormParameterLocation);
		//hardcodat eftersom vi vet formtatet av formul�ret
		std::vector<std::string> FirstFieldValues = MBUtility::Split(FieldParameters, "; ");
		std::string FileNameHeader = "filename=\"";
		std::string FileName = GetResourceFolderPath() + FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
		int FilesWithSameName = 0;
		while (std::filesystem::exists(FileName))
		{
			NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"FileAlreadyExists\"}";
			NewDocument.RequestStatus = MrPostOGet::HTTPRequestStatus::Conflict;
			//TODO close makar inte mycket sense f�r svaret kommer inte skickas, borde ist�llet finnas sett extra options som �r "close after send"
			AssociatedConnection->Close();
			return(NewDocument);
		}
		int FileDataLocation = RequestData.find("\r\n\r\n", FirstFormParameterLocation) + 4;
		std::ofstream NewFile(FileName, std::ios::out | std::ios::binary);
		int TotalDataWritten = RequestData.size() - FileDataLocation;
		NewFile.write(&RequestData[FileDataLocation], RequestData.size() - FileDataLocation);
		clock_t WriteTimer = clock();
		std::string NewData;
		while (AssociatedConnection->DataIsAvailable() && AssociatedConnection->IsConnected() && AssociatedConnection->IsValid())
		{
			NewFile.seekp(0, std::ostream::end);
			NewData = AssociatedConnection->GetNextChunkData();
			TotalDataWritten += NewData.size();
			int BoundaryLocation = NewData.find(Boundary);
			if (BoundaryLocation != NewData.npos)
			{
				int TrailingCharacters = Boundary.size() + 2 + 2;
				NewData.resize(NewData.size() - TrailingCharacters);
			}
			NewFile.write(&NewData[0], NewData.size());
		}
		std::cout << "Total data written: " << TotalDataWritten << std::endl;
		std::cout << "Total time: " << (clock() - WriteTimer) / double(CLOCKS_PER_SEC) << std::endl;

		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"ok\"}";
		return(NewDocument);
	}

	bool MBDB_Website::DBGet_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DB")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		std::string DatabaseResourcePath = GetResourceFolderPath();
		std::string URLResource = MrPostOGet::GetRequestResource(RequestData);
		std::string DatabaseResourceToGet = URLResource.substr(URLResource.find_first_of("DB/") + 3);
		bool AbsoluteExists = std::filesystem::exists(DatabaseResourcePath + DatabaseResourceToGet);
		bool IndexExists = std::filesystem::exists(DatabaseResourcePath + DatabaseResourceToGet + "/index.html");
		if (!AbsoluteExists && !IndexExists)
		{
			MrPostOGet::HTTPDocument Invalid;
			Invalid.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
			Invalid.Type = MBMIME::MIMEType::HTML;
			Invalid.DocumentData = "File not found";
			return(Invalid);
		}
		if (std::filesystem::is_directory(DatabaseResourcePath + DatabaseResourceToGet) && IndexExists)
		{
			DatabaseResourceToGet += "/index.html";
		}
		std::string RangeData = MrPostOGet::GetHeaderValue("Range", RequestData);
		std::string IntervallsData = RangeData.substr(RangeData.find_first_of("=") + 1);
		MBUtility::ReplaceAll(&IntervallsData, "\r", "");
		MBUtility::ReplaceAll(&IntervallsData, "\n", "");
		std::vector<FiledataIntervall> ByteIntervalls = {};
		if (RangeData != "")
		{
			std::vector<std::string> Intervalls = MBUtility::Split(MBUtility::ReplaceAll(IntervallsData, " ", ""), ",");
			for (int i = 0; i < Intervalls.size(); i++)
			{
				FiledataIntervall NewIntervall = { uint64_t(-1),uint64_t(-1) };
				std::vector<std::string> IntervallNumbers = MBUtility::Split(Intervalls[i], "-");
				if (IntervallNumbers[0] != "")
				{
					NewIntervall.FirstByte = std::stoll(IntervallNumbers[0]);
				}
				if (IntervallNumbers[1] != "")
				{
					NewIntervall.LastByte = std::stoll(IntervallNumbers[1]);
				}
				ByteIntervalls.push_back(NewIntervall);
			}
		}
		MrPostOGet::HTTPDocument ReturnValue = AssociatedServer->GetResource(DatabaseResourcePath + DatabaseResourceToGet, ByteIntervalls);
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetEmbeddedVideo(std::string const& VideoPath, std::string const& WebsiteResourcePath)
	{
		std::string ReturnValue = "";
		std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
		std::unordered_map<std::string, bool> BrowserSupported = { {"mp4",true},{"webm",true},{"ogg", true} };
		if (BrowserSupported.find(FileExtension) != BrowserSupported.end())
		{
			std::unordered_map<std::string, std::string> VariableValues = {};
			VariableValues["ElementID"] = VideoPath;
			VariableValues["MediaType"] = "video";
			VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
			VariableValues["FileType"] = FileExtension;
			ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
		}
		else
		{
			ReturnValue = "<p>Native broswer streaming not Supported</p><br><a href=\"/DB/" + VideoPath + "\">/DB/" + VideoPath + "</a>";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetEmbeddedAudio(std::string const& VideoPath, std::string const& WebsiteResourcePath)
	{
		std::unordered_map<std::string, std::string> VariableValues = {};
		std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
		VariableValues["ElementID"] = VideoPath;
		VariableValues["MediaType"] = "audio";
		VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
		VariableValues["FileType"] = FileExtension;
		std::string ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetEmbeddedImage(std::string const& ImagePath)
	{
		std::string ReturnValue = "<image src=\"/DB/" + ImagePath + "\" style=\"max-width:100%\"></image>";
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetEmbeddedPDF(std::string const& ImagePath)
	{
		return("<iframe src=\"/DB/" + ImagePath + "\" style=\"width: 100%; height: 100%;max-height: 100%; max-width: 100%;\"></iframe>");
	}
	bool MBDB_Website::DBView_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBView")
			{
				return(true);
			}
		}
		return(false);
	}
	MBDB::MBDB_Object MBDB_Website::p_LoadMBDBObject(std::string const& ValidatedUser, std::string const& ObjectPath, MBError* OutError)
	{

		MBDB::MBDB_Object ReturnValue;
		//std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
		//MBError EvaluationResult = ReturnValue.LoadObject(ObjectPath, ValidatedUser, m_ReadonlyDatabase);
		PluginReference<MBSite_DBPlugin> DBPlugin = GetPluginReference<MBSite_DBPlugin>(0);

		MBError EvaluationResult = DBPlugin->LoadMBDBObject(0,MBSiteUser(),FileLocationType::DBFile,ObjectPath,ReturnValue);
		if (OutError != nullptr)
		{
			*OutError = std::move(EvaluationResult);
		}
		return(ReturnValue);
	}

	std::string MBDB_Website::p_GetEmbeddedMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		MBError EvaluationError(true);
		//MBDB::MBDB_Object ObjectToEmbedd = p_LoadMBDBObject(Permissions.AssociatedUser, GetResourceFolderPath() + MBDBResource, &EvaluationError);
		MBDB::MBDB_Object ObjectToEmbedd;
		std::string ObjectFilePath = GetResourceFolderPath() + MBDBResource;
		if (std::filesystem::exists(ObjectFilePath))
		{
			std::string ObjectData = MrPostOGet::LoadWholeFile(ObjectFilePath);
			ObjectToEmbedd = MBDB::MBDB_Object::ParseObject(ObjectData, 0, nullptr, &EvaluationError);
		}
		else
		{
			EvaluationError = false;
			EvaluationError.ErrorMessage = "Object does not exist";
		}
		MrPostOGet::HTMLNode NodeToEmbedd = MrPostOGet::HTMLNode::CreateElement("a");
		NodeToEmbedd["href"] = "/DBView/" + MBDBResource;

		if (EvaluationError)
		{
			if (ObjectToEmbedd.HasAttribute("Banner"))
			{
				MBDB::MBDB_Object& BannerObject = ObjectToEmbedd.GetAttribute("Banner");
				if (!BannerObject.IsEvaluated())
				{
					//std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
					//MBDB::MBDBO_EvaluationInfo EvaluationInfo;
					//EvaluationInfo.AssociatedDatabase = m_ReadonlyDatabase;
					//EvaluationInfo.EvaluatingUser = Permissions.AssociatedUser;
					//EvaluationInfo.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(GetResourceFolderPath() + MBDBResource).parent_path());
					//BannerObject.Evaluate(EvaluationInfo, &EvaluationError);
					//PluginReference<MBSite_DBPlugin> DBPlugin = GetPluginReference<MBSite_DBPlugin>();
					PluginReference<MBSite_DBPlugin> DBPlugin = GetPluginReference<MBSite_DBPlugin>(0);
					if (DBPlugin == nullptr)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Error embedding resource: failed to find required MBSite_DBPlugin plugin";
					}
					else
					{
						DBPlugin->EvaluateMBDBObject(0, MBSiteUser(), FileLocationType::DBFile, MBDBResource, BannerObject);
					}
				}
				//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">" + p_GetEmbeddedResource(ObjectToEmbedd.GetAttribute("Banner"),HTMLFolder,Permissions) + "</a>"
				if (EvaluationError)
				{
					if (BannerObject.GetType() != MBDB::MBDBO_Type::String)
					{
						NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Invalid banner type"));
					}
					else
					{
						NodeToEmbedd.AppendChild(p_GetEmbeddedImage(BannerObject.GetStringData()));
					}
				}
				else
				{
					NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Error embedding resource: " + EvaluationError.ErrorMessage));
				}
			}
			else
			{
				//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">" + MBDBResource + "</a>";
				NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode(MBDBResource));
			}
		}
		else
		{
			//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">Error embedding resource: " + EvaluationError.ErrorMessage + "</a>";
			NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Error embedding resource: " + EvaluationError.ErrorMessage));
		}
		ReturnValue = NodeToEmbedd.ToString();
		return(ReturnValue);
	}
	bool MBDB_Website::p_ObjectIsMBPlaylist(MBDB::MBDB_Object const& ObjectToCheck)
	{
		bool ReturnValue = true;
		if (ObjectToCheck.HasAttribute("Banner") == false)
		{
			ReturnValue = false;
		}
		else
		{
			if (ObjectToCheck.GetAttribute("Banner").GetType() != MBDB::MBDBO_Type::String)
			{
				ReturnValue = false;
			}
		}
		if (ObjectToCheck.HasAttribute("Songs") == false)
		{
			ReturnValue = false;
		}
		else
		{
			if (ObjectToCheck.GetAttribute("Songs").GetType() != MBDB::MBDBO_Type::Array)
			{
				ReturnValue = false;
			}
		}
		if (ObjectToCheck.HasAttribute("Name") == false)
		{
			ReturnValue = false;
		}
		else
		{
			if (ObjectToCheck.GetAttribute("Name").GetType() != MBDB::MBDBO_Type::String)
			{
				ReturnValue = false;
			}
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_ViewMBDBPlaylist(MBDB::MBDB_Object const& EvaluatedObject, std::string const& HTMLFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		//Attributer som den har, Name,Banner,Songs,Tags kanske?
		//MrPostOGet::HTMLNode NodeToAdd = MrPostOGet::HTMLNode::CreateElement("div");

		bool PlaylistIsValid = p_ObjectIsMBPlaylist(EvaluatedObject);

		std::cout << EvaluatedObject.ToJason() << std::endl;

		if (PlaylistIsValid)
		{
			MrPostOGet::HTMLNode NameNode = MrPostOGet::HTMLNode::CreateElement("h2");
			NameNode["style"] = "text-align: center";
			NameNode["class"] = "center";
			NameNode.AppendChild(MrPostOGet::HTMLNode(EvaluatedObject.GetAttribute("Name").GetStringData()));

			MrPostOGet::HTMLNode TableNode = MrPostOGet::HTMLNode::CreateElement("table");
			TableNode["style"] = "width: 100%";
			auto const& PlaylistSongs = EvaluatedObject.GetAttribute("Songs").GetArrayData();
			for (auto const& Song : PlaylistSongs)
			{
				MrPostOGet::HTMLNode NewRow = MrPostOGet::HTMLNode::CreateElement("tr");
				//en song best�r av en path till den, samt namn
				auto const& SongColumns = Song.GetArrayData();
				MrPostOGet::HTMLNode SongColumn = MrPostOGet::HTMLNode::CreateElement("td");
				SongColumn["style"] = "height: 100%";
				SongColumn.AppendChild(MrPostOGet::HTMLNode(p_GetEmbeddedResource(SongColumns[0].GetStringData(), HTMLFolder, Permissions)));
				MrPostOGet::HTMLNode NameColumn = MrPostOGet::HTMLNode::CreateElement("td");
				NameColumn["style"] = "height: 100%";
				NameColumn.AppendChild(MrPostOGet::HTMLNode(SongColumns[1].GetStringData()));
				NewRow.AppendChild(std::move(SongColumn));
				NewRow.AppendChild(std::move(NameColumn));
				TableNode.AppendChild(std::move(NewRow));
			}
			ReturnValue += MrPostOGet::HTMLNode(p_GetEmbeddedImage(EvaluatedObject.GetAttribute("Banner").GetStringData())).ToString();
			ReturnValue += NameNode.ToString();
			ReturnValue += TableNode.ToString();
			MrPostOGet::HTMLNode PlaylistElement = MrPostOGet::HTMLNode::CreateElement("script");
			ReturnValue += "<script src=\"/DBLibrary.js\"></script>";
			PlaylistElement["src"] = "/DBCreatePlaylist.js";
			ReturnValue += PlaylistElement.ToString();
		}
		else
		{
			ReturnValue = "<p>Playlist is formatted incorrectly</p>";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_ViewMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		MBError EvaluationError(true);
		MBDB::MBDB_Object ObjectToView = p_LoadMBDBObject(Permissions.AssociatedUser, GetResourceFolderPath() + MBDBResource, &EvaluationError);
		//MrPostOGet::HTMLNode NodeToEmbedd = MrPostOGet::HTMLNode::CreateElement("div");
		if (EvaluationError)
		{
			bool ShouldDisplayText = false;
			if (ObjectToView.HasAttribute("Type"))
			{
				if (ObjectToView.GetAttribute("Type").GetType() == MBDB::MBDBO_Type::String)
				{
					std::string ObjectType = ObjectToView.GetAttribute("Type").GetStringData();
					if (ObjectType == "MBPlaylist")
					{
						ReturnValue = p_ViewMBDBPlaylist(ObjectToView, HTMLFolder, Permissions);
					}
					else
					{
						ShouldDisplayText = true;
					}
				}
				else
				{
					ShouldDisplayText = true;
				}
			}
			else
			{
				ShouldDisplayText = true;
			}
			if (ShouldDisplayText)
			{
				MrPostOGet::HTMLNode TextDisplay = MrPostOGet::HTMLNode::CreateElement("p");


				TextDisplay["class"] = "MBTextArea";
				TextDisplay.AppendChild(MrPostOGet::HTMLNode(ObjectToView.ToFormattedJason()));
				ReturnValue += "<h2>" + MBDBResource + "</h2>";
				ReturnValue += TextDisplay.ToString();
			}
		}
		else
		{
			MrPostOGet::HTMLNode NodeToEmbedd = MrPostOGet::HTMLNode::CreateElement("p");
			NodeToEmbedd["style"] = "color: red; text-align: center";
			NodeToEmbedd["class"] = "center";
			NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Error in evaluating object: " + EvaluationError.ErrorMessage));
			ReturnValue = NodeToEmbedd.ToString();
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_DBEdit_GetTextfileEditor(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";

		std::string FileToEditPath = GetResourceFolderPath() + MBDBResource;
		if (!std::filesystem::exists(FileToEditPath))
		{
			ReturnValue = "<p style=\"color: red\">Error in editing file: file doesnt exist</p>";
		}
		else
		{
			//<form id="BlippUpdate" method="post" enctype ="multipart/form-data" class="center">
			MrPostOGet::HTMLNode FormNode = MrPostOGet::HTMLNode::CreateElement("form");
			FormNode["id"] = "EditForm";
			FormNode["name"] = "EditForm";
			FormNode["method"] = "post";
			FormNode["enctype"] = "multipart/form-data";
			MrPostOGet::HTMLNode SubmitButton = MrPostOGet::HTMLNode::CreateElement("input");
			SubmitButton["type"] = "submit";
			SubmitButton["class"] = "MBButton";
			SubmitButton["value"] = "Send Update";
			SubmitButton["form"] = "EditForm";

			//FormNode.AppendChild(std::move(SubmitButton));

			MrPostOGet::HTMLNode FileName = MrPostOGet::HTMLNode::CreateElement("h2");
			FileName.AppendChild(MrPostOGet::HTMLNode(MBDBResource));

			MrPostOGet::HTMLNode TextArea = MrPostOGet::HTMLNode::CreateElement("textarea");
			TextArea["style"] = "overflow-y: scroll; resize: none; width: 100%; height: 100%";
			TextArea["form"] = "EditForm";
			TextArea["class"] = "MBTextArea";
			TextArea.AppendChild(MrPostOGet::HTMLNode(MrPostOGet::LoadWholeFile(FileToEditPath)));
			TextArea["name"] = "InputElement";

			//FormNode.AppendChild(std::move(TextArea));
			//FormNode.AppendChild(std::move(SubmitButton));

			ReturnValue += FileName.ToString();
			ReturnValue += FormNode.ToString();
			ReturnValue += TextArea.ToString();
			ReturnValue += SubmitButton.ToString();
		}


		return(ReturnValue);
	}
	MBError MBDB_Website::p_DBEdit_Textfile_Update(MrPostOGet::HTTPClientRequest const& Request, std::string const& MBDBResource, DBPermissionsList const& Permissions)
	{
		MBError UpdateError(true);

		//Post request med enbart 1 v�rde, den nya texten
		//ANTAGANDE all data som skickas �r h�r, blir fel annars
		//std::string DataToParse = reqe

		MBMIME::MIMEMultipartDocumentExtractor DocumentExtractor(Request.RawRequestData.data(), Request.RawRequestData.size(), 0);
		DocumentExtractor.ExtractHeaders();
		DocumentExtractor.ExtractHeaders();
		std::string NewData = DocumentExtractor.ExtractPartData();

		std::ofstream FileToUpdate = std::ofstream(GetResourceFolderPath() + MBDBResource, std::ios::out | std::ios::binary);
		if (!FileToUpdate.is_open() || !FileToUpdate.good())
		{
			UpdateError = false;
			UpdateError.ErrorMessage = "Error updating file";
		}
		else
		{
			FileToUpdate << NewData;
		}

		return(UpdateError);
	}

	std::string MBDB_Website::p_ViewResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
		MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
		if (ResourceExtension != "mbdbo")
		{
			ReturnValue = p_GetEmbeddedResource(MBDBResource, ResourceFolder, Permissions);
		}
		else
		{
			ReturnValue = p_ViewMBDBObject(MBDBResource, ResourceFolder, Permissions);
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_EditResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
		MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
		if (ResourceExtension == "mbdb")
		{
			ReturnValue = p_DBEdit_GetTextfileEditor(MBDBResource, ResourceFolder, Permissions);
		}
		else
		{
			ReturnValue = "<p class=\"center\" style=\"text-align: center\">Resource is not Editable</p>";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::p_GetEmbeddedResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions)
	{
		std::string ReturnValue = "";
		std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
		MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
		bool IsMBDBResource = true;
		if (IsMBDBResource)
		{
			if (!std::filesystem::exists(GetResourceFolderPath() + MBDBResource))
			{
				return("<p>File does not exist<p>");
			}
		}
		if (ResourceMedia == MBMIME::MediaType::Image)
		{
			ReturnValue = p_GetEmbeddedImage(MBDBResource);
		}
		else if (ResourceMedia == MBMIME::MediaType::Video)
		{
			ReturnValue = p_GetEmbeddedVideo(MBDBResource, ResourceFolder);
		}
		else if (ResourceMedia == MBMIME::MediaType::Audio)
		{
			ReturnValue = p_GetEmbeddedAudio(MBDBResource, ResourceFolder);
		}
		else if (ResourceMedia == MBMIME::MediaType::PDF)
		{
			ReturnValue = p_GetEmbeddedPDF(MBDBResource);
		}
		else if (ResourceExtension == "mbdbo")
		{
			ReturnValue = p_GetEmbeddedMBDBObject(MBDBResource, ResourceFolder, Permissions);
		}
		else
		{
			ReturnValue = "<p>File in unrecognized format</p><br><a href=\"/DB/" + MBDBResource + "\">/DB/" + MBDBResource + "</a>";
		}
		return(ReturnValue);
	}
	bool MBDB_Website::p_Edit_Predicate(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer)
	{
		//TODO borde det h�r helt enkelt vara en del av parsingen ist�llet?
		std::string NormalizedPath = RequestToHandle.RequestResource;
		if (NormalizedPath.substr(0, 8) == "/DBEdit/" && NormalizedPath.size() > 8) //vi vill inte deala med att scrolal till dem nu
		{
			return(true);
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::p_Edit_ResponseGenerator(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer* Server)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::string FileExtension = "";
		size_t LastDot = Request.RequestResource.find_last_of('.');

		std::string MBDBResource = Request.RequestResource.substr(Request.RequestResource.find("DBEdit/") + 7);
		std::string FileToEditPath = GetResourceFolderPath() + MBDBResource;
		std::string HTMLFolderPath = Server->GetResourcePath("mrboboget.se");
		std::string TemplateData = MrPostOGet::LoadFileWithPreprocessing(HTMLFolderPath + "DBEditTemplate.html", HTMLFolderPath);
		std::string EmbeddedElement = "";
		std::unordered_map<std::string, std::string> MapKeys = {};
		MapKeys["UpdateResponse"] = "";
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(Request.RawRequestData);

		if (ConnectionPermissions.Edit && ConnectionPermissions.Read)
		{
			if (Request.Type == MrPostOGet::HTTPRequestType::POST)
			{
				MBError Result = p_DBEdit_Textfile_Update(Request, MBDBResource, ConnectionPermissions);
				if (Result)
				{
					MapKeys["UpdateResponse"] = "<p style=\"color: green\">Successfully updated file</p>";
				}
				else
				{
					MapKeys["UpdateResponse"] = "<p style=\"color: red\">Error updating file" + Result.ErrorMessage + "</p>";
				}
			}
			if (LastDot != Request.RequestResource.npos)
			{
				FileExtension = Request.RequestResource.substr(LastDot + 1);
			}
			if (FileExtension == "txt" || "mbdbo" || MBGetFileSize(FileToEditPath) > 10000)
			{
				EmbeddedElement = p_DBEdit_GetTextfileEditor(MBDBResource, HTMLFolderPath, ConnectionPermissions);
			}
			else
			{
				EmbeddedElement = "<p style=\"color: red\">Editing of filetype not supported</p>";
			}
		}
		else
		{
			EmbeddedElement = "<p style=\"color: red\">Require Permissions to edit and read</p>";
		}
		MapKeys["EditElement"] = std::move(EmbeddedElement);
		ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(TemplateData, MapKeys);
		return(ReturnValue);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::string EmbeddedElement = "";

		std::string HandlerName = "DBView";
		std::string ResourcePath = MrPostOGet::GetRequestResource(RequestData);
		std::string DBResourcesPath = GetResourceFolderPath();
		std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
		std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
        std::string HTMLResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
        DBPermissionsList Permissions = m_GetConnectionPermissions(RequestData);
        if(Permissions.AssociatedUser == "")
        {
            ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "InvalidPermissions.html", HTMLResourcePath);
            return(ReturnValue);
        }
		if (!std::filesystem::exists(DBResourcesPath + DBResource))
		{
			MrPostOGet::HTTPDocument Invalid;
			Invalid.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
			Invalid.Type = MBMIME::MIMEType::HTML;
			Invalid.DocumentData = "File not found";
			return(Invalid);
		}
		if (!std::filesystem::is_directory(DBResourcesPath + DBResource) && DBResource != "")
		{
			DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
			MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
			//EmbeddedElement = p_GetEmbeddedResource(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
			EmbeddedElement = p_ViewResource(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"), ConnectionPermissions);
			std::unordered_map<std::string, std::string> MapData = {};
			MapData["EmbeddedMedia"] = EmbeddedElement;
			ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "DBViewTemplate.html", HTMLResourcePath), MapData);
		}
		else
		{
			ReturnValue.Type = MBMIME::MIMEType::HTML;
			ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se") + "DBViewFolder.html", AssociatedServer->GetResourcePath("mrboboget.se"));
		}
		//= AssociatedServer->GetResource(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBViewTemplate.html");
		return(ReturnValue);
	}

	bool MBDB_Website::DBViewEmbedd_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBViewEmbedd")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		MrPostOGet::HTTPDocument ReturnValue = MrPostOGet::HTTPDocument();
		std::string HandlerName = "DBViewEmbedd/";
		std::string ResourcePath = MrPostOGet::GetRequestResource(RequestData);
		std::string DBResourcesPath = GetResourceFolderPath();
		std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
		std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
		MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
		ReturnValue.Type = MBMIME::DocumentTypeFromFileExtension(ResourceExtension);
		if (ResourceMedia == MBMIME::MediaType::Image)
		{
			ReturnValue.DocumentData = p_GetEmbeddedImage(DBResource);
		}
		else if (ResourceMedia == MBMIME::MediaType::Video)
		{
			ReturnValue.DocumentData = p_GetEmbeddedVideo(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		}
		else if (ResourceMedia == MBMIME::MediaType::Audio)
		{
			ReturnValue.DocumentData = p_GetEmbeddedAudio(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		}
		return(ReturnValue);
	}

	bool MBDB_Website::DBAdd_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBAdd")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		//std::string RequestResource = MBSockets::GetReqestResource(RequestData);
		//std::string TableName = RequestData.substr(RequestResource.find("DBAdd/") + 6);
		//std::vector < std::string> ExistingTableNames = {};
		//l�ter all denna kod k�ras i javascript, blir det enklaste
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
        auto Perms = m_GetConnectionPermissions(RequestData);
        if(Perms.AssociatedUser == "")
        {
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "InvalidPermissions.html", ResourcePath);
        }
        else
        {
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "DBAdd.html", ResourcePath);
        }
		return(ReturnValue);
	}

	bool MBDB_Website::DBGeneralAPI_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBGeneralAPI")
			{
				return(true);
			}
		}
		return(false);
	}

	std::string MBDB_Website::DBGeneralAPIGetDirective(std::string const& RequestBody)
	{
		size_t FirstSpace = RequestBody.find(" ");
		if (FirstSpace == RequestBody.npos)
		{
			FirstSpace = RequestBody.size();
		}
		std::string APIDirective = RequestBody.substr(0, FirstSpace);
		return(APIDirective);
	}
	std::vector<std::string> MBDB_Website::DBGeneralAPIGetArguments(std::string const& RequestBody, MBError* OutError)
	{
		size_t FirstSpace = RequestBody.find(" ");
		std::vector<std::string> ReturnValue = {};
		size_t ParsePosition = FirstSpace + 1;
		while (ParsePosition != std::string::npos && ParsePosition < RequestBody.size())
		{
			size_t SizeEnd = RequestBody.find(" ", ParsePosition);
			size_t ArgumentSize = 0;
			if (SizeEnd == std::string::npos)
			{
				break;
			}
			try
			{
				ArgumentSize = std::stoi(RequestBody.substr(ParsePosition, SizeEnd - ParsePosition));
			}
			catch (const std::exception&)
			{
				*OutError = MBError(false);
				OutError->ErrorMessage = "Failed to parse argument size";
				break;
			}
			ReturnValue.push_back(RequestBody.substr(SizeEnd + 1, ArgumentSize));
			ParsePosition = SizeEnd + 1 + ArgumentSize + 1;
		}
		//std::vector<std::string> ReturnValue = Split(RequestBody.substr(FirstSpace + 1),",");
		return(ReturnValue);
	}


	//std::string MBDB_Website::GetTableNamesBody(std::vector<std::string> const& Arguments)
	//{
	//	std::vector<std::string> TableNames = {};
	//	{
	//		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
	//		TableNames = m_ReadonlyDatabase->GetAllTableNames();
	//	}
	//	std::string JSONTableNames = "\"TableNames\":" + MakeJasonArray(TableNames);
	//	std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\"," + JSONTableNames + "}";
	//	return(JsonResponse);
	//}
	//std::string MBDB_Website::GetTableInfoBody(std::vector<std::string> const& Arguments)
	//{
	//	//f�rsta argumentet �r tablen vi vill ha
	//	if (Arguments.size() == 0)
	//	{
	//		return("");
	//	}
	//	std::vector<MBDB::ColumnInfo> TableInfo = {};
	//	{
	//		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
	//		TableInfo = m_ReadonlyDatabase->GetColumnInfo(Arguments[0]);
	//	}
	//	std::string JSONResponse = "{\"MBAPI_Status\":\"ok\",\"TableInfo\":" + MakeJasonArray(TableInfo) + "}";
	//	return(JSONResponse);
	//}
	inline long long StringToInt(std::string const& IntData, MBError* OutError = nullptr)
	{
		long long ReturnValue = 0;
		try
		{
			ReturnValue = std::stoll(IntData);
		}
		catch (const std::exception&)
		{
			*OutError = MBError(false);
			OutError->ErrorMessage = "Failed to parse int";
		}
		return(ReturnValue);
	}
	//std::vector<MBDB::MBDB_RowData> MBDB_Website::EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
	//	std::vector<int> ColumnIndex, std::string TableName, MBError* OutError)
	//{
	//	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	//	std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
	//	MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);
	//
	//	std::vector<MBDB::ColumnInfo> TableColumnInfo = WritableDatabase->GetColumnInfo(TableName);
	//	for (size_t i = 0; i < ColumnValues.size(); i++)
	//	{
	//		if (TableColumnInfo[ColumnIndex[i]].ColumnType == MBDB::ColumnSQLType::Int)
	//		{
	//			MBDB::MaxInt NewInt = StringToInt(ColumnValues[i], OutError);
	//			if (!OutError)
	//			{
	//				break;
	//			}
	//			*OutError = NewStatement->BindInt(NewInt, i + 1);
	//		}
	//		else
	//		{
	//			*OutError = NewStatement->BindString(ColumnValues[i], i + 1);
	//			if (!OutError)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//	if (OutError)
	//	{
	//		QuerryResult = WritableDatabase->GetAllRows(NewStatement, OutError);
	//	}
	//	NewStatement->FreeData();
	//	WritableDatabase->FreeSQLStatement(NewStatement);
	//	return(QuerryResult);
	//}
	//std::string MBDB_Website::DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments)
	//{
	//	std::string ReturnValue = "";
	//	std::string SQLCommand = "INSERT INTO " + Arguments[0] + "("; //VALUES (";
	//	std::vector<std::string> ColumnNames = {};
	//	std::vector<std::string> ColumnValues = {};
	//	std::vector<int> ColumnIndex = {};
	//	MBError DataBaseError(true);
	//
	//
	//	for (size_t i = 1; i < Arguments.size(); i++)
	//	{
	//		size_t FirstColon = Arguments[i].find_first_of(":");
	//		size_t SecondColon = Arguments[i].find(":", FirstColon + 1);
	//		size_t NewColumnIndex = -1;
	//		NewColumnIndex = StringToInt(Arguments[i].substr(FirstColon + 1, SecondColon - FirstColon), &DataBaseError);
	//		if (!DataBaseError)
	//		{
	//			ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	//			return(ReturnValue);
	//		}
	//		ColumnNames.push_back(Arguments[i].substr(0, FirstColon));
	//		ColumnValues.push_back(Arguments[i].substr(SecondColon + 1));
	//		ColumnIndex.push_back(NewColumnIndex);
	//		SQLCommand += ColumnNames[i - 1];
	//		if (i + 1 < Arguments.size())
	//		{
	//			SQLCommand += ",";
	//		}
	//	}
	//	SQLCommand += ") VALUES(";
	//	for (size_t i = 1; i < Arguments.size(); i++)
	//	{
	//		SQLCommand += "?";
	//		if (i + 1 < Arguments.size())
	//		{
	//			SQLCommand += ",";
	//		}
	//	}
	//	SQLCommand += ");";
	//	std::vector<MBDB::MBDB_RowData> QuerryResult = EvaluateBoundSQLStatement(SQLCommand, ColumnValues, ColumnIndex, Arguments[0], &DataBaseError);
	//	if (DataBaseError)
	//	{
	//		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	//	}
	//	else
	//	{
	//		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	//	}
	//	return(ReturnValue);
	//}
	//std::string MBDB_Website::DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments)
	//{
	//	//UPDATE table_name
	//	//SET column1 = value1, column2 = value2, ...
	//	//	WHERE condition;
	//	std::string ReturnValue = "";
	//	std::string SQLCommand = "UPDATE " + Arguments[0] + " SET "; //VALUES (";
	//	std::vector<std::string> ColumnNames = {};
	//	std::vector<std::string> OldColumnValues = {};
	//	std::vector<std::string> NewColumnValues = {};
	//	MBError DataBaseError(true);
	//
	//
	//	for (size_t i = 1; i < Arguments.size(); i++)
	//	{
	//		if ((i % 3) == 1)
	//		{
	//			ColumnNames.push_back(Arguments[i]);
	//		}
	//		if ((i % 3) == 2)
	//		{
	//			OldColumnValues.push_back(Arguments[i]);
	//		}
	//		if ((i % 3) == 0)
	//		{
	//			NewColumnValues.push_back(Arguments[i]);
	//		}
	//	}
	//	for (size_t i = 0; i < OldColumnValues.size(); i++)
	//	{
	//		SQLCommand += ColumnNames[i] + "=?";
	//		if (i + 1 < OldColumnValues.size())
	//		{
	//			SQLCommand += ", ";
	//		}
	//	}
	//	SQLCommand += " WHERE ";
	//	for (size_t i = 0; i < ColumnNames.size(); i++)
	//	{
	//		SQLCommand += "(" + ColumnNames[i] + "=?";
	//		if (OldColumnValues[i] == "null")
	//		{
	//			SQLCommand += " OR " + ColumnNames[i] + " IS NULL";
	//		}
	//		SQLCommand += ")";
	//		if (i + 1 < ColumnNames.size())
	//		{
	//			SQLCommand += " AND ";
	//		}
	//	}
	//	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	//	{
	//		std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
	//		MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);
	//
	//		//DEBUG GREJER
	//		//MBDB::SQLStatement* DebugStatement = WritableDatabase->GetSQLStatement("SELECT * FROM Music WHERE RowNumber=82");
	//		//std::vector<MBDB::MBDB_RowData> DebugResult = WritableDatabase->GetAllRows(DebugStatement);
	//		//auto DebugTuple = DebugResult[0].GetTuple<int, std::string, std::string, std::string, std::string, std::string, std::string>();
	//		//std::ofstream DebugFile("./DebugDatabaseData.txt", std::ios::out|std::ios::binary);
	//		//std::ofstream DebugInput("./DebugInputData.txt", std::ios::out | std::ios::binary);
	//		//DebugFile << std::get<6>(DebugTuple);
	//		//DebugInput << Arguments[21];
	//
	//		std::vector<MBDB::ColumnInfo> ColumnInfo = WritableDatabase->GetColumnInfo(Arguments[0]);
	//		std::vector<MBDB::ColumnSQLType> ColumnTypes = {};
	//		for (size_t i = 0; i < ColumnInfo.size(); i++)
	//		{
	//			ColumnTypes.push_back(ColumnInfo[i].ColumnType);
	//		}
	//		NewStatement->BindValues(NewColumnValues, ColumnTypes, 0);
	//		NewStatement->BindValues(OldColumnValues, ColumnTypes, NewColumnValues.size());
	//		QuerryResult = WritableDatabase->GetAllRows(NewStatement, &DataBaseError);
	//		NewStatement->FreeData();
	//		WritableDatabase->FreeSQLStatement(NewStatement);
	//	}
	//	if (DataBaseError)
	//	{
	//		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	//	}
	//	else
	//	{
	//		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	//	}
	//	return(ReturnValue);
	//}
	//DBAPI_GetFolderContents
	enum class MBDirectoryEntryType
	{
		Directory,
		RegularFile,
		Null
	};
	struct MBDirectoryEntry
	{
		std::filesystem::path Path = "";
		MBDirectoryEntryType Type = MBDirectoryEntryType::Null;
	};
	std::vector<MBDirectoryEntry> GetDirectoryEntries(std::string const& DirectoryPath, MBError* OutError = nullptr)
	{
		std::vector<MBDirectoryEntry> ReturnValue = {};
		if (!std::filesystem::exists(DirectoryPath))
		{
			if (OutError != nullptr)
			{
				*OutError = false;
				OutError->ErrorMessage = "Directory does not exists";
			}
			return(ReturnValue);
		}
		std::filesystem::directory_iterator DirectoryIterator(DirectoryPath);
		while (DirectoryIterator != std::filesystem::end(DirectoryIterator))
		{
			MBDirectoryEntry NewDirectoryEntry;
			NewDirectoryEntry.Path = DirectoryIterator->path();
			if (DirectoryIterator->is_directory())
			{
				NewDirectoryEntry.Type = MBDirectoryEntryType::Directory;
			}
			else if (DirectoryIterator->is_regular_file())
			{
				NewDirectoryEntry.Type = MBDirectoryEntryType::RegularFile;
			}
			DirectoryIterator++;
			ReturnValue.push_back(NewDirectoryEntry);
		}
		if (OutError != nullptr)
		{
			*OutError = true;
		}
		return(ReturnValue);
	}
	std::string MakePathRelative(std::filesystem::path const& PathToProcess, std::string DirectoryToMakeRelative)
	{
		bool RelativeFolderTraversed = false;
		std::filesystem::path::iterator PathIterator = PathToProcess.begin();
		std::string ReturnValue = "";
		while (PathIterator != PathToProcess.end())
		{
			if (RelativeFolderTraversed)
			{
				ReturnValue += "/" + PathIterator->generic_string();
			}
			if (PathIterator->generic_string() == DirectoryToMakeRelative)
			{
				RelativeFolderTraversed = true;
			}
			PathIterator++;
		}
		return(ReturnValue);
	}
	std::string MBDirectoryEntryTypeToString(MBDirectoryEntryType TypeToConvert)
	{
		if (TypeToConvert == MBDirectoryEntryType::Directory)
		{
			return("Directory");
		}
		else if (TypeToConvert == MBDirectoryEntryType::RegularFile)
		{
			return("RegularFile");
		}
	}
	std::string ToJason(MBDirectoryEntry const& EntryToEncode)
	{
		std::filesystem::exists(EntryToEncode.Path.generic_string());
		std::string ReturnValue = "{\"Path\":" + ToJason(EntryToEncode.Path.generic_string()) + ",";
		ReturnValue += "\"Type\":" + ToJason(MBDirectoryEntryTypeToString(EntryToEncode.Type)) + "}";
		return(ReturnValue);
	}
	//First argument is relative path in MBDBResources folder
	std::string MBDB_Website::DBAPI_GetFolderContents(std::vector<std::string> const& Arguments)
	{
		std::string ReturnValue = "";
		MBError ErrorResult(true);
		std::vector<MBDirectoryEntry> DirectoryEntries = GetDirectoryEntries(GetResourceFolderPath() + Arguments[0], &ErrorResult);
		if (!ErrorResult)
		{
			return("{\"MBDBAPI_Status\":\"" + ErrorResult.ErrorMessage + "\"}");
		}
		for (size_t i = 0; i < DirectoryEntries.size(); i++)
		{
			DirectoryEntries[i].Path = MakePathRelative(DirectoryEntries[i].Path, "MBDBResources");
		}
		std::string ErrorPart = "\"MBDBAPI_Status\":\"ok\"";
		std::string DirectoryPart = "\"DirectoryEntries\":" + MakeJasonArray<MBDirectoryEntry>(DirectoryEntries);
		ReturnValue = "{" + ErrorPart + "," + DirectoryPart + "}";
		return(ReturnValue);
	}
	//std::string MBDB_Website::DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments)
	//{
	//	//ett arguments som �r WhereStringen, ghetto aff egetntligen men men,m�ste vara p� en immutable table s� vi inte fuckar grejer
	//	std::string ReturnValue = "";
	//	if (Arguments.size() < 2)
	//	{
	//		return("{\"MBDBAPI_Status\":\"Invalid number of arguments\"}");
	//	}
	//	std::vector<MBDB::MBDB_RowData> RowResponse = {};
	//	MBError DatabaseError(true);
	//	{
	//		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
	//		std::string SQlQuerry = "SELECT * FROM " + Arguments[0] + " " + Arguments[1];
	//		RowResponse = WritableDatabase->GetAllRows(SQlQuerry, &DatabaseError);
	//	}
	//	if (!DatabaseError)
	//	{
	//		return("{\"MBDBAPI_Status\":" + ToJason(DatabaseError.ErrorMessage) + "}");
	//	}
	//	ReturnValue = "{\"MBDBAPI_Status\":\"ok\",\"Rows\":" + MakeJasonArray(RowResponse) + "}";
	//	return(ReturnValue);
	//}
	std::string MBDB_Website::DBAPI_Login(std::vector<std::string> const& Arguments)
	{
		if (Arguments.size() != 2)
		{
			return("{\"MBDBAPI_Status\":\"Invalid Function call\"}");
		}
		std::vector<MBDB::MBDB_RowData> UserResult = m_GetUser(Arguments[0], Arguments[1]);
		if (UserResult.size() == 0)
		{
			return("{\"MBDBAPI_Status\":\"Invalid UserName or Password\"}");
		}
		return("{\"MBDBAPI_Status\":\"ok\"}");
	}
	std::string MBDB_Website::DBAPI_GetAvailableIndexes(std::vector<std::string> const& Arguments)
	{
		std::string ReturnValue = "{\"MBDBAPI_Status\":\"ok\",";
		std::vector<std::string> AvailableIndexes = {};
		std::filesystem::directory_iterator DirectoryIterator(GetResourceFolderPath() + "Indexes");
		for (auto& DirectoryEntry : DirectoryIterator)
		{
			AvailableIndexes.push_back(DirectoryEntry.path().filename().generic_string());
		}
		ReturnValue += ToJason(std::string("AvailableIndexes")) + ":" + MakeJasonArray(AvailableIndexes) + "}";
		return(ReturnValue);
	}
	bool MBDB_Website::DB_IndexExists(std::string const& IndexToCheck)
	{
		return(std::filesystem::exists(GetResourceFolderPath() + "/Indexes/" + IndexToCheck));
	}
	std::string MBDB_Website::DBAPI_GetIndexSearchResult(std::vector<std::string> const& Arguments)
	{
		std::string ReturnValue = "{\"MBDBAPI_Status\":";
		if (Arguments.size() < 3)
		{
			ReturnValue += ToJason("Invalid function call") + "}";
		}
		if (!DB_IndexExists(Arguments[0]))
		{
			ReturnValue += ToJason("Index doesn't exists") + "}";
		}
		else
		{
			ReturnValue += "\"ok\",";
			std::lock_guard<std::mutex> Lock(__DBIndexMapMutex);
			MBSearchEngine::MBIndex& IndexToSearch = (*__DBIndexMap)[Arguments[0]];
			std::vector<std::string> Result = {};
			if (Arguments[1] == "Boolean")
			{
				Result = IndexToSearch.EvaluteBooleanQuerry(Arguments[2]);
			}
			else
			{
				Result = IndexToSearch.EvaluteVectorModelQuerry(Arguments[2]);
			}
			ReturnValue += ToJason(std::string("IndexSearchResult")) + ":" + MakeJasonArray(Result);
			ReturnValue += "}";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::DBAPI_GetBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions)
	{
		std::string ReturnValue = "";
		std::string MBDBResources = GetResourceFolderPath();
		std::lock_guard<std::mutex> Lock(m_BlippFileMutex);
		std::string BlippArchives = "/operationblipp/archives/";
		if (Arguments.size() == 1 && Arguments[0] == "Dev")
		{
			BlippArchives = "/operationblipp/Dev/archives/";
		}
		std::string LatestUserDownload = MrPostOGet::LoadWholeFile(MBDBResources + BlippArchives + "LatestAccess");
		if (UserPermissions.AssociatedUser != "")
		{
			if (LatestUserDownload == "")
			{
				std::ofstream LatestAccess = std::ofstream(MBDBResources + BlippArchives + "LatestAccess", std::ios::out | std::ios::binary);
				LatestAccess << UserPermissions.AssociatedUser;
				LatestAccess.flush();
				LatestAccess.close();
				ReturnValue = MrPostOGet::LoadWholeFile(MBDBResources + BlippArchives + "latest");
				std::ofstream AccessLog = std::ofstream(MBDBResources + BlippArchives + "AccessLog", std::ios::out|std::ios::app);
				AccessLog << "Dowload " + p_GetTimestamp() + " (" + UserPermissions.AssociatedUser + ")"<<std::endl;
			}
			else
			{
				ReturnValue = "{\"MBDBAPI_Status\":\"CardLocked\"}";
			}
		}
		else
		{
			ReturnValue = "{\"MBDBAPI_Status\":\"LoginRequired\"}";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::DBAPI_UploadBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions)
	{
		std::string ReturnValue = "";
		std::string MBDBResources = GetResourceFolderPath();
		std::lock_guard<std::mutex> Lock(m_BlippFileMutex);
		std::string BlippArchives = "/operationblipp/archives/";
		if (Arguments.size() > 1 && Arguments[1] == "Dev")
		{
			BlippArchives = "/operationblipp/Dev/archives/";
		}
		std::string LatestUserDownload = MrPostOGet::LoadWholeFile(MBDBResources + BlippArchives + "LatestAccess");
		if (UserPermissions.AssociatedUser != "")
		{
			std::string const& FileData = Arguments[0];
			std::string Timestamp = p_GetTimestamp();
			if (LatestUserDownload == UserPermissions.AssociatedUser)
			{
				std::ofstream LatestFile = std::ofstream(MBDBResources + BlippArchives + "latest", std::ios::out | std::ios::binary);
				std::ofstream LatestDateFile = std::ofstream(MBDBResources + BlippArchives + "LatestDate", std::ios::out | std::ios::binary);
				LatestFile << FileData;
				LatestFile.flush();
				LatestFile.close();
				LatestDateFile << Timestamp;
				LatestDateFile.flush();
				LatestDateFile.close();
				std::string ArchiveFilepath = MBDBResources + BlippArchives + Timestamp + " (" + UserPermissions.AssociatedUser + ")";
				std::ofstream ArchiveFile = std::ofstream(ArchiveFilepath, std::ios::out | std::ios::binary);
				ArchiveFile << FileData;
				ArchiveFile.flush();
				ArchiveFile.close();

				std::ofstream LatestAccess = std::ofstream(MBDBResources + BlippArchives + "LatestAccess", std::ios::out | std::ios::binary);
				//ANTAGANDE h�r har personen som acessar redan verifierats vara korrekt
				LatestAccess << "";
				LatestAccess.flush();
				LatestAccess.close();

				std::ofstream AccessLog = std::ofstream(MBDBResources + BlippArchives + "AccessLog", std::ios::out | std::ios::app);
				AccessLog << "Dowload " + p_GetTimestamp() + " (" + UserPermissions.AssociatedUser + ")" << std::endl;

				ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
			}
			else
			{
				ReturnValue = "{\"MBDBAPI_Status\":\"CardLocked\"}";
			}
		}
		else
		{
			ReturnValue = "{\"MBDBAPI_Status\":\"LoginRequired\"}";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::DBAPI_UnlockBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions)
	{
		std::string ReturnValue = "";
		std::string MBDBResources = GetResourceFolderPath();
		std::lock_guard<std::mutex> Lock(m_BlippFileMutex);
		std::string BlippArchives = "/operationblipp/archives/";
		if (Arguments.size() == 1 && Arguments[0] == "Dev")
		{
			BlippArchives = "/operationblipp/Dev/archives/";
		}
		std::string LatestUserDownload = MrPostOGet::LoadWholeFile(MBDBResources + BlippArchives + "LatestAccess");
		if (UserPermissions.AssociatedUser != "")
		{
			std::string const& FileData = Arguments[0];
			std::string Timestamp = p_GetTimestamp();
			if (LatestUserDownload == UserPermissions.AssociatedUser)
			{
				std::ofstream LatestAccess = std::ofstream(MBDBResources + BlippArchives + "LatestAccess", std::ios::out | std::ios::binary);
				LatestAccess << "";
				LatestAccess.flush();
				LatestAccess.close();

				std::ofstream AccessLog = std::ofstream(MBDBResources + BlippArchives + "AccessLog", std::ios::out | std::ios::app);
				AccessLog << "Unlock " + p_GetTimestamp() + " (" + UserPermissions.AssociatedUser + ")"<<std::endl;
				ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
			}
			else
			{
				if (Arguments.size() > 0 && Arguments[0] == "Dev")
				{
					std::ofstream LatestAccess = std::ofstream(MBDBResources + BlippArchives + "LatestAccess", std::ios::out | std::ios::binary);
					LatestAccess << UserPermissions.AssociatedUser;
					LatestAccess.flush();
					LatestAccess.close();

					std::ofstream AccessLog = std::ofstream(MBDBResources + BlippArchives + "AccessLog", std::ios::out | std::ios::app);
					AccessLog << "Unlock " + p_GetTimestamp() + " (" + UserPermissions.AssociatedUser + ")"<<std::endl;

					ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
				}
				else
				{
					ReturnValue = "{\"MBDBAPI_Status\":\"CardLocked\"}";
				}
			}
		}
		else
		{
			ReturnValue = "{\"MBDBAPI_Status\":\"LoginRequired\"}";
		}
		return(ReturnValue);
	}
	std::string MBDB_Website::DBAPI_UploadBlippBugReport(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions)
	{
		std::lock_guard<std::mutex> Lock(m_BlippFileMutex);
		std::string ReturnValue = "";
		if (Arguments.size() < 2)
		{
			ReturnValue = "{\"MBDBAPI_Status\":\"InvalidArguments\"}";
			return(ReturnValue);
		}
		std::string MBDBResources = GetResourceFolderPath();
		std::string BlippBugDirectory = "/operationblipp/archives/CrashReports/";
		if (Arguments.size() == 3 && Arguments[2] == "Dev")
		{
			BlippBugDirectory = "/operationblipp/Dev/archives/CrashReports/";
		}
		std::string NewFileName = p_GetTimestamp() + " (" + UserPermissions.AssociatedUser + ")";
		std::ofstream NewFile = std::ofstream(MBDBResources+BlippBugDirectory+NewFileName,std::ios::out);
		NewFile << Arguments[0] << std::endl;
		NewFile << "---STACK TRACE---" << std::endl;
		NewFile << Arguments[1] << std::endl;
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
		return(ReturnValue);
	}
	//
	MrPostOGet::HTTPDocument MBDB_Website::DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		std::string RequestType = MrPostOGet::GetRequestType(RequestData);
		std::string RequestBody = MrPostOGet::GetRequestContent(RequestData);
		MrPostOGet::HTTPDocument ReturnValue;
		std::string Resourcepath = AssociatedServer->GetResourcePath("mrboboget.se");
		ReturnValue.Type = MBMIME::MIMEType::json;

		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		MBSiteUser NewUser;
		NewUser.m_Username = ConnectionPermissions.AssociatedUser;
		if (ConnectionPermissions.IsNull)
		{
			NewUser.m_GeneralPermissions |= (ConnectionPermissions.Read) * uint64_t(GeneralPermissions::View);
			NewUser.m_GeneralPermissions |= (ConnectionPermissions.Upload) * uint64_t(GeneralPermissions::Upload);
			NewUser.m_GeneralPermissions |= (ConnectionPermissions.Edit) * uint64_t(GeneralPermissions::Edit);

		}
		MBError ParseError = true;
		MBParsing::JSONObject Request = MBParsing::ParseJSONObject(RequestBody, 0, nullptr, &ParseError);
		MBSAPI_DirectiveResponse Response;
		Response.DirectiveResponse = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		if (ParseError)
		{	
			if (Request.HasAttribute("UserVerification"))
			{
				MBParsing::JSONObject const& UserVerification = Request.GetAttribute("UserVerification");
				std::string VerificationType = UserVerification.GetAttribute("VerificationType").GetStringData();
				if (VerificationType == "PasswordHash")
				{
					std::string const& UserName = UserVerification.GetAttribute("User").GetStringData();
					std::string const& PasswordHash = UserVerification.GetAttribute("Password").GetStringData();
					if (m_GetUser(UserName, PasswordHash).size() > 0)
					{
						//just nu har alla samma privilegier för enkelhetsn skull, att reada
						NewUser.m_Username = UserName;
						NewUser.m_GeneralPermissions = uint64_t(GeneralPermissions::View);
					}
				}
			}
			try 
			{
				std::string Directive = Request.GetAttribute("Directive").GetStringData();
				MBParsing::JSONObject const& DirectiveArguments = Request.GetAttribute("DirectiveArguments");
				bool RequestResponded = false;
				{
					std::lock_guard<std::mutex> Lock(m_APIHandlersMutex);
					if (m_PluginAPIHandlers.find(Directive) != m_PluginAPIHandlers.end())
					{
						Response = m_PluginAPIHandlers[Directive]->HandleDirective(NewUser, Directive, DirectiveArguments);
						RequestResponded = true;
					}
				}
				if (!RequestResponded && Directive == "Login")
				{
					//ReturnValue.DocumentData = DBAPI_Login(APIDirectiveArguments);
					std::string Username = DirectiveArguments.GetAttribute("Username").GetStringData();
					std::string Password = DirectiveArguments.GetAttribute("Password").GetStringData();
					if (m_GetUser(Username,Password).size() != 0)
					{
						ReturnValue.ExtraHeaders["Set-Cookie"].push_back("DBUsername=" + Username+ "; Secure; Max-Age=604800; Path=/");
						ReturnValue.ExtraHeaders["Set-Cookie"].push_back("DBPassword=" + Password + "; Secure; Max-Age=604800; Path=/");
						Response.Status = "ok";
					}
					else
					{
						Response.Status = "Invalid credentials";
					}
				}
				else if (!RequestResponded)
				{
					Response.Status = "Invalid Directive";
				}
			}
			catch (std::exception const& Exception)
			{
				Response.Status = "Unkown exception";
			}
		}
		else
		{
			Response.Status = "Failed to parse request";
		}
		MBParsing::JSONObject CompleteResponse = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		CompleteResponse["MBDBAPI_Status"] = Response.Status;
		CompleteResponse["DirectiveResponse"] = std::move(Response.DirectiveResponse);
		//std::cout << ReturnValue.DocumentData << std::endl;
		ReturnValue.DocumentData = CompleteResponse.ToString();
		return(ReturnValue);
	}
	bool MBDB_Website::DBUpdate_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "DBUpdate")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
        auto Perms = m_GetConnectionPermissions(RequestData);
        if(Perms.AssociatedUser == "")
        {
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "InvalidPermissions.html", ResourcePath);
        }
        else
        {
            ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "DBUpdate.html", ResourcePath);
        }
		return(ReturnValue);
	}
	bool MBDB_Website::DBOperationBlipp_Predicate(std::string const& RequestData)
	{
		std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
		std::vector<std::string> Directorys = MBUtility::Split(RequestResource, "/");
		if (Directorys.size() >= 1)
		{
			if (Directorys[0] == "operationblipp")
			{
				return(true);
			}
		}
		return(false);
	}
	MrPostOGet::HTTPDocument MBDB_Website::DBOperatinBlipp_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		std::string RequestType = MrPostOGet::GetRequestType(RequestData);
		std::string MBDBResources = GetResourceFolderPath();
		std::string HTMLFolder = AssociatedServer->GetResourcePath("mrboboget.se");
		std::string DefaultPage = HTMLFolder + "/operationblipp.html";
		std::vector<std::string> PathComponents = MBUtility::Split(MrPostOGet::GetRequestResource(RequestData), "/");
		if (PathComponents.size() == 2)
		{
			if (PathComponents.back() == "")
			{
				PathComponents.resize(1);
			}
		}
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		std::string LastTimestamp = MrPostOGet::LoadWholeFile(MBDBResources + "/operationblipp/archives/LatestDate");
		std::string LatestUserDownload = MrPostOGet::LoadWholeFile(MBDBResources + "/operationblipp/archives/LatestAccess");


		if (ConnectionPermissions.AssociatedUser == "" || ConnectionPermissions.AssociatedUser == "guest")
		{
			ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::NotFound;
			ReturnValue.DocumentData = "<h1>Not found 404</h1>";
			return(ReturnValue);
		}
		if (RequestType == "GET")
		{
			if (PathComponents.size() == 1)
			{
				std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
				std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage",""},{"LatestDate",LastTimestamp} };
				ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
				ReturnValue.DocumentData = ResourceData;
				ReturnValue.Type = MBMIME::MIMEType::HTML;
			}
			else
			{
				if (PathComponents[1] == "archives")
				{
					//std::string TableData = "";
					//std::unordered_map<std::string, std::string> VariableMap = {};
					//MrPostOGet::HTMLNode ArchiveTable = MrPostOGet::HTMLNode::CreateElement("table");

				}
				else if (PathComponents[1] == "latest")
				{
					if (LatestUserDownload == "")
					{
						std::ofstream LatestAccess = std::ofstream(MBDBResources + "/operationblipp/archives/LatestAccess", std::ios::out | std::ios::binary);
						LatestAccess << ConnectionPermissions.AssociatedUser;
						LatestAccess.flush();
						LatestAccess.close();
						ReturnValue = AssociatedServer->GetResource(MBDBResources + "/operationblipp/archives/latest");
					}
					else
					{
						std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
						std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Can't acces latest file: User " + LatestUserDownload + " hasn't updated the file"},{"LatestDate",LastTimestamp} };
						ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
						ReturnValue.DocumentData = ResourceData;
						ReturnValue.Type = MBMIME::MIMEType::HTML;
					}
				}
			}
		}
		if (RequestType == "POST")
		{
			try
			{
				size_t HeaderStart = RequestData.find("\r\n") + 2;
				MBMIME::MIMEMultipartDocumentExtractor MimeExtractor(RequestData.data(), RequestData.size(), HeaderStart);
				MimeExtractor.ExtractHeaders();
				MimeExtractor.ExtractHeaders();
				//vi vill bara parsa ett visst antal stycken, om dem inte �r r�tt s� bara avbryter vi
				std::string FileData = MimeExtractor.ExtractPartData();
				std::string Timestamp = p_GetTimestamp();
				if (LatestUserDownload == ConnectionPermissions.AssociatedUser)
				{
					std::ofstream LatestFile = std::ofstream(MBDBResources + "/operationblipp/archives/latest", std::ios::out | std::ios::binary);
					std::ofstream LatestDateFile = std::ofstream(MBDBResources + "/operationblipp/archives/LatestDate", std::ios::out | std::ios::binary);
					LatestFile << FileData;
					LatestFile.flush();
					LatestFile.close();
					LatestDateFile << Timestamp;
					LatestDateFile.flush();
					LatestDateFile.close();
					std::string ArchiveFilepath = MBDBResources + "/operationblipp/archives/" + Timestamp + " (" + ConnectionPermissions.AssociatedUser + ")";
					std::ofstream ArchiveFile = std::ofstream(ArchiveFilepath, std::ios::out | std::ios::binary);
					ArchiveFile << FileData;
					ArchiveFile.flush();
					ArchiveFile.close();

					std::ofstream LatestAccess = std::ofstream(MBDBResources + "/operationblipp/archives/LatestAccess", std::ios::out | std::ios::binary);
					//ANTAGANDE h�r har personen som acessar redan verifierats vara korrekt
					LatestAccess << "";
					LatestAccess.flush();
					LatestAccess.close();

					std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
					std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Succesfully uploaded file"},{"LatestDate",Timestamp} };
					ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
					ReturnValue.DocumentData = ResourceData;
					ReturnValue.Type = MBMIME::MIMEType::HTML;
				}
				else
				{
					std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
					std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Error updating latest: Need to download the latest before updating"},{"LatestDate",LastTimestamp} };
					ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
					ReturnValue.DocumentData = ResourceData;
					ReturnValue.Type = MBMIME::MIMEType::HTML;
				}
			}
			catch (const std::exception&)
			{
				std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
				std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Error uploading file"},{"LatestDate",LastTimestamp} };
				ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
				ReturnValue.DocumentData = ResourceData;
				ReturnValue.Type = MBMIME::MIMEType::HTML;
			}
		}
		return(ReturnValue);
	};
	std::string MBDB_Website::p_GetGlobalResourceFolder()
	{
		std::lock_guard<std::mutex> Lock(m_GlobalResourceFolderMutex);
		return(m_GlobalResourceFolder);
	}
	std::string MBDB_Website::p_GetPluginResourceFolder(PluginID AssociatedPlugin)
	{
		std::string ReturnValue;
		{
			std::lock_guard<std::mutex> Lock(m_PluginNameMutex);
			ReturnValue = "./MBWebsite/Plugins/" + m_PluginNames[AssociatedPlugin] + "/";
		}
		return(ReturnValue);
	}
	void MBDB_Website::AddPlugin(std::unique_ptr<MBSitePlugin> PluginToAdd)
	{
		PluginID NewID = 0;
		{
			std::lock_guard<std::mutex> PluginLock(m_PluginMapMutex);
			std::lock_guard<std::mutex> NameLock(m_PluginNameMutex);
			m_CurrentID += 1;
			NewID = m_CurrentID;
			PluginToAdd->m_AssociatedSite = this;
			m_PluginNames[m_CurrentID] = PluginToAdd->GetPluginName();
			m_LoadedPlugins[m_CurrentID] = std::move(PluginToAdd);
		}
		m_LoadedPlugins[NewID]->OnCreate(NewID);
	}
	void MBDB_Website::RegisterMBSiteAPIHandler(PluginID const& AssociatedPlugin, MBSite_APIHandler* NewAPIHandler)
	{
		std::lock_guard<std::mutex> Lock(m_APIHandlersMutex);
		std::vector<std::string> HandledTopDomains = NewAPIHandler->HandledDirectives();
		for (size_t i = 0; i < HandledTopDomains.size(); i++)
		{
			m_PluginAPIHandlers[HandledTopDomains[i]] = NewAPIHandler;
		}	
	}
	void MBDB_Website::RegisterMBSiteHTTPRequestHandler(PluginID const& AssociatedPlugin, MBSite_HTTPHandler* RequestHandler)
	{
		std::lock_guard<std::mutex> Lock(m_HTTPHandlersMutex);
		std::vector<std::string> HandledTopDomains = RequestHandler->HandledTopDirectories();
		for (size_t i = 0; i < HandledTopDomains.size(); i++)
		{
			m_PluginHTTPHandlers[HandledTopDomains[i]] = RequestHandler;
		}
	}
	std::string MBDB_Website::_GetPluginFileAbsolutePath(PluginID const& AssociatedPlugin, std::string const& PluginFilePath)
	{
		std::string ReturnValue;
		std::string PluginName = "";
		{
			std::lock_guard<std::mutex> Lock(m_PluginNameMutex);
			PluginName = m_PluginNames[AssociatedPlugin];
		}
		std::string PluginResourceFolder = GetResourceFolderPath() + "../Plugins/" + PluginName + "/"; //p_GetPluginResourceFolder(AssociatedPlugin);
		ReturnValue = PluginResourceFolder + PluginFilePath;
		return(ReturnValue);
	}
	std::string MBDB_Website::LoadFileWithPreProcessing(PluginID AssociatedPlugin, std::string const& Filepath)
	{
		std::string ReturnValue;
		//kommer inte ihåg hur den andra var implemeneterad så för att vara säker B)
		std::string GlobalResourcePath = p_GetGlobalResourceFolder()+"/";
		std::string PluginResourceFolder = p_GetPluginResourceFolder(AssociatedPlugin);
		ReturnValue = MrPostOGet::LoadFileWithPreprocessing(PluginResourceFolder + "/" + Filepath, GlobalResourcePath);
		return(ReturnValue);
	}
	MBError MBDB_Website::ReadFile(PluginID AssociatedPlugin, MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& Filepath, 
		std::unique_ptr<MBUtility::MBSearchableInputStream>* OutInStream)
	{
		MBError ReturnValue = true;
		//*OutInStream = nullptr;
		std::unique_ptr<MBUtility::MBSearchableInputStream> NewStream = nullptr;
		if (Location == FileLocationType::PluginStaticResource)
		{
			std::string PluginFolder = p_GetPluginResourceFolder(AssociatedPlugin);
			if (std::filesystem::exists(PluginFolder + Filepath))
			{
				NewStream = std::unique_ptr<MBUtility::MBSearchableInputStream>(new MBUtility::MBFileInputStream(PluginFolder + Filepath));
			}
			else
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "PluginStaticResource file doesnt exist";
			}
		}
		else if (Location == FileLocationType::DBFile)
		{
			std::string DBTopDirectory = GetResourceFolderPath();
			if (std::filesystem::exists(DBTopDirectory + Filepath))
			{
				NewStream = std::unique_ptr<MBUtility::MBSearchableInputStream>(new MBUtility::MBFileInputStream(DBTopDirectory + Filepath));
			}
			else
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "Database file doesnt exist";
			}
		}
		else if (Location == FileLocationType::GlobalStaticResource)
		{
			std::string GlobalResourceFolder = p_GetGlobalResourceFolder();
			if (std::filesystem::exists(GlobalResourceFolder + Filepath))
			{
				NewStream = std::unique_ptr<MBUtility::MBSearchableInputStream>(new MBUtility::MBFileInputStream(GlobalResourceFolder + Filepath));
			}
			else
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "Database file doesnt exist";
			}
		}
		else
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Invalid FileLocationType";
		}
		if (OutInStream != nullptr)
		{
			*OutInStream = std::move(NewStream);
		}
		return(ReturnValue);
	}
	MBError MBDB_Website::WriteFile(PluginID AssociatedPlugin, MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& Filepath, 
		std::unique_ptr<MBUtility::MBSearchableOutputStream>* OutOutputStream)
	{
		MBError ReturnValue = false;
	

		if (OutOutputStream != nullptr)
		{
			*OutOutputStream = nullptr;
		}
		return(ReturnValue);
	}
	MBError MBDB_Website::ListDirectory(PluginID AssociatedPlugin, MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& DirectoryPath, std::vector<FilesystemObjectInfo>* OutInfo)
	{
		MBError ReturnValue = true;
		

		std::vector<FilesystemObjectInfo> NewOutInfo = {};
		std::string DirectoryToIteratePath = "";
		if (Location == FileLocationType::DBFile)
		{
			if (AssociatedUser.GeneralPermissions() & uint64_t(GeneralPermissions::List))
			{
				ReturnValue = false;
				ReturnValue.ErrorMessage = "User needs \"List\" permissions";
			}
			else
			{
				DirectoryToIteratePath = GetResourceFolderPath() +"/"+ DirectoryPath;
			}
		}
		else if (Location == FileLocationType::PluginStaticResource)
		{
			DirectoryToIteratePath = p_GetPluginResourceFolder(AssociatedPlugin) + "/" + DirectoryPath;
		}
		else if (Location == FileLocationType::GlobalStaticResource)
		{
			DirectoryToIteratePath = p_GetGlobalResourceFolder() + "/" + DirectoryPath;
		}
		else
		{
			throw std::runtime_error("Invalid location type");
		}
		if (DirectoryToIteratePath != "")
		{
			std::filesystem::directory_iterator Iterator(DirectoryToIteratePath);
			for (auto const& Entry : Iterator)
			{
				FilesystemObjectInfo NewInfo;
				NewInfo.Name = MBUnicode::PathToUTF8(Entry.path().filename());
				if (Entry.is_directory())
				{
					NewInfo.Type = FilesystemType::Directory;
				}
				else
				{
					NewInfo.Type = FilesystemType::File;
				}
				NewOutInfo.push_back(std::move(NewInfo));
			}
		}
		if (OutInfo != nullptr)
		{
			*OutInfo = std::move(NewOutInfo);
		}
		return(ReturnValue);
	}


	//BEGIN MBSite_MBEmbedder
	std::string MBSite_MBEmbedder::GetPluginName() const
	{
		return("MBEmbed");
	}
	void MBSite_MBEmbedder::OnCreate(PluginID AssociatedID)
	{
		m_PluginID = AssociatedID;
	}
	void MBSite_MBEmbedder::OnDestroy()
	{

	}
	std::string MBSite_MBEmbedder::p_GetEmbeddedVideo(std::string const& VideoPath)
	{
		std::string ReturnValue = "";
		std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
		std::unordered_map<std::string, bool> BrowserSupported = { {"mp4",true},{"webm",true},{"ogg", true} };
		if (BrowserSupported.find(FileExtension) != BrowserSupported.end())
		{
			std::unordered_map<std::string, std::string> VariableValues = {};
			VariableValues["ElementID"] = VideoPath;
			VariableValues["MediaType"] = "video";
			VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
			VariableValues["FileType"] = FileExtension;
			ReturnValue = MrPostOGet::ReplaceMPGVariables(p_LoadStreamTemplate(), VariableValues);
		}
		else
		{
			ReturnValue = "<p>Native broswer streaming not Supported</p><br><a href=\"/DB/" + VideoPath + "\">/DB/" + VideoPath + "</a>";
		}
		return(ReturnValue);
	}
	std::string MBSite_MBEmbedder::p_GetEmbeddedAudio(std::string const& VideoPath)
	{
		std::unordered_map<std::string, std::string> VariableValues = {};
		std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
		VariableValues["ElementID"] = VideoPath;
		VariableValues["MediaType"] = "audio";
		VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
		VariableValues["FileType"] = FileExtension;
		//std::string ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
		std::string ReturnValue = MrPostOGet::ReplaceMPGVariables(p_LoadStreamTemplate(), VariableValues);
		return(ReturnValue);
	}
	std::string MBSite_MBEmbedder::p_GetEmbeddedImage(std::string const& ImagePath)
	{
		std::string ReturnValue = "<image src=\"/DB/" + ImagePath + "\" style=\"max-width:100%\"></image>";
		return(ReturnValue);
	}
	std::string MBSite_MBEmbedder::p_GetEmbeddedPDF(std::string const& ImagePath)
	{
		return("<iframe src=\"/DB/" + ImagePath + "\" style=\"width: 100%; height: 100%;max-height: 100%; max-width: 100%;\"></iframe>");
	}
	std::string MBSite_MBEmbedder::p_LoadStreamTemplate()
	{
		std::string ReturnValue;
		std::unique_ptr<MBUtility::MBSearchableInputStream> InputStream = nullptr;
		GetSite().ReadFile(m_PluginID, MBSiteUser() , FileLocationType::PluginStaticResource, "/DirectFileStreamTemplate.html", &InputStream);
		if (InputStream == nullptr)
		{
			throw std::runtime_error("Couldnt read required plugin resource: MBEmbed /DirectFileStreamTemplate.html");
		}
		ReturnValue = h_ReadWholeInput(InputStream);
		return(ReturnValue);
	}
	std::string MBSite_MBEmbedder::p_GetEmbeddedMBDBObject(std::string const& MBDBResource)
	{
		std::string ReturnValue = "";
		MBError EvaluationError(true);
		//MBDB::MBDB_Object ObjectToEmbedd = p_LoadMBDBObject(Permissions.AssociatedUser, GetResourceFolderPath() + MBDBResource, &EvaluationError);
		MBDB::MBDB_Object ObjectToEmbedd;
		//std::string ObjectFilePath = GetResourceFolderPath() + MBDBResource;
		//if (std::filesystem::exists(ObjectFilePath))
		//{
		//	std::string ObjectData = MrPostOGet::LoadWholeFile(ObjectFilePath);
		//	ObjectToEmbedd = MBDB::MBDB_Object::ParseObject(ObjectData, 0, nullptr, &EvaluationError);
		//}
		std::unique_ptr<MBUtility::MBSearchableInputStream> FileInput = nullptr;
		//Förutsätter att en godtycklig person kan läsa in en fil
		EvaluationError = GetSite().ReadFile(m_PluginID, MBSiteUser(), FileLocationType::DBFile, MBDBResource,&FileInput);
		if (FileInput != nullptr)
		{
			std::string ObjectData = h_ReadWholeInput(FileInput);
			ObjectToEmbedd = MBDB::MBDB_Object::ParseObject(ObjectData, 0, nullptr, &EvaluationError);
		}
		else
		{
			EvaluationError = false;
			//EvaluationError.ErrorMessage = "Object does not exist";
		}
		MrPostOGet::HTMLNode NodeToEmbedd = MrPostOGet::HTMLNode::CreateElement("a");
		NodeToEmbedd["href"] = "/DBView/" + MBDBResource;

		if (EvaluationError)
		{
			if (ObjectToEmbedd.HasAttribute("Banner"))
			{
				MBDB::MBDB_Object& BannerObject = ObjectToEmbedd.GetAttribute("Banner");
				if (!BannerObject.IsEvaluated())
				{
					//int asdasd% = 2;
					//std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
					//MBDB::MBDBO_EvaluationInfo EvaluationInfo;
					//EvaluationInfo.AssociatedDatabase = m_ReadonlyDatabase;
					//EvaluationInfo.EvaluatingUser = "";//lite osäker på om detta ger problem
					//EvaluationInfo.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(GetResourceFolderPath() + MBDBResource).parent_path());
					//BannerObject.Evaluate(EvaluationInfo, &EvaluationError);
					PluginReference<MBSite_DBPlugin> DBPlugin = GetSite().GetPluginReference<MBSite_DBPlugin>(m_PluginID);
					if (DBPlugin == nullptr)
					{
						EvaluationError = false;
						EvaluationError.ErrorMessage = "Error embedding resource: failed to find required MBSite_DBPlugin plugin";
					}
					else
					{
						DBPlugin->EvaluateMBDBObject(m_PluginID, MBSiteUser(), FileLocationType::DBFile, MBDBResource, BannerObject);
					}
				}
				//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">" + p_GetEmbeddedResource(ObjectToEmbedd.GetAttribute("Banner"),HTMLFolder,Permissions) + "</a>"
				if (EvaluationError)
				{
					if (BannerObject.GetType() != MBDB::MBDBO_Type::String)
					{
						NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Invalid banner type"));
					}
					else
					{
						NodeToEmbedd.AppendChild(p_GetEmbeddedImage(BannerObject.GetStringData()));
					}
				}
				else
				{
					NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Error embedding resource: " + EvaluationError.ErrorMessage));
				}
			}
			else
			{
				//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">" + MBDBResource + "</a>";
				NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode(MBDBResource));
			}
		}
		else
		{
			//ReturnValue = "<a href=\"/DBView/" + MBDBResource + "\">Error embedding resource: " + EvaluationError.ErrorMessage + "</a>";
			NodeToEmbedd.AppendChild(MrPostOGet::HTMLNode("Error embedding resource: " + EvaluationError.ErrorMessage));
		}
		ReturnValue = NodeToEmbedd.ToString();
		return(ReturnValue);
	}
	MrPostOGet::HTMLNode MBSite_MBEmbedder::EmbeddResource(PluginID CallingPlugin,MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& Filepath)
	{
		MrPostOGet::HTMLNode NodeToReturn;
		std::string ReturnValue = "";
		std::string ResourceExtension = Filepath.substr(Filepath.find_last_of(".") + 1);
		MBMIME::MediaType ResourceMedia = MBMIME::GetMediaTypeFromExtension(ResourceExtension);
		bool IsMBDBResource = true;

		std::string ResourceHTTPURL = Filepath;
		if (IsMBDBResource)
		{
			std::unique_ptr<MBUtility::MBSearchableInputStream> FileInput = nullptr;
			MBError ReadFileError = GetSite().ReadFile(CallingPlugin, AssociatedUser, Location, Filepath,&FileInput);
			if (!ReadFileError)
			{
				ReturnValue = "<p>File does not exist<p>";
			}
		}
		if (ResourceMedia == MBMIME::MediaType::Image)
		{
			ReturnValue = p_GetEmbeddedImage(ResourceHTTPURL);
		}
		else if (ResourceMedia == MBMIME::MediaType::Video)
		{
			ReturnValue = p_GetEmbeddedVideo(ResourceHTTPURL);
		}
		else if (ResourceMedia == MBMIME::MediaType::Audio)
		{
			ReturnValue = p_GetEmbeddedAudio(ResourceHTTPURL);
		}
		else if (ResourceMedia == MBMIME::MediaType::PDF)
		{
			ReturnValue = p_GetEmbeddedPDF(ResourceHTTPURL);
		}
		else if (ResourceExtension == "mbdbo")
		{
			ReturnValue = p_GetEmbeddedMBDBObject(ResourceHTTPURL);
		}
		else
		{
			ReturnValue = "<p>File in unrecognized format</p><br><a href=\"/DB/" + Filepath + "\">/DB/" + Filepath + "</a>";
		}
		return(MrPostOGet::HTMLNode(ReturnValue,0));
	}
	//END MBSite_MBEmbedder

	//BEGIN MBSite_DBPlugin
	std::string MBSite_DBPlugin::GetPluginName() const
	{
		return("MBSQL");
	}
	bool MBSite_DBPlugin::p_DependanciesLoaded()
	{
		return(m_EmbeddPlugin != nullptr);
	}
	void MBSite_DBPlugin::p_LoadDependancies()
	{
		m_EmbeddPlugin = GetSite().GetPluginReference<MBSite_MBEmbedder>(m_PluginID);
		if (m_EmbeddPlugin == nullptr)
		{
			//mest för debug, vet inte om vi kan garanetar inbyggd dependancy managing
			throw std::runtime_error("Required dependancy MBEmbedder not present");
		}
	}
	bool MBSite_DBPlugin::p_VerifyTableName(std::string const& TableNameToVerify)
	{
		std::lock_guard<std::mutex> ReadLock(m_ReadonlyDatabaseMutex);
		std::vector<std::string> TableNames = m_ReadonlyDatabase->GetAllTableNames();
		bool Exists = false;
		for (size_t i = 0; i < TableNames.size(); i++)
		{
			if (TableNameToVerify == TableNames[i])
			{
				Exists = true;
				break;
			}
		}
		return(Exists);
	}
	bool MBSite_DBPlugin::p_VerifyColumnName(std::string const& Table, std::vector<std::string> const& ColumnNames)
	{
		std::lock_guard<std::mutex> ReadLock(m_ReadonlyDatabaseMutex);
		std::vector<MBDB::ColumnInfo> ColumnsInfo = m_ReadonlyDatabase->GetColumnInfo(Table);
		std::vector<std::string> StoredNames;
		for (size_t i = 0; i < ColumnsInfo.size(); i++)
		{
			StoredNames.push_back(ColumnsInfo[i].ColumnName);
		}
		return(StoredNames == ColumnNames);
	}
	std::vector<int> MBSite_DBPlugin::p_GetColumnIndexes(std::string const& TableName, std::vector<std::string> const& ColumnNames)
	{
		std::lock_guard<std::mutex> ReadLock(m_ReadonlyDatabaseMutex);
		std::vector<MBDB::ColumnInfo> ColumnsInfo = m_ReadonlyDatabase->GetColumnInfo(TableName);
		std::vector<int> ReturnValue;
		for (size_t i = 0; i < ColumnNames.size(); i++)
		{
			int Index = -1;
			for (size_t j = 0; j < ColumnsInfo.size(); j++)
			{
				//StoredNames.push_back(ColumnsInfo[i].ColumnName);
				if (ColumnNames[i] == ColumnsInfo[j].ColumnName)
				{
					Index = j;
					break;
				}
			}
			if (Index == -1)
			{
				ReturnValue.clear();
				break;
			}
			ReturnValue.push_back(Index);
		}
		return(ReturnValue);
	}
	void MBSite_DBPlugin::OnCreate(PluginID AssociatedID)
	{
		//läser config
		m_PluginID = AssociatedID;
		std::string DatabasePath = GetSite()._GetPluginFileAbsolutePath(m_PluginID, "/TopDatabase");
		if (DatabasePath != "")
		{
			if (m_ReadonlyDatabase == nullptr)
			{
				m_ReadonlyDatabase = std::unique_ptr<MBDB::MrBoboDatabase>(new MBDB::MrBoboDatabase(DatabasePath, MBDB::DBOpenOptions::ReadOnly));
			}
			if (m_WriteableDatabase == nullptr)
			{
				m_WriteableDatabase = std::unique_ptr<MBDB::MrBoboDatabase>( new MBDB::MrBoboDatabase(DatabasePath, MBDB::DBOpenOptions::ReadWrite));
			}
		}
		GetSite().RegisterMBSiteAPIHandler(m_PluginID,this);
		GetSite().RegisterMBSiteHTTPRequestHandler(m_PluginID,this);
		p_LoadDependancies();
	}
	void MBSite_DBPlugin::OnDestroy()
	{
		//inget
	}
	std::vector<std::string> MBSite_DBPlugin::HandledDirectives() const
	{
		std::vector<std::string> ReturnValue = { "GetTableNames","GetTableInfo","SearchTableWithWhere","AddEntryToTable", "UpdateTableRow","QueryDatabase"};
		return(ReturnValue);
	}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_GetTableNames(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& ClientRequest, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		std::vector<std::string> TableNames;
		{
			std::lock_guard<std::mutex> Lock(m_ReadonlyDatabaseMutex);
			TableNames = m_ReadonlyDatabase->GetAllTableNames();
		}
		ReturnValue["TableNames"] = MBParsing::JSONObject(TableNames);
		return(ReturnValue);
	}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_GetTableInfo(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& ClientRequest, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		std::string TableName = ClientRequest.GetMapData().at("TableName").GetStringData();
		std::vector<MBDB::ColumnInfo> ColumnInfo = {};
		{
			std::lock_guard<std::mutex> Lock(m_ReadonlyDatabaseMutex);
			ColumnInfo = m_ReadonlyDatabase->GetColumnInfo(TableName);
		}
		std::vector<MBParsing::JSONObject> ColumnJSONObjects = {};
		for (size_t i = 0; i < ColumnInfo.size(); i++)
		{
			ColumnJSONObjects.push_back(MBParsing::ParseJSONObject(ToJason(ColumnInfo[i]),0,nullptr,nullptr));
		}
		ReturnValue["ColumnInfo"] = MBParsing::JSONObject(std::move(ColumnJSONObjects));
		return(ReturnValue);
	}
	//std::vector<MBDB::MBDB_RowData> MBSite_DBPlugin::p_EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
	//	std::vector<int> ColumnIndex, std::string TableName, MBError* OutError)
	//{
	//	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	//	std::lock_guard<std::mutex> Lock(m_WriteableDatabaseMutex);
	//	MBDB::SQLStatement* NewStatement = m_WriteableDatabase->GetSQLStatement(SQLCommand);
	//
	//	std::vector<MBDB::ColumnInfo> TableColumnInfo = m_WriteableDatabase->GetColumnInfo(TableName);
	//	for (size_t i = 0; i < ColumnValues.size(); i++)
	//	{
	//		if (TableColumnInfo[ColumnIndex[i]].ColumnType == MBDB::ColumnSQLType::Int)
	//		{
	//			MBDB::MaxInt NewInt = StringToInt(ColumnValues[i], OutError);
	//			if (!OutError)
	//			{
	//				break;
	//			}
	//			*OutError = NewStatement->BindInt(NewInt, i + 1);
	//		}
	//		else
	//		{
	//			*OutError = NewStatement->BindString(ColumnValues[i], i + 1);
	//			if (!OutError)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//	if (OutError)
	//	{
	//		QuerryResult = m_WriteableDatabase->GetAllRows(NewStatement, OutError);
	//	}
	//	NewStatement->FreeData();
	//	m_WriteableDatabase->FreeSQLStatement(NewStatement);
	//	return(QuerryResult);
	//}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_SearchTableWithWhere(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue;
		//if (Arguments.size() < 2)
		//{
		//	return("{\"MBDBAPI_Status\":\"Invalid number of arguments\"}");
		//}
		std::string TableName = DirectiveArguments.GetAttribute("TableName").GetStringData();
		if (!p_VerifyTableName(TableName))
		{
			Status = "Invalid table name";
			return(ReturnValue);
		}
		std::vector<MBDB::MBDB_RowData> RowResponse = {};
		MBError DatabaseError(true);
		{
			std::lock_guard<std::mutex> Lock(m_ReadonlyDatabaseMutex);
			std::string SQlQuerry = "SELECT * FROM " + TableName + " " + DirectiveArguments.GetAttribute("Query").GetStringData();
			RowResponse = m_ReadonlyDatabase->GetAllRows(SQlQuerry, &DatabaseError);
		}
		if (!DatabaseError)
		{
			return(MBParsing::ParseJSONObject("{\"MBDBAPI_Status\":" + ToJason(DatabaseError.ErrorMessage) + "}",0,nullptr,nullptr));
		}
		ReturnValue = MBParsing::ParseJSONObject("{\"MBDBAPI_Status\":\"ok\",\"Rows\":" + MakeJasonArray(RowResponse) + "}",0,nullptr,nullptr);
		return(ReturnValue);
	}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_UpdateTableRow(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue;
		std::string TableName = DirectiveArguments.GetAttribute("TableName").GetStringData();
		if (!p_VerifyTableName(TableName))
		{
			Status = "Invalid table name";
			return(ReturnValue);
			//return(MBParsing::ParseJSONObject("{\"MBDBAPI_Status\": Invalid table name}", 0, nullptr, nullptr));
		}
		std::string SQLCommand = "UPDATE " + TableName + " SET "; //VALUES (";
		std::vector<std::string> ColumnNames = {};
		std::vector<std::string> OldColumnValues = {};
		std::vector<std::string> NewColumnValues = {};
		MBError DataBaseError(true);
		
		std::vector<MBParsing::JSONObject> const& ColumnNameData = DirectiveArguments.GetAttribute("ColumnNames").GetArrayData();
		std::vector<MBParsing::JSONObject> const& OldColumnData = DirectiveArguments.GetAttribute("OldColumnValues").GetArrayData();
		std::vector<MBParsing::JSONObject> const& NewColumnData = DirectiveArguments.GetAttribute("NewColumnValues").GetArrayData();
		if (ColumnNameData.size() != OldColumnData.size() || ColumnNameData.size() != NewColumnData.size())
		{
			Status = "Length of ColumnNames and OldColumnValues and NewColumnValues has to be the same";
			return(ReturnValue);
		}
		for (size_t i = 0; i < ColumnNameData.size(); i++)
		{
			ColumnNames.push_back(ColumnNameData[i].GetStringData());
			OldColumnValues.push_back(OldColumnData[i].GetStringData());
			NewColumnValues.push_back(NewColumnData[i].GetStringData());
		}
		bool ColumnsExist = p_VerifyColumnName(TableName, ColumnNames);
		if (!ColumnsExist)
		{
			Status = "Invalid column names specified";
			return(ReturnValue);
		}
		for (size_t i = 0; i < OldColumnValues.size(); i++)
		{
			SQLCommand += ColumnNames[i] + "=?";
			if (i + 1 < OldColumnValues.size())
			{
				SQLCommand += ", ";
			}
		}
		SQLCommand += " WHERE ";
		for (size_t i = 0; i < ColumnNames.size(); i++)
		{
			SQLCommand += "(" + ColumnNames[i] + "=?";
			if (OldColumnValues[i] == "null")
			{
				SQLCommand += " OR " + ColumnNames[i] + " IS NULL";
			}
			SQLCommand += ")";
			if (i + 1 < ColumnNames.size())
			{
				SQLCommand += " AND ";
			}
		}
		std::vector<MBDB::MBDB_RowData> QuerryResult = {};
		{
			std::lock_guard<std::mutex> Lock(m_WriteableDatabaseMutex);
			MBDB::SQLStatement NewStatement = m_WriteableDatabase->GetSQLStatement(SQLCommand);

			//DEBUG GREJER
			//MBDB::SQLStatement* DebugStatement = WritableDatabase->GetSQLStatement("SELECT * FROM Music WHERE RowNumber=82");
			//std::vector<MBDB::MBDB_RowData> DebugResult = WritableDatabase->GetAllRows(DebugStatement);
			//auto DebugTuple = DebugResult[0].GetTuple<int, std::string, std::string, std::string, std::string, std::string, std::string>();
			//std::ofstream DebugFile("./DebugDatabaseData.txt", std::ios::out|std::ios::binary);
			//std::ofstream DebugInput("./DebugInputData.txt", std::ios::out | std::ios::binary);
			//DebugFile << std::get<6>(DebugTuple);
			//DebugInput << Arguments[21];

			std::vector<MBDB::ColumnInfo> ColumnInfo = m_WriteableDatabase->GetColumnInfo(TableName);
			std::vector<MBDB::ColumnSQLType> ColumnTypes = {};
			for (size_t i = 0; i < ColumnInfo.size(); i++)
			{
				ColumnTypes.push_back(ColumnInfo[i].ColumnType);
			}
			NewStatement.BindValues(NewColumnValues, ColumnTypes, 0);
			NewStatement.BindValues(OldColumnValues, ColumnTypes, NewColumnValues.size());
			QuerryResult = m_WriteableDatabase->GetAllRows(NewStatement, &DataBaseError);
		}
		if (DataBaseError)
		{
			ReturnValue = MBParsing::ParseJSONObject("{}", 0, nullptr, nullptr);
		}
		else
		{
			Status = DataBaseError.ErrorMessage;
			//ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
		}
		return(ReturnValue);
	}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_AddEntryToTable(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue;
		std::string TableName = DirectiveArguments.GetAttribute("TableName").GetStringData();
		std::lock_guard<std::mutex> WritableLock(m_WriteableDatabaseMutex);
		if (!p_VerifyTableName(TableName))
		{
			Status = "Invalid table name";
			return(ReturnValue);
			//return(MBParsing::ParseJSONObject("{\"MBDBAPI_Status\": Invalid table name}", 0, nullptr, nullptr));
		}
		std::string SQLCommand = "INSERT INTO " + TableName + "("; //VALUES (";
		std::vector<std::string> ColumnNames = {};
		std::vector<std::string> ColumnValues = {};
		MBError DataBaseError(true);

		std::vector<MBParsing::JSONObject> const& ColumnNamesData = DirectiveArguments.GetAttribute("ColumnNames").GetArrayData();
		std::vector<MBParsing::JSONObject> const& ColumnValuesData = DirectiveArguments.GetAttribute("ColumnValues").GetArrayData();
		if (ColumnNames.size() != ColumnValues.size())
		{
			Status = "Size of ColumnNames and ColumnValues has to be the same";
			return(ReturnValue);
		}
		//ny semantik, alla columns måste få ett värde

		for (size_t i = 0; i < ColumnNamesData.size(); i++)
		{
			ColumnNames.push_back(ColumnNamesData[i].GetStringData());
			ColumnValues.push_back(ColumnValuesData[i].GetStringData());
			SQLCommand += ColumnNamesData[i].GetStringData();
			if (i + 1 < ColumnNamesData.size())
			{
				SQLCommand += ",";
			}
		}

		//for (size_t i = 1; i < Arguments.size(); i++)
		//{
		//	size_t FirstColon = Arguments[i].find_first_of(":");
		//	size_t SecondColon = Arguments[i].find(":", FirstColon + 1);
		//	size_t NewColumnIndex = -1;
		//	NewColumnIndex = StringToInt(Arguments[i].substr(FirstColon + 1, SecondColon - FirstColon), &DataBaseError);
		//	if (!DataBaseError)
		//	{
		//		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
		//		return(ReturnValue);
		//	}
		//	ColumnNames.push_back(Arguments[i].substr(0, FirstColon));
		//	ColumnValues.push_back(Arguments[i].substr(SecondColon + 1));
		//	ColumnIndex.push_back(NewColumnIndex);
		//	SQLCommand += ColumnNames[i - 1];
		//	if (i + 1 < Arguments.size())
		//	{
		//		SQLCommand += ",";
		//	}
		//}
		SQLCommand += ") VALUES(";
		for (size_t i = 0; i < ColumnValues.size(); i++)
		{
			SQLCommand += "?";
			if (i + 1 < ColumnValues.size())
			{
				SQLCommand += ",";
			}
		}
		SQLCommand += ");";

		//bool ColumnsExist = p_VerifyColumnName(TableName, ColumnNames);
		std::vector<int> ColumnIndexes = p_GetColumnIndexes(TableName, ColumnNames);
		if (ColumnIndexes.size() != ColumnNames.size())
		{
			Status = "Invalid column names";
			return(ReturnValue);
		}
		//ANTAGANDE eftersom p_VerifyColumnName är i rätt ordning går det att binda direkt
		std::vector<MBDB::ColumnSQLType> ColumnTypes;
		std::vector<MBDB::ColumnInfo> ColumnInfo;
		{
			std::lock_guard<std::mutex> ReadLock(m_ReadonlyDatabaseMutex);
			ColumnInfo = m_ReadonlyDatabase->GetColumnInfo(TableName);
		}
		for (size_t i = 0; i < ColumnIndexes.size(); i++)
		{
			ColumnTypes.push_back(ColumnInfo[ColumnIndexes[i]].ColumnType);
		}
		MBDB::SQLStatement StatementToExecute = m_WriteableDatabase->GetSQLStatement(SQLCommand);
		StatementToExecute.BindValues(ColumnValues, ColumnTypes, 0);
		std::vector<MBDB::MBDB_RowData> QuerryResult = m_WriteableDatabase->GetAllRows(StatementToExecute, &DataBaseError);

		//std::vector<MBDB::MBDB_RowData> QuerryResult = p_EvaluateBoundSQLStatement(SQLCommand, ColumnValues, ColumnIndex, TableName, &DataBaseError);
		if (DataBaseError)
		{
			ReturnValue = MBParsing::ParseJSONObject("{}",0,nullptr,nullptr);
		}
		else
		{
			Status = DataBaseError.ErrorMessage;
		}
		return(ReturnValue);
	}
	MBParsing::JSONObject MBSite_DBPlugin::p_API_QueryDatabase(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments, std::string& Status)
	{
		MBParsing::JSONObject ReturnValue = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		//
		std::string Query = DirectiveArguments.GetAttribute("Query").GetStringData();
		std::vector<MBDB::MBDB_RowData> RowData;
		MBError DatabaseError = true;
		{
			std::lock_guard<std::mutex> ReadLock(m_ReadonlyDatabaseMutex);
			MBDB::SQLStatement StatementToExecute = m_ReadonlyDatabase->GetSQLStatement(Query);
			RowData = m_ReadonlyDatabase->GetAllRows(StatementToExecute, &DatabaseError);
		}
		if (!DatabaseError)
		{
			Status = "Error executing query: " + DatabaseError.ErrorMessage;
		}
		else
		{
			//OBS förutsätter att querryn får plats i minnet
			Status = "ok";
			std::vector<MBParsing::JSONObject> Result;
			for (size_t i = 0; i < RowData.size(); i++)
			{
				Result.push_back(MBParsing::ParseJSONObject(ToJason(RowData[i]), 0, nullptr, nullptr));
			}
			ReturnValue["QueryResult"] = std::move(Result);
			ReturnValue["ColumnCount"] = MBParsing::JSONObject(intmax_t(Result.size()));
		}
		return(ReturnValue);
	}
	MBSAPI_DirectiveResponse MBSite_DBPlugin::HandleDirective(MBSiteUser const& AssociatedUser,std::string const& DirectiveName,MBParsing::JSONObject const& DirectiveArguments)
	{
		MBSAPI_DirectiveResponse ReturnValue;
		try
		{
			ReturnValue.Status = "ok";
			if (DirectiveName == "GetTableNames")
			{
				ReturnValue.DirectiveResponse = p_API_GetTableNames(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
			else if (DirectiveName == "AddEntryToTable")
			{
				ReturnValue.DirectiveResponse = p_API_AddEntryToTable(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
			else if (DirectiveName == "SearchTableWithWhere")
			{
				ReturnValue.DirectiveResponse = p_API_SearchTableWithWhere(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
			else if (DirectiveName == "GetTableInfo")
			{
				ReturnValue.DirectiveResponse = p_API_GetTableInfo(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
			else if (DirectiveName == "UpdateTableRow")
			{
				ReturnValue.DirectiveResponse = p_API_UpdateTableRow(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
			else if (DirectiveName == "QueryDatabase")
			{
				ReturnValue.DirectiveResponse = p_API_QueryDatabase(AssociatedUser, DirectiveArguments, ReturnValue.Status);
			}
		}
		catch(std::exception const& e)
		{
			//eventuell säkerhets risk om man lekar implemention info, men just nu är det mest för min egen del och för att göra skrivandet enklare
			ReturnValue.Status ="Uncaught exception when handling directive:"+ std::string(e.what());
			ReturnValue.DirectiveResponse = MBParsing::JSONObject();
		}
		return(ReturnValue);
	}
	std::vector<std::string>  MBSite_DBPlugin::HandledTopDirectories() const
	{
		std::vector<std::string> ReturnValue = { "DBSite"};
		return(ReturnValue);	
	}
	MrPostOGet::HTTPDocument MBSite_DBPlugin::p_HTTP_DBSite(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPServerSocket* ServerSocket)
	{
		MrPostOGet::HTTPDocument NewDocument;
		NewDocument.Type = MBMIME::MIMEType::HTML;
		//NewDocument.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBSite.html", AssociatedServer->GetResourcePath("mrboboget.se"));
        if (!(AssociatedUser.GeneralPermissions() & uint64_t(GeneralPermissions::Upload)))
        {
            NewDocument.DocumentData = GetSite().LoadFileWithPreProcessing(m_PluginID,"/InvalidPermissions.html");
            return(NewDocument);
        }
        NewDocument.DocumentData = GetSite().LoadFileWithPreProcessing(m_PluginID, "/DBSite.html");
		std::unordered_map<std::string, std::string> MapKeys = {};

		std::string QuerryString = "";
		if (Request.SearchParameters.find("SQLQuerry") != Request.SearchParameters.end())
		{
			QuerryString = Request.SearchParameters.at("SQLQuerry");
		}
		if (QuerryString == "")
		{
			MapKeys = { {"SQLResult",""} };
		}
		else
		{
			if (!(AssociatedUser.GeneralPermissions() & uint64_t(GeneralPermissions::View)))
			{
				MapKeys = { {"SQLResult","<p style=\"color:red; text-align:center\" class=\"center\">Error in SQL Querry: Require permissions to read</p>"} };
			}
			else
			{
				std::string SQLCommand = QuerryString;
				bool CommandSuccesfull = true;
				MBError SQLError(true);
				std::vector<MBDB::MBDB_RowData> SQLResult = {};
				{
					std::lock_guard<std::mutex> Lock(m_ReadonlyDatabaseMutex);
					SQLResult = m_ReadonlyDatabase->GetAllRows(SQLCommand, &SQLError);
					if (!SQLError)
					{
						CommandSuccesfull = false;
					}
				}
				if (CommandSuccesfull)
				{
					std::string TotalRowData = "";
					for (size_t i = 0; i < SQLResult.size(); i++)
					{
						MrPostOGet::HTMLNode NewRow = MrPostOGet::HTMLNode::CreateElement("tr");
						for (size_t j = 0; j < SQLResult[0].GetNumberOfColumns(); j++)
						{
							MrPostOGet::HTMLNode NewColumn = MrPostOGet::HTMLNode::CreateElement("td");
							NewColumn["style"] = "height: 100%";
							std::string ElementData = SQLResult[i].ColumnToString(j);
							if (h_StringIsPath(ElementData))
							{
								//ElementData = p_GetEmbeddedResource(ElementData, AssociatedServer->GetResourcePath("mrboboget.se"), ConnectionPermissions);
								ElementData = m_EmbeddPlugin->EmbeddResource(m_PluginID,AssociatedUser, FileLocationType::DBFile, ElementData).ToString();
							}
							NewColumn.AppendChild(MrPostOGet::HTMLNode(ElementData));
							NewRow.AppendChild(std::move(NewColumn));
						}
						TotalRowData += NewRow.ToString();
					}
					MapKeys["SQLResult"] = std::move(TotalRowData);
				}
				else
				{
					MapKeys = { {"SQLResult","<p style=\"color:red; text-align:center\" class=\"center\">Error in SQL Querry: " + SQLError.ErrorMessage + "</p>"} };
				}
			}
		}
		NewDocument.DocumentData = MrPostOGet::ReplaceMPGVariables(NewDocument.DocumentData, MapKeys);
		return(NewDocument);
	}
	MrPostOGet::HTTPDocument MBSite_DBPlugin::p_HTTP_DBAdd(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPServerSocket* ServerSocket)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		return(ReturnValue);
		ReturnValue.RequestStatus = MrPostOGet::HTTPRequestStatus::OK;
		ReturnValue.Type = MBMIME::MIMEType::HTML;
		if ((AssociatedUser.GeneralPermissions() & uint64_t(GeneralPermissions::View)) && (AssociatedUser.GeneralPermissions() & uint64_t(GeneralPermissions::List)))
		{

		}
		else
		{
			//MrPostOGet::HTMLNode NewNode = MrPostOGet::HTMLNode(;
			MrPostOGet::HTMLNode ErrorNode = MrPostOGet::HTMLNode::CreateElement("p");
			ErrorNode["id"] = "ViewDiv";
			ErrorNode["class"] = "center";
			ErrorNode["style"] = "color:red; text-align:center";
		}
	}
	MrPostOGet::HTTPDocument MBSite_DBPlugin::p_HTTP_DBUpdate(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPServerSocket* ServerSocket)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		return(ReturnValue);
	}
	MrPostOGet::HTTPDocument MBSite_DBPlugin::GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MBSiteUser const& AssociatedUser, MrPostOGet::HTTPServerSocket* ServerSocket)
	{
		MrPostOGet::HTTPDocument ReturnValue;
		std::vector<std::string> Directories = MBUtility::Split(Request.RequestResource, "/");
		std::string TopDirectory = Directories.at(1);
		if (TopDirectory == "DBSite")
		{
			ReturnValue = p_HTTP_DBSite(AssociatedUser, Request, ServerSocket);
		}
		else if (TopDirectory == "DBAdd")
		{
			ReturnValue = p_HTTP_DBAdd(AssociatedUser, Request, ServerSocket);
		}
		else if (TopDirectory == "DBUpdate")
		{
			ReturnValue = p_HTTP_DBUpdate(AssociatedUser, Request, ServerSocket);
		}
		return(ReturnValue); 
	}
	MBError MBSite_DBPlugin::LoadMBDBObject(PluginID CallingPlugin, MBSiteUser const& AssociatedUser, FileLocationType LocationType, std::string const& ObjectPath, MBDB::MBDB_Object& OutObject)
	{
		MBError ReturnValue = true;
		if (LocationType != FileLocationType::DBFile)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Invalid location type";
			return(ReturnValue);
		}
		std::string ObjectData;
		std::unique_ptr<MBUtility::MBSearchableInputStream> ObjectInput = nullptr;
		ReturnValue = GetSite().ReadFile(CallingPlugin, AssociatedUser, LocationType, ObjectPath, &ObjectInput);
		if (!ReturnValue)
		{
			return(ReturnValue);
		}
		ObjectData = h_ReadWholeInput(ObjectInput);
		MBDB::MBDB_Object NewObject = MBDB::MBDB_Object::ParseObject(ObjectData, 0, nullptr, &ReturnValue);
		if (ReturnValue)
		{
			OutObject = std::move(NewObject);
		}
		//TopPath += ObjectPath;
		//MBDB::MBDB_Object NewObject = MBDB::MBDB_Object()
		return(ReturnValue);
	}
	MBError MBSite_DBPlugin::EvaluateMBDBObject(PluginID CallingPlugin, MBSiteUser const& AssociatedUser, FileLocationType LocationType, std::string const& ObjectPath, MBDB::MBDB_Object& OutObject)
	{
		MBError ReturnValue = true;
		if (LocationType != FileLocationType::DBFile)
		{
			ReturnValue = false;
			ReturnValue.ErrorMessage = "Invalid location type";
			return(ReturnValue);
		}
		std::lock_guard<std::mutex> Lock(m_ReadonlyDatabaseMutex);
		MBDB::MBDBO_EvaluationInfo InfoToUse;
		InfoToUse.AssociatedDatabase = m_ReadonlyDatabase.get();
		InfoToUse.EvaluatingUser = "";
		InfoToUse.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(GetSite().GetResourceFolderPath() + ObjectPath).parent_path());
		OutObject.Evaluate(InfoToUse, &ReturnValue);
		return(ReturnValue);
	}
	//END MBSite_DBPlugin

	//BEGIN MBSite_FilesystemAPIPlugin
	std::string MBSite_FilesystemAPIPlugin::GetPluginName() const
	{
		return("MBFilesystemAPI");
	}
	void MBSite_FilesystemAPIPlugin::OnCreate(PluginID AssociatedID)
	{
		m_PluginID = AssociatedID;
		GetSite().RegisterMBSiteAPIHandler(m_PluginID,this);
	}
	void MBSite_FilesystemAPIPlugin::OnDestroy()
	{

	}
	MBSAPI_DirectiveResponse MBSite_FilesystemAPIPlugin::p_HandleFileExists(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments)
	{
		MBSAPI_DirectiveResponse ReturnValue;
		ReturnValue.DirectiveResponse = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		std::string const& FilePath = DirectiveArguments.GetAttribute("FilePath").GetStringData();
		MBError AccessError = GetSite().ReadFile(m_PluginID, AssociatedUser, FileLocationType::DBFile, FilePath, nullptr);
		if (AccessError)
		{
			ReturnValue.DirectiveResponse["FileExists"] = true;
			ReturnValue.DirectiveResponse["DirectoriesExists"] = true;
		}
		else
		{
			ReturnValue.DirectiveResponse["FileExists"] = false;

			//ganska fult och långsamt med it is what it is
			std::vector<std::string> DirectoriesComponent = MBUtility::Split(FilePath, "/");
			std::vector<std::string> NonEmptyDirectoryComponents = {};
			for (auto const& Component : DirectoriesComponent)
			{
				if (Component != "")
				{
					NonEmptyDirectoryComponents.push_back(Component);
				}
			}
			bool DirectoriesExists = true;
			std::string CurrentPath = "/";
			for (int i = 0; i < NonEmptyDirectoryComponents.size()-1; i++)
			{
				CurrentPath += NonEmptyDirectoryComponents[i];
				MBError Result = GetSite().ListDirectory(m_PluginID, AssociatedUser, FileLocationType::DBFile, CurrentPath, nullptr);
				if (!Result)
				{
					DirectoriesExists = false;
					break;
				}
			}
			ReturnValue.DirectiveResponse["DirectoriesExists"] = DirectoriesExists;
		}
		return(ReturnValue);
	}
	MBSAPI_DirectiveResponse MBSite_FilesystemAPIPlugin::p_HandleGetFolderContents(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments)
	{
		MBSAPI_DirectiveResponse ReturnValue;
		std::vector<FilesystemObjectInfo> ObjectInfo;
		ReturnValue.DirectiveResponse = MBParsing::JSONObject(std::map<std::string, MBParsing::JSONObject>());
		MBError AccessError = GetSite().ListDirectory(m_PluginID, AssociatedUser,FileLocationType::DBFile, DirectiveArguments.GetAttribute("DirectoryName").GetStringData(), &ObjectInfo);
		if (AccessError)
		{
			ReturnValue.Status= "ok";
			std::vector<MBParsing::JSONObject> DirectoryEntries = {};
			std::sort(ObjectInfo.begin(),ObjectInfo.end());
			for (auto const& Entry : ObjectInfo)
			{
				MBParsing::JSONObject NewEntry = MBParsing::JSONObject(std::map<std::string,MBParsing::JSONObject>());
				NewEntry["Name"] = MBParsing::JSONObject(Entry.Name);
				if (Entry.Type == FilesystemType::Directory)
				{
					NewEntry["Type"] = "Directory";
				}
				else 
				{
					NewEntry["Type"] = "File";
				}
				DirectoryEntries.push_back(std::move(NewEntry));
			}
			ReturnValue.DirectiveResponse["DirectoryEntries"] = MBParsing::JSONObject(std::move(DirectoryEntries));
		}
		else
		{
			ReturnValue.Status = AccessError.ErrorMessage;
		}
		return(ReturnValue);
	}
	std::vector<std::string> MBSite_FilesystemAPIPlugin::HandledDirectives() const
	{
		std::vector<std::string> ReturnValue = { "FileExists","GetFolderContents" };
		return(ReturnValue);
	}
	MBSAPI_DirectiveResponse MBSite_FilesystemAPIPlugin::HandleDirective(MBSiteUser const& AssociatedUser, std::string const& DirectiveName, MBParsing::JSONObject const& DirectiveArguments)
	{
		MBSAPI_DirectiveResponse ReturnValue;
		if (DirectiveName == "FileExists")
		{
			ReturnValue = p_HandleFileExists(AssociatedUser,DirectiveArguments);
		}
		else if (DirectiveName == "GetFolderContents")
		{
			ReturnValue = p_HandleGetFolderContents(AssociatedUser, DirectiveArguments);
		}
		return(ReturnValue);
	}
	//END MBSite_FilesystemAPIPlugin
}
