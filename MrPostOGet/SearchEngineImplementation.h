#pragma once
#include <SearchEngine/MBSearchEngine.h>
#include <MrPostOGet/MrPostOGet.h>
bool DefaultSearchPredicate(const std::string& RequestData)
{
	//resourcen ska vara search=
	return(MBSockets::GetRequestType(RequestData) == "GET" && MBSockets::GetReqestResource(RequestData).substr(0, 7) == "search=");
}
std::string DefaultSearchRespone(const std::string& RequestData, const std::string& ResourcePath)
{
	std::string Response = "<HTML><head><link rel=\"stylesheet\"href=\"/WebsidanStil.css\"></head>\n<BODY>\n<h1 align=\"left\" style=\"text-align:left\">Enter the web</h1><div style=\"text-align:left;\"><input type=\"text\" id=\"SearchBox\" name=\"fname\"></div>";
	Response += "<script src=\"RedirectToSearch.js\"></script>";
	std::vector<std::string> HyperlinkResponses = SearchWebsites(ReplaceAll(MBSockets::GetReqestResource(RequestData).substr(7), "+", " "));
	for (size_t i = 0; i < HyperlinkResponses.size(); i++)
	{
		//<a href = "url">link text< / a>
		std::string Linktext = "<a href=\"" + HyperlinkResponses[i] + "\">" + HyperlinkResponses[i] + "</a><br>";
		Response += Linktext;
	}
	Response += "</BODY>\n</HTML>";
	return(Response);
}
MrPostOGet::RequestHandler DefaultSearch = { &DefaultSearchPredicate,&DefaultSearchRespone };