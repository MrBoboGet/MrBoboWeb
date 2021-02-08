#pragma once
#include <MrPostOGet/MrPostOGet.h>
#include <MrBoboDatabase/MrBoboDatabase.h>
#include <MrPostOGet/MrPostOGet.h>
bool DBSite_Predicate(std::string const& RequestData);
std::string DBSite_ResponseGenerator(std::string const& RequestData,MrPostOGet::HTTPServer* AssociatedServer);