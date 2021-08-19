#define NOMINMAX
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MinaStringOperations.h>
#include <MBSearchEngine/MBSearchEngine.h>
#include <filesystem>
#include <MBSearchEngine/MBUnicode.h>
#include <MrBoboDatabase/MBDBObjectScript.h>
#include <MBMime/MBMime.h>
#include <ctime>
#include <chrono>
//username cookie = 
//password cookie = 
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
	if (m_ReadonlyDatabase == nullptr)
	{
		m_ReadonlyDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/TestDatabas", 0);
	}
	if (WritableDatabase == nullptr)
	{
		WritableDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/TestDatabas", 1);
	}
	if (m_LoginDatabase == nullptr)
	{
		m_LoginDatabase = new MBDB::MrBoboDatabase(__MBTopResourceFolder + "/MBGLoginDatabase", 0);
	}
	//läser in mbdb config filen och initaliserar directoryn med rätt
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
	delete m_ReadonlyDatabase;
	delete WritableDatabase;
	delete m_LoginDatabase;
	delete __DBIndexMap;
}

MBDB_Website::MBDB_Website()
{
	m_InitDatabase();
	__LegacyRequestHandlers = {
		{ &MBDB_Website::DBLogin_Predicate,				&MBDB_Website::DBLogin_ResponseGenerator },
		{ &MBDB_Website::DBSite_Predicate,				&MBDB_Website::DBSite_ResponseGenerator },
		{ &MBDB_Website::UploadFile_Predicate,			&MBDB_Website::UploadFile_ResponseGenerator },
		{ &MBDB_Website::DBGet_Predicate,				&MBDB_Website::DBGet_ResponseGenerator },
		{ &MBDB_Website::DBView_Predicate,				&MBDB_Website::DBView_ResponseGenerator },
		{ &MBDB_Website::DBViewEmbedd_Predicate,		&MBDB_Website::DBViewEmbedd_ResponseGenerator },
		{ &MBDB_Website::DBAdd_Predicate,				&MBDB_Website::DBAdd_ResponseGenerator },
		{ &MBDB_Website::DBGeneralAPI_Predicate,		&MBDB_Website::DBGeneralAPI_ResponseGenerator },
		{ &MBDB_Website::DBUpdate_Predicate,			&MBDB_Website::DBUpdate_ResponseGenerator },
		{ &MBDB_Website::DBOperationBlipp_Predicate,	&MBDB_Website::DBOperatinBlipp_ResponseGenerator } };
	__NumberOfHandlers.store(__LegacyRequestHandlers.size());
	__HandlersData.store(__LegacyRequestHandlers.data());
	__ModernRequestHandlers = { {&MBDB_Website::p_Edit_Predicate,&MBDB_Website::p_Edit_ResponseGenerator} };
	__ModernHandlersCount.store(__ModernRequestHandlers.size());
	__ModernHandlersData.store(__ModernRequestHandlers.data());
}
bool MBDB_Website::HandlesRequest(MrPostOGet::HTTPClientRequest const& RequestToHandle, MrPostOGet::HTTPClientConnectionState const& ConnectionState, MrPostOGet::HTTPServer* AssociatedServer)
{
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
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::GenerateResponse(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const& ConnectionState
	, MBSockets::HTTPServerSocket* Connection, MrPostOGet::HTTPServer* Server)
{
	LegacyRequestHandler* HandlersData = __HandlersData.load();
	for (size_t i = 0; i < __NumberOfHandlers.load(); i++)
	{
		if ((this->*(HandlersData[i].Predicate))(Request.RawRequestData))
		{
			return((this->*(HandlersData[i].Generator))(Request.RawRequestData,Server,Connection));
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
	assert(false);
}

std::string MBDB_Website::GetResourceFolderPath()
{
	std::lock_guard<std::mutex> Lock(__MBTopResourceFolderMutex);
	return(m_MBDBResourceFolder);
	//return(__MBTopResourceFolder + "/MBDBResources/");
}
void InitDatabase()
{
	//utgår från den foldern som programmet körs i
}
int MBGWebsiteMain()
{
#ifndef NDEBUG
	std::cout << "Is Debug" << std::endl;
#endif // DEBUG
	MBSockets::Init();

	MrPostOGet::HTTPServer TestServer("./ServerResources/mrboboget.se/HTMLResources/", 443);
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
		MBDB::SQLStatement* NewStatement = m_LoginDatabase->GetSQLStatement(SQLStatement);
		NewStatement->BindString(UserName, 1);
		NewStatement->BindString(PasswordHash, 2);
		QuerryResult = m_LoginDatabase->GetAllRows(NewStatement);
		NewStatement->FreeData();
		m_LoginDatabase->FreeSQLStatement(NewStatement);
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
		std::tuple<int, std::string, std::string, int, int, int> UserInfo = QuerryResult[0].GetTuple<int, std::string, std::string, int, int, int>();
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

	ReturnValue = MBUtility::ReplaceAll(std::string(buf),":","_");
	
	return(ReturnValue);
}
bool MBDB_Website::p_StringIsPath(std::string const& StringToCheck)
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
bool MBDB_Website::DBLogin_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBLogin")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string ServerResources = AssociatedServer->GetResourcePath("mrboboget.se");
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::unordered_map<std::string, std::string> FileVariables = {};
	
	std::string RequestUsername = m_GetUsername(RequestData);
	std::string RequestPassword = sp_GetPassword(RequestData); 
	FileVariables["LoginValue"] = RequestUsername;
	if (m_GetUser(RequestUsername,RequestPassword).size() != 0)
	{
		FileVariables["LoginValue"] = "Currently logged in as: " + FileVariables["LoginValue"];
	}
	ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(ServerResources + "DBLogin.html", ServerResources),FileVariables);
	return(ReturnValue);
}

bool MBDB_Website::DBSite_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >=1)
	{
		if (Directorys[0] == "DBSite")
		{
			return(true);
		}
	}
	return(false);
}

