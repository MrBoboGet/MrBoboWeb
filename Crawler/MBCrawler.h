#pragma once
#include <MBUtility/MBErrorHandling.h>
MBError IndexWebsite(std::string const& WebsiteURL, std::string const& OutFolderDirectory,bool OnlyAddNew);
void MakeWebsiteDirectoryRelative(std::string WebsitedDirectory, std::string WebsiteOutDirectory, std::string WebsiteDomain);
MBError CreateWebsiteIndex(std::string const& WebsiteFolder, std::string const& OutIndexFilename);
MBError CreateWebsiteIndex(std::string const& WebsiteURLHeader, std::string const& WebsiteFolder, std::string const& OutIndexFilename);