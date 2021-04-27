#include <MBCrawler.h>
#include <MrBoboSockets.h>
#include <MBStrings.h>
#include <filesystem>
#include <MrPostOGet/MrPostOGet.h>
#include <atomic>
#include <MBSearchEngine/MBSearchEngine.h>
bool HyperlinkIsRelative(std::string const& LinkToCheck)
{
	if (LinkToCheck.size() == 0)
	{
		return(false);
	}
	if (LinkToCheck[0] == '/')
	{
		return(false);
	}
	std::string AbsoluteHTTP = "http://";
	std::string AbsoluteHTTPS = "https://";
	if (LinkToCheck.substr(0, AbsoluteHTTP.size()) == AbsoluteHTTP || LinkToCheck.substr(0, AbsoluteHTTPS.size()) == AbsoluteHTTPS)
	{
		return(false);
	}
	return(true);
}
bool HyperlinkIsOnSite(std::string LinkToCheck, std::string const& DomainName)
{
	if (HyperlinkIsRelative(LinkToCheck))
	{
		return(true);
	}
	std::string WebsiteURL = DomainName;
	if (LinkToCheck[0] != '/')
	{
		std::vector<std::string> UnAllowedPrefixes = { "http://","https://" };
		for (size_t i = 0; i < UnAllowedPrefixes.size(); i++)
		{
			if (LinkToCheck.substr(0, UnAllowedPrefixes[i].size()) == UnAllowedPrefixes[i])
			{
				LinkToCheck = LinkToCheck.substr(UnAllowedPrefixes[i].size());
			}
		}
	}
	if (LinkToCheck.substr(0, WebsiteURL.size()) == WebsiteURL)
	{
		return(true);
	}
	return(false);
}
class WebsiteCrawlerCoordinator;
void IndexWebsiteResource(std::string WebsiteURL, std::string ResourceToIndex, WebsiteCrawlerCoordinator* Crawler);
class WebsiteCrawlerCoordinator
{
	friend void IndexWebsiteResource(std::string WebsiteURL, std::string ResourceToIndex, WebsiteCrawlerCoordinator* Crawler);
private:
	std::mutex m_IndexedResourcesMutex;
	std::unordered_map<std::string, bool> m_IndexedResources = {};
	std::unordered_map<std::string, bool> m_QueuedResources = {};
	std::unordered_map<std::string, bool> m_FailedResources = {};
	std::mutex m_OutputFolderMutex;
	std::string m_OutputFolder = "";
	MBSockets::ThreadPool m_ThreadPool = MBSockets::ThreadPool(16);
	std::mutex m_WebsiteURLMutex;
	std::string m_WebsiteURL;