MBSockets::HTTPDocument MBDB_Website::DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	MBSockets::HTTPDocument NewDocument;
	NewDocument.Type = MBSockets::HTTPDocumentType::HTML;
	NewDocument.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBSite.html", AssociatedServer->GetResourcePath("mrboboget.se"));
	std::unordered_map<std::string, std::string> MapKeys = {};
	
	std::string QuerryString = "";
	std::string RequestURL = MBSockets::GetReqestResource(RequestData);
	size_t QuerryStringPosition = RequestURL.find("?SQLQuerry=");
	if (QuerryStringPosition != RequestURL.npos)
	{
		QuerryString =RequestURL.substr(QuerryStringPosition + 11);
	}
	if (QuerryString == "")
	{
		MapKeys = { {"SQLResult",""} };
	}
	else
	{
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		if (!ConnectionPermissions.Read)
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
				std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
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
						if (p_StringIsPath(ElementData))
						{
							ElementData = p_GetEmbeddedResource(ElementData, AssociatedServer->GetResourcePath("mrboboget.se"), ConnectionPermissions);
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
				MapKeys = { {"SQLResult","<p style=\"color:red; text-align:center\" class=\"center\">Error in SQL Querry: "+ SQLError.ErrorMessage+"</p>"} };
			}
		}
	}
	NewDocument.DocumentData = MrPostOGet::ReplaceMPGVariables(NewDocument.DocumentData, MapKeys);
	return(NewDocument);
}

