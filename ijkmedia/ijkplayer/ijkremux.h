#ifndef IJKPLAYER__IJKREMUX_H
#define IJKPLAYER__IJKREMUX_H

#include <stdint.h>
#include <stdlib.h>

#include "ffmpeg.h"

 bool ffmpeg_data_init(struct ffmpeg_data *data);

 bool open_video_codec(struct ffmpeg_data *data);
 void ffmpeg_data_free(struct ffmpeg_data *data);


 void ffmepg_write_video(struct ffmpeg_data *data, struct video_data *frame);
 void ffmepg_write_audio(struct ffmpeg_data   *data , struct audio_data *frame);


 void ffmepg_write_avpacket(struct ffmpeg_data   *data , AVPacket packet);

#endif//IJKPLAYER__IJKREMUX_H
