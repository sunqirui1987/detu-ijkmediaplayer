//
//  ijk_ffplay_decoder.c
//  IJKMediaPlayer
//
//  Created by chao on 2017/6/12.
//  Copyright © 2017 detu. All rights reserved.
//

#include "ijk_ffplay_decoder.h"
#include "ijkplayer/ijkplayer_internal.h"
#include "ijksdl/ijksdl_vout_internal.h"
#include "pipeline/ffpipeline_win.h"
#include "ijksdl_vout_win_ffmpeg.h"
#include "ijkplayer/ff_ffmsg_queue.h"

#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

struct IjkFfplayDecoder{
	IjkMediaPlayer *ijk_media_player;

	void *opaque;
	IjkFfplayDecoderCallBack *ijk_ffplayer_deocdecallback;

	sVideoFrame *current_frame;
};

typedef void(*msg_call_back)(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2);
static msg_call_back s_user_msg_callback;

static void message_loop_n(IjkMediaPlayer *mp)
{
	void* opaque = (void*)ijkmp_get_weak_thiz(mp);

	while (1) {
		AVMessage msg;
		int retval = ijkmp_get_msg(mp, &msg, 1);
		if (retval < 0)
			break;

		assert(retval > 0);

		switch (msg.what) {
		case FFP_MSG_FLUSH:
			MPTRACE("FFP_MSG_FLUSH:\n");
			s_user_msg_callback(opaque, IJK_MSG_FLUSH, 0, 0);
			break;
		case FFP_MSG_ERROR:
			MPTRACE("FFP_MSG_ERROR: %d\n", msg.arg1);
			s_user_msg_callback(opaque, IJK_MSG_ERROR, msg.arg1, 0);
			break;
		case FFP_MSG_PREPARED:
			MPTRACE("FFP_MSG_PREPARED.\n");
			s_user_msg_callback(opaque, IJK_MSG_PREPARED, 0, 0);
			break;
		case FFP_MSG_COMPLETED:
			MPTRACE("FFP_MSG_COMPLETED.\n");
			s_user_msg_callback(opaque, IJK_MSG_COMPLETED, 0, 0);
			break;
		case FFP_MSG_VIDEO_SIZE_CHANGED:
			MPTRACE("FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
			s_user_msg_callback(opaque, IJK_MSG_VIDEO_SIZE_CHANGED, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_SAR_CHANGED:
			MPTRACE("FFP_MSG_SAR_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
			s_user_msg_callback(opaque, IJK_MSG_SAR_CHANGED, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_VIDEO_RENDERING_START:
			MPTRACE("FFP_MSG_VIDEO_RENDERING_START.\n");
			s_user_msg_callback(opaque, IJK_MSG_VIDEO_RENDERING_START, 0, 0);
			break;
		case FFP_MSG_AUDIO_RENDERING_START:
			MPTRACE("FFP_MSG_AUDIO_RENDERING_START.\n");
			s_user_msg_callback(opaque, IJK_MSG_AUDIO_RENDERING_START, 0, 0);
			break;
		case FFP_MSG_VIDEO_ROTATION_CHANGED:
			MPTRACE("FFP_MSG_VIDEO_ROTATION_CHANGED: %d\n", msg.arg1);
			s_user_msg_callback(opaque, IJK_MSG_VIDEO_ROTATION_CHANGED, msg.arg1, 0);
			break;
		case FFP_MSG_BUFFERING_START:
			MPTRACE("FFP_MSG_BUFFERING_START.\n");
			s_user_msg_callback(opaque, IJK_MSG_BUFFERING_START, 0, 0);
			break;
		case FFP_MSG_BUFFERING_END:
			MPTRACE("FFP_MSG_BUFFERING_END:\n");
			s_user_msg_callback(opaque, IJK_MSG_BUFFERING_END, 0, 0);
			break;
		case FFP_MSG_BUFFERING_UPDATE:
			s_user_msg_callback(opaque, IJK_MSG_BUFFERING_UPDATE, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_BUFFERING_BYTES_UPDATE:
			//s_user_msg_callback(opaque, IJK_MSG_BUFFERING_BYTES_UPDATE, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_BUFFERING_TIME_UPDATE:
			//s_user_msg_callback(opaque, IJK_MSG_BUFFERING_TIME_UPDATE, msg.arg1, msg.arg2);
			break;
		case FFP_MSG_SEEK_COMPLETE:
			MPTRACE("FFP_MSG_SEEK_COMPLETE:\n");
			s_user_msg_callback(opaque, IJK_MSG_SEEK_COMPLETE, 0, 0);
			break;
		case FFP_MSG_PLAYBACK_STATE_CHANGED:
			s_user_msg_callback(opaque, IJK_MSG_PLAYBACK_STATE_CHANGED, 0, 0);
			break;
		default:
			//ALOGE("unknown FFP_MSG_xxx(%d)\n", msg.what);
			break;
		}
	}
}

static int message_loop(void *arg)
{
	IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;

	message_loop_n(mp);

	ijkmp_dec_ref_p(&mp);

	MPTRACE("message_loop exit.\n");
	return 0;
}

static int video_callback(void *arg, SDL_VoutOverlay* overlay)
{
	IjkFfplayDecoder *play_decoder = (IjkFfplayDecoder*)arg;

	play_decoder->current_frame->w = overlay->w;
	play_decoder->current_frame->h = overlay->h;
	play_decoder->current_frame->format = overlay->format;
	play_decoder->current_frame->planes = overlay->planes;

	for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i) {
		play_decoder->current_frame->data[i] = overlay->pixels[i];
		play_decoder->current_frame->linesize[i] = overlay->pitches[i];
	}

	play_decoder->ijk_ffplayer_deocdecallback->func_get_frame(play_decoder->opaque, play_decoder->current_frame);

	return 0;
}

static ijk_inject_callback s_decoder_inject_callback;
int decoder_inject_callback(void *opaque, int type, void *data, size_t data_size)
{
	if (s_decoder_inject_callback)
		return s_decoder_inject_callback(opaque, type, data, data_size);
	return 0;
}

int ijkFfplayDecoder_init(void)
{
	ijkmp_global_init();
	ijkmp_global_set_inject_callback(decoder_inject_callback);

	return 0;
}

int ijkFfplayDecoder_uninit(void)
{
	ijkmp_global_uninit();

	return 0;
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
	ijk_ffplay_decoder->current_frame = (sVideoFrame *)calloc(1, sizeof(sVideoFrame));

	return ijk_ffplay_decoder;

fail:
	ijkmp_dec_ref_p(&mp);
	return NULL;
}

int ijkFfplayDecoder_setLogLevel(IJKLogLevel log_level)
{
	ijkmp_global_set_log_level(log_level);

	return 0;
}

int ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, void* opaque, IjkFfplayDecoderCallBack* callback)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	//save opaque, used in callback func
	ijkmp_set_weak_thiz(decoder->ijk_media_player, opaque);
	decoder->opaque = opaque;

	decoder->ijk_ffplayer_deocdecallback = callback;
	s_user_msg_callback = callback->func_state_change;

	//set callback to ijksdl_vout_win_ffmpeg
	SDL_VoutWin_SetVideoDataCallback((void *)decoder, decoder->ijk_media_player->ffplayer->vout, video_callback);

	return 0;
}

int ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* file_absolute_path)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_set_data_source(decoder->ijk_media_player, file_absolute_path);
	if (ret == 0){
		ALOGV("setDataSource success: path %s.\n", file_absolute_path);
	}else {
		ALOGV("setDataSource failed: path %s.\n", file_absolute_path);
		return -1;
	}

	return 0;
}

int ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_prepare_async(decoder->ijk_media_player);
	if (ret == 0){
		ALOGV("ijkFfplayDecoder_prepare success.\n");
	} else {
		ALOGV("ijkFfplayDecoder_prepare failed.\n");
		return -1;
	}

	return 0;
}

int ijkFfplayDecoder_start(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_start(decoder->ijk_media_player);
	if (ret == 0){
		ALOGV("ijkFfplayDecoder_start success.\n");
	} else {
		ALOGV("ijkFfplayDecoder_start failed.\n");
		return 0;
	}

	return;
}

int ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_pause(decoder->ijk_media_player);
	if (ret == 0){
		ALOGV("ijkFfplayDecoder_pause success.\n");
	} else {
		ALOGV("ijkFfplayDecoder_pause failed.\n");
		return -1;
	}

	return 0;
}

int ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_stop(decoder->ijk_media_player);
	if (ret == 0){
		ALOGV("ijkFfplayDecoder_stop success.\n");
	}
	else {
		ALOGV("ijkFfplayDecoder_stop failed.\n");
		return -1;
	}

	ijkmp_shutdown(decoder->ijk_media_player);

	return 0;
}

int ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	int ret = ijkmp_seek_to(decoder->ijk_media_player, msec);
	if (ret == 0){
		ALOGV("ijkFfplayDecoder_seekTo success.\n");
	} else {
		ALOGV("ijkFfplayDecoder_seekTo failed.\n");
		return -1;
	}

	return 0;
}

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return false;
	}

	bool ret = ijkmp_is_playing(decoder->ijk_media_player);
	ALOGV("ijkFfplayDecoder is %s.\n", ret ? "playing" : "not playing");
	return ret;

}

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	long ret = ijkmp_get_current_position(decoder->ijk_media_player);
	ALOGV("current position %d\n", ret);
	return ret;
}

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	long ret = ijkmp_get_duration(decoder->ijk_media_player);
	ALOGV("duration long: %d\n", ret);
	return ret;
}

int ijkFfplayDecoder_release(IjkFfplayDecoder* decoder)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	ijkmp_dec_ref_p(&(decoder->ijk_media_player));

	if (decoder->current_frame){
		free(decoder->current_frame); 
	}
	if (decoder){
		free(decoder);
	}
	return -1;
}

int ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float volume)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	pthread_mutex_lock(&decoder->ijk_media_player->mutex);
	if (decoder->ijk_media_player && decoder->ijk_media_player->ffplayer && decoder->ijk_media_player->ffplayer->pipeline) {
		ffpipeline_win_set_volume(decoder->ijk_media_player->ffplayer->pipeline, volume, volume);
	}
	pthread_mutex_unlock(&decoder->ijk_media_player->mutex);
	ALOGV("set volume: %f.\n", volume);
	
	return 0;
}

int ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, long value)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	ijkmp_set_option_int(decoder->ijk_media_player, opt_category, key, value);

	return 0;
}

int ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, const char* value)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	ijkmp_set_option(decoder->ijk_media_player, opt_category, key, value);

	return 0;
}

int ijkFfplayDecoder_getVideoCodecInfo(IjkFfplayDecoder* decoder, char **codec_info)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	pthread_mutex_lock(&decoder->ijk_media_player->mutex);
	int ret = ffp_get_video_codec_info(decoder->ijk_media_player->ffplayer, codec_info);
	pthread_mutex_unlock(&decoder->ijk_media_player->mutex);
	
	return 0;
}

int ijkFfplayDecoder_getAudioCodecInfo(IjkFfplayDecoder* decoder, char **codec_info)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	pthread_mutex_lock(&decoder->ijk_media_player->mutex);
	int ret = ffp_get_audio_codec_info(decoder->ijk_media_player->ffplayer, codec_info);
	pthread_mutex_unlock(&decoder->ijk_media_player->mutex);
	
	return 0;
}

long ijkFfplayDecoder_getPropertyLong(IjkFfplayDecoder* decoder, int id, long default_value)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	long ret = ijkmp_get_property_int64(decoder->ijk_media_player, id, default_value);
	return ret;
}

float ijkFfplayDecoder_getPropertyFloat(IjkFfplayDecoder* decoder, int id, float default_value)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	float ret = ijkmp_get_property_float(decoder->ijk_media_player, id, default_value);
	return ret;
}

static char* fillMetaInternal(IjkMediaMeta *meta, const char *key, const char *default_value)
{
	char *value = ijkmeta_get_string_l(meta, key);
	if (value == NULL){
		value = default_value;
	} else {
		ALOGV("meta: %s\n", value);
	}

	return value;
}

int ijkFfplayDecoder_getMediaMeta(IjkFfplayDecoder* decoder, ijkMetadata* metadata)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	bool is_locked = false;
	char *media_info = NULL;
	IjkMediaMeta *meta = NULL;
	meta = ijkmp_get_meta_l(decoder->ijk_media_player);
	if (!meta){
		ALOGV("IjkMediaPlayer get meta error.\n");
		return -1;
	}

	ijkmeta_lock(meta);
	is_locked = true;

	media_info = fillMetaInternal(meta, IJKM_KEY_DURATION_US, NULL);
	metadata->duration_ms = atol(media_info) / 1000;

	size_t count = ijkmeta_get_children_count_l(meta);
	for (size_t i = 0; i < count; ++i) {
		IjkMediaMeta *streamRawMeta = ijkmeta_get_child_l(meta, i);
		if (streamRawMeta) {
			const char *type = ijkmeta_get_string_l(streamRawMeta, IJKM_KEY_TYPE);
			if (type) {
				if (0 == strcmp(type, IJKM_VAL_TYPE__VIDEO)) {
					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_CODEC_NAME, NULL);
					memcpy(metadata->video_code_name, media_info, sizeof(metadata->video_code_name));

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_CODEC_LONG_NAME, NULL);
					memcpy(metadata->video_code_long_name, media_info, sizeof(metadata->video_code_long_name));

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_BITRATE, NULL);
					metadata->video_bitrate = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_WIDTH, NULL);
					metadata->width = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_HEIGHT, NULL);
					metadata->height = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_FPS_NUM, NULL);
					metadata->video_fps_num = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_FPS_DEN, NULL);
					metadata->video_fps_den = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_TBR_NUM, NULL);
					metadata->video_tbr_num = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_TBR_DEN, NULL);
					metadata->video_tbr_den = atoi(media_info);
				} else if (0 == strcmp(type, IJKM_VAL_TYPE__AUDIO)) {
					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_CODEC_NAME, NULL);
					memcpy(metadata->audio_code_name, media_info, sizeof(metadata->video_code_name));

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_CODEC_LONG_NAME, NULL);
					memcpy(metadata->audio_code_long_name, media_info, sizeof(metadata->audio_code_long_name));

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_BITRATE, NULL);
					metadata->audio_bitrate = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_SAMPLE_RATE, NULL);
					metadata->audio_samples_per_sec = atoi(media_info);

					media_info = fillMetaInternal(streamRawMeta, IJKM_KEY_CHANNEL_LAYOUT, NULL);
					metadata->audio_channel_layout = atoi(media_info);
				}
			}
		}
	}

	if (is_locked && meta){
		ijkmeta_unlock(meta);
	}

	return 0;
}

int ijkFfplayDecoder_setHwDecoderName(IjkFfplayDecoder* decoder, const char* decoder_name)
{
	if (!decoder->ijk_media_player || !decoder){
		ALOGV("IjkMediaPlayer is NULL.\n");
		return -1;
	}

	ijkmp_set_decoder_name(decoder->ijk_media_player, decoder_name);
	return 0;
}
