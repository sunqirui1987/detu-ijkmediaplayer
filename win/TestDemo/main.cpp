#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#if defined(WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

extern "C"
{
#include "../win/ijkplayer/ijk_ffplay_decoder.h"
#include "libavformat/url.h"
#include "SDL.h"
}

FILE *fp_yuv;
int y_size;
int screen_w = 0, screen_h = 0;
SDL_Window *screen;
SDL_Renderer* sdlRenderer;
SDL_Texture* sdlTexture;
SDL_Rect sdlRect;

void video_callback(void* opaque, VideoFrame *frame_callback)
{
	//int y_size = frame_callback->w * frame_callback->h;
	//fwrite(frame_callback->data[0], 1, y_size, fp_yuv);	   //Y 
	//fwrite(frame_callback->data[1], 1, y_size / 4, fp_yuv);  //U
	//fwrite(frame_callback->data[2], 1, y_size / 4, fp_yuv);  //V
	//fflush(fp_yuv);

	SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
			frame_callback->data[0], frame_callback->linesize[0],
			frame_callback->data[1], frame_callback->linesize[1],
			frame_callback->data[2], frame_callback->linesize[2]);

			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);

	SYSTEMTIME win_time;
	GetLocalTime(&win_time);
	printf("%02d:%02d:%02d.%03d\n", win_time.wHour, win_time.wMinute, win_time.wSecond, win_time.wMilliseconds);
}

void msg_callback(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2)
{

}

int main(int argc, char** argv)
{
	//SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	screen_w = 320;
	screen_h = 240;
	y_size = screen_w * screen_h;
	screen = SDL_CreateWindow("Simplest ffmpeg player Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL);
	if (!screen) {
		printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
		return -1;
	}

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;
	//SDL End------------------------
	fp_yuv = fopen("use_callback.yuv","wb+");

	ijkFfplayDecoder_init();

	IjkFfplayDecoder *ijk_ffplay_decoder = ijkFfplayDecoder_create();

	ijkFfplayDecoder_setLogLevel(ijk_ffplay_decoder, k_IJK_LOG_ERROR);

	IjkFfplayDecoderCallBack *decoder_callback = (IjkFfplayDecoderCallBack *)malloc(sizeof(IjkFfplayDecoderCallBack));
	decoder_callback->func_get_frame = video_callback;
	decoder_callback->func_state_change = msg_callback;
	ijkFfplayDecoder_setDecoderCallBack(ijk_ffplay_decoder, NULL, decoder_callback);

	ijkFfplayDecoder_setOptionStringValue(ijk_ffplay_decoder, 1, "allowed_media_types", "video");

	ijkFfplayDecoder_setDataSource(ijk_ffplay_decoder, "test.flv");

	ijkFfplayDecoder_prepare(ijk_ffplay_decoder);

	ijkFfplayDecoder_start(ijk_ffplay_decoder);

	ijkFfplayDecoder_setVolume(ijk_ffplay_decoder, 0.8, 0.8);

	bool ret = ijkFfplayDecoder_isPlaying(ijk_ffplay_decoder);
	printf("ijkFfplayDecoder_isPlaying: %s.\n", ret ? "true" : "false");

	Sleep(5000);

	char *videoinfo = (char*)malloc(2048);
	ijkFfplayDecoder_getVideoCodecInfo(ijk_ffplay_decoder, &videoinfo);
	char *audioinfo = (char*)malloc(2048);
	ijkFfplayDecoder_getAudioCodecInfo(ijk_ffplay_decoder, &audioinfo);
	free(videoinfo), free(audioinfo);
	getchar();
	return 0;
}