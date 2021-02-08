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
std::string DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer)
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
	return(AssociatedServer->GenerateResponse(NewDocument));
}