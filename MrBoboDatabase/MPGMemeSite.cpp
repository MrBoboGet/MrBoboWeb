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
MBDB::MrBoboDatabase* DBSite_Database = nullptr;
MBSockets::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection)
{
	if (DBSite_Database == nullptr)
	{
		DBSite_Database = new MBDB::MrBoboDatabase("./TestDatabas", 0);
	}
	std::string RequestType = MBSockets::GetRequestType(RequestData);
	MBSockets::HTTPDocument NewDocument;
	if (RequestType == "GET")
	{
		NewDocument = AssociatedServer->GetResource(AssociatedServer->GetResourcePath("mrboboget.se")+ "/DBSite.html");
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
			SQLResult = DBSite_Database->GetAllRows(SQLCommand);
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
	std::string FileName ="./"+ FirstFieldValues[2].substr(FileNameHeader.size(), FirstFieldValues[2].size() - 1 - FileNameHeader.size());
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
	return(NewDocument);
}