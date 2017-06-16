//
//  ijk_ffplay_decoder.c
//  IJKMediaPlayer
//
//  Created by chao on 2017/6/12.
//  Copyright © 2017年 detu. All rights reserved.
//

#include "ijk_ffplay_decoder.h"
#include "ijkplayer/ijkplayer_internal.h"
#include "ijksdl/ijksdl_vout_internal.h"
#include "pipeline/ffpipeline_win.h"
#include "ijksdl_vout_win_ffmpeg.h"
#include "ijkplayer/ff_ffmsg_queue.h"


struct IjkFfplayDecoder{
	IjkMediaPlayer *ijk_media_player;

	void *opaque;
	IjkFfplayDecoderCallBack *ijk_ffplayer_deocdecallback;
};


static void message_loop_n(IjkFfplayDecoder *ijk_ffplay_decoder)
{
	while (1) {
		AVMessage msg;

		int retval = ijkmp_get_msg(ijk_ffplay_decoder->ijk_media_player, &msg, 1);
		if (retval < 0)
			break;

		// block-get should never return 0
		assert(retval > 0);

		switch (msg.what) {
		case FFP_MSG_FLUSH:
			MPTRACE("FFP_MSG_FLUSH:\n");
			ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_FLUSH, 0, 0);
			break;
		case FFP_MSG_ERROR:
			MPTRACE("FFP_MSG_ERROR: %d\n", msg.arg1);
			ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_ERROR, -1, msg.arg1);
			break;
		case FFP_MSG_PREPARED:
			MPTRACE("FFP_MSG_PREPARED:\n");
			ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_PREPARED, 0, 0);
			break;
		case FFP_MSG_COMPLETED:
			MPTRACE("FFP_MSG_COMPLETED:\n");
			ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_COMPLETED, 0, 0);
			break;
		case FFP_MSG_VIDEO_SIZE_CHANGED:
			MPTRACE("FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
			ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_VIDEO_SIZE_CHANGED, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_SAR_CHANGED:
			MPTRACE("FFP_MSG_SAR_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
			//post_event(env, weak_thiz, MEDIA_SET_VIDEO_SAR, msg.arg1, msg.arg2);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_SAR_CHANGED, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_VIDEO_RENDERING_START:
			MPTRACE("FFP_MSG_VIDEO_RENDERING_START:\n");
			//post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_VIDEO_RENDERING_START, 0);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_VIDEO_RENDERING_START, 0, 0);
			break;
		case FFP_MSG_AUDIO_RENDERING_START:
			MPTRACE("FFP_MSG_AUDIO_RENDERING_START:\n");
			//post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_AUDIO_RENDERING_START, 0);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_AUDIO_RENDERING_START, 0, 0);
			break;
		case FFP_MSG_VIDEO_ROTATION_CHANGED:
			MPTRACE("FFP_MSG_VIDEO_ROTATION_CHANGED: %d\n", msg.arg1);
			//post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_VIDEO_ROTATION_CHANGED, msg.arg1);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_VIDEO_ROTATION_CHANGED, 0, 0);
			break;
		case FFP_MSG_BUFFERING_START:
			MPTRACE("FFP_MSG_BUFFERING_START:\n");
			//post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_START, 0);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_BUFFERING_START, 0, 0);
			break;
		case FFP_MSG_BUFFERING_END:
			MPTRACE("FFP_MSG_BUFFERING_END:\n");
			//post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_END, 0);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_BUFFERING_END, 0, 0);
			break;
		case FFP_MSG_BUFFERING_UPDATE:
			// MPTRACE("FFP_MSG_BUFFERING_UPDATE: %d, %d", msg.arg1, msg.arg2);
			//post_event(env, weak_thiz, MEDIA_BUFFERING_UPDATE, msg.arg1, msg.arg2);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_BUFFERING_UPDATE, 0, 0);
			break;
		case FFP_MSG_BUFFERING_BYTES_UPDATE:
			break;
		case FFP_MSG_BUFFERING_TIME_UPDATE:
			break;
		case FFP_MSG_SEEK_COMPLETE:
			MPTRACE("FFP_MSG_SEEK_COMPLETE:\n");
			//post_event(env, weak_thiz, MEDIA_SEEK_COMPLETE, 0, 0);
			//ijk_ffplay_decoder->ijk_ffplayer_deocdecallback->func_state_change(ijk_ffplay_decoder->opaque, FFP_MSG_SEEK_COMPLETE, 0, 0);
			break;
		case FFP_MSG_PLAYBACK_STATE_CHANGED:
			break;
		default:
			ALOGE("unknown FFP_MSG_xxx(%d)\n", msg.what);
			break;
		}
	}
}

