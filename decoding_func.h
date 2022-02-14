#pragma once

//FFMPEG Libraries
//#define inline __inline
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
	//#include <libavutil/pixdesc.h>
}
#include <inttypes.h>
#include <map>
#include <queue>
#include <iostream>
#include <string>

//PortAudio library
#include "portaudio.h"

//Sound_Data object stores enough data for one frame that consists of a left and right sample
typedef struct {
    float left_sound;
    float right_sound;
    double timestamp;
} Sound_Data;

//External video variables
extern std::map<double, uint8_t*> video_frame_map;
extern std::map<double, uint8_t*>::iterator video_frame_iterator;
extern int video_width, video_height;

//External audio variable
extern std::queue<Sound_Data> sample_queue;


//Decoding functions
bool decode_video(const char* filepath);
bool decode_audio(const char* filepath);

static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

void initialize_audio(PaStream** stream);

void stop_audio(PaStream** stream);
