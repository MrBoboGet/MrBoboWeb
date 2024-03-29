#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <filesystem>
inline bool MBCrawlerSite_Predicate(std::string const& RequestData)
{
	//bara f�r att testa
	return(true);
}
inline MrPostOGet::HTTPDocument MBCrawlerSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MrPostOGet::HTTPServerSocket* AssociatedConnection)
{
	std::string RequestResource = MrPostOGet::GetRequestResource(RequestData);
	std::filesystem::path ResourcePath = RequestResource;
	std::string ResourceToGet = "";
	if (ResourcePath.has_stem())
	{
		ResourceToGet = "RelativeRemar.se/" + ResourcePath.parent_path().generic_string()+"/"+"__" + ResourcePath.filename().generic_string();
	}
	else
	{
		//�r en directory, d� tar vi fram directory entrien
		ResourceToGet = "RelativeRemar.se/" + ResourcePath.generic_string() + "/__DirectoryResource";
	}
	MrPostOGet::HTTPDocument ReturnValue = AssociatedServer->GetResource(ResourceToGet);
	return(ReturnValue);
}