bool MBDB_Website::UploadFile_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "UploadFile" && MBSockets::GetRequestType(RequestData) == "POST") 
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::UploadFile_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//Content-Type: multipart/form-data; boundary=---------------------------226143532618736951363968904467
	DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
	MBSockets::HTTPDocument NewDocument;
	NewDocument.Type = MBSockets::HTTPDocumentType::json;
	std::string Boundary = "";
	std::vector<std::string> ContentTypes = MBSockets::GetHeaderValues("Content-Type", RequestData);
	std::string FormType = "multipart/form-data";
	std::string BoundaryHeader = "; boundary=";
	if (!ConnectionPermissions.Upload)
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permission to upload\"}";
		NewDocument.RequestStatus = MBSockets::HTTPRequestStatus::Conflict;
		AssociatedConnection->Close();
		return(NewDocument);
	}
	for (size_t i = 0; i < ContentTypes.size(); i++)
	{
		if (ContentTypes[i].substr(0, FormType.size()) == FormType)
		{
			Boundary = ContentTypes[i].substr(FormType.size() + BoundaryHeader.size(),ContentTypes[i].size()-FormType.size()-BoundaryHeader.size()-1);
			break;
		}
	}
	if (Boundary == "")
	{
		assert(false);
	}
	size_t FirstBoundaryLocation = RequestData.find(Boundary);
	size_t FirstFormParameterLocation = RequestData.find(Boundary, FirstBoundaryLocation + Boundary.size())+Boundary.size()+2;
	size_t EndOfFirstParameters = RequestData.find("\r\n",FirstFormParameterLocation);
	std::string FieldParameters = RequestData.substr(FirstFormParameterLocation, EndOfFirstParameters - FirstFormParameterLocation);
	//hardcodat eftersom vi vet formtatet av formuläret
	std::vector<std::string> FirstFieldValues = Split(FieldParameters, "; ");
	std::string FileNameHeader = "filename=\"";
	std::string FileName =GetResourceFolderPath()+ FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
	int FilesWithSameName = 0;
	while(std::filesystem::exists(FileName))
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"FileAlreadyExists\"}";
		NewDocument.RequestStatus = MBSockets::HTTPRequestStatus::Conflict;
		//TODO close makar inte mycket sense för svaret kommer inte skickas, borde istället finnas sett extra options som är "close after send"
		AssociatedConnection->Close();
		return(NewDocument);
	}
	int FileDataLocation = RequestData.find("\r\n\r\n", FirstFormParameterLocation) + 4;
	std::ofstream NewFile(FileName, std::ios::out | std::ios::binary);
	int TotalDataWritten = RequestData.size()- FileDataLocation;
	NewFile.write(&RequestData[FileDataLocation], RequestData.size() - FileDataLocation);
	clock_t WriteTimer = clock();
	std::string NewData;
	while(AssociatedConnection->DataIsAvailable())
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
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DB")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string DatabaseResourcePath = GetResourceFolderPath();
	std::string URLResource = MBSockets::GetReqestResource(RequestData);
	std::string DatabaseResourceToGet = URLResource.substr(URLResource.find_first_of("DB/") + 3);
	if (!std::filesystem::exists(DatabaseResourcePath+DatabaseResourceToGet))
	{
		MBSockets::HTTPDocument Invalid;
		Invalid.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
		Invalid.Type = MBSockets::HTTPDocumentType::HTML;
		Invalid.DocumentData = "File not found";
		return(Invalid);
	}
	std::string RangeData = MBSockets::GetHeaderValue("Range", RequestData);
	std::string IntervallsData = RangeData.substr(RangeData.find_first_of("=") + 1);
	ReplaceAll(&IntervallsData, "\r", "");
	ReplaceAll(&IntervallsData, "\n", "");
	std::vector<FiledataIntervall> ByteIntervalls = {};
	if (RangeData != "")
	{
		std::vector<std::string> Intervalls = Split(ReplaceAll(IntervallsData," ",""), ",");
		for (int i = 0; i < Intervalls.size(); i++)
		{
			FiledataIntervall NewIntervall = { uint64_t(-1),uint64_t(-1) };
			std::vector<std::string> IntervallNumbers = Split(Intervalls[i], "-");
			if (IntervallNumbers[0] != "")
			{
				NewIntervall.FirstByte = std::stoll(IntervallNumbers[0]);
			}
			if (IntervallNumbers[1] != "")
			{
				NewIntervall.FirstByte = std::stoll(IntervallNumbers[1]);
			}
			ByteIntervalls.push_back(NewIntervall);
		}
	}


	MBSockets::HTTPDocument ReturnValue = AssociatedServer->GetResource(DatabaseResourcePath+DatabaseResourceToGet,ByteIntervalls);
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
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
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
	std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
	MBError EvaluationResult = ReturnValue.LoadObject(ObjectPath, ValidatedUser, m_ReadonlyDatabase);
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
	NodeToEmbedd["href"] = "/DBView/"+MBDBResource;

	if (EvaluationError)
	{
		if (ObjectToEmbedd.HasAttribute("Banner"))
		{
			MBDB::MBDB_Object& BannerObject = ObjectToEmbedd.GetAttribute("Banner");
			if (!BannerObject.IsEvaluated())
			{
				std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
				MBDB::MBDBO_EvaluationInfo EvaluationInfo;
				EvaluationInfo.AssociatedDatabase = m_ReadonlyDatabase;
				EvaluationInfo.EvaluatingUser = Permissions.AssociatedUser;
				EvaluationInfo.ObjectDirectory = MBUnicode::PathToUTF8(std::filesystem::path(GetResourceFolderPath() + MBDBResource).parent_path());
				BannerObject.Evaluate(EvaluationInfo, &EvaluationError);
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
std::string MBDB_Website::p_ViewMBDBPlaylist(MBDB::MBDB_Object const& EvaluatedObject, std::string const& HTMLFolder,DBPermissionsList const& Permissions)
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
			//en song består av en path till den, samt namn
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
					ReturnValue = p_ViewMBDBPlaylist(ObjectToView, HTMLFolder,Permissions);
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

	//Post request med enbart 1 värde, den nya texten
	//ANTAGANDE all data som skickas är här, blir fel annars
	//std::string DataToParse = reqe

	MBMIME::MIMEMultipartDocumentExtractor DocumentExtractor(Request.RawRequestData.data(),Request.RawRequestData.size(),0);
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
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	if (ResourceExtension != "mbdbo")
	{
		ReturnValue = p_GetEmbeddedResource(MBDBResource, ResourceFolder, Permissions);
	}
	else
	{
		ReturnValue = p_ViewMBDBObject(MBDBResource,ResourceFolder, Permissions);
	}
	return(ReturnValue);
}
std::string MBDB_Website::p_EditResource(std::string const& MBDBResource, std::string const& ResourceFolder, DBPermissionsList const& Permissions)
{
	std::string ReturnValue = "";
	std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
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
std::string MBDB_Website::p_GetEmbeddedResource(std::string const& MBDBResource,std::string const& ResourceFolder,DBPermissionsList const& Permissions)
{
	std::string ReturnValue = "";
	std::string ResourceExtension = MBDBResource.substr(MBDBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	bool IsMBDBResource = true;
	if (IsMBDBResource)
	{
		if (!std::filesystem::exists(GetResourceFolderPath() + MBDBResource))
		{
			return("<p>File does not exist<p>");
		}
	}
	if (ResourceMedia == MBSockets::MediaType::Image)
	{
		ReturnValue = p_GetEmbeddedImage(MBDBResource);
	}
	else if (ResourceMedia == MBSockets::MediaType::Video)
	{
		ReturnValue = p_GetEmbeddedVideo(MBDBResource, ResourceFolder);
	}
	else if (ResourceMedia == MBSockets::MediaType::Audio)
	{
		ReturnValue = p_GetEmbeddedAudio(MBDBResource, ResourceFolder);
	}
	else if (ResourceMedia == MBSockets::MediaType::PDF)
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
	//TODO borde det här helt enkelt vara en del av parsingen istället?
	std::string NormalizedPath = RequestToHandle.RequestResource;
	if (NormalizedPath.substr(0, 8) == "/DBEdit/" && NormalizedPath.size() > 8) //vi vill inte deala med att scrolal till dem nu
	{
		return(true);
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::p_Edit_ResponseGenerator(MrPostOGet::HTTPClientRequest const& Request, MrPostOGet::HTTPClientConnectionState const&, MBSockets::HTTPServerSocket*, MrPostOGet::HTTPServer* Server)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
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
MBSockets::HTTPDocument MBDB_Website::DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string EmbeddedElement = "";

	std::string HandlerName = "DBView";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = GetResourceFolderPath();
	std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
	std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
	if (!std::filesystem::exists(DBResourcesPath+DBResource))
	{
		MBSockets::HTTPDocument Invalid;
		Invalid.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
		Invalid.Type = MBSockets::HTTPDocumentType::HTML;
		Invalid.DocumentData = "File not found";
		return(Invalid);
	}
	if (!std::filesystem::is_directory(DBResourcesPath + DBResource) && DBResource != "")
	{
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
		//EmbeddedElement = p_GetEmbeddedResource(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		EmbeddedElement = p_ViewResource(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"),ConnectionPermissions);
		std::unordered_map<std::string, std::string> MapData = {};
		MapData["EmbeddedMedia"] = EmbeddedElement;
		std::string HTMLResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
		ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "DBViewTemplate.html", HTMLResourcePath), MapData);
	}
	else
	{
		ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
		ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se")+"DBViewFolder.html", AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	//= AssociatedServer->GetResource(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBViewTemplate.html");
	return(ReturnValue);
}

bool MBDB_Website::DBViewEmbedd_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBViewEmbedd")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue = MBSockets::HTTPDocument();
	std::string HandlerName = "DBViewEmbedd/";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = GetResourceFolderPath();
	std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
	std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	ReturnValue.Type = MBSockets::DocumentTypeFromFileExtension(ResourceExtension);
	if (ResourceMedia == MBSockets::MediaType::Image)
	{
		ReturnValue.DocumentData = p_GetEmbeddedImage(DBResource);
	}
	else if (ResourceMedia == MBSockets::MediaType::Video)
	{
		ReturnValue.DocumentData = p_GetEmbeddedVideo(DBResource,AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	else if (ResourceMedia == MBSockets::MediaType::Audio)
	{
		ReturnValue.DocumentData = p_GetEmbeddedAudio(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	return(ReturnValue);
}

bool MBDB_Website::DBAdd_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBAdd")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	//std::string TableName = RequestData.substr(RequestResource.find("DBAdd/") + 6);
	//std::vector < std::string> ExistingTableNames = {};
	//låter all denna kod köras i javascript, blir det enklaste
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath+"DBAdd.html", ResourcePath);
	return(ReturnValue);
}

bool MBDB_Website::DBGeneralAPI_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
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
	std::string APIDirective = RequestBody.substr(0, FirstSpace);
	return(APIDirective);
}
std::vector<std::string> MBDB_Website::DBGeneralAPIGetArguments(std::string const& RequestBody,MBError* OutError)
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
		}
		ReturnValue.push_back(RequestBody.substr(SizeEnd + 1, ArgumentSize));
		ParsePosition = SizeEnd + 1 + ArgumentSize + 1;
	}
	//std::vector<std::string> ReturnValue = Split(RequestBody.substr(FirstSpace + 1),",");
	return(ReturnValue);
}


