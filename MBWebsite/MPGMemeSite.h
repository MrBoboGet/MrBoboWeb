#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrPostOGet/MrPostOGet.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <MBUtility/MBErrorHandling.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
#include <atomic>
#include <MBUtility/MBInterfaces.h>
#include <MBPacketManager/MBPacketManager.h>

namespace MBWebsite
{
	struct DBPermissionsList
	{
		std::string AssociatedUser = "";
		bool Read = false;
		bool Edit = false;
		bool Upload = false;
		bool IsNull = true;
	};
	class MBDB_Website;
	struct ModernRequestHandler
	{
		bool (MBDB_Website::* Predicate)(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer);
		MrPostOGet::HTTPDocument(MBDB_Website::* Generator)(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*);
	};
	struct LegacyRequestHandler
	{
		bool (MBDB_Website::* Predicate)(std::string const&);
		MrPostOGet::HTTPDocument(MBDB_Website::* Generator)(std::string const&, MrPostOGet::HTTPServer*, MrPostOGet::HTTPServerSocket*);
	};


	class MBDB_Website_BaiscPasswordAuthenticator : public MBUtility::MBBasicUserAuthenticator
	{
	private:
		MBDB_Website* m_AssociatedServer = nullptr;
	public:
		MBDB_Website_BaiscPasswordAuthenticator(MBDB_Website* AssociatedServer);
		virtual bool VerifyUser(std::string const& Username, std::string const& Password) override;
	};

	class MBDB_Website_GitHandler : public MrPostOGet::HTTPRequestHandler
	{
	private:
		std::mutex m_InternalsMutex;
		std::string m_TopLevelDirectory = "";
		std::string m_URLPrefix = "";

		std::mutex m_UploadMutex;

		MBUtility::MBBasicUserAuthenticator* m_UserAuthenticator = nullptr;

		void p_SetGCIVariables(MrPostOGet::HTTPClientRequest const& AssociatedRequest);
		MrPostOGet::HTTPDocument p_GetAuthenticationPrompt(MrPostOGet::HTTPClientRequest const& AssociatedRequest);
		bool p_VerifyAuthentication(MrPostOGet::HTTPClientRequest const& AssociatedRequest);
	public:
		MBDB_Website_GitHandler(std::string const& TopResourceDirectory, MBUtility::MBBasicUserAuthenticator* Authenticator);
		void SetURLPrefix(std::string const& PathPrefix);
		void SetTopDirectory(std::string const& DirectoryToSet);

		bool HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer) override;
		MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*) override;
	};

	struct MBPP_UploadPacketTuple
	{
		std::string UpdatedPacket = "";
		MBPM::MBPP_ComputerInfo ComputerInfo;
		std::vector<std::string> RemovedObjects = {};
	};
	class MBDB_Website_MBPP_UploadIncorporator : public MBPM::MBPP_UploadChangesIncorporator
	{
	private:
	public:
		std::vector<MBPP_UploadPacketTuple> UpdatedPackets = {};
		void IncorporatePacketChanges(std::string const& UpdatedPacket, MBPM::MBPP_ComputerInfo ComputerInfo, std::vector<std::string> const& RemovedObjects) override;
		virtual ~MBDB_Website_MBPP_UploadIncorporator() override
		{

		}
	};
	class MBDB_Website_MBPP_Handler : public MrPostOGet::HTTPRequestHandler
	{
	private:
		std::string m_PacketsDirectory = "";
		MBUtility::MBBasicUserAuthenticator* m_UserAuthenticator = nullptr;
		MBDB_Website_MBPP_UploadIncorporator m_UploadIncorporator;
		//TODO fixa separat upload/write mutex s� man l�sa samtidigt s� l�nge ingen annan f�rs�ker ladda upp
		std::mutex m_WriteMutex;

		std::string p_GetComputerDiffTopDirectory(std::string const& PacketName, MBPM::MBPP_ComputerInfo ComputerInfo);
		void p_IncorporatePacketChanges(MBPM::MBPP_ComputerInfo ComputerInfo,std::string const& UpdatedPacket, std::vector<std::string> const& RemovedObjects);
	public:
		//void AddPacketDirectory(std::string const& NewDirectory);
		MBDB_Website_MBPP_Handler(std::string const& PacketDirectory, MBUtility::MBBasicUserAuthenticator* Authenticator);
		bool HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer) override;
		MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*) override;
	};

	class MBDB_Website : public MrPostOGet::HTTPRequestHandler
	{
	private:
		friend class MBDB_Website_BaiscPasswordAuthenticator;

		std::mutex m_ReadonlyMutex;
		MBDB::MrBoboDatabase* m_ReadonlyDatabase = nullptr;
		std::mutex WritableDatabaseMutex;
		MBDB::MrBoboDatabase* WritableDatabase = nullptr;
		std::mutex m_LoginDatabaseMutex;
		MBDB::MrBoboDatabase* m_LoginDatabase = nullptr;
		std::mutex __DBIndexMapMutex;
		std::unordered_map<std::string, MBSearchEngine::MBIndex>* __DBIndexMap = nullptr;
		std::mutex __MBTopResourceFolderMutex;
		std::string __MBTopResourceFolder = "./";
		void m_InitDatabase();

		std::unique_ptr<MBUtility::MBBasicUserAuthenticator> m_BasicPasswordAuthenticator = nullptr;

		std::string p_GetTimestamp();
		bool p_StringIsPath(std::string const& StringToCheck);
		std::vector<MBDB::MBDB_RowData> m_GetUser(std::string const& UserName, std::string const& PasswordHash);
		DBPermissionsList m_GetConnectionPermissions(std::string const& RequestData);


		static std::string m_GetUsername(std::string const& RequestData);
		static std::string sp_GetPassword(std::string const& RequestData);


		MBDB::MBDB_Object p_LoadMBDBObject(std::string const& ValidatedUser, std::string const& ObjectPath, MBError* OutError);

		std::string p_GetEmbeddedMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);

		bool p_ObjectIsMBPlaylist(MBDB::MBDB_Object const& ObjectToCheck);
		std::string p_ViewMBDBPlaylist(MBDB::MBDB_Object const& EvaluatedObject, std::string const& HTMLFolder, DBPermissionsList const& Permissions);

		std::string p_ViewMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);
		std::string p_DBEdit_GetTextfileEditor(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);
		MBError p_DBEdit_Textfile_Update(MrPostOGet::HTTPClientRequest const& Request, std::string const& MBDBResource, DBPermissionsList const& Permissions);

		std::string p_GetEmbeddedVideo(std::string const& VideoPath, std::string const& WebsiteResourcePath);
		std::string p_GetEmbeddedAudio(std::string const& VideoPath, std::string const& WebsiteResourcePath);
		std::string p_GetEmbeddedImage(std::string const& ImagePath);
		std::string p_GetEmbeddedPDF(std::string const& ImagePath);


		std::string p_GetEmbeddedResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);
		std::string p_ViewResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);
		std::string p_EditResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);


		std::vector<MBDB::MBDB_RowData> EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
			std::vector<int> ColumnIndex, std::string TableName, MBError* OutError);
		std::string GetTableNamesBody(std::vector<std::string> const& Arguments);
		std::string GetTableInfoBody(std::vector<std::string> const& Arguments);
		bool DB_IndexExists(std::string const& IndexToCheck);

		std::string DBGeneralAPIGetDirective(std::string const& RequestBody);
		std::vector<std::string> DBGeneralAPIGetArguments(std::string const& RequestBody, MBError* OutError = nullptr);

		std::string DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments);
		std::string DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments);
		std::string DBAPI_GetFolderContents(std::vector<std::string> const& Arguments);
		std::string DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments);
		std::string DBAPI_Login(std::vector<std::string> const& Arguments);
		std::string DBAPI_GetAvailableIndexes(std::vector<std::string> const& Arguments);
		std::string DBAPI_GetIndexSearchResult(std::vector<std::string> const& Arguments);

		std::mutex m_BlippFileMutex;
		std::string DBAPI_GetBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions);
		std::string DBAPI_UploadBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions);
		std::string DBAPI_UnlockBlippFile(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions);
		std::string DBAPI_UploadBlippBugReport(std::vector<std::string> const& Arguments, DBPermissionsList const& UserPermissions);

		std::vector<LegacyRequestHandler> __LegacyRequestHandlers = {};
		std::atomic<size_t> __NumberOfHandlers{ 0 };
		std::atomic<LegacyRequestHandler*> __HandlersData{ nullptr };

		std::string m_MBDBResourceFolder = "";

		//Modern handlers
		std::vector<ModernRequestHandler> __ModernRequestHandlers = {};
		std::atomic<size_t> __ModernHandlersCount{ 0 };
		std::atomic<ModernRequestHandler*> __ModernHandlersData{ nullptr };

		//InternalHandlers Handlers
		//std::vector<std::unique_ptr<MrPostOGet::HTTPRequestHandler>> __InternalHandlers = {};
		//std::atomic<size_t> __InternalHandlersCount{ 0 };
		//std::atomic<std::unique_ptr<MrPostOGet::HTTPRequestHandler>*> __InternalHandlersData{ nullptr };
		std::unique_ptr<MBDB_Website_GitHandler> m_GitHandler = nullptr;
		std::unique_ptr<MBDB_Website_MBPP_Handler> m_MPPHandler = nullptr;

		bool p_Edit_Predicate(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer);
		MrPostOGet::HTTPDocument p_Edit_ResponseGenerator(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*);

		bool DBSite_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedSocket);

		bool UploadFile_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument UploadFile_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBGet_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBView_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBViewEmbedd_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBAdd_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBGeneralAPI_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBUpdate_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBLogin_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		bool DBOperationBlipp_Predicate(std::string const& RequestData);
		MrPostOGet::HTTPDocument DBOperatinBlipp_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection);

		~MBDB_Website();
	public:
		std::string GetResourceFolderPath();
		MBDB_Website();
		bool HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer) override;
		MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*) override;
	};
	//bool p_StringIsExternalWebsite(std::string const& StringToCheck);
	//void InitDatabase();

	int MBGWebsiteMain();
}