static int message_loop(void *arg)
{
	IjkFfplayDecoder *ijk_ffplay_decoder = (IjkFfplayDecoder*)arg;

	message_loop_n(ijk_ffplay_decoder);

	ijkmp_dec_ref_p(&ijk_ffplay_decoder->ijk_media_player);

	MPTRACE("message_loop exit");
	return 0;
}

static ijk_inject_callback s_inject_callback;
int inject_callback(void *opaque, int type, void *data, size_t data_size)
{
	if (s_inject_callback)
		return s_inject_callback(opaque, type, data, data_size);
	return 0;
}

void ijkFfplayDecoder_init(void)
{
	ijkmp_global_init();
	ijkmp_global_set_inject_callback(inject_callback);
}

void ijkFfplayDecoder_uninit(void)
{
	ijkmp_global_uninit();
}

IjkFfplayDecoder *ijkFfplayDecoder_create(void)
{
	IjkMediaPlayer *mp = ijkmp_create(message_loop);
	if (!mp)
		goto fail;

	mp->ffplayer->vout = SDL_VoutWin_CreateForWindows();
	if (!mp->ffplayer->vout)
		goto fail;
	ALOGV("create vout success.\n");

	mp->ffplayer->pipeline = ffpipeline_create_from_win(mp->ffplayer);
	if (!mp->ffplayer->pipeline)
		goto fail;
	ALOGV("create pipeline success.\n");

	IjkFfplayDecoder *ijk_ffplay_decoder = (IjkFfplayDecoder *)malloc(sizeof(IjkFfplayDecoder));
	memset(ijk_ffplay_decoder, 0, sizeof(IjkFfplayDecoder));
	ijk_ffplay_decoder->ijk_media_player = mp;

	return ijk_ffplay_decoder;

fail:
	ijkmp_dec_ref_p(&mp);
	return NULL;
}

void ijkFfplayDecoder_delete(IjkFfplayDecoder *decoder)
{
	if (!decoder->ijk_media_player){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return;
	}

	ijkmp_dec_ref_p(&(decoder->ijk_media_player));
}

void ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* fileAbsolutePath)
{
	if (!decoder->ijk_media_player){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return;
	}

	int ret = ijkmp_set_data_source(decoder->ijk_media_player, fileAbsolutePath);
	if (ret == 0){
		ALOGV("setDataSource success: path %s.\n", fileAbsolutePath);
	}
}

void ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		ijkmp_prepare_async(decoder->ijk_media_player);
		//ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

void ijkFfplayDecoder_start(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		ijkmp_start(decoder->ijk_media_player);
		//ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

void ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		ijkmp_pause(decoder->ijk_media_player);
		ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

void ijkFfplayDecoder_reset(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		ijkmp_shutdown(decoder->ijk_media_player);
		ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

void ijkFfplayDecoder_release(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		ijkmp_shutdown(decoder->ijk_media_player);
		ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder)
{
	if (decoder->ijk_media_player){
		bool ret = ijkmp_is_playing(decoder->ijk_media_player);
		//ijkmp_dec_ref_p(&(decoder->ijk_media_player));
		return ret;
	}
	return false;
}

void ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec)
{
	if (decoder->ijk_media_player){
		ijkmp_seek_to(decoder->ijk_media_player, msec);
		ijkmp_dec_ref_p(&(decoder->ijk_media_player));
	}
}

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder)
{

}

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder)
{

}

void ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float leftVolume, float rightVolume)
{

}

void ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, const char* key, long value)
{

}

void ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, const char* key, const char* value)
{

}

void ijkFfplayDecoder_setLogLevel(IjkFfplayDecoder* decoder, IJKLogLevel logLevel)
{
	ijkmp_global_set_log_level(k_IJK_LOG_DEBUG);
}

void ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, void* opaque, IjkFfplayDecoderCallBack* callBack)
{
	if (decoder->ijk_media_player){
		decoder->opaque = opaque;
		decoder->ijk_ffplayer_deocdecallback = callBack;

		//TODO set callback to ijksdl_vout_win_ffmpeg
	}
}