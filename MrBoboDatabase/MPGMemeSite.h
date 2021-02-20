#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrPostOGet/MrPostOGet.h>
bool DBSite_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBSite_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedSocket);

bool UploadFile_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument UploadFile_ResponseGenerator(std::string const& RequestData,MrPostOGet::HTTPServer* AssociatedServer,MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBGet_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBGet_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBViewer_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBViewer_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);

bool DBViewEmbedd_Predicate(std::string const& RequestData);
MBSockets::HTTPDocument DBViewEmbedd_ResponseGenerator(std::string const& RequestData, MrPostOGet::HTTPServer* AssociatedServer, MBSockets::HTTPServerSocket* AssociatedConnection);