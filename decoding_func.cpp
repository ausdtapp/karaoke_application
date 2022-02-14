#include "decoding_func.h"


std::map<double, uint8_t*> video_frame_map;
std::map<double, uint8_t*>::iterator video_frame_iterator;
int video_width, video_height;
std::queue<Sound_Data> sample_queue;


//****************************************************************************************************************
//****************************************************************************************************************
// 
//Video functions


//This function decodes the video frames of the mp4 at the filepath specified and fills the map with timestamp the frame occurs and the frame data. It also returns the width and height that the video should be streamed at. 

bool decode_video(const char* filepath) {


    //Create format context

    const char* video_filepath = filepath;
    AVFormatContext* av_format_context;
    AVCodecContext* av_codec_context;
    AVCodecParameters* av_codec_params;
    const AVCodec* av_codec;
    AVFrame* av_frame;
    AVPacket* av_packet;
    int video_stream_index;
    AVRational time_base;
    SwsContext* sws_scaler_context;


    av_format_context = avformat_alloc_context();
    if (avformat_open_input(&av_format_context, video_filepath, NULL, NULL) != 0) {
        std::cout << "AV file not opened" << std::endl;
    }

    //Set stream index to 0 to target video frames
    video_stream_index = 0;
    av_codec_params = av_format_context->streams[0]->codecpar;
    av_codec = avcodec_find_decoder(av_codec_params->codec_id);
    
    //Pulls video's height and width entries from the codec parameters and assign to global video_width and video_height to use later in rendering function
    video_width = av_codec_params->width;
    video_height = av_codec_params->height;
    time_base = av_format_context->streams[0]->time_base;


    av_codec_context = avcodec_alloc_context3(av_codec);

    if (!av_codec_context) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_context, av_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_context, av_codec, NULL) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }
    av_frame = av_frame_alloc();
    if (!av_frame) {
        printf("Couldn't allocate AVFrame\n");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        printf("Couldn't allocate AVPacket\n");
        return false;
    }

    //Ensures the video_frame_map is empty before putting in video frames
    if (!video_frame_map.empty()) {
        std::map<double, uint8_t*> temp;
        std::swap(video_frame_map, temp);
    }


    int response;

    //Iterates through the all the frames contained in the data packet, resamples them, and places them in the map
    while (av_read_frame(av_format_context, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_context, av_packet);
        if (response < 0) {
            printf("Failed to decode packet\n");
            return false;
        }

        response = avcodec_receive_frame(av_codec_context, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
            break;
        }
        else if (response < 0) {
            printf("Failed to decode packet\n");
            return false;
        }

        //Frame data loaded into a buffer of uint8_t
        uint8_t* frame_data = new uint8_t[av_frame->width * av_frame->height * 4];

        uint8_t* dest[4] = { frame_data, NULL, NULL, NULL };
        int dest_linesize[4] = { video_width * 4, 0, 0, 0 };

        //Rescales pixel format to fit OpenGL contraints
        sws_scaler_context = sws_getContext(av_codec_context->width, av_codec_context->height, av_codec_context->pix_fmt, av_codec_context->width, av_codec_context->height, AV_PIX_FMT_RGB0, SWS_BICUBIC, NULL, NULL, NULL);

        if (sws_scaler_context == NULL) {
            std::cout << "PROBLEM WITH SWS SCALER" << std::endl;
        }

        response = sws_scale(sws_scaler_context, av_frame->data, av_frame->linesize, 0, av_codec_context->height, dest, dest_linesize);
        if (response < 0) {
            printf("sws_scale failed");
        }

        //Stores time and frame data as a pair in our global map variable
        video_frame_map.insert(std::pair<double, uint8_t*>((int)((av_frame->pts * (double)time_base.num / (double)time_base.den) * 1000.0) / 1000.0, frame_data));
    }
    av_packet_unref(av_packet);

    //Set video iterator to beginning of the new video_frame_map
    video_frame_iterator = video_frame_map.begin();
    
    return true;
}






//****************************************************************************************************************
//****************************************************************************************************************
// 
//Audio functions

//This callback function is called everytime a buffer needs to be filled with audio data while PaStream is running (ruding the video stream)
//It uses the global sample_queue object to fill the output with the amount of frames per buffer needed, where every frame consists of one queue entry that consists of a Sound_Data object that contains a left and right sample for each speaker
static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {

    float* output_data = (float*)outputBuffer;

    for (int i = 0; i < framesPerBuffer; i++) {
        Sound_Data data = sample_queue.front();
        *output_data++ = data.right_sound * .5;
        *output_data++ = data.left_sound * .5;
        sample_queue.pop();
    }


    return paContinue;
}




//This function decodes the audio frames of the mp4 at the filepath specified and fills a queue with the audio frame data

