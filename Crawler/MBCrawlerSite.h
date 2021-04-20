#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <filesystem>
inline bool MBCrawlerSite_Predicate(std::string const& RequestData)
{
	//bara för att testa
	return(true);
}
inline MBSockets::HTTPDocument MBCrawlerSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestResource = MBSockets::GetReqestResource(RequestData);
	std::filesystem::path ResourcePath = RequestResource;
	std::string ResourceToGet = "";
	if (ResourcePath.has_stem())
	{
		ResourceToGet = "RelativeRemar.se/" + ResourcePath.parent_path().generic_string()+"/"+"__" + ResourcePath.filename().generic_string();
	}
	else
	{
		//är en directory, då tar vi fram directory entrien
		ResourceToGet = "RelativeRemar.se/" + ResourcePath.generic_string() + "/__DirectoryResource";
	}
	MBSockets::HTTPDocument ReturnValue = AssociatedServer->GetResource(ResourceToGet);
	return(ReturnValue);
}
