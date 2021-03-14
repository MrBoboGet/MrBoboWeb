#define NOMINMAX
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MinaStringOperations.h>
std::string MBDBGetResourceFolderPath()
{
	return("./MBDBResources/");
}

//username cookie = 
//password cookie = 
enum class DBPermissions
{
	Read,
	Edit,
	Upload,
	Null,
};
std::mutex DatabaseMutex;
//MBDB::MrBoboDatabase DBSite_Database("./TestDatabas",0);
MBDB::MrBoboDatabase* WebsiteDatabase = nullptr;
std::mutex WritableDatabaseMutex;
MBDB::MrBoboDatabase* WritableDatabase = nullptr;
std::mutex LoginDatabaseMutex;
MBDB::MrBoboDatabase* LoginDatabase = nullptr;
void InitDatabase()
{
	if (WebsiteDatabase == nullptr)
	{
		WebsiteDatabase = new MBDB::MrBoboDatabase("./TestDatabas", 0);
	}
	if (WritableDatabase == nullptr)
	{
		WritableDatabase = new MBDB::MrBoboDatabase("./TestDatabas", 1);
	}
	if (LoginDatabase == nullptr)
	{
		LoginDatabase = new MBDB::MrBoboDatabase("./MBGLoginDatabase", 0);
	}
}
struct DBPermissionsList
{
	bool Read = false;
	bool Edit = false;
	bool Upload = false;
	bool IsNull = true;
};
struct Cookie
{
	std::string Name = "";
	std::string Value = "";
};
std::vector<Cookie> GetCookiesFromRequest(std::string const& RequestData)
{
	std::vector<Cookie> ReturnValue = {};
	std::vector<std::string> Cookies = Split(MBSockets::GetHeaderValue("Cookie", RequestData), "; ");
	for (size_t i = 0; i < Cookies.size(); i++)
	{
		size_t FirstEqualSignPos = Cookies[i].find_first_of("=");
		std::string CookieName = Cookies[i].substr(0, FirstEqualSignPos);
		std::string CookieValue = Cookies[i].substr(FirstEqualSignPos+1);
		ReturnValue.push_back({ CookieName, CookieValue });
	}
	return(ReturnValue);
}
std::vector<MBDB::MBDB_RowData> DB_GetUser(std::string const& UserName, std::string const& PasswordHash)
{
	std::string SQLStatement = "SELECT * FROM Users WHERE UserName=?;";
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	{
		std::lock_guard<std::mutex> Lock(LoginDatabaseMutex);
		MBDB::SQLStatement* NewStatement = LoginDatabase->GetSQLStatement(SQLStatement);
		NewStatement->BindString(UserName, 1);
		QuerryResult = LoginDatabase->GetAllRows(NewStatement);
		NewStatement->FreeData();
		LoginDatabase->FreeSQLStatement(NewStatement);
	}
	return(QuerryResult);
}
DBPermissionsList GetConnectionPermissions(std::string const& RequestData)
{
	DBPermissionsList ReturnValue;
	std::vector<Cookie> Cookies = GetCookiesFromRequest(RequestData);
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
	std::string SQLStatement = "SELECT * FROM Users WHERE UserName=?;";
	std::vector<MBDB::MBDB_RowData> QuerryResult = DB_GetUser(UserName, PasswordHash);
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
std::string DBGetUsername(std::string const& RequestData)
{
	std::string ReturnValue = "";
	std::vector<Cookie> Cookies = GetCookiesFromRequest(RequestData);
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
bool DBLogin_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBLogin_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string ServerResources = AssociatedServer->GetResourcePath("mrboboget.se");
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::unordered_map<std::string, std::string> FileVariables = {};
	
	FileVariables["LoginValue"] = DBGetUsername(RequestData);
	if (FileVariables["LoginValue"] != "")
	{
		FileVariables["LoginValue"] = "Currently logged in as: " + FileVariables["LoginValue"];
	}
	ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(ServerResources + "DBLogin.html", ServerResources),FileVariables);
	return(ReturnValue);
}

bool DBSite_Predicate(std::string const& RequestData)
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