std::string MBDB_Website::GetTableNamesBody(std::vector<std::string> const& Arguments)
{
	std::vector<std::string> TableNames = {};
	{
		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
		TableNames = m_ReadonlyDatabase->GetAllTableNames();
	}
	std::string JSONTableNames = "\"TableNames\":"+MakeJasonArray(TableNames);
	std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\"," + JSONTableNames + "}";
	return(JsonResponse);
}
std::string MBDB_Website::GetTableInfoBody(std::vector<std::string> const& Arguments)
{
	//första argumentet är tablen vi vill ha
	if (Arguments.size() == 0)
	{
		return("");
	}
	std::vector<MBDB::ColumnInfo> TableInfo = {};
	{
		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
		TableInfo = m_ReadonlyDatabase->GetColumnInfo(Arguments[0]);
	}
	std::string JSONResponse = "{\"MBAPI_Status\":\"ok\",\"TableInfo\":" + MakeJasonArray(TableInfo) + "}";
	return(JSONResponse);
}
inline long long StringToInt(std::string const& IntData, MBError* OutError = nullptr)
{
	long long ReturnValue = 0;
	try
	{
		ReturnValue = std::stoi(IntData);
	}
	catch (const std::exception&)
	{
		*OutError = MBError(false);
		OutError->ErrorMessage = "Failed to parse int";
	}
	return(ReturnValue);
}
std::vector<MBDB::MBDB_RowData> MBDB_Website::EvaluateBoundSQLStatement(std::string SQLCommand, std::vector<std::string> const& ColumnValues,
	std::vector<int> ColumnIndex,std::string TableName,MBError* OutError)
{
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
	MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);

	std::vector<MBDB::ColumnInfo> TableColumnInfo = WritableDatabase->GetColumnInfo(TableName);
	for (size_t i = 0; i < ColumnValues.size(); i++)
	{
		if (TableColumnInfo[ColumnIndex[i]].ColumnType == MBDB::ColumnSQLType::Int)
		{
			MBDB::MaxInt NewInt = StringToInt(ColumnValues[i], OutError);
			if (!OutError)
			{
				break;
			}
			*OutError = NewStatement->BindInt(NewInt, i + 1);
		}
		else
		{
			*OutError = NewStatement->BindString(ColumnValues[i], i + 1);
			if (!OutError)
			{
				break;
			}
		}
	}
	if (OutError)
	{
		QuerryResult = WritableDatabase->GetAllRows(NewStatement, OutError);
	}
	NewStatement->FreeData();
	WritableDatabase->FreeSQLStatement(NewStatement);
	return(QuerryResult);
}
std::string MBDB_Website::DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	std::string SQLCommand = "INSERT INTO " + Arguments[0] + "("; //VALUES (";
	std::vector<std::string> ColumnNames = {};
	std::vector<std::string> ColumnValues = {};
	std::vector<int> ColumnIndex = {};
	MBError DataBaseError(true);


	for (size_t i = 1; i < Arguments.size(); i++)
	{
		size_t FirstColon = Arguments[i].find_first_of(":");
		size_t SecondColon = Arguments[i].find(":", FirstColon + 1);
		size_t NewColumnIndex = -1;
		NewColumnIndex = StringToInt(Arguments[i].substr(FirstColon + 1, SecondColon - FirstColon),&DataBaseError);
		if (!DataBaseError)
		{
			ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
			return(ReturnValue);
		}
		ColumnNames.push_back(Arguments[i].substr(0, FirstColon));
		ColumnValues.push_back(Arguments[i].substr(SecondColon + 1));
		ColumnIndex.push_back(NewColumnIndex);
		SQLCommand += ColumnNames[i - 1];
		if (i + 1 < Arguments.size())
		{
			SQLCommand += ",";
		}
	}
	SQLCommand += ") VALUES(";
	for (size_t i = 1; i < Arguments.size(); i++)
	{
		SQLCommand += "?";
		if (i + 1 < Arguments.size())
		{
			SQLCommand += ",";
		}
	}
	SQLCommand += ");";
	std::vector<MBDB::MBDB_RowData> QuerryResult = EvaluateBoundSQLStatement(SQLCommand,ColumnValues,ColumnIndex,Arguments[0],&DataBaseError);
	if (DataBaseError)
	{
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	}
	else
	{
		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	}
	return(ReturnValue);
}
std::string MBDB_Website::DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments)
{
	//UPDATE table_name
	//SET column1 = value1, column2 = value2, ...
	//	WHERE condition;
	std::string ReturnValue = "";
	std::string SQLCommand = "UPDATE " + Arguments[0] + " SET "; //VALUES (";
	std::vector<std::string> ColumnNames = {};
	std::vector<std::string> OldColumnValues = {};
	std::vector<std::string> NewColumnValues = {};
	MBError DataBaseError(true);


	for (size_t i = 1; i < Arguments.size(); i++)
	{
		if ((i % 3) == 1)
		{
			ColumnNames.push_back(Arguments[i]);
		}
		if ((i % 3) == 2)
		{
			OldColumnValues.push_back(Arguments[i]);
		}
		if ((i % 3) == 0)
		{
			NewColumnValues.push_back(Arguments[i]);
		}
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
		SQLCommand += "("+ColumnNames[i] + "=?";
		if (OldColumnValues[i] == "null")
		{
			SQLCommand += " OR "+ColumnNames[i]+ " IS NULL";
		}
		SQLCommand += ")";
		if (i + 1 < ColumnNames.size())
		{
			SQLCommand += " AND ";
		}
	}
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	{
		std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
		MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);
	
		//DEBUG GREJER
		//MBDB::SQLStatement* DebugStatement = WritableDatabase->GetSQLStatement("SELECT * FROM Music WHERE RowNumber=82");
		//std::vector<MBDB::MBDB_RowData> DebugResult = WritableDatabase->GetAllRows(DebugStatement);
		//auto DebugTuple = DebugResult[0].GetTuple<int, std::string, std::string, std::string, std::string, std::string, std::string>();
		//std::ofstream DebugFile("./DebugDatabaseData.txt", std::ios::out|std::ios::binary);
		//std::ofstream DebugInput("./DebugInputData.txt", std::ios::out | std::ios::binary);
		//DebugFile << std::get<6>(DebugTuple);
		//DebugInput << Arguments[21];

		std::vector<MBDB::ColumnInfo> ColumnInfo = WritableDatabase->GetColumnInfo(Arguments[0]);
		std::vector<MBDB::ColumnSQLType> ColumnTypes = {};
		for (size_t i = 0; i < ColumnInfo.size(); i++)
		{
			ColumnTypes.push_back(ColumnInfo[i].ColumnType);
		}
		NewStatement->BindValues(NewColumnValues, ColumnTypes, 0);
		NewStatement->BindValues(OldColumnValues, ColumnTypes, NewColumnValues.size());
		QuerryResult = WritableDatabase->GetAllRows(NewStatement,&DataBaseError);
		NewStatement->FreeData();
		WritableDatabase->FreeSQLStatement(NewStatement);
	}
	if (DataBaseError)
	{
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	}
	else
	{
		ReturnValue = "{\"MBDBAPI_Status\":" + ToJason(DataBaseError.ErrorMessage) + "}";
	}
	return(ReturnValue);
}
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
std::vector<MBDirectoryEntry> GetDirectoryEntries(std::string const& DirectoryPath,MBError* OutError = nullptr)
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
		else if(DirectoryIterator->is_regular_file())
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
			ReturnValue += "/"+PathIterator->generic_string();
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
	std::string ReturnValue = "{\"Path\":" + ToJason(EntryToEncode.Path.generic_string())+",";
	ReturnValue += "\"Type\":"+ToJason(MBDirectoryEntryTypeToString(EntryToEncode.Type))+"}";
	return(ReturnValue);
}
//First argument is relative path in MBDBResources folder
std::string MBDB_Website::DBAPI_GetFolderContents(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	MBError ErrorResult(true);
	std::vector<MBDirectoryEntry> DirectoryEntries = GetDirectoryEntries(GetResourceFolderPath()+Arguments[0],&ErrorResult);
	if (!ErrorResult)
	{
		return("{\"MBDBAPI_Status\":\""+ErrorResult.ErrorMessage+"\"}");
	}
	for (size_t i = 0; i < DirectoryEntries.size(); i++)
	{
		DirectoryEntries[i].Path = MakePathRelative(DirectoryEntries[i].Path, "MBDBResources");
	}
	std::string ErrorPart = "\"MBDBAPI_Status\":\"ok\"";
	std::string DirectoryPart = "\"DirectoryEntries\":"+MakeJasonArray<MBDirectoryEntry>(DirectoryEntries);
	ReturnValue = "{" + ErrorPart + "," + DirectoryPart + "}";
	return(ReturnValue);
}
std::string MBDB_Website::DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments)
{
	//ett arguments som är WhereStringen, ghetto aff egetntligen men men,måste vara på en immutable table så vi inte fuckar grejer
	std::string ReturnValue = "";
	if (Arguments.size() < 2)
	{
		return("{\"MBDBAPI_Status\":\"Invalid number of arguments\"}");
	}
	std::vector<MBDB::MBDB_RowData> RowResponse = {};
	MBError DatabaseError(true);
	{
		std::lock_guard<std::mutex> Lock(m_ReadonlyMutex);
		std::string SQlQuerry = "SELECT * FROM " + Arguments[0] + " " + Arguments[1];
		RowResponse = WritableDatabase->GetAllRows(SQlQuerry, &DatabaseError);
	}
	if (!DatabaseError)
	{
		return("{\"MBDBAPI_Status\":"+ToJason(DatabaseError.ErrorMessage)+"}");
	}
	ReturnValue = "{\"MBDBAPI_Status\":\"ok\",\"Rows\":" + MakeJasonArray(RowResponse)+"}";
	return(ReturnValue);
}
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
	ReturnValue += ToJason(std::string("AvailableIndexes"))+":"+MakeJasonArray(AvailableIndexes)+"}";
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
		ReturnValue += ToJason("Invalid function call")+"}";
	}
	if (!DB_IndexExists(Arguments[0]))
	{
		ReturnValue += ToJason("Index doesn't exists")+"}";
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
		ReturnValue += ToJason(std::string("IndexSearchResult"))+":"+MakeJasonArray(Result);
		ReturnValue += "}";
	}
	return(ReturnValue);
}
//
MBSockets::HTTPDocument MBDB_Website::DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	std::string RequestBody = MrPostOGet::GetRequestContent(RequestData);
	MBSockets::HTTPDocument ReturnValue;
	std::string Resourcepath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.Type = MBSockets::HTTPDocumentType::json;
	if (RequestType == "POST")
	{
		//tar fram api funktionen

		//eftersom det kan vara svårt att parsa argument med godtycklig text har varje argument först hur många bytes argumentent är
		size_t FirstSpace = RequestBody.find(" ");
		std::string APIDirective = DBGeneralAPIGetDirective(RequestBody);
		std::vector<std::string> APIDirectiveArguments = DBGeneralAPIGetArguments(RequestBody);
		DBPermissionsList ConnectionPermissions = m_GetConnectionPermissions(RequestData);
		if (APIDirective == "GetTableNames")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = GetTableNamesBody(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "GetTableInfo")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = GetTableInfoBody(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "AddEntryToTable")
		{
			if (ConnectionPermissions.Upload)
			{
				//Funktions prototyp TableNamn + ColumnName:stringcolumm data
				ReturnValue.DocumentData = DBAPI_AddEntryToTable(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require Upload permissions\"}";
			}
		}
		else if (APIDirective == "GetFolderContents")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = DBAPI_GetFolderContents(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "SearchTableWithWhere")
		{
			if (ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = DBAPI_SearchTableWithWhere(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to read\"}";
			}
		}
		else if (APIDirective == "UpdateTableRow")
		{
			if (ConnectionPermissions.Upload)
			{
				ReturnValue.DocumentData = DBAPI_UpdateTableRow(APIDirectiveArguments);
			}
			else
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to Edit\"}";
			}
		}
		else if (APIDirective == "Login")
		{
			ReturnValue.DocumentData = DBAPI_Login(APIDirectiveArguments);
			std::string StringToCompare = "{\"MBDBAPI_Status\":\"ok\"}";
			if (ReturnValue.DocumentData.substr(0,StringToCompare.size()) == StringToCompare)
			{
				ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBUsername=" + APIDirectiveArguments[0] + "; Secure; " + "Max-Age=604800; Path=/");
				ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBPassword=" + APIDirectiveArguments[1] + "; Secure; " + "Max-Age=604800; Path=/");
			}
		}
		else if (APIDirective == "FileExists")
		{
			if (!ConnectionPermissions.Read)
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permissions to Read\"}";
			}
			else if (APIDirectiveArguments.size() != 1)
			{
				ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"Invalid function call\"}";
			}
			else
			{
				std::string NewDocumentData = "{\"MBDBAPI_Status\":\"ok\",";
				NewDocumentData += "\"FileExists\":" + ToJason(std::filesystem::exists(GetResourceFolderPath() + APIDirectiveArguments[0]))+",";
				std::filesystem::path NewFilepath(APIDirectiveArguments[0]);
				NewFilepath = NewFilepath.parent_path();
				std::filesystem::path BaseFilepath(GetResourceFolderPath());
				bool FoldersExists = true;
				for (auto const& Directory : NewFilepath)
				{
					BaseFilepath += "/";
					BaseFilepath += Directory;
					if (!std::filesystem::exists(BaseFilepath))
					{
						FoldersExists = false;
						break;
					}
				}
				NewDocumentData += "\"DirectoriesExists\":" + ToJason(FoldersExists)+"}";
				ReturnValue.DocumentData = NewDocumentData;
			}
		}
		else if(APIDirective == "GetAvailableIndexes")
		{
			ReturnValue.DocumentData = DBAPI_GetAvailableIndexes(APIDirectiveArguments);
		}
		else if (APIDirective == "GetIndexSearchResult")
		{
			ReturnValue.DocumentData = DBAPI_GetIndexSearchResult(APIDirectiveArguments);
		}
		else
		{
			ReturnValue.DocumentData = "{\"MBDBAPI_Status\":\"UnknownCommand\"}";
			//ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(Resourcepath + "404.html", Resourcepath);
		}
	}
	else
	{
		ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
		ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(Resourcepath+"404.html", Resourcepath);
	}
	std::cout << ReturnValue.DocumentData << std::endl;
	return(ReturnValue);
}
bool MBDB_Website::DBUpdate_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "DBUpdate")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "DBUpdate.html", ResourcePath);
	return(ReturnValue);
}
bool MBDB_Website::DBOperationBlipp_Predicate(std::string const& RequestData)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::vector<std::string> Directorys = Split(RequestResource, "/");
	if (Directorys.size() >= 1)
	{
		if (Directorys[0] == "operationblipp")
		{
			return(true);
		}
	}
	return(false);
}
MBSockets::HTTPDocument MBDB_Website::DBOperatinBlipp_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	std::string MBDBResources = GetResourceFolderPath();
	std::string HTMLFolder = AssociatedServer->GetResourcePath("mrboboget.se");
	std::string DefaultPage = HTMLFolder + "/operationblipp.html";
	std::vector<std::string> PathComponents = MBUtility::Split(MBSockets::GetReqestResource(RequestData),"/");
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
		ReturnValue.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
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
			ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
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
					std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Can't acces latest file: User "+LatestUserDownload+" hasn't updated the file"},{"LatestDate",LastTimestamp} };
					ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
					ReturnValue.DocumentData = ResourceData;
					ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
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
			//vi vill bara parsa ett visst antal stycken, om dem inte är rätt så bara avbryter vi
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
				std::string ArchiveFilepath = MBDBResources + "/operationblipp/archives/"+Timestamp+" ("+ConnectionPermissions.AssociatedUser+")";
				std::ofstream ArchiveFile = std::ofstream(ArchiveFilepath, std::ios::out | std::ios::binary);
				ArchiveFile << FileData;
				ArchiveFile.flush();
				ArchiveFile.close();

				std::ofstream LatestAccess = std::ofstream(MBDBResources + "/operationblipp/archives/LatestAccess",std::ios::out|std::ios::binary);
				//ANTAGANDE här har personen som acessar redan verifierats vara korrekt
				LatestAccess << "";
				LatestAccess.flush();
				LatestAccess.close();

				std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
				std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Succesfully uploaded file"},{"LatestDate",Timestamp}	 };
				ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
				ReturnValue.DocumentData = ResourceData;
				ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
			}
			else
			{
				std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
				std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Error updating latest: Need to download the latest before updating"},{"LatestDate",LastTimestamp} };
				ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
				ReturnValue.DocumentData = ResourceData;
				ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
			}	
		}
		catch (const std::exception&)
		{
			std::string ResourceData = MrPostOGet::LoadFileWithPreprocessing(DefaultPage, AssociatedServer->GetResourcePath("mrboboget.se"));
			std::unordered_map<std::string, std::string> VariableMap = { {"UploadMessage","Error uploading file"},{"LatestDate",LastTimestamp} };
			ResourceData = MrPostOGet::ReplaceMPGVariables(ResourceData, VariableMap);
			ReturnValue.DocumentData = ResourceData;
			ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
		}
	}
	return(ReturnValue);
}