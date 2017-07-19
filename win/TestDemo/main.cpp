#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

extern "C"
{
#include "../win/ijkplayer/ijk_ffplay_decoder.h"
#include "libavformat/url.h"
#include "SDL.h"
}

SDL_Window   *screen;
SDL_Renderer *sdlRenderer;
SDL_Texture  *sdlTexture;
SDL_Rect     sdlRect;

static bool  sdl_init_flag = false;

void video_callback(void* opaque, VideoFrame *frame_callback)
{
	if (!sdl_init_flag){
		sdl_init_flag = true;
		int screen_w = frame_callback->w;
		int screen_h = frame_callback->h;
		int y_size = screen_w * screen_h;
		screen = SDL_CreateWindow("Simplest ffmpeg player Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL);
		if (!screen) {
			printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
			return ;
		}

		sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
		sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.w = screen_w;
		sdlRect.h = screen_h;
	}

	SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
			frame_callback->data[0], frame_callback->linesize[0],
			frame_callback->data[1], frame_callback->linesize[1],
			frame_callback->data[2], frame_callback->linesize[2]);

			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
}

void msg_callback(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2)
{
	switch (ijk_msgint)
	{
	case IJK_MSG_FLUSH:
		break;
	case IJK_MSG_ERROR:
		break;
	case IJK_MSG_PREPARED:
		printf("\nIJK_MSG_PREPARED call back......\n");
		break;
	case IJK_MSG_COMPLETED:
		break;
	case IJK_MSG_VIDEO_SIZE_CHANGED:
		break;
	case IJK_MSG_SAR_CHANGED:
		break;
	case IJK_MSG_VIDEO_RENDERING_START:
		break;
	case IJK_MSG_AUDIO_RENDERING_START:
		break;
	case IJK_MSG_VIDEO_ROTATION_CHANGED:
		break;
	case IJK_MSG_BUFFERING_START:
		break;
	case IJK_MSG_BUFFERING_END:
		break;
	case IJK_MSG_BUFFERING_UPDATE:
		break;
	case IJK_MSG_BUFFERING_BYTES_UPDATE:
		break;
	case IJK_MSG_BUFFERING_TIME_UPDATE:
		break;
	case IJK_MSG_SEEK_COMPLETE:
		break;
	case IJK_MSG_PLAYBACK_STATE_CHANGED:
		break;
	case IJK_MSG_TIMED_TEXT:
		break;
	case IJK_MSG_ACCURATE_SEEK_COMPLETE:
		break;
	case IJK_MSG_VIDEO_DECODER_OPEN:
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	//SDL init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	ijkFfplayDecoder_init();

	IjkFfplayDecoder *ijk_ffplay_decoder = ijkFfplayDecoder_create();

	ijkFfplayDecoder_setLogLevel(ijk_ffplay_decoder, k_IJK_LOG_ERROR);

	IjkFfplayDecoderCallBack *decoder_callback = (IjkFfplayDecoderCallBack *)malloc(sizeof(IjkFfplayDecoderCallBack));
	decoder_callback->func_get_frame = video_callback;
	decoder_callback->func_state_change = msg_callback;
	ijkFfplayDecoder_setDecoderCallBack(ijk_ffplay_decoder, NULL, decoder_callback);

	ijkFfplayDecoder_setDataSource(ijk_ffplay_decoder, "test.flv");

	ijkFfplayDecoder_prepare(ijk_ffplay_decoder);

	ijkFfplayDecoder_start(ijk_ffplay_decoder);

	ijkFfplayDecoder_setVolume(ijk_ffplay_decoder, 1.0, 1.0);

	bool ret = ijkFfplayDecoder_isPlaying(ijk_ffplay_decoder);
	printf("ijkFfplayDecoder_isPlaying: %s.\n", ret ? "true" : "false");

	Sleep(10000);

	//current position and duration
	long position = ijkFfplayDecoder_getCurrentPosition(ijk_ffplay_decoder);
	long duration = ijkFfplayDecoder_getDuration(ijk_ffplay_decoder);

	//code info
	char *videoinfo = (char*)malloc(2048);
	ijkFfplayDecoder_getVideoCodecInfo(ijk_ffplay_decoder, &videoinfo);
	char *audioinfo = (char*)malloc(2048);
	ijkFfplayDecoder_getAudioCodecInfo(ijk_ffplay_decoder, &audioinfo);
	free(videoinfo), free(audioinfo);
	getchar();
	return 0;
}