MBSockets::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	MBSockets::HTTPDocument NewDocument;
	if (RequestType == "GET")
	{
		NewDocument.Type = MBSockets::HTTPDocumentType::HTML;
		NewDocument.DocumentData = MrPostOGet::LoadFileWithPreprocessing(AssociatedServer->GetResourcePath("mrboboget.se")+ "/DBSite.html",AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	else if(RequestType == "POST")
	{
		NewDocument.Type = MBSockets::HTTPDocumentType::json;
		//vi beh�ver parsa kroppen s� vi f�r SQL koden som den enkoder i sin kropp
		std::string SQLCommand = MrPostOGet::GetRequestContent(RequestData);
		std::cout << SQLCommand << std::endl;
		bool CommandSuccesfull = true;
		MBError SQLError(true);
		std::vector<MBDB::MBDB_RowData> SQLResult = {};
		{
			std::lock_guard<std::mutex> Lock(DatabaseMutex);
			SQLResult = WebsiteDatabase->GetAllRows(SQLCommand,&SQLError);
			if(!SQLError)
			{
				CommandSuccesfull = false;
			}
		}
		//std::vector<std::string> ColumnNames = { "MemeID","MemeSource","MemeTags" };
		if (CommandSuccesfull)
		{
			std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\",\"Rows\":[";
			size_t NumberOfRows = SQLResult.size();
			for (size_t i = 0; i < NumberOfRows; i++)
			{
				JsonResponse += SQLResult[i].ToJason();
				if (i + 1 < NumberOfRows)
				{
					JsonResponse += ",";
				}
			}
			JsonResponse += "]}";
			NewDocument.DocumentData = JsonResponse;
		}
		else
		{
			NewDocument.DocumentData = "{\"MBDBAPI_Status\":" + ToJason(SQLError.ErrorMessage) + ",\"Rows\":[]}";
		}
		//std::cout << "JsonResponse sent: " << JsonResponse << std::endl;
	}
	return(NewDocument);
}

bool UploadFile_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument UploadFile_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//Content-Type: multipart/form-data; boundary=---------------------------226143532618736951363968904467
	DBPermissionsList ConnectionPermissions = GetConnectionPermissions(RequestData);
	MBSockets::HTTPDocument NewDocument;
	NewDocument.Type = MBSockets::HTTPDocumentType::json;
	std::string Boundary = "";
	std::vector<std::string> ContentTypes = MBSockets::GetHeaderValues("Content-Type", RequestData);
	std::string FormType = "multipart/form-data";
	std::string BoundaryHeader = "; boundary=";
	if (!ConnectionPermissions.Upload)
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"Invalid Permissions: Require permission to upload\"}";
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
	int FirstBoundaryLocation = RequestData.find(Boundary);
	int FirstFormParameterLocation = RequestData.find(Boundary, FirstBoundaryLocation + Boundary.size())+Boundary.size()+2;
	int EndOfFirstParameters = RequestData.find("\r\n",FirstFormParameterLocation);
	std::string FieldParameters = RequestData.substr(FirstFormParameterLocation, EndOfFirstParameters - FirstFormParameterLocation);
	//hardcodat eftersom vi vet formtatet av formul�ret
	std::vector<std::string> FirstFieldValues = Split(FieldParameters, "; ");
	std::string FileNameHeader = "filename=\"";
	std::string FileName ="./MBDBResources/"+ FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
	int FilesWithSameName = 0;
	while(std::filesystem::exists(FileName))
	{
		NewDocument.DocumentData = "{\"MBDBAPI_Status\":\"FileAlreadyExists\"}";
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

bool DBGet_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string DatabaseResourcePath = "./MBDBResources/";
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
			FiledataIntervall NewIntervall = { -1,-1 };
			std::vector<std::string> IntervallNumbers = Split(Intervalls[i], "-");
			if (IntervallNumbers[0] != "")
			{
				NewIntervall.FirstByte = std::stoi(IntervallNumbers[0]);
			}
			if (IntervallNumbers[1] != "")
			{
				NewIntervall.FirstByte = std::stoi(IntervallNumbers[1]);
			}
			ByteIntervalls.push_back(NewIntervall);
		}
	}


	MBSockets::HTTPDocument ReturnValue = AssociatedServer->GetResource(DatabaseResourcePath+DatabaseResourceToGet,ByteIntervalls);
	return(ReturnValue);
}
std::string GetEmbeddedVideo(std::string const& VideoPath, std::string const& WebsiteResourcePath)
{
	std::string ReturnValue = "";
	std::string FileExtension = MrPostOGet::GetFileExtension(VideoPath);
	std::unordered_map<std::string, std::string> VariableValues = {};
	VariableValues["ElementID"] = VideoPath;
	VariableValues["MediaType"] = "video";
	VariableValues["PlaylistPath"] = "/DB/" + VideoPath;
	VariableValues["FileType"] = FileExtension;
	ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/DirectFileStreamTemplate.html", VariableValues);
	//else
	//{
	//	std::unordered_map<std::string, std::string> VariableValues = {};
	//	VariableValues["ElementID"] = VideoPath;
	//	VariableValues["MediaType"] = "video";
	//	VariableValues["PlaylistPath"] = "/DB/" + VideoPath + "_Stream/MasterPlaylist.m3u8";
	//	ReturnValue = MrPostOGet::LoadFileWithVariables(WebsiteResourcePath + "/EmbeddStreamTemplate.html", VariableValues);
	//}
	return(ReturnValue);
}
std::string GetEmbeddedAudio(std::string const& VideoPath, std::string const& WebsiteResourcePath)
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
std::string GetEmbeddedImage(std::string const& ImagePath)
{
	std::string ReturnValue = "<image src=\"/DB/" + ImagePath + "\" style=\"max-width:100%\"></image>";
	return(ReturnValue);
}
bool DBView_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBView_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string EmbeddedElement = "";

	std::string HandlerName = "DBView";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = "./MBDBResources/";
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
		MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
		if (ResourceMedia == MBSockets::MediaType::Image)
		{
			EmbeddedElement = GetEmbeddedImage(DBResource);
		}
		else if (ResourceMedia == MBSockets::MediaType::Video)
		{
			EmbeddedElement = GetEmbeddedVideo(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		}
		else if (ResourceMedia == MBSockets::MediaType::Audio)
		{
			EmbeddedElement = GetEmbeddedAudio(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
		}
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

bool DBViewEmbedd_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue = MBSockets::HTTPDocument();
	std::string HandlerName = "DBViewEmbedd/";
	std::string ResourcePath = MBSockets::GetReqestResource(RequestData);
	std::string DBResourcesPath = "./MBDBResources/";
	std::string DBResource = ResourcePath.substr(ResourcePath.find_first_of(HandlerName) + HandlerName.size());
	std::string ResourceExtension = DBResource.substr(DBResource.find_last_of(".") + 1);
	MBSockets::MediaType ResourceMedia = MBSockets::GetMediaTypeFromExtension(ResourceExtension);
	ReturnValue.Type = MBSockets::DocumentTypeFromFileExtension(ResourceExtension);
	if (ResourceMedia == MBSockets::MediaType::Image)
	{
		ReturnValue.DocumentData = GetEmbeddedImage(DBResource);
	}
	else if (ResourceMedia == MBSockets::MediaType::Video)
	{
		ReturnValue.DocumentData = GetEmbeddedVideo(DBResource,AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	else if (ResourceMedia == MBSockets::MediaType::Audio)
	{
		ReturnValue.DocumentData = GetEmbeddedAudio(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
	}
	return(ReturnValue);
}

bool DBAdd_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBAdd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	//std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	//std::string TableName = RequestData.substr(RequestResource.find("DBAdd/") + 6);
	//std::vector < std::string> ExistingTableNames = {};
	//l�ter all denna kod k�ras i javascript, blir det enklaste
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath+"DBAdd.html", ResourcePath);
	return(ReturnValue);
}

bool DBGeneralAPI_Predicate(std::string const& RequestData)
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

std::string DBGeneralAPIGetDirective(std::string const& RequestBody)
{
	size_t FirstSpace = RequestBody.find(" ");
	std::string APIDirective = RequestBody.substr(0, FirstSpace);
	return(APIDirective);
}
std::vector<std::string> DBGeneralAPIGetArguments(std::string const& RequestBody,MBError* OutError = nullptr)
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


std::string GetTableNamesBody(std::vector<std::string> const& Arguments)
{
	std::vector<std::string> TableNames = {};
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		TableNames = WebsiteDatabase->GetAllTableNames();
	}
	std::string JSONTableNames = "\"TableNames\":"+MakeJasonArray(TableNames);
	//std::string JsonRespone = "{[";
	//size_t TableNamesSize = TableNames.size();
	//for (size_t i = 0; i < TableNamesSize; i++)
	//{
	//	JsonRespone += MBDB::ToJason(TableNames[i]);
	//	if (i + 1 < TableNamesSize)
	//	{
	//		JsonRespone += ",";
	//	}
	//}
	//JsonRespone += "]}";
	std::string JsonResponse = "{\"MBDBAPI_Status\":\"ok\"," + JSONTableNames + "}";
	return(JsonResponse);
}
std::string GetTableInfoBody(std::vector<std::string> const& Arguments)
{
	//f�rsta argumentet �r tablen vi vill ha
	if (Arguments.size() == 0)
	{
		return("");
	}
	std::vector<MBDB::ColumnInfo> TableInfo = {};
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		TableInfo = WebsiteDatabase->GetColumnInfo(Arguments[0]);
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
std::vector<MBDB::MBDB_RowData> EvaluateBoundSQLStatement(std::string SQLCommand,std::vector<std::string> const& ColumnValues,
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
std::string DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	std::string SQLCommand = "INSERT INTO " + Arguments[0] + "("; //VALUES (";
	std::vector<std::string> ColumnNames = {};
	std::vector<std::string> ColumnValues = {};
	std::vector<int> ColumnIndex = {};
	MBError DataBaseError(true);


	for (size_t i = 1; i < Arguments.size(); i++)
	{
		int FirstColon = Arguments[i].find_first_of(":");
		int SecondColon = Arguments[i].find(":", FirstColon + 1);
		int NewColumnIndex = -1;
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
std::string DBAPI_UpdateTableRow(std::vector<std::string> const& Arguments)
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
std::vector<MBDirectoryEntry> GetDirectoryEntries(std::string const& DirectoryPath)
{
	std::vector<MBDirectoryEntry> ReturnValue = {};
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
std::string DBAPI_GetFolderContents(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	std::vector<MBDirectoryEntry> DirectoryEntries = GetDirectoryEntries(MBDBGetResourceFolderPath()+Arguments[0]);
	for (size_t i = 0; i < DirectoryEntries.size(); i++)
	{
		DirectoryEntries[i].Path = MakePathRelative(DirectoryEntries[i].Path, "MBDBResources");
	}
	std::string ErrorPart = "\"MBDBAPI_Status\":\"ok\"";
	std::string DirectoryPart = "\"DirectoryEntries\":"+MakeJasonArray<MBDirectoryEntry>(DirectoryEntries);
	ReturnValue = "{" + ErrorPart + "," + DirectoryPart + "}";
	return(ReturnValue);
}
std::string DBAPI_SearchTableWithWhere(std::vector<std::string> const& Arguments)
{
	//ett arguments som �r WhereStringen, ghetto aff egetntligen men men,m�ste vara p� en immutable table s� vi inte fuckar grejer
	std::string ReturnValue = "";
	if (Arguments.size() < 2)
	{
		return("{\"MBDBAPI_Status\":\"Invalid number of arguments\"}");
	}
	std::vector<MBDB::MBDB_RowData> RowResponse = {};
	MBError DatabaseError(true);
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
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
std::string DBAPI_Login(std::vector<std::string> const& Arguments)
{
	if (Arguments.size() != 2)
	{
		return("{\"MBDBAPI_Status\":\"Invalid Function call\"}");
	}
	std::vector<MBDB::MBDB_RowData> UserResult = DB_GetUser(Arguments[0], Arguments[1]);
	if (UserResult.size() == 0)
	{
		return("{\"MBDBAPI_Status\":\"Invalid UserName or Password\"}");
	}
	return("{\"MBDBAPI_Status\":\"ok\"}");
}
//
MBSockets::HTTPDocument DBGeneralAPI_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	std::string RequestBody = MrPostOGet::GetRequestContent(RequestData);
	MBSockets::HTTPDocument ReturnValue;
	std::string Resourcepath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.Type = MBSockets::HTTPDocumentType::json;
	if (RequestType == "POST")
	{
		//tar fram api funktionen

		//eftersom det kan vara sv�rt att parsa argument med godtycklig text har varje argument f�rst hur m�nga bytes argumentent �r
		size_t FirstSpace = RequestBody.find(" ");
		std::string APIDirective = DBGeneralAPIGetDirective(RequestBody);
		std::vector<std::string> APIDirectiveArguments = DBGeneralAPIGetArguments(RequestBody);
		DBPermissionsList ConnectionPermissions = GetConnectionPermissions(RequestData);
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
			ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBUsername=" + APIDirectiveArguments[0]+"; Secure; "+"Max-Age=604800; Path=/");
			ReturnValue.ExtraHeaders.push_back("Set-Cookie: DBPassword=" + APIDirectiveArguments[1]+"; Secure; "+"Max-Age=604800; Path=/");
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
bool DBUpdate_Predicate(std::string const& RequestData)
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
MBSockets::HTTPDocument DBUpdate_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	MBSockets::HTTPDocument ReturnValue;
	ReturnValue.Type = MBSockets::HTTPDocumentType::HTML;
	std::string ResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::LoadFileWithPreprocessing(ResourcePath + "DBUpdate.html", ResourcePath);
	return(ReturnValue);
}