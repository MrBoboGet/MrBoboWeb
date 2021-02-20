
#ifdef _WIN32
#include <unknwn.h>
#include<strmif.h>
#endif // __WIN32__
#include <vector>
#include <MrBoboMedia/MBMedia.h>
#include <iostream>
#include <filesystem>
extern "C"
{
	#include <ffmpeg/libavcodec/avcodec.h>
#include<ffmpeg/libavformat/avformat.h>
}
class MBDecodeContext
{
public:
	AVFormatContext* InputFormatContext = nullptr;
	std::vector<AVCodecParameters*> InputCodecParameters = {};
	std::vector<AVCodec*> InputCodecs = {};
	std::vector<AVCodecContext*> DecodeCodecContext = {};
	MBDecodeContext(std::string InputFilepath)
	{
		//transmux exempel
		InputFormatContext = avformat_alloc_context();
		//allokerar format kontexten, information om filtyp och innehåll,läser bara headers och etc
		avformat_open_input(&InputFormatContext, InputFilepath.c_str(), NULL, NULL);
		//läsar in data om själva datastreamsen
		avformat_find_stream_info(InputFormatContext, NULL);
		for (size_t i = 0; i < InputFormatContext->nb_streams; i++)
		{
			AVCodecParameters* NewInputCodecParamters = InputFormatContext->streams[i]->codecpar;
			InputCodecParameters.push_back(NewInputCodecParamters);
			AVCodec* NewInputCodec = avcodec_find_decoder(NewInputCodecParamters->codec_id);
			std::cout << NewInputCodec->name << std::endl;
			InputCodecs.push_back(NewInputCodec);
			//givet en codec och codec parameters så kan vi encoda/decoda data, men eftersom det är statefull kräver vi en encode/decode context
			AVCodecContext* NewCodexContext = avcodec_alloc_context3(NewInputCodec);
			avcodec_parameters_to_context(NewCodexContext, NewInputCodecParamters);
			//sedan måste vi öppna den, vet inte riktigt varför, initializerar den kanske?
			avcodec_open2(NewCodexContext, NewInputCodec, NULL);
			DecodeCodecContext.push_back(NewCodexContext);
		}
		//all data för att decoda insamlad
	}
	void FreeMemory()
	{
		avformat_close_input(&InputFormatContext);
		for (size_t i = 0; i < DecodeCodecContext.size(); i++)
		{
			avcodec_free_context(&DecodeCodecContext[i]);
		}
	}
};
class MBEncodeContext
{
public:
	std::string OutFileName = "";
	AVFormatContext* OutputFormatContext = nullptr;
	MBAudioCodecs TargetAudioCodec = MBAudioCodecs::Null;
	MBVideoCodecs TargetVideoCodec = MBVideoCodecs::Null;
	MBEncodeContext(std::string OutputFilepath)
	{
		avformat_alloc_output_context2(&OutputFormatContext, NULL, NULL, OutputFilepath.c_str());
		//all data för att decoda insamlad
	}
	MBEncodeContext(std::string OutputFilepath, MBAudioCodecs AudioCodecToUse, MBVideoCodecs VideoCodecToUse)
	{
		TargetAudioCodec = AudioCodecToUse;
		TargetVideoCodec = VideoCodecToUse;
		OutFileName = OutputFilepath;
		avformat_alloc_output_context2(&OutputFormatContext, NULL, NULL, OutputFilepath.c_str());
		//all data för att decoda insamlad
	}
	void FreeMemory()
	{

	}
};
std::string MBVideoCodecToString(MBVideoCodecs CodecToDecode)
{
	if (CodecToDecode == MBVideoCodecs::H265)
	{
		return("hevc_mf");
	}
	else if (CodecToDecode == MBVideoCodecs::H264)
	{
		return("h264");
	}
	return("");
}
std::string MBAudioCodecToString(MBAudioCodecs CodecToDecode)
{
	if (CodecToDecode == MBAudioCodecs::AAC)
	{
		return("aac");
	}
}
MBError InternalTranscode(MBDecodeContext* DecodeData, MBEncodeContext* EncodeData)
{
	AVCodec* VideoCodec = avcodec_find_encoder_by_name(MBVideoCodecToString(EncodeData->TargetVideoCodec).c_str());
	AVCodec* AudioCodec = avcodec_find_encoder_by_name(MBAudioCodecToString(EncodeData->TargetAudioCodec).c_str());
	AVCodecContext* VideoEncodeContext = avcodec_alloc_context3(VideoCodec);
	AVCodecContext* AudioEncodeContext = avcodec_alloc_context3(AudioCodec);
	//letar fram den första video codexen
	int FirstVideoIndex = -1;
	AVCodecParameters* FirstVideoParameters = nullptr;
	for (size_t i = 0; i < DecodeData->InputCodecParameters.size(); i++)
	{
		if (DecodeData->InputCodecParameters[i]->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			FirstVideoIndex = i;
			FirstVideoParameters = DecodeData->InputCodecParameters[i];
			break;
		}
	}
	AVRational input_framerate = av_guess_frame_rate(DecodeData->InputFormatContext, DecodeData->InputFormatContext->streams[FirstVideoIndex], NULL);
	AVCodecContext* FirstVideoContext = DecodeData->DecodeCodecContext[FirstVideoIndex];
	VideoEncodeContext->height = FirstVideoParameters->height;
	VideoEncodeContext->width = FirstVideoParameters->width;
	VideoEncodeContext->pix_fmt = VideoCodec->pix_fmts[0];
	//control rate
	VideoEncodeContext->bit_rate		= 2 * 1000 * 1000;
	VideoEncodeContext->rc_buffer_size	= 4 * 1000 * 1000;
	VideoEncodeContext->rc_max_rate		= 2 * 1000 * 10000;
	VideoEncodeContext->rc_min_rate		= 2.5 * 1000 * 1000;
	//timebase
	//testar för enkelhetens och vetenskapens skull att bara kopiera över alla data
	VideoEncodeContext->time_base = av_inv_q(input_framerate);

	int FirstAudioIndex = -1;
	for (size_t i = 0; i < DecodeData->InputCodecParameters.size(); i++)
	{
		if (DecodeData->InputCodecParameters[i]->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			FirstAudioIndex = i;
			break;
		}
	}
	AVCodecContext* FirstAudioContext = DecodeData->DecodeCodecContext[FirstAudioIndex];
	//AudioEncodeContext->height = FirstAudioContext->height;
	//AudioEncodeContext->width = FirstAudioContext->width;
	//AudioEncodeContext->pix_fmt = AudioCodec->pix_fmts[0];
	//control rate
	AudioEncodeContext->bit_rate		= 2 * 1000 * 1000;
	AudioEncodeContext->rc_buffer_size	= 4 * 1000 * 1000;
	AudioEncodeContext->rc_max_rate		= 2 * 1000 * 1000;
	AudioEncodeContext->rc_min_rate		= 2.5 * 1000 * 1000;
	//timebase
	//testar för enkelhetens och vetenskapens skull att bara kopiera över alla data
	AudioEncodeContext->time_base = FirstAudioContext->time_base;
	AudioEncodeContext->sample_fmt = AudioCodec->sample_fmts[0];
	//sample rate vad det nu betyder wtf
	AudioEncodeContext->sample_rate = FirstAudioContext->sample_rate;
	//nu har vi två förhoppningsvis fungerande decoders för ljud, då är det bara att faktiskt encoda våran frame
	avcodec_open2(VideoEncodeContext, VideoCodec, NULL);
	avcodec_open2(AudioEncodeContext, AudioCodec, NULL);


	//öppnar filen med nya streams som vi sedan kan skriva med

	for (size_t i = 0; i < DecodeData->InputFormatContext->nb_streams; i++)
	{
		AVStream* InputStream = DecodeData->InputFormatContext->streams[i];
		AVStream* OutputStream = NULL;
		//AVStream* InputStream = InputFormatContext->streams[i];
		OutputStream = avformat_new_stream(EncodeData->OutputFormatContext, NULL);
		//ser till att streamsen har rätt codex data som vi vill byta till
		if (InputStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			avcodec_parameters_from_context(OutputStream->codecpar, VideoEncodeContext);
		}
		else if (InputStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			avcodec_parameters_from_context(OutputStream->codecpar, AudioEncodeContext);
		}
		else
		{
			avcodec_parameters_copy(OutputStream->codecpar, DecodeData->InputCodecParameters[i]);
			//ser till att streamen tar global headers om format contexten gör det
			if (EncodeData->OutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			{
				OutputStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}
		}

	}
	avio_open(&EncodeData->OutputFormatContext->pb, EncodeData->OutFileName.c_str(), AVIO_FLAG_WRITE);
	avformat_write_header(EncodeData->OutputFormatContext, NULL);
	AVFrame* InputFrame = av_frame_alloc();
	AVPacket* InputPacket = av_packet_alloc();
	AVStream* InStream = NULL;
	AVStream* OutStream = NULL;
	while (av_read_frame(DecodeData->InputFormatContext, InputPacket) >= 0)
	{
		AVMediaType PacketMediaType = DecodeData->InputCodecParameters[InputPacket->stream_index]->codec_type;
		int StreamIndex = InputPacket->stream_index;
		InStream = DecodeData->InputFormatContext->streams[InputPacket->stream_index];
		OutStream = EncodeData->OutputFormatContext->streams[InputPacket->stream_index];
		if (PacketMediaType == AVMEDIA_TYPE_AUDIO || PacketMediaType == AVMEDIA_TYPE_VIDEO)
		{
			AVPacket* OutputPacket = av_packet_alloc();
			int Response = -1;
			AVCodecContext* DecodeContextToUse = DecodeData->DecodeCodecContext[StreamIndex];
			int response = avcodec_send_frame(DecodeContextToUse, InputFrame);
			while (response >= 0) 
			{
				response = avcodec_receive_packet(DecodeContextToUse, OutputPacket);
				if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) 
				{
					break;
				}
				else if (response < 0) 
				{
					return -1;
				}


				OutputPacket->stream_index = StreamIndex;
				OutputPacket->duration = OutStream->time_base.den / OutStream->time_base.num / InStream->avg_frame_rate.num * InStream->avg_frame_rate.den;

				av_packet_rescale_ts(OutputPacket, InStream->time_base, OutStream->time_base);
				response = av_interleaved_write_frame(EncodeData->OutputFormatContext, OutputPacket);
			}

			av_packet_unref(OutputPacket);
			av_packet_free(&OutputPacket);
		}
		else
		{
			//tar bara och skriver in den direkt eftersom vi inte vill hålla på och 

			InputPacket->pts = av_rescale_q_rnd(InputPacket->pts, InStream->time_base, OutStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			InputPacket->dts = av_rescale_q_rnd(InputPacket->dts, InStream->time_base, OutStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			InputPacket->duration = av_rescale_q(InputPacket->duration, InStream->time_base, OutStream->time_base);
			InputPacket->pos = -1;
			int ErrorCode = av_interleaved_write_frame(EncodeData->OutputFormatContext, InputPacket);
			if (ErrorCode < 0)
			{
				std::cout << "Error demuxin packet" << std::endl;
				break;
			}
			av_packet_unref(InputPacket);
		}
		//InputPacket->
		//int response = avcodec_send_packet(decoder_video_avcc, InputPacket);
		//while (response >= 0) 
		//{
		//	response = avcodec_receive_frame(decoder_video_avcc, InputFrame);
		//	if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) 
		//	{
		//		break;
		//	}
		//	else if (response < 0) {
		//		return response;
		//	}
		//	if (response >= 0) {
		//		encode(encoder_avfc, decoder_video_avs, encoder_video_avs, decoder_video_avcc, InputPacket->stream_index);
		//	}
		//	av_frame_unref(InputFrame);
		//}
		//av_packet_unref(InputPacket);
	}
	av_frame_unref(InputFrame);
	av_write_trailer(EncodeData->OutputFormatContext);
	return(MBError(false));
}
MBError Transcode(std::string InputFilepath, std::string OutputFilepath, MBVideoCodecs NewVideoCodec, MBAudioCodecs NewAudioCodec)
{
	MBDecodeContext DecodeContextToUse(InputFilepath);
	MBEncodeContext EncodeContextToUse(OutputFilepath, NewAudioCodec, NewVideoCodec);
	MBError ReturnValue = InternalTranscode(&DecodeContextToUse, &EncodeContextToUse);
	return(ReturnValue);
}
MBError Remux(std::string InputFilepath, std::string OutputFilepath)
{
	//transmux exempel
	AVFormatContext* InputFormatContext = avformat_alloc_context();
	//allokerar format kontexten, information om filtyp och innehåll,läser bara headers och etc
	avformat_open_input(&InputFormatContext, InputFilepath.c_str(), NULL, NULL);
	//läsar in data om själva datastreamsen
	avformat_find_stream_info(InputFormatContext, NULL);
	std::vector<AVCodecParameters*> InputCodecParameters = {};
	std::vector<AVCodec*> InputCodecs = {};
	std::vector<AVCodecContext*> DecodeCodecContext = {};
	for (size_t i = 0; i < InputFormatContext->nb_streams; i++)
	{
		AVCodecParameters* NewInputCodecParamters = InputFormatContext->streams[i]->codecpar;
		InputCodecParameters.push_back(NewInputCodecParamters);
		AVCodec* NewInputCodec = avcodec_find_decoder(NewInputCodecParamters->codec_id);
		std::cout << NewInputCodec->name << std::endl;
		InputCodecs.push_back(NewInputCodec);
		//givet en codec och codec parameters så kan vi encoda/decoda data, men eftersom det är statefull kräver vi en encode/decode context
		AVCodecContext* NewCodexContext = avcodec_alloc_context3(NewInputCodec);
		avcodec_parameters_to_context(NewCodexContext, NewInputCodecParamters);
		//sedan måste vi öppna den, vet inte riktigt varför, initializerar den kanske?
		avcodec_open2(NewCodexContext, NewInputCodec, NULL);
		DecodeCodecContext.push_back(NewCodexContext);
	}
	//all data för att decoda insamlad



	AVFormatContext* OutputFormatContext;
	avformat_alloc_output_context2(&OutputFormatContext, NULL, NULL, OutputFilepath.c_str());
	//skapar format kontexten som är vår out fil, sedan måste vi lägga till vad denna fil ska innehålla
	for (size_t i = 0; i < InputFormatContext->nb_streams; i++)
	{
		AVStream* OutputStream = NULL;
		//AVStream* InputStream = InputFormatContext->streams[i];
		OutputStream = avformat_new_stream(OutputFormatContext, NULL);
		avcodec_parameters_copy(OutputStream->codecpar, InputCodecParameters[i]);
		//ser till att streamen tar global headers om format contexten gör det
		if (OutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		{
			OutputStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	//tror den bara kommer gissa output format pga filename
	int ErrorCode = avio_open(&OutputFormatContext->pb, OutputFilepath.c_str(), AVIO_FLAG_WRITE);
	if (ErrorCode < 0)
	{
		std::cout << "Error opening output file" << std::endl;
	}
	ErrorCode = avformat_write_header(OutputFormatContext, NULL);
	if (ErrorCode < 0)
	{
		std::cout << "Error occurred when opening output file" << std::endl;
	}

	AVPacket ReadPacket;
	while (true)
	{
		ErrorCode = av_read_frame(InputFormatContext, &ReadPacket);
		if (ErrorCode < 0)
		{
			//all data är inläst
			break;
		}
		//vi skriver till den nya filen packet för packet
		AVStream* InStream = NULL;
		AVStream* OutStream = NULL;
		InStream = InputFormatContext->streams[ReadPacket.stream_index];
		OutStream = OutputFormatContext->streams[ReadPacket.stream_index];

		//Copy packet
		ReadPacket.pts = av_rescale_q_rnd(ReadPacket.pts, InStream->time_base, OutStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		ReadPacket.dts = av_rescale_q_rnd(ReadPacket.dts, InStream->time_base, OutStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		ReadPacket.duration = av_rescale_q(ReadPacket.duration, InStream->time_base, OutStream->time_base);
		ReadPacket.pos = -1;

		ErrorCode = av_interleaved_write_frame(OutputFormatContext, &ReadPacket);
		if (ErrorCode < 0)
		{
			std::cout << "Error demuxin packet" << std::endl;
			break;
		}
		av_packet_unref(&ReadPacket);
	}
	av_write_trailer(OutputFormatContext);

	avformat_close_input(&InputFormatContext);
	avio_closep(&OutputFormatContext->pb);

	avformat_free_context(OutputFormatContext);
	return(MBError(true));
}

MBError CreateHLSStream(std::string InputFilePath,std::string OutputFolderName, float TargetDuration)
{
	std::filesystem::create_directory(OutputFolderName);
	clock_t Timer = clock();
	std::string CommandString = "ffmpeg -hide_banner -loglevel error -i ";
	CommandString += InputFilePath;
	std::string Resolution = "720p";
	CommandString += " -c:a aac -ar 48000 -b:a 128k -c:v h264 -profile:v main -crf 20 -g 48 -keyint_min 96 -sc_threshold 0 -b:v 2500k -maxrate 2675k -bufsize 3750k -hls_time 8 -hls_playlist_type vod -hls_segment_filename ";
	//skapar ett nytt directory som vi sparar filerna i
	std::string HLS_FileName = OutputFolderName + "/" + Resolution + "_%03d.ts";
	std::string PlayListFilename = OutputFolderName + "/" + "MasterPlaylist" + ".m3u8";
	CommandString += HLS_FileName;
	CommandString += " " + PlayListFilename;
	std::system(CommandString.c_str());
	std::cout << (clock() - Timer) / double(CLOCKS_PER_SEC) << std::endl;
	return(MBError(false));
}