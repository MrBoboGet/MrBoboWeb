#pragma once
#include <MrBoboSockets.h>
#include <istream>
#include <fstream>
#include <vector>
#include <StringGrejer.h>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <stdio.h>
#include <ctime>

#ifdef __linux__
#include <pthread.h>
#endif // __linux__


//TDODO fixa så att DirExists faktiskt göär något 
//#include <io.h>
//#include <direct.h>
//#define MAXHEADERSIZE 1024
std::vector<std::string> GetAllHyperlinks(const std::string& HTMLString)
{
	int Index = 0;
	std::vector<std::string> ReturnValue;
	while (true)
	{
		Index = HTMLString.find("href=\"",Index);
		if (Index == std::string::npos)
		{
			break;
		}
		int NextQuotationMark = HTMLString.find("\"", Index + 6);
		ReturnValue.push_back(HTMLString.substr(Index + 6, (NextQuotationMark)-(Index+6)));
		Index = NextQuotationMark + 1;
	}
	//kollar efter om det finns en "Loctation: " grej för om den har flyttat
	int LocationPosition = HTMLString.find("Location: ");
	if (LocationPosition != HTMLString.npos)
	{
		int NextEndline = HTMLString.find("\n", LocationPosition);
		ReturnValue.push_back(HTMLString.substr(LocationPosition + 10, NextEndline - (LocationPosition + 10)));
	}
	return(ReturnValue);
}
void IndexWebsiteSubroutine(std::string ResourceToIndex, const std::string TopDomain, MBSockets::ThreadPool* ConnectedThreadPool);
std::mutex AccessToThreadPoolVector;
std::mutex AccessToGlobals;
std::atomic<int> FilesIndexed = { 0 };
std::unordered_map<std::string, bool> VisitedUrls = std::unordered_map<std::string, bool>(0);
std::vector<std::string> URLsToVisit = std::vector<std::string>(0);
std::condition_variable WaitConditional;
std::mutex ClockLock;
int SecondsToWait = 5;
unsigned int nthreads = std::thread::hardware_concurrency();
bool DirExists(const std::string& dirName_in)
{
	/*
		DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
	*/
	return(false);
}
int _mkdir(std::string Path)
{
	//TODO fixa så denna faktiskt gör något
	return(-1);
}
int IndexWebsite(std::string TopDomain,std::string ResourceToStartWith = "")
{
	//skapar socketen vi kommer använda
	VisitedUrls.clear();
	URLsToVisit.clear();
	if (!DirExists(ReplaceAll(ReplaceAll(TopDomain, "/", "_"), ":", "_").c_str()))
	{
		if (_mkdir(ReplaceAll(ReplaceAll(TopDomain, "/", "_"), ":", "_").c_str()) == -1)
		{
			std::cout << "Could not make directory" << std::endl;
			return(-1);
		}
	}
	MBSockets::ThreadPool ThreadPoolen(nthreads);
	ThreadPoolen.AddTask(IndexWebsiteSubroutine, ResourceToStartWith, TopDomain, &ThreadPoolen);
	ThreadPoolen.Start();
	clock_t Timer = clock();
	clock_t TimerBefore = -CLOCKS_PER_SEC*SecondsToWait;
	int NumberOfCycles = 0;
	while (true)
	{
		Timer = clock();
		if ((Timer-TimerBefore)/CLOCKS_PER_SEC > SecondsToWait)
		{
			TimerBefore = Timer;
			NumberOfCycles += 1;
			if (NumberOfCycles%4 == 0)
			{
				std::cout << "\n" << std::endl;
				std::cout << "Number of active threads " << ThreadPoolen.NumberOfActiveWorkers() << std::endl;
				std::cout << "Number of queued tasks " << ThreadPoolen.NumberOfQueuedTasks() << std::endl<<std::endl;
			}
			WaitConditional.notify_one();
		}
	}
	ThreadPoolen.WaitForThreadsToFinish();
	return(0);
}
void IndexWebsiteSubroutine(std::string ResourceToIndex, const std::string TopDomain,MBSockets::ThreadPool* ConnectedThreadPool)
{
	//för att indexa allt så tar vi bara 30 filer först
	if (FilesIndexed > 30)
	{
		return;
	}
	{
		std::unique_lock<std::mutex> Locken(ClockLock);
		WaitConditional.wait(Locken);
	}
	MBSockets::HTTPConnectSocket HttpConnecter(TopDomain, "80", MBSockets::TraversalProtocol::TCP);
	HttpConnecter.Connect();
	//tar reda på storleken av header och body datan, så vi bara behöver allokera så pass mycket minne som behövs
	std::string BodyContent = HttpConnecter.GetDataFromRequest("GET",ResourceToIndex);
	std::fstream FileToSave;
	FilesIndexed += 1;
	//tar bort \r så det ser lite mindre aids ut?
	ReplaceAll(&BodyContent, "\r", "");
	FileToSave.open(ReplaceAll(TopDomain, "/", "_") + "/" + ReplaceAll(ResourceToIndex, "/", "_") + ".txt", std::fstream::out);
	//tillskillnad från förrut inkluderar vi header filen när vi savar, men tbh är det helt okej, bara försumbart långsam,amre och tar lika mycket minne
	FileToSave.write(BodyContent.c_str(), BodyContent.size());
	FileToSave.close();

	//egentligen vill vi inte skapa en ny string på detta sätt då det dubblar minnesanvänding för denna sida, men men
	//std::string BodyContent(Buffer, LengthOfData);
	//delete[] Buffer;
	std::vector<std::string> HyperLinks = GetAllHyperlinks(BodyContent);
	//nu kollar vi att våra hyperlinks faktiskt tillhärsidan vi vill indexa, vill inte behöva indexa hela youtube för detta
	for (size_t i = 0; i < HyperLinks.size(); i++)
	{
		std::string TempString = HyperLinks[i];
		std::cout << HyperLinks[i] << std::endl;
		ReplaceAll(&TempString, "https://", "");
		ReplaceAll(&TempString, "http://", "");
		ReplaceAll(&TempString, "www.", "");
		//vi kan också kolla om får string inte innehåller någon av detta, dvs tempstring.size() == hyperlinks[i].size(), för att också få in relativa hrefs
		if(TempString.substr(0,TopDomain.size()-4) == TopDomain.substr(4) || TempString.size() == HyperLinks[i].size())
		{
			//nu använder vi våran mutex för att se till att ingen annan grej gör detta medans
			std::string NextResourceToGet = "";
			if (TempString.size() == HyperLinks[i].size())
			{
				//realtiv link, finns ett antal olika cases, den enklaste är om vi är i ett directory och en relativ path som inte börjar på ./ eller /
				ReplaceAll(&TempString, "./", "");
				if (ResourceToIndex.back() == '/' && (TempString.front() != '.' && TempString.front() != '/'))
				{
					//vi är i en folder och referar till ett element i den foldern
					NextResourceToGet = ResourceToIndex + TempString;
				}
				if (ResourceToIndex.back() != '/' && (TempString.front() != '.' && TempString.front() != '/'))
				{
					//vi är inte i en folder, och referar till ett element i foldern denna fil är i
					int PositionOfLastSlash = -1;
					for (size_t i = ResourceToIndex.size() - 1; i > 0; i--)
					{
						if (ResourceToIndex[i] == '/')
						{
							PositionOfLastSlash = i;
							break;
						}
					}
					if (PositionOfLastSlash != -1)
					{
						NextResourceToGet = ResourceToIndex.substr(0, PositionOfLastSlash + 1) + TempString;
					}
					else
					{
						NextResourceToGet = TempString;
					}
				}
				if (TempString.front() == '/')
				{
					NextResourceToGet = TempString.substr(1, TempString.size() - 1);
				}
				if (TempString.substr(0, 3) == "../")
				{
					//vi går till förrgående directory
					int PositionOfSecondLastSlash = -1;
					for (size_t i = ResourceToIndex.size() - 1; i > 0; i--)
					{
						if (ResourceToIndex[i] == '/' && PositionOfSecondLastSlash == -1)
						{
							PositionOfSecondLastSlash = i;
						}
						else if (ResourceToIndex[i] == '/' && PositionOfSecondLastSlash != -1)
						{
							PositionOfSecondLastSlash = i;
							break;
						}
					}
					if (PositionOfSecondLastSlash != -1)
					{
						NextResourceToGet = ResourceToIndex.substr(0, PositionOfSecondLastSlash + 1) + TempString;
					}
					else
					{
						NextResourceToGet = TempString;
					}
				}
			}
			else
			{
				NextResourceToGet = TempString.substr(TopDomain.size() - 3);
			}
			std::cout << NextResourceToGet << std::endl;
			const std::lock_guard<std::mutex> Lock(AccessToGlobals);
			if (VisitedUrls.find(NextResourceToGet) == VisitedUrls.end())
			{
				VisitedUrls[NextResourceToGet] = true;
				std::cout << "Hyperlink added " << NextResourceToGet << std::endl;
				//Denna url har inte visitats innan
				//vi kollar då om vi det finns en worker som är ledig, gör det det så skapar vi automatisk en ny 
				ConnectedThreadPool->AddTask(IndexWebsiteSubroutine, NextResourceToGet, TopDomain, ConnectedThreadPool);
			}
		}
	}
}