	float m_WaitTime = 0.1;
	std::mutex m_ClockLock;
	std::condition_variable m_WaitConditional;
	void WaitForNextRequest()
	{
		std::unique_lock<std::mutex> Lock(m_ClockLock);
		m_WaitConditional.wait(Lock);
	}
	bool ResourceIsQueued(std::string const& ResourceToCheck)
	{
		std::lock_guard<std::mutex> Lock(m_IndexedResourcesMutex);
		return(m_QueuedResources.find(ResourceToCheck) != m_QueuedResources.end());
	}
	void AddFailedResource(std::string ResourceToAdd)
	{
		std::lock_guard<std::mutex> Lock(m_IndexedResourcesMutex);
		m_FailedResources[ResourceToAdd] = true;
	}
	std::atomic<bool> m_OnlyAddNew = false;
public:
	bool ResourceIsIndexed(std::string StringToCheck)
	{
		std::lock_guard<std::mutex> Lock(m_IndexedResourcesMutex);
		return(m_IndexedResources.find(StringToCheck) != m_IndexedResources.end());
	}
	void AddIndexedResource(std::string ResourceToAdd)
	{
		std::lock_guard<std::mutex> Lock(m_IndexedResourcesMutex);
		m_IndexedResources[ResourceToAdd] = true;
	}
	std::string GetOutputFolder()
	{
		std::lock_guard<std::mutex> Lock(m_OutputFolderMutex);
		return(m_OutputFolder);
	}
	void AddResourceToIndex(std::string ResourceToIndex)
	{
		if (!ResourceIsIndexed(ResourceToIndex) && !ResourceIsQueued(ResourceToIndex))
		{
			m_ThreadPool.AddTask(IndexWebsiteResource,m_WebsiteURL, ResourceToIndex, this);
			m_QueuedResources[ResourceToIndex] = true;
		}
	}
	WebsiteCrawlerCoordinator(std::string const& WebsiteURL,std::string const& OutputFolder,bool OnlyAddNew)
	{
		m_OnlyAddNew = OnlyAddNew;
		m_OutputFolder = OutputFolder;
		m_WebsiteURL = WebsiteURL;
	}
	void StartIndexing(std::string FirstResource)
	{
		AddResourceToIndex(FirstResource);
		m_ThreadPool.Start();
		std::clock_t Timer = clock();
		while (m_ThreadPool.NumberOfActiveWorkers() > 0 || m_ThreadPool.NumberOfQueuedTasks() > 0)
		{
			if ((clock() - Timer) / double(CLOCKS_PER_SEC) > m_WaitTime)
			{
				m_WaitConditional.notify_one();
				Timer = clock();
			}
		}
		m_ThreadPool.WaitForThreadsToFinish();
		std::cout << "Failed to save following resources" << std::endl;
		for (auto KeyPair : m_FailedResources)
		{
			std::cout << KeyPair.first << std::endl;
		}
	}
	bool HyperlinkIsOnCrawlSite(std::string LinkToCheck)
	{
		std::string WebsiteURL = "";
		{
			std::lock_guard<std::mutex> Lock(m_WebsiteURLMutex);
			WebsiteURL = m_WebsiteURL;
		}
		return(HyperlinkIsOnSite(LinkToCheck, WebsiteURL));
	}
};
MBError IndexWebsite(std::string const& WebsiteURL, std::string const& OutFolderDirectory,bool OnlyAddNew)
{
	MBError ReturnValue(true);
	WebsiteCrawlerCoordinator Coordinator(WebsiteURL,OutFolderDirectory,OnlyAddNew);
	Coordinator.StartIndexing("/");
	return(ReturnValue);
}
std::vector<std::string> GetHTTPHyperlinks(const std::string& HTMLString)
{
	//TODO skriv om denna kod med HTML pasern som förmodligen kommer behöva skrivas i framtiden
	size_t NewLinkPosition = 0;
	std::vector<std::string> ReturnValue;
	while (NewLinkPosition < HTMLString.size())
	{
		NewLinkPosition = std::min(HTMLString.find("href=\"", NewLinkPosition),HTMLString.find("src=\"",NewLinkPosition));
		if (NewLinkPosition == std::string::npos)
		{
			break;
		}
		if (HTMLString[NewLinkPosition] == 'h')
		{
			int NextQuotationMark = HTMLString.find("\"", NewLinkPosition + 6);
			ReturnValue.push_back(HTMLString.substr(NewLinkPosition + 6, (NextQuotationMark)-(NewLinkPosition + 6)));
			NewLinkPosition = NextQuotationMark + 1;
		}
		else
		{
			int NextQuotationMark = HTMLString.find("\"", NewLinkPosition + 5);
			ReturnValue.push_back(HTMLString.substr(NewLinkPosition + 5, (NextQuotationMark)-(NewLinkPosition + 5)));
			NewLinkPosition = NextQuotationMark + 1;
		}
	}
	//kollar efter om det finns en "Loctation: " grej för om den har flyttat
	int LocationPosition = HTMLString.find("Location: ",0,HTMLString.find("\r\n\r\n"));
	if (LocationPosition != HTMLString.npos)
	{
		int NextEndline = HTMLString.find("\r\n", LocationPosition);
		ReturnValue.push_back(HTMLString.substr(LocationPosition + 10, NextEndline - (LocationPosition + 10)));
	}
	return(ReturnValue);
}
MBSockets::HTTPDocumentType GetResponseDocumentType(std::string const& DocumentToCheck)
{
	MBSockets::HTTPDocumentType ReturnValue = MBSockets::HTTPDocumentType::Null;
	std::string TypeString = MBSockets::GetHeaderValue("Content-Type", DocumentToCheck);
	//vill bara ha delen som visar typen
	size_t SemicolonLocation = TypeString.find(';');
	if (SemicolonLocation != TypeString.npos)
	{
		TypeString = TypeString.substr(0, SemicolonLocation);
	}
	if (TypeString == "text/html")
	{
		ReturnValue = MBSockets::HTTPDocumentType::HTML;
	}
	return(ReturnValue);
}
std::string MakePathAbsolute(std::string const& AbsoluteBase, std::string const& RelativePath)
{
	std::filesystem::path ResultPath = AbsoluteBase;
	if (ResultPath.has_stem())
	{
		ResultPath = ResultPath.parent_path();
	}
	ResultPath /= RelativePath;
	return(ResultPath.lexically_normal().generic_string());
}
std::string MakeAbsoluteHTTPLinkRelativeToWebsite(std::string WebsiteName,std::string PathToMakeAbsolute)
{
	size_t WebsiteNameLocation = PathToMakeAbsolute.find(WebsiteName);
	std::string ReturnValue = PathToMakeAbsolute.substr(WebsiteNameLocation+WebsiteName.size());
	return(std::filesystem::path(ReturnValue).lexically_normal().generic_string());
}
void IndexWebsiteResource(std::string WebsiteURL,std::string ResourceToIndex,WebsiteCrawlerCoordinator* Crawler)
{
	try
	{
		Crawler->WaitForNextRequest();
		MBSockets::HTTPConnectSocket HTTPSocket(WebsiteURL, "80", MBSockets::TraversalProtocol::TCP);
		HTTPSocket.Connect();
		if (!HTTPSocket.IsValid())
		{
			return;
		}
		//DEBUG GREJER
		//ResourceToIndex = "/daniel/games/iji03.zip";
		//std::cout << "Get Data From " << ResourceToIndex << std::endl;
		std::string ResourceData = HTTPSocket.GetDataFromRequest("GET", ResourceToIndex);
		//std::cout << "Data From " << ResourceToIndex << " Recieved"<<std::endl;
		if (MBSockets::GetHeaderValue("Content-Length", ResourceData) == "")
		{
			std::cout << "Is Chunked" << std::endl;
		}
		else
		{
			//size_t BodyBeginning = ResourceData.find("\r\n\r\n") + 4;
			//if (ResourceData.size() - BodyBeginning != std::stoi(MBSockets::GetHeaderValue("Content-Length", ResourceData)))
			//{
			//	std::cout << "Content-Length doesnt match" << std::endl;
			//}
		}


		Crawler->AddIndexedResource(ResourceToIndex);
		std::vector<std::string> NewResourcesToIndex = {};
		if (GetResponseDocumentType(ResourceData) == MBSockets::HTTPDocumentType::HTML)
		{
			NewResourcesToIndex = GetHTTPHyperlinks(ResourceData);
		}
		for (size_t i = 0; i < NewResourcesToIndex.size(); i++)
		{
			if (Crawler->HyperlinkIsOnCrawlSite(NewResourcesToIndex[i]))
			{
				if (!HyperlinkIsRelative(NewResourcesToIndex[i]))
				{
					Crawler->AddResourceToIndex(MakeAbsoluteHTTPLinkRelativeToWebsite(WebsiteURL, NewResourcesToIndex[i]));
				}
				else
				{
					Crawler->AddResourceToIndex(MakePathAbsolute(ResourceToIndex, NewResourcesToIndex[i]));
				}
			}
			else
			{
				//External source 
				std::cout << "External href not downloaded: " << NewResourcesToIndex[i] << std::endl;
			}
		}
		//ANTAGANDE resursen denna funktion indexerar har inte sparats innan
		//ANTAGANDE Resursen vi får är alltid absolut
		//nu sparar vi resursen
		std::string OutFolder = Crawler->GetOutputFolder();
		std::filesystem::path NewFilePath = OutFolder + ResourceToIndex;
		NewFilePath = NewFilePath.lexically_normal();
		if (!NewFilePath.has_filename())
		{
			NewFilePath /= "__DirectoryResource";
		}
		else
		{
			std::filesystem::path ModifiedPath = "__"+NewFilePath.filename().generic_string();
			NewFilePath = NewFilePath.parent_path();
			NewFilePath = NewFilePath.append(ModifiedPath.generic_string());
		}
		if (!std::filesystem::exists(NewFilePath.parent_path()))
		{
			std::filesystem::create_directories(NewFilePath.parent_path());
		}
		//std::cout << "Saved " << ResourceToIndex << std::endl;
		std::fstream NewFile(NewFilePath, std::ios::binary | std::ios::out);
		//Vi vill inte ha med headern utan enbart kroppen
		size_t BodyBeginning = ResourceData.find("\r\n\r\n") + 4;
		NewFile.write(&ResourceData[BodyBeginning], ResourceData.size() - BodyBeginning);
		while (HTTPSocket.DataIsAvailable())
		{
			//std::cout << "Getting Partial data from " << ResourceToIndex << std::endl;
			ResourceData = HTTPSocket.HTTPGetData();
			//std::cout << "Data from " << ResourceToIndex << " recieved" << std::endl;
			NewFile << ResourceData;
		}
		std::cout << "Saved " << ResourceToIndex << std::endl;
		//NewFile << ResourceData;
	}
	catch (const std::exception&)
	{
		Crawler->AddFailedResource(ResourceToIndex);
	}
}
std::string ReadWholeHTMLDocument(std::string DocumentFilepath)
{
	size_t DocumentSize = std::filesystem::file_size(DocumentFilepath);
	std::string ReturnValue = std::string(DocumentSize, 0);
	std::fstream HTMLDocument = std::fstream(DocumentFilepath, std::ios::in | std::ios::binary);
	HTMLDocument.read(&ReturnValue[0], DocumentSize);
	return(ReturnValue);
}
MBError CopyHTMLFileRelative(std::string const& InputFilepath,std::string const& OutputFilepath,std::string const& WebsiteDomain)
{
	MBError ReturnValue(true);
	size_t ParseOffset = 0;
	std::string InputData = ReadWholeHTMLDocument(InputFilepath);
	std::fstream OutputFile = std::fstream(OutputFilepath, std::ios::out | std::ios::binary);
	while (ParseOffset < InputData.size())
	{
		int LinkTokenSize = 6;
		size_t NextLink = InputData.find("href=\"", ParseOffset);
		if (NextLink != InputData.npos)
		{
			if (InputData[NextLink] == 's')
			{
				LinkTokenSize = 5;
			}
		}
		//TODO Kanske blir fel om filepathen innehåller ", men inte helt säker på att det ens går
		size_t NextLinkEnd = InputData.find("\"", NextLink + LinkTokenSize);
		if (NextLink != InputData.npos)
		{
			OutputFile << InputData.substr(ParseOffset, NextLink + LinkTokenSize-ParseOffset);
			std::string OldFilepath = InputData.substr(NextLink + LinkTokenSize, NextLinkEnd - (NextLink + LinkTokenSize));
			std::string NewFilePath = "";
			if (HyperlinkIsOnSite(OldFilepath,WebsiteDomain))
			{
				if (!HyperlinkIsRelative(OldFilepath))
				{
					NewFilePath = MakeAbsoluteHTTPLinkRelativeToWebsite(WebsiteDomain, OldFilepath);
				}
				else
				{
					NewFilePath = OldFilepath;
				}
			}
			else
			{
				NewFilePath = OldFilepath;
			}
			OutputFile << NewFilePath<<"\"";
			ParseOffset = NextLinkEnd + 1;
		}
		else
		{
			OutputFile << InputData.substr(ParseOffset);
			break;
		}
	}


	return(ReturnValue);
}
void MakeSubdirectoryRelative(std::string SubdirectoryPath,std::string OutputPath, std::string WebsiteDomain)
{
	std::filesystem::directory_iterator TopDirectory = std::filesystem::directory_iterator(SubdirectoryPath);

}
void CopyHTMLResourceRelatively(std::string InputFilepath, std::string OutputFilepath, std::string WebsiteDomain)
{
	std::filesystem::path OutputFilepathPath = OutputFilepath;
	if (!std::filesystem::exists(OutputFilepathPath.parent_path()))
	{
		std::filesystem::create_directories(OutputFilepathPath.parent_path());
	}
	if (MrPostOGet::MimeSniffDocument(InputFilepath) != MBSockets::HTTPDocumentType::HTML)
	{
		std::filesystem::copy(InputFilepath, OutputFilepath);
	}
	else
	{
		CopyHTMLFileRelative(InputFilepath, OutputFilepath, WebsiteDomain);
	}
}
void MakeWebsiteDirectoryRelative(std::string WebsitedDirectory,std::string WebsiteOutDirectory, std::string WebsiteDomain)
{
	std::filesystem::directory_iterator TopDirectory = std::filesystem::directory_iterator(WebsitedDirectory);
	std::filesystem::create_directory(WebsiteOutDirectory);
	std::vector<std::thread*> SubEntries = {};
	for (auto& DirectoryEntry : TopDirectory)
	{
		if (!DirectoryEntry.is_directory())
		{
			CopyHTMLResourceRelatively(DirectoryEntry.path().generic_string(), WebsiteOutDirectory + DirectoryEntry.path().filename().generic_string(), WebsiteDomain);
		}
		else
		{
			std::string NewDirectoryPath = WebsiteOutDirectory + DirectoryEntry.path().stem().generic_string();
			SubEntries.push_back(new std::thread(MakeWebsiteDirectoryRelative, DirectoryEntry.path().generic_string()+"/", NewDirectoryPath+"/", WebsiteDomain));
			//MakeSubdirectoryRelative(DirectoryEntry.path().generic_string(),WebsiteOutDirectory+
		}
	}
	for (size_t i = 0; i < SubEntries.size(); i++)
	{
		SubEntries[i]->join();
		delete SubEntries[i];
	}
	std::cout << "Finished copying " << WebsitedDirectory << std::endl;
}

MBError CreateWebsiteIndex(std::string const& WebsiteFolder, std::string const& OutIndexFilename)
{
	MBError ReturnValue(true);
	std::filesystem::recursive_directory_iterator DirectoryIterator(WebsiteFolder);
	MBSearchEngine::MBIndex NewIndex;
	std::vector<std::string> FilesWithErrors = { };
	for (auto& DirectoryEntry : DirectoryIterator)
	{
		std::string Filepath = DirectoryEntry.path().generic_string();
		//if (DEBUGFilepath.find("__ijiguide_secrets.php") != DEBUGFilepath.npos)
		//{
		//	std::cout << "Innehåller iji" << std::endl;
		//}
		if (MrPostOGet::MimeSniffDocument(DirectoryEntry.path().generic_string()) == MBSockets::HTTPDocumentType::HTML)
		{
			std::string FileData = MrPostOGet::LoadWholeFile(DirectoryEntry.path().generic_string());
			MBError IndexResult = NewIndex.IndexHTMLData(FileData, DirectoryEntry.path().generic_string());
			if (!IndexResult)
			{
				FilesWithErrors.push_back(Filepath);
			}
		}
	}
	NewIndex.Finalize();
	NewIndex.Save(OutIndexFilename);
	std::cout << "Failed to index following files due to html parsing errors:" << std::endl;
	for (size_t i = 0; i < FilesWithErrors.size(); i++)
	{
		std::cout << FilesWithErrors[i] << std::endl;
	}
	//debug grej
	//MBSearchEngine::MBIndex DebugIndex(OutIndexFilename);
	//DebugIndex.Load(OutIndexFilename);
	return(ReturnValue);
}
MBError CreateWebsiteIndex(std::string const& WebsiteURLHeader, std::string const& WebsiteFolder, std::string const& OutIndexFilename)
{
	MBError ReturnValue(true);
	std::filesystem::recursive_directory_iterator DirectoryIterator(WebsiteFolder);
	MBSearchEngine::MBIndex NewIndex;
	std::vector<std::string> FilesWithErrors = { };
	for (auto& DirectoryEntry : DirectoryIterator)
	{
		std::string Filepath = DirectoryEntry.path().generic_string();
		//if (DEBUGFilepath.find("__ijiguide_secrets.php") != DEBUGFilepath.npos)
		//{
		//	std::cout << "Innehåller iji" << std::endl;
		//}
		if (MrPostOGet::MimeSniffDocument(DirectoryEntry.path().generic_string()) == MBSockets::HTTPDocumentType::HTML)
		{
			std::string FileData = MrPostOGet::LoadWholeFile(DirectoryEntry.path().generic_string());
			std::string RelativePath = std::filesystem::relative(DirectoryEntry.path().parent_path(), WebsiteFolder).generic_string();
			std::string FileName = DirectoryEntry.path().filename().generic_string().substr(2);
			if (FileName == "DirectoryResource")
			{
				FileName = "";
			}
			std::string FileIdentifier = WebsiteURLHeader + RelativePath+"/" + FileName;
			MBError IndexResult = NewIndex.IndexHTMLData(FileData, FileIdentifier);
			if (!IndexResult)
			{
				FilesWithErrors.push_back(Filepath);
			}
		}
	}
	NewIndex.Finalize();
	NewIndex.Save(OutIndexFilename);
	std::cout << "Failed to index following files due to html parsing errors:" << std::endl;
	for (size_t i = 0; i < FilesWithErrors.size(); i++)
	{
		std::cout << FilesWithErrors[i] << std::endl;
	}
	//debug grej
	//MBSearchEngine::MBIndex DebugIndex(OutIndexFilename);
	//DebugIndex.Load(OutIndexFilename);
	return(ReturnValue);
}