bool decode_audio(const char* filepath) {

    //Create format context

    const char* audio_filepath = filepath;
    AVFormatContext* av_format_context_audio;
    AVCodecContext* av_codec_context_audio;
    AVCodecParameters* av_codec_params_audio;
    const AVCodec* av_codec_audio;        
    AVFrame* av_frame_audio;
    AVPacket* av_packet_audio;
    int width;
    int height;
    int audio_stream_index;
    AVRational time_base;
    SwsContext* sws_scaler_context;

    int totalSamps;
    double SR;         
    static const int FPB = 2048; /* Frames per buffer: 46 ms buffers. */

    //Allocates context
    av_format_context_audio = avformat_alloc_context();
    if (avformat_open_input(&av_format_context_audio, filepath, NULL, NULL) != 0) {
        std::cout << "AV file not opened" << std::endl;
    }


    //Identifies codec needed to decode audio type
    audio_stream_index = 1; //Set to 1 for audio stream
    av_codec_params_audio = av_format_context_audio->streams[audio_stream_index]->codecpar;
    av_codec_audio = avcodec_find_decoder(av_codec_params_audio->codec_id);

    
    int audio_frame_size = av_codec_params_audio->frame_size;
    time_base = av_format_context_audio->streams[audio_stream_index]->time_base;

    av_codec_context_audio = avcodec_alloc_context3(av_codec_audio);
        if (!av_codec_context_audio) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_context_audio, av_codec_params_audio) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_context_audio, av_codec_audio, NULL) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }
    av_frame_audio = av_frame_alloc();
    if (!av_frame_audio) {
        printf("Couldn't allocate AVFrame\n");
        return false;
    }
    av_packet_audio = av_packet_alloc();
    if (!av_packet_audio) {
        printf("Couldn't allocate AVPacket\n");
        return false;
    }

    //Keeps track of errors
    int response;

    //Make sure the sample_queue is empty to fill with new song values
    if (!sample_queue.empty()) {
        std::queue <Sound_Data> new_queue;
    std:swap(sample_queue, new_queue);
    }

    //Iterates through each frame of the packet of audio data, reformats the unsigned 8 bit integer format into 32 bit floats and places them into the global sample_queue
    while (av_read_frame(av_format_context_audio, av_packet_audio) >= 0) {
        if (av_packet_audio->stream_index != audio_stream_index) {
            av_packet_unref(av_packet_audio);
            continue;
        }

        response = avcodec_send_packet(av_codec_context_audio, av_packet_audio);
        if (response < 0) {
            printf("Failed to decode packet\n");
            return false;
        }

        response = avcodec_receive_frame(av_codec_context_audio, av_frame_audio);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet_audio);
            continue;
            break;
        }
        else if (response < 0) {
            printf("Failed to decode packet\n");
            return false;
        }

        SR = av_frame_audio->sample_rate; //SAMPLE RATE TO PASS TO OPEN STREAM FUNCTION
        int bytes_per_sample = av_get_bytes_per_sample(av_codec_context_audio->sample_fmt);
        int samples_per_frame = av_frame_audio->nb_samples;
        int samples_sample_rate = av_frame_audio->sample_rate;

        //Calculating value for how many unsigned 8 bit integers to iterate through
        int buffer_entries = samples_per_frame * bytes_per_sample;




        for (int x = 0; x < buffer_entries; x += 4) {
            //Create sound data object to store the left and right samples
            Sound_Data extracted_data;
            extracted_data.timestamp = (int)((av_frame_audio->pts * (double)time_base.num / (double)time_base.den) * 1000.0) / 1000.0;

            //Utilizes bit shifting to change the format of each sample's four unint8_t values to one 32 bit float value for both left and right samples
            uint8_t temp_left[] = { av_frame_audio->data[0][x], av_frame_audio->data[0][x + 1], av_frame_audio->data[0][x + 2], av_frame_audio->data[0][x + 3] };
            uint8_t temp_right[] = { av_frame_audio->data[1][x], av_frame_audio->data[1][x + 1], av_frame_audio->data[1][x + 2], av_frame_audio->data[1][x + 3] };

            //Use memcpy to load 4 unsigned 8 bit ints into one float value for each left/right sample
            memcpy(&(extracted_data.left_sound), &temp_left, sizeof(extracted_data.left_sound));
            memcpy(&(extracted_data.right_sound), &temp_right, sizeof(extracted_data.right_sound));

            //Add entry to queue
            sample_queue.push(extracted_data);
        }
    }
    return true;
}


//This function initializes and starts the audio stream that uses the sample_queue
void initialize_audio(PaStream** stream) {
    PaError err;
    PaStreamParameters  outputParameters;

    err = Pa_Initialize();
    if (err != paNoError) std::cout << "Error at initialize" << std::endl;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;


    int placeholder = 1;

    //Take PaStream object from main and starts the stream with specified output parameters and sample rate, as well as our callback function
    err = Pa_OpenStream(&(*stream), 0, &outputParameters, 44100, 256, paClipOff, patestCallback, &placeholder);
    if (err != paNoError) {
        std::cout << "error after default stream" << std::endl;
    }

    //Starts the audio stream
    err = Pa_StartStream(*stream);
    if (err != paNoError) std::cout << "error after start stream" << std::endl;
}


//Stops audio stream
void stop_audio(PaStream** stream) {

    PaError err;
    err = Pa_StopStream(*stream);
    if (err != paNoError) std::cout << "error after sleep" << std::endl;
    err = Pa_CloseStream(*stream);
    if (err != paNoError) std::cout << "error after close stream" << std::endl;
    err = Pa_Terminate();

}