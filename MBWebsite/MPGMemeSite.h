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






















	//Nya api:t
	template<typename T>
	class PluginReference
	{
	private:
		T* m_PrivatePointer = nullptr;
	public:
		PluginReference() { };
		PluginReference(T* NewPointer)
		{
			m_PrivatePointer = NewPointer;
		}
		bool operator==(T* PointerToCompare) const
		{
			return(m_PrivatePointer == PointerToCompare);
		}
		bool operator!=(T* PointerToCompare) const
		{
			return(!(m_PrivatePointer == PointerToCompare));
		}
		T* operator->()
		{
			return(m_PrivatePointer);
		}
		const T* operator->() const
		{
			return(m_PrivatePointer);
		}
	};

	enum class GeneralPermissions
	{
		View = 1,
		List = 1<<1,
		Edit = 1<<2,
		Upload = 1<<3,
	};
	class MBSiteUser
	{
	private:
		friend class MBDB_Website;
		std::string m_Username;
		uint64_t m_GeneralPermissions = uint64_t(GeneralPermissions::View);
	public:
		std::string GetUsername() const
		{
			return(m_Username);
		}
		uint64_t GeneralPermissions() const
		{
			return(m_GeneralPermissions);
		}
	};

	enum class FilesystemType
	{
		Directory,
		File,
		Null,
	};
	struct FilesystemObjectInfo
	{
		//permissions?
		FilesystemType Type = FilesystemType::Null;
		std::string Name = "";
		bool operator<(FilesystemObjectInfo const& OtherInfo)
		{
			bool ReturnValue = false;
			if (uint64_t(Type) < uint64_t(OtherInfo.Type))
			{
				ReturnValue = true;
			}
			else
			{
				ReturnValue = Name < OtherInfo.Name;
			}
			return(ReturnValue);
		}
	};

	enum class FileLocationType
	{
		DBFile,
		GlobalStaticResource,
		PluginStaticResource,
		PluginFile,
		Null
	};


	typedef uint64_t PluginID;
	class MBSitePlugin
	{
	private:
		friend class MBDB_Website;
		MBDB_Website* m_AssociatedSite = nullptr;
	protected:
		MBDB_Website& GetSite() { return(*m_AssociatedSite); };
	public:
		//sker när den skapas, där kan det antas att GetSite fungerar
		virtual std::string GetPluginName() const = 0;
		virtual void OnCreate(PluginID AssociatedID) = 0;
		//Sker när den förstörsz
		virtual void OnDestroy() = 0;
		virtual ~MBSitePlugin()
		{

		}
	};
	struct MBSAPI_DirectiveResponse
	{
		std::string Status = ""; //OK stör för fungerande, allt annat för error
		MBParsing::JSONObject DirectiveResponse;
	};
	class MBSite_APIHandler
	{
	private:
	public:
		//uppmuntrar best practice, varje plugin directive är helt oberoende, men kan eventuellt ändra så att den tar in en user och komplett objekt för att avgöra vad den handalr
		virtual std::vector<std::string> HandledDirectives() const = 0;
		//Med stora datamängden kanske vi vill returna en input stream...
		virtual MBSAPI_DirectiveResponse HandleDirective(MBSiteUser const& AssociatedUser,std::string const& DirectiveName,MBParsing::JSONObject const& DirectiveArguments) = 0;
	};
	class MBSite_HTTPHandler
	{
	private:

	public:
		virtual std::vector<std::string>  HandledTopDirectories() const = 0;
		virtual MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const& Request,MBSiteUser const& AssociatedUser, MrPostOGet::HTTPServerSocket* ServerSocket) = 0;
	};

	//Cirkulär dependancy mellam MBEmbedder och DBPlugin men är helt enkelt how det do be just nu
	//class MBSite_DBPlugin;
	class MBSite_MBEmbedder : public MBSitePlugin
	{
	private:
		PluginID m_PluginID;

		std::string p_LoadStreamTemplate();

		std::string p_GetEmbeddedMBDBObject(std::string const& MBDBResource);
		std::string p_GetEmbeddedVideo(std::string const& VideoPath);
		std::string p_GetEmbeddedAudio(std::string const& VideoPath);
		std::string p_GetEmbeddedImage(std::string const& ImagePath);
		std::string p_GetEmbeddedPDF(std::string const& ImagePath);

	public:
		virtual std::string GetPluginName() const override;
		virtual void OnCreate(PluginID AssociatedID) override;
		virtual void OnDestroy() override;

		MrPostOGet::HTMLNode EmbeddResource(PluginID CallingPlugin,MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& Filepath);
	};

	class MBSite_DBPlugin : public MBSitePlugin, MBSite_APIHandler, MBSite_HTTPHandler
	{
	private:
		//dependant plugins
		PluginReference<MBSite_MBEmbedder> m_EmbeddPlugin;



		PluginID m_PluginID;
		std::mutex m_ReadonlyDatabaseMutex;
		std::unique_ptr<MBDB::MrBoboDatabase> m_ReadonlyDatabase = nullptr;
		std::mutex m_WriteableDatabaseMutex;
		std::unique_ptr<MBDB::MrBoboDatabase> m_WriteableDatabase = nullptr;


		bool p_DependanciesLoaded();
		void p_LoadDependancies();

		bool p_VerifyTableName(std::string const& TableNameToVerify);
		//Checkar att kolumnera har både rätt ordning och existerar i tabellen
		bool p_VerifyColumnName(std::string const& Table, std::vector<std::string> const& ColumnNames);

		//std::vector<MBDB::MBDB_RowData> p_EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
		//	std::vector<int> ColumnIndex, std::string TableName, MBError* OutError);

		MBParsing::JSONObject p_API_GetTableNames(MBSiteUser const& AssociatedUser,MBParsing::JSONObject const& ClientRequest,std::string& Status);
		MBParsing::JSONObject p_API_GetTableInfo(MBSiteUser const& AssociatedUser,MBParsing::JSONObject const& ClientRequest,std::string& Status);
		MBParsing::JSONObject p_API_QueryDatabase(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments,std::string& Status);
		MBParsing::JSONObject p_API_SearchTableWithWhere(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments,std::string& Status);
		MBParsing::JSONObject p_API_UpdateTableRow(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments,std::string& Status);
		MBParsing::JSONObject p_API_AddEntryToTable(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments,std::string& Status);

		MrPostOGet::HTTPDocument p_HTTP_DBSite(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request,MrPostOGet::HTTPServerSocket* ServerSocket);
		MrPostOGet::HTTPDocument p_HTTP_DBAdd(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request,MrPostOGet::HTTPServerSocket* ServerSocket);
		MrPostOGet::HTTPDocument p_HTTP_DBUpdate(MBSiteUser const& AssociatedUser, MrPostOGet::HTTPClientRequest const& Request,MrPostOGet::HTTPServerSocket* ServerSocket);
	public:
		virtual std::string GetPluginName() const override;
		virtual void OnCreate(PluginID AssociatedID) override;
		virtual void OnDestroy() override;
		virtual std::vector<std::string> HandledDirectives() const override; 
		virtual MBSAPI_DirectiveResponse HandleDirective(MBSiteUser const& AssociatedUser,std::string const& DirectiveName, MBParsing::JSONObject const& DirectiveArguments) override;
		virtual std::vector<std::string>  HandledTopDirectories() const;
		virtual MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MBSiteUser const& AssociatedUser, MrPostOGet::HTTPServerSocket* ServerSocket) override;

		MBError LoadMBDBObject(PluginID CallingPlugin, MBSiteUser const& AssociatedUser, FileLocationType LocationType, std::string const& ObjectPath,MBDB::MBDB_Object& OutObject);
		MBError EvaluateMBDBObject(PluginID CallingPlugin, MBSiteUser const& AssociatedUser, FileLocationType LocationType, std::string const& ObjectPath, MBDB::MBDB_Object& OutObject);
	};

	class MBSite_FilesystemAPIPlugin : public MBSitePlugin, MBSite_APIHandler
	{
	private:
		PluginID m_PluginID;

		MBSAPI_DirectiveResponse p_HandleFileExists(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments);
		MBSAPI_DirectiveResponse p_HandleGetFolderContents(MBSiteUser const& AssociatedUser, MBParsing::JSONObject const& DirectiveArguments);
	public:
		virtual std::string GetPluginName() const override;
		virtual void OnCreate(PluginID AssociatedID) override;
		virtual void OnDestroy() override;
		virtual std::vector<std::string> HandledDirectives() const override;
		virtual MBSAPI_DirectiveResponse HandleDirective(MBSiteUser const& AssociatedUser, std::string const& DirectiveName, MBParsing::JSONObject const& DirectiveArguments) override;
	};
	//class MBSite_DBViewPlugin : public MBSitePlugin, MBSite_APIHandler, MBSite_HTTPHandler
	//{
	//private:
	//public:
	//	virtual void OnCreate(PluginID AssociatedID) override;
	//	virtual void OnDestroy() override;
	//	virtual std::vector<std::string> HandledDirectives() const override;
	//	virtual MBParsing::JSONObject HandleDirective(MBParsing::JSONObject const& DirectiveObject) override;
	//	virtual std::vector<std::string>  HandledTopDirectories() const;
	//	virtual MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MBSiteUser const& AssociatedUser, MrPostOGet::HTTPServerSocket* ServerSocket) override;
	//};
	//
	//class MBSite_DBEditPlugin : public MBSitePlugin, MBSite_APIHandler, MBSite_HTTPHandler
	//{
	//private:
	//	PluginID m_PluginID;
	//	std::mutex m_ReadonlyDatabaseMutex;
	//	std::unique_ptr<MBDB::MrBoboDatabase> m_ReadonlyDatabase = nullptr;
	//	std::mutex m_WriteableDatabaseMutex;
	//	std::unique_ptr<MBDB::MrBoboDatabase> m_WriteableDatabase = nullptr;
	//public:
	//	virtual void OnCreate(PluginID AssociatedID) override;
	//	virtual void OnDestroy() override;
	//	virtual std::vector<std::string> HandledDirectives() const override;
	//	virtual MBParsing::JSONObject HandleDirective(MBParsing::JSONObject const& DirectiveObject) override;
	//	virtual std::vector<std::string>  HandledTopDirectories() const;
	//	virtual MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MBSiteUser const& AssociatedUser, MrPostOGet::HTTPServerSocket* ServerSocket) override;
	//};

	class MBDB_Website : public MrPostOGet::HTTPRequestHandler
	{
	private:
		friend class MBDB_Website_BaiscPasswordAuthenticator;

		PluginID m_CurrentID = 0;

		std::mutex m_PluginMapMutex;
		std::map<PluginID, std::unique_ptr<MBSitePlugin>> m_LoadedPlugins = {};
		std::mutex m_HTTPHandlersMutex;
		std::map<std::string, MBSite_HTTPHandler*> m_PluginHTTPHandlers = {};
		std::mutex m_APIHandlersMutex;
		std::map<std::string, MBSite_APIHandler*> m_PluginAPIHandlers = {};
		std::mutex m_PluginNameMutex;
		std::map<PluginID, std::string> m_PluginNames = {};


		//std::mutex m_ReadonlyMutex;
		//MBDB::MrBoboDatabase* m_ReadonlyDatabase = nullptr;
		//std::mutex WritableDatabaseMutex;
		//MBDB::MrBoboDatabase* WritableDatabase = nullptr;
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


		bool p_ObjectIsMBPlaylist(MBDB::MBDB_Object const& ObjectToCheck);
		std::string p_ViewMBDBPlaylist(MBDB::MBDB_Object const& EvaluatedObject, std::string const& HTMLFolder, DBPermissionsList const& Permissions);

		std::string p_ViewMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);
		std::string p_DBEdit_GetTextfileEditor(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);
		MBError p_DBEdit_Textfile_Update(MrPostOGet::HTTPClientRequest const& Request, std::string const& MBDBResource, DBPermissionsList const& Permissions);

		std::string p_GetEmbeddedMBDBObject(std::string const& MBDBResource, std::string const& HTMLFolder, DBPermissionsList const& Permissions);
		std::string p_GetEmbeddedVideo(std::string const& VideoPath, std::string const& WebsiteResourcePath);
		std::string p_GetEmbeddedAudio(std::string const& VideoPath, std::string const& WebsiteResourcePath);
		std::string p_GetEmbeddedImage(std::string const& ImagePath);
		std::string p_GetEmbeddedPDF(std::string const& ImagePath);
		std::string p_GetEmbeddedResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);


		std::string p_ViewResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);
		std::string p_EditResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions);


		//std::vector<MBDB::MBDB_RowData> EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
		//	std::vector<int> ColumnIndex, std::string TableName, MBError* OutError);
		//std::string GetTableNamesBody(std::vector<std::string> const& Arguments);
		//std::string GetTableInfoBody(std::vector<std::string> const& Arguments);
		bool DB_IndexExists(std::string const& IndexToCheck);

		std::string DBGeneralAPIGetDirective(std::string const& RequestBody);
		std::vector<std::string> DBGeneralAPIGetArguments(std::string const& RequestBody, MBError* OutError = nullptr);

		//std::string DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments);
		//std::string DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments);
		//std::string DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments);

		std::string DBAPI_GetFolderContents(std::vector<std::string> const& Arguments);
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

		//bool DBSite_Predicate(std::string const& RequestData);
		//MrPostOGet::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedSocket);

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



		std::mutex m_GlobalResourceFolderMutex;
		std::atomic<bool> m_GlobalResourceFolderLoaded{ false };
		std::string m_GlobalResourceFolder = "";
		std::string p_GetGlobalResourceFolder();
		std::string p_GetPluginResourceFolder(PluginID AssociatedPlugin);




	public:
		~MBDB_Website();
		//Implicit här att ett plugin bara kan ha en api handler och en request handler
		void AddPlugin(std::unique_ptr<MBSitePlugin> PluginToAdd);

		void RegisterMBSiteAPIHandler(PluginID const& AssociatedPlugin,MBSite_APIHandler* NewAPIHandler);
		void RegisterMBSiteHTTPRequestHandler(PluginID const& AssociatedPlugin, MBSite_HTTPHandler* RequestHandler);

		std::string _GetPluginFileAbsolutePath(PluginID const& AssociatedPlugin, std::string const& PluginFilePath);

		//Mest här för legacy anledningar innan jag vet hur api borde ersättas, letar automiskt igenom Plugin resources och GlobalResources
		//utgår också här att användare inte behövs för att verifieras
		std::string LoadFileWithPreProcessing(PluginID AssociatedPlugin, std::string const& Filepath);

		MBError ReadFile(PluginID AssociatedPlugin,MBSiteUser const& AssociatedUser,FileLocationType Location, std::string const& Filepath,std::unique_ptr<MBUtility::MBSearchableInputStream>* OutInStream);
		MBError WriteFile(PluginID AssociatedPlugin,MBSiteUser const& AssociatedUser, FileLocationType Location, std::string const& Filepath, 
			std::unique_ptr<MBUtility::MBSearchableOutputStream>* OutOutputStream);
		MBError ListDirectory(PluginID AssociatedPlugin,MBSiteUser const& AssociatedUser, FileLocationType Location,std::string const& DirectoryPath, std::vector<FilesystemObjectInfo>* OutInfo);
		
		template<typename T>
		PluginReference<T> GetPluginReference(PluginID const& AssociatedPluginID)
		{
			PluginReference<T> ReturnValue = PluginReference<T>(nullptr);
			{
				std::lock_guard<std::mutex> Lock(m_PluginMapMutex);
				for (auto const& Plugin : m_LoadedPlugins)
				{
					T* PluginPointer = dynamic_cast<T*>(Plugin.second.get());
					if (PluginPointer != nullptr)
					{
						ReturnValue = PluginReference<T>(PluginPointer);
						break;
					}
				}
			}
			return(ReturnValue);
		}

		//MBError ReadConfig(MBSiteUser const& AssociatedUser, std::string const& ConfigPath, std::ifstream* InputStream);

		[[deprecated]]
		std::string GetResourceFolderPath();
		MBDB_Website();
		bool HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer) override;
		MrPostOGet::HTTPDocument GenerateResponse(MrPostOGet::HTTPClientRequest const&, MrPostOGet::HTTPClientConnectionState const&, MrPostOGet::HTTPServerSocket*, MrPostOGet::HTTPServer*) override;
	};
	//bool p_StringIsExternalWebsite(std::string const& StringToCheck);
	//void InitDatabase();

	int MBGWebsiteMain();
}