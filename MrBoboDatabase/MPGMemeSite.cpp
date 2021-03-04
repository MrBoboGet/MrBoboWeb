#define NOMINMAX
#include <MrBoboDatabase/MPGMemeSite.h>
#include <MinaStringOperations.h>
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
std::mutex DatabaseMutex;
//MBDB::MrBoboDatabase DBSite_Database("./TestDatabas",0);
MBDB::MrBoboDatabase* WebsiteDatabase = nullptr;
std::mutex WritableDatabaseMutex;
MBDB::MrBoboDatabase* WritableDatabase = nullptr;
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
		//vi behöver parsa kroppen så vi får SQL koden som den enkoder i sin kropp
		std::string SQLCommand = MrPostOGet::GetRequestContent(RequestData);
		std::cout << SQLCommand << std::endl;
		bool CommandSuccesfull = true;
		MBError SQLError(true);
		std::vector<MBDB::MBDB_RowData> SQLResult = {};
		{
			std::lock_guard<std::mutex> Lock(DatabaseMutex);
			SQLResult = WebsiteDatabase->GetAllRows(SQLCommand);
			if(!SQLError)
			{
				CommandSuccesfull = false;
			}
		}
		//std::vector<std::string> ColumnNames = { "MemeID","MemeSource","MemeTags" };
		std::string JsonResponse = "{\"Rows\":[";
		if (CommandSuccesfull)
		{
			size_t NumberOfRows = SQLResult.size();
			for (size_t i = 0; i < NumberOfRows; i++)
			{
				JsonResponse += SQLResult[i].ToJason();
				if (i + 1 < NumberOfRows)
				{
					JsonResponse += ",";
				}
			}
		}
		JsonResponse += "]}";
		std::cout << "JsonResponse sent: " << JsonResponse << std::endl;
		NewDocument.DocumentData = JsonResponse;
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
	MBSockets::HTTPDocument NewDocument;
	std::string Boundary = "";
	std::vector<std::string> ContentTypes = MBSockets::GetHeaderValues("Content-Type", RequestData);
	std::string FormType = "multipart/form-data";
	std::string BoundaryHeader = "; boundary=";
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
	//hardcodat eftersom vi vet formtatet av formuläret
	std::vector<std::string> FirstFieldValues = Split(FieldParameters, "; ");
	std::string FileNameHeader = "filename=\"";
	std::string FileName ="./MBDBResources/"+ FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
	int FilesWithSameName = 0;
	while(std::filesystem::exists(FileName))
	{
		FileName += std::to_string(FilesWithSameName);
		FilesWithSameName += 1;
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
	
	
	NewDocument.Type = MBSockets::HTTPDocumentType::json;
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
	std::string ReturnValue = "<image src=\"/DB/" + ImagePath + "\"></image>";
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
		EmbeddedElement = GetEmbeddedVideo(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
	}

	std::unordered_map<std::string, std::string> MapData = {};
	MapData["EmbeddedMedia"] = EmbeddedElement;
	std::string HTMLResourcePath = AssociatedServer->GetResourcePath("mrboboget.se");
	ReturnValue.DocumentData = MrPostOGet::ReplaceMPGVariables(MrPostOGet::LoadFileWithPreprocessing(HTMLResourcePath + "DBViewTemplate.html", HTMLResourcePath), MapData);
	//= AssociatedServer->GetResource(AssociatedServer->GetResourcePath("mrboboget.se") + "/DBViewTemplate.html");
	if (EmbeddedElement == "" || ReturnValue.DocumentData == "")
	{
		ReturnValue.RequestStatus = MBSockets::HTTPRequestStatus::NotFound;
	}
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
		ReturnValue.DocumentData = GetEmbeddedVideo(DBResource, AssociatedServer->GetResourcePath("mrboboget.se"));
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
	//låter all denna kod köras i javascript, blir det enklaste
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
	std::string JsonResponse = MBDB::MakeJasonArray(TableNames, "TableNames");
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
	return(JsonResponse);
}
std::string GetTableInfoBody(std::vector<std::string> const& Arguments)
{
	//första argumentet är tablen vi vill ha
	if (Arguments.size() == 0)
	{
		return("");
	}
	std::vector<MBDB::ColumnInfo> TableInfo = {};
	{
		std::lock_guard<std::mutex> Lock(DatabaseMutex);
		TableInfo = WebsiteDatabase->GetColumnInfo(Arguments[0]);
	}
	return(MBDB::MakeJasonArray(TableInfo,"TableInfo"));
}
long long StringToInt(std::string const& IntData, MBError* OutError = nullptr)
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
std::string DBAPI_AddEntryToTable(std::vector<std::string> const& Arguments)
{
	std::string ReturnValue = "";
	std::string SQLCommand = "INSERT INTO " + Arguments[0]+" VALUES (";
	for (size_t i = 1; i < Arguments.size(); i++)
	{
		SQLCommand += "?";
		if (i + 1 < Arguments.size())
		{
			SQLCommand += ",";
		}
	}
	SQLCommand += ")";
	std::vector<MBDB::MBDB_RowData> QuerryResult = {};
	MBError DataBaseError(true);
	{
		std::lock_guard<std::mutex> Lock(WritableDatabaseMutex);
		MBDB::SQLStatement* NewStatement = WritableDatabase->GetSQLStatement(SQLCommand);
		std::vector<MBDB::ColumnInfo> TableColumnInfo = WritableDatabase->GetColumnInfo(Arguments[0]);
		for (size_t i = 1; i < Arguments.size(); i++)
		{
			if (TableColumnInfo[i-1].ColumnType == MBDB::ColumnSQLType::Int)
			{
				MBDB::MaxInt NewInt = StringToInt(Arguments[i],&DataBaseError);
				if (!DataBaseError)
				{
					break;
				}
				DataBaseError = NewStatement->BindInt(NewInt, i);
			}
			else
			{
				DataBaseError = NewStatement->BindString(Arguments[i], i);
				if (!DataBaseError)
				{
					break;
				}
			}
		}
		if (DataBaseError)
		{
			QuerryResult = WritableDatabase->GetAllRows(NewStatement, &DataBaseError);
		}
	}
	if (DataBaseError)
	{
		ReturnValue = "{\"MBDBAPI_Status\":\"ok\"}";
	}
	else
	{
		ReturnValue = "{\"MBDBAPI_Status\":" + MBDB::ToJason(DataBaseError.ErrorMessage) + "}";
	}
	return(ReturnValue);
}
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

		//eftersom det kan vara svårt att parsa argument med godtycklig text har varje argument först hur många bytes argumentent är
		size_t FirstSpace = RequestBody.find(" ");
		std::string APIDirective = DBGeneralAPIGetDirective(RequestBody);
		std::vector<std::string> APIDirectiveArguments = DBGeneralAPIGetArguments(RequestBody);
		if (APIDirective == "GetTableNames")
		{
			ReturnValue.DocumentData = GetTableNamesBody(APIDirectiveArguments);
		}
		else if (APIDirective == "GetTableInfo")
		{
			ReturnValue.DocumentData = GetTableInfoBody(APIDirectiveArguments);
		}
		else if (APIDirective == "AddEntryToTable")
		{
			//Funktions prototyp TableNamn + ColumnName:stringcolumm data
			ReturnValue.DocumentData = DBAPI_AddEntryToTable(APIDirectiveArguments);
		}
		else
		{
			ReturnValue.DocumentData = "{\"DBGAError\":\"UnknownCommand\"";
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