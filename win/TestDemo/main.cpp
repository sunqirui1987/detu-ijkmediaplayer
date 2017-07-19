#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

extern "C"
{
#include "../win/ijkplayer/ijk_ffplay_decoder.h"
#include "libavformat/url.h"
#include "SDL.h"
}

IjkFfplayDecoder *ijk_ffplay_decoder;

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
		ijkFfplayDecoder_start(ijk_ffplay_decoder);
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

	ijk_ffplay_decoder = ijkFfplayDecoder_create();

	ijkFfplayDecoder_setLogLevel(ijk_ffplay_decoder, k_IJK_LOG_ERROR);

	IjkFfplayDecoderCallBack *decoder_callback = (IjkFfplayDecoderCallBack *)malloc(sizeof(IjkFfplayDecoderCallBack));
	decoder_callback->func_get_frame = video_callback;
	decoder_callback->func_state_change = msg_callback;
	ijkFfplayDecoder_setDecoderCallBack(ijk_ffplay_decoder, NULL, decoder_callback);

	ijkFfplayDecoder_setDataSource(ijk_ffplay_decoder, "test.flv");

	ijkFfplayDecoder_prepare(ijk_ffplay_decoder);

	static float volume = 1.0;
	static bool  is_pause = false;
	while (1){
		char input = ' ';
		scanf("%c", &input);
		if (input == 'p'){	//pause and play
			if (!is_pause){
				printf("pause player now.\n");
				is_pause = true;
				ijkFfplayDecoder_pause(ijk_ffplay_decoder);

				Sleep(50);

				bool ret = ijkFfplayDecoder_isPlaying(ijk_ffplay_decoder);
				printf("ijkFfplayDecoder_isPlaying: %s.\n", ret ? "true" : "false");
			} else {
				printf("resume player now.\n");
				is_pause = false;
				ijkFfplayDecoder_start(ijk_ffplay_decoder);

				Sleep(50);

				bool ret = ijkFfplayDecoder_isPlaying(ijk_ffplay_decoder);
				printf("ijkFfplayDecoder_isPlaying: %s.\n", ret ? "true" : "false");
			}
		}
		if (input == '+'){	//increase sound, 0.05 percent
			if (volume < 1.0){
				volume += 0.05;
				printf("increase volume now.\n");
				ijkFfplayDecoder_setVolume(ijk_ffplay_decoder, volume, volume);
			} else {
				printf("volume is bigest already.\n");
			}
		}
		if (input == '-'){	//decrease sound, 0.05 percent
			if (volume > 0.1){
				volume -= 0.05;
				printf("decrease volume now.\n");
				ijkFfplayDecoder_setVolume(ijk_ffplay_decoder, volume, volume);
			} else {
				printf("volume is zero already.\n");
			}
		}
		if (input == 'g'){	//get info 
			//current position and duration
			long position = ijkFfplayDecoder_getCurrentPosition(ijk_ffplay_decoder);
			long duration = ijkFfplayDecoder_getDuration(ijk_ffplay_decoder);
			printf("position:%f, duration:%f.\n", position, duration);

			//code info
			char *videoinfo = (char*)malloc(2048);
			char *audioinfo = (char*)malloc(2048);
			ijkFfplayDecoder_getVideoCodecInfo(ijk_ffplay_decoder, &videoinfo);
			ijkFfplayDecoder_getAudioCodecInfo(ijk_ffplay_decoder, &audioinfo);
			printf("videoinfo:%s, audioinfo:%s.\n", videoinfo, audioinfo);
			free(videoinfo), free(audioinfo);
		}
		if (input == 's'){	//seek, default seek to 15s position
			printf("seek to 15s position.\n");
			ijkFfplayDecoder_seekTo(ijk_ffplay_decoder, 15000);
		}
		if (input == 'S'){	//stop
			//TODO
		}
		if (input == 'r'){	//restart
			//TODO
		}

		Sleep(500);
	}

	return 0;
}