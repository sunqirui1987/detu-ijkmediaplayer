#ifndef IJKSDL_WIN__DIRECRTSOUND_H
#define IJKSDL_WIN__DIRECRTSOUND_H

#include <dsound.h>
#include "ijksdl/ijksdl_stdinc.h"
#include "ijksdl/ijksdl_audio.h"
#include "ijksdl/ijksdl_aout.h"

typedef struct SDL_Win_DirectSound{
	LPDIRECTSOUND sound;
	LPDIRECTSOUNDBUFFER mixbuf;
	LPDIRECTSOUNDCAPTURE capture;
	LPDIRECTSOUNDCAPTUREBUFFER capturebuf;
	int num_buffers;
	DWORD lastchunk;
	Uint8 *locked_buf;
}SDL_Win_DirectSound;

#endif
