#pragma once
#include <MBUtility/MBErrorHandling.h>
#include <string>
enum class MBVideoCodecs
{
	H264,
	H265,
	Null
};
enum class MBAudioCodecs
{
	AAC,
	Null
};
MBError CreateHLSStream(std::string FilePath,std::string OutputFoldername,float TargetDuration);
MBError Transcode(std::string InputFilepath, std::string OutputFilepath, MBVideoCodecs NewVideoCodec, MBAudioCodecs NewAudioCodec);
MBError CreateHLSFile(std::string FilePath, float TargetDuration, float FileIndex);