/*
	//för att indexa allt så tar vi bara 30 filer först
	if (FilesIndexed > 30)
	{
		return;
	}
	{
		std::unique_lock<std::mutex> Locken(ClockLock);
		WaitConditional.wait(Locken);
	}
	MBSockets::HTTPConnectSocket HttpConnecter(TopDomain, "80", MBSockets::TraversalProtocol::TCP);
	HttpConnecter.Connect();
	//tar reda på storleken av header och body datan, så vi bara behöver allokera så pass mycket minne som behövs
	HttpConnecter.Head();
	char* Buffer = new char[MAXHEADERSIZE];
	int LengthOfData;
	int BodySize = 0;
	std::string HeaderContent;
	while ((LengthOfData = HttpConnecter.RecieveData(Buffer, MAXHEADERSIZE)) > 0 && BodySize == 0)
	{
		//nu har vi fått headern i buffer
		HeaderContent = std::string(Buffer, LengthOfData);
	/*
		int LengthPos = HeaderContent.find("Content-Length: ");
		int FirstEndlineAfterContentPos = HeaderContent.find("\n",LengthPos);
		BodySize = std::stoi(HeaderContent.substr(LengthPos+16,FirstEndlineAfterContentPos-(LengthPos+16)));
		
BodySize = std::stoi(MBSockets::GetHeaderValue("Content-Length", HeaderContent));
break;
	}

	//nu har vi exakt hur lång vår request är, headerns storlek + bodysize
	int RequestSize = HeaderContent.size() + BodySize;
	delete[] Buffer;
	Buffer = new char[RequestSize];
	HttpConnecter.Get(ResourceToIndex);
	while ((LengthOfData = HttpConnecter.RecieveData(Buffer, RequestSize)) > 0)
	{
		//nu har vi fått headern i buffer
		//nu sparar vi detta som en fil
		//if (MBSockets::GetHeaderValue(")
		//{

		//}
		std::fstream FileToSave;
		FilesIndexed += 1;
		FileToSave.open(ReplaceAll(TopDomain, "/", "_") + "/" + ReplaceAll(ResourceToIndex, "/", "_") + ".txt", std::fstream::out);
		FileToSave.write(&Buffer[HeaderContent.size()], BodySize);
		FileToSave.close();
		break;
	}

	//egentligen vill vi inte skapa en ny string på detta sätt då det dubblar minnesanvänding för denna sida, men men
	std::string BodyContent(Buffer, LengthOfData);
	delete[] Buffer;
	std::vector<std::string> HyperLinks = GetAllHyperlinks(BodyContent);
	//nu kollar vi att våra hyperlinks faktiskt tillhärsidan vi vill indexa, vill inte behöva indexa hela youtube för detta
	for (size_t i = 0; i < HyperLinks.size(); i++)
	{
		std::string TempString = HyperLinks[i];
		std::cout << HyperLinks[i] << std::endl;
		ReplaceAll(&TempString, "https://", "");
		ReplaceAll(&TempString, "http://", "");
		ReplaceAll(&TempString, "www.", "");
		std::cout << TempString << std::endl;
		//vi kan också kolla om får string inte innehåller någon av detta, dvs tempstring.size() == hyperlinks[i].size(), för att också få in relativa hrefs
		if (TempString.substr(0, TopDomain.size() - 4) == TopDomain.substr(4))
		{
			//nu använder vi våran mutex för att se till att ingen annan grej gör detta medans
			const std::lock_guard<std::mutex> Lock(AccessToGlobals);
			if (VisitedUrls.find(HyperLinks[i]) == VisitedUrls.end())
			{
				VisitedUrls[HyperLinks[i]] = true;
				std::cout << "Hyperlink added " << HyperLinks[i] << std::endl;
				std::cout << "Resource to get is " << TempString.substr(TopDomain.size() - 3) << std::endl;
				//Denna url har inte visitats innan
				//vi kollar då om vi det finns en worker som är ledig, gör det det så skapar vi automatisk en ny 
				ConnectedThreadPool->AddTask(IndexWebsiteSubroutine, TempString.substr(TopDomain.size() - 3), TopDomain, ConnectedThreadPool);
			}
		}
	}
*/