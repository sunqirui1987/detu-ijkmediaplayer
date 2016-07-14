#include "ijkremux.h"
#include "ff_cmdutils.h"

/* ------------------------------------------------------------------------- */

static bool new_stream(struct ffmpeg_data *data, AVStream **stream,
                       AVCodec **codec, enum AVCodecID id, const char *name)
{
    *codec = (!!name && *name) ?
    avcodec_find_encoder_by_name(name) :
    avcodec_find_encoder(id);
    
    if (!*codec) {
        av_log(NULL, AV_LOG_WARNING, "Couldn't find encoder '%s'",
             avcodec_get_name(id));
        return false;
    }
    
    *stream = avformat_new_stream(data->output, *codec);
    if (!*stream) {
        av_log(NULL, AV_LOG_WARNING, "Couldn't create stream for encoder '%s'",
             avcodec_get_name(id));
        return false;
    }
    
    (*stream)->id = data->output->nb_streams-1;
    return true;
}

bool open_video_codec(struct ffmpeg_data *data)
{
    AVCodecContext *context = data->video->codec;

    int ret;
    
    if (strcmp(data->vcodec->name, "libx264") == 0)
        av_opt_set(context->priv_data, "preset", "veryfast", 0);
    
    av_opt_set(context->priv_data, "tune", "zerolatency", 0);
 
    
    ret = avcodec_open2(context, data->vcodec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "Failed to open video codec: %s",
             av_err2str(ret));
        return false;
    }
    
    data->vframe = av_frame_alloc();
    if (!data->vframe) {
        av_log(NULL, AV_LOG_WARNING, "Failed to allocate video frame");
        return false;
    }
    
    data->vframe->format = context->pix_fmt;
    data->vframe->width  = context->width;
    data->vframe->height = context->height;
    data->vframe->colorspace = data->config.color_space;
    data->vframe->color_range = data->config.color_range;
    
    ret = avpicture_alloc(&data->dst_picture, context->pix_fmt,
                          context->width, context->height);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "Failed to allocate dst_picture: %s",
             av_err2str(ret));
        return false;
    }
    
    *((AVPicture*)data->vframe) = data->dst_picture;
    return true;
}

static bool init_swscale(struct ffmpeg_data *data, AVCodecContext *context)
{
    data->swscale = sws_getContext(
                                   data->config.width, data->config.height,
                                   data->config.format,
                                   data->config.scale_width, data->config.scale_height,
                                   context->pix_fmt,
                                   SWS_BICUBIC, NULL, NULL, NULL);
    
    if (!data->swscale) {
        av_log(NULL, AV_LOG_WARNING, "Could not initialize swscale");
        return false;
    }
    
    return true;
}

static bool create_video_stream(struct ffmpeg_data *data)
{
    enum AVPixelFormat closest_format;
    AVCodecContext *context;
    
  
    
    if (!new_stream(data, &data->video, &data->vcodec,
                    data->output->oformat->video_codec,
                    data->config.video_encoder))
        return false;
    
    closest_format = get_closest_format(data->config.format,
                                        data->vcodec->pix_fmts);
    
    context                 = data->video->codec;
    context->bit_rate       = data->config.video_bitrate * 1000;
    context->width          = data->config.scale_width;
    context->height         = data->config.scale_height;
    context->time_base      = (AVRational){ 1,data->config.fps};
    context->gop_size       = 120;
    context->pix_fmt        = closest_format;
    context->colorspace     = data->config.color_space;
    context->color_range    = data->config.color_range;
    context->thread_count   = 0;
    
    data->video->time_base = context->time_base;
    
    if (data->output->oformat->flags & AVFMT_GLOBALHEADER)
        context->flags |= CODEC_FLAG_GLOBAL_HEADER;
    
    if (!open_video_codec(data))
        return false;
    
    if (context->pix_fmt    != data->config.format ||
        data->config.width  != data->config.scale_width ||
        data->config.height != data->config.scale_height) {
        
        if (!init_swscale(data, context))
            return false;
    }
    
    return true;
}

static bool open_audio_codec(struct ffmpeg_data *data)
{
    AVCodecContext *context = data->audio->codec;
   
    int ret;
    
  
    
    data->aframe = av_frame_alloc();
    if (!data->aframe) {
        av_log(NULL, AV_LOG_WARNING, "Failed to allocate audio frame");
        return false;
    }
    
    context->strict_std_compliance = -2;
    
    ret = avcodec_open2(context, data->acodec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "Failed to open audio codec: %s",
             av_err2str(ret));
        return false;
    }
    
    data->frame_size = context->frame_size ? context->frame_size : 1024;
    
    ret = av_samples_alloc(data->samples, NULL, context->channels,
                           data->frame_size, context->sample_fmt, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "Failed to create audio buffer: %s",
             av_err2str(ret));
        return false;
    }
    
    return true;
}

static bool create_audio_stream(struct ffmpeg_data *data)
{
    AVCodecContext *context;
   
    
    if (!new_stream(data, &data->audio, &data->acodec,
                    data->output->oformat->audio_codec,
                    data->config.audio_encoder))
        return false;
    
    context              = data->audio->codec;
    context->bit_rate    = data->config.audio_bitrate * 1000;
    context->time_base   = (AVRational){ 1, data->config.audio_samples_per_sec };
    context->channels    = get_audio_channels(data->config.audio_speakers);
    context->sample_rate =  data->config.audio_samples_per_sec;
    context->channel_layout =
    av_get_default_channel_layout(context->channels);
    context->sample_fmt  = data->acodec->sample_fmts ?
    data->acodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    
    data->audio->time_base = context->time_base;
    
    data->audio_samplerate = data->config.audio_samples_per_sec;
    data->audio_format = convert_ffmpeg_sample_format(context->sample_fmt);
    data->audio_planes = get_audio_planes(data->audio_format, data->config.audio_speakers);
    data->audio_size = get_audio_size(data->audio_format, data->config.audio_speakers, 1);
    
    if (data->output->oformat->flags & AVFMT_GLOBALHEADER)
        context->flags |= CODEC_FLAG_GLOBAL_HEADER;
    
    return open_audio_codec(data);
}

static inline bool init_streams(struct ffmpeg_data *data)
{
    AVOutputFormat *format = data->output->oformat;
    
    if (format->video_codec != AV_CODEC_ID_NONE)
        if (!create_video_stream(data))
            return false;
    
    if (format->audio_codec != AV_CODEC_ID_NONE)
        if (!create_audio_stream(data))
            return false;
    
    return true;
}

static inline bool open_output_file(struct ffmpeg_data *data)
{
    AVOutputFormat *format = data->output->oformat;
    int ret;
    
    if ((format->flags & AVFMT_NOFILE) == 0) {
        ret = avio_open(&data->output->pb, data->config.url,
                        AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_WARNING, "Couldn't open '%s', %s",
                 data->config.url, av_err2str(ret));
            return false;
        }
    }
    
    strncpy(data->output->filename, data->config.url,
            sizeof(data->output->filename));
    data->output->filename[sizeof(data->output->filename) - 1] = 0;
    
    AVDictionary *dict = NULL;
    if ((ret = av_dict_parse_string(&dict, data->config.muxer_settings,
                                    "=", " ", 0))) {
        av_log(NULL, AV_LOG_WARNING, "Failed to parse muxer settings: %s\n%s",
             av_err2str(ret), data->config.muxer_settings);
        
        av_dict_free(&dict);
        return false;
    }
    
    
    ret = avformat_write_header(data->output, &dict);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "Error opening '%s': %s",
             data->config.url, av_err2str(ret));
        return false;
    }
    
    av_dict_free(&dict);
    
    return true;
}

static void close_video(struct ffmpeg_data *data)
{
    avcodec_close(data->video->codec);
    avpicture_free(&data->dst_picture);
    
    // This format for some reason derefs video frame
    // too many times
    if (data->vcodec->id == AV_CODEC_ID_A64_MULTI ||
        data->vcodec->id == AV_CODEC_ID_A64_MULTI5)
        return;
    
    av_frame_free(&data->vframe);
}

static void close_audio(struct ffmpeg_data *data)
{
    for (size_t i = 0; i < MAX_AV_PLANES; i++)
        circlebuf_free(&data->excess_frames[i]);
    
    av_freep(&data->samples[0]);
    avcodec_close(data->audio->codec);
    av_frame_free(&data->aframe);
}

void ffmpeg_data_free(struct ffmpeg_data *data)
{
    if (data->initialized)
        av_write_trailer(data->output);
    
    if (data->video)
        close_video(data);
    if (data->audio)
        close_audio(data);
    
    if (data->output) {
        if ((data->output->oformat->flags & AVFMT_NOFILE) == 0)
            avio_close(data->output->pb);
        
        avformat_free_context(data->output);
    }
    
    memset(data, 0, sizeof(struct ffmpeg_data));
}

static enum AVCodecID get_codec_id(const char *name, int id)
{
    AVCodec *codec;
    
    if (id != 0)
        return (enum AVCodecID)id;
    
    if (!name || !*name)
        return AV_CODEC_ID_NONE;
    
    codec = avcodec_find_encoder_by_name(name);
    if (!codec)
        return AV_CODEC_ID_NONE;
    
    return codec->id;
}

static void set_encoder_ids(struct ffmpeg_data *data)
{
    data->output->oformat->video_codec = get_codec_id(
                                                      data->config.video_encoder,
                                                      data->config.video_encoder_id);
    
    data->output->oformat->audio_codec = get_codec_id(
                                                      data->config.audio_encoder,
                                                      data->config.audio_encoder_id);
}

bool ffmpeg_data_init(struct ffmpeg_data *data)
{
    bool is_rtmp = false;

    if (!data->config.url || !*data->config.url)
        return false;
    
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
    is_rtmp = false;
    
    print_codecs(1);
    
    AVOutputFormat *output_format = av_guess_format(
                                                    is_rtmp ? "flv" : data->config.format_name,
                                                    data->config.url,
                                                    is_rtmp ? NULL : data->config.format_mime_type);
    
    if (output_format == NULL) {
        av_log(NULL, AV_LOG_WARNING, "Couldn't find matching output format with "
             " parameters: name=%s, url=%s, mime=%s",
             safe_str(is_rtmp ?
                      "flv" :	data->config.format_name),
             safe_str(data->config.url),
             safe_str(is_rtmp ?
                      NULL : data->config.format_mime_type));
        goto fail;
    }
    
    avformat_alloc_output_context2(&data->output, output_format,
                                   NULL, NULL);
    
    if (is_rtmp) {
        data->output->oformat->video_codec = AV_CODEC_ID_H264;
        data->output->oformat->audio_codec = AV_CODEC_ID_AAC;
    } else {
        if (data->config.format_name)
            set_encoder_ids(data);
    }
    
    if (!data->output) {
        av_log(NULL, AV_LOG_WARNING, "Couldn't create avformat context");
        goto fail;
    }
    
    if (!init_streams(data))
        goto fail;
    if (!open_output_file(data))
        goto fail;
    
    av_dump_format(data->output, 0, NULL, 1);
    
    data->initialized = true;
    return true;
    
fail:
    av_log(NULL, AV_LOG_WARNING, "ffmpeg_data_init failed");
    ffmpeg_data_free(data);
    return false;
}

void ffmepg_write_video(struct ffmpeg_data *data, struct video_data *frame)
{
   
  
    // codec doesn't support video or none configured
    if (!data->video)
        return;
    
    AVCodecContext *context = data->video->codec;
    AVPacket packet = {0};
    int ret = 0, got_packet;
    
    av_init_packet(&packet);
    
    if (!data->start_timestamp)
        data->start_timestamp = frame->timestamp;
    
    if (!!data->swscale)
        sws_scale(data->swscale, (const uint8_t *const *)frame->data,
                  (const int*)frame->linesize,
                  0, data->config.height, data->dst_picture.data,
                  data->dst_picture.linesize);
    else
        copy_data(&data->dst_picture, frame, context->height);
    
    if (data->output->flags & AVFMT_RAWPICTURE) {
        packet.flags        |= AV_PKT_FLAG_KEY;
        packet.stream_index  = data->video->index;
        packet.data          = data->dst_picture.data[0];
        packet.size          = sizeof(AVPicture);
        
        
        ret= av_interleaved_write_frame(data->output, &packet);
        if (ret < 0) {
            av_log(NULL, AV_LOG_WARNING, "send: Error writing packet:  %s", av_err2str(ret));
            av_packet_unref(&packet);
            return ;
        }
    } else {
        data->vframe->pts = data->total_frames;
        ret = avcodec_encode_video2(context, &packet, data->vframe,
                                    &got_packet);
        if (ret < 0) {
            av_log(NULL, AV_LOG_WARNING, "receive_video: Error encoding "
                 "video: %s", av_err2str(ret));
            return;
        }
        
        if (!ret && got_packet && packet.size) {
            packet.pts = rescale_ts(packet.pts, context,
                                    data->video->time_base);
            packet.dts = rescale_ts(packet.dts, context,
                                    data->video->time_base);
            packet.duration = (int)av_rescale_q(packet.duration,
                                                context->time_base,
                                                data->video->time_base);
            
        
            ret = av_interleaved_write_frame(data->output, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_WARNING, "send: Error writing packet:  %s", av_err2str(ret));
                av_free_packet(&packet);
                return ;
            }
            
        } else {
            ret = 0;
        }
    }
    
    if (ret != 0) {
        av_log(NULL, AV_LOG_WARNING, "receive_video: Error writing video: %s",
             av_err2str(ret));
    }
    
    data->total_frames++;
}

static void encode_audio(struct ffmpeg_data *data ,
                         struct AVCodecContext *context, size_t block_size)
{
    
    
    AVPacket packet = {0};
    int ret, got_packet;
    size_t total_size = data->frame_size * block_size * context->channels;
    
    data->aframe->nb_samples = data->frame_size;
    data->aframe->pts = av_rescale_q(data->total_samples,
                                     (AVRational){1, context->sample_rate},
                                     context->time_base);
    
    ret = avcodec_fill_audio_frame(data->aframe, context->channels,
                                   context->sample_fmt, data->samples[0],
                                   (int)total_size, 1);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "encode_audio: avcodec_fill_audio_frame "
             "failed: %s", av_err2str(ret));
        return;
    }
    
    data->total_samples += data->frame_size;
    
    ret = avcodec_encode_audio2(context, &packet, data->aframe,
                                &got_packet);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "encode_audio: Error encoding audio: %s",
             av_err2str(ret));
        return;
    }
    
    if (!got_packet)
        return;
    
    packet.pts = rescale_ts(packet.pts, context, data->audio->time_base);
    packet.dts = rescale_ts(packet.dts, context, data->audio->time_base);
    packet.duration = (int)av_rescale_q(packet.duration, context->time_base,
                                        data->audio->time_base);
    packet.stream_index = data->audio->index;
    
    ret = av_interleaved_write_frame(data->output, &packet);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "send: Error writing packet:  %s", av_err2str(ret));
        av_packet_unref(&packet);
        return ;
    }
}

static bool prepare_audio(struct ffmpeg_data *data,
                          const struct audio_data *frame, struct audio_data *output)
{
    *output = *frame;
    
    if (frame->timestamp < data->start_timestamp) {
        uint64_t duration = (uint64_t)frame->frames * 1000000000 /
        (uint64_t)data->audio_samplerate;
        uint64_t end_ts = (frame->timestamp + duration);
        uint64_t cutoff;
        
        if (end_ts <= data->start_timestamp)
            return false;
        
        cutoff = data->start_timestamp - frame->timestamp;
        cutoff = cutoff * (uint64_t)data->audio_samplerate /
        1000000000;
        
        for (size_t i = 0; i < data->audio_planes; i++)
            output->data[i] += data->audio_size * (uint32_t)cutoff;
        output->frames -= (uint32_t)cutoff;
    }
    
    return true;
}

void ffmepg_write_audio(struct ffmpeg_data   *data , struct audio_data *frame)
{

    size_t frame_size_bytes;
    struct audio_data in;
    
    // codec doesn't support audio or none configured
    if (!data->audio)
        return;
    
    AVCodecContext *context = data->audio->codec;
    
    if (!data->start_timestamp)
        return;
    if (!prepare_audio(data, frame, &in))
        return;
    
    frame_size_bytes = (size_t)data->frame_size * data->audio_size;
    
    for (size_t i = 0; i < data->audio_planes; i++)
        circlebuf_push_back(&data->excess_frames[i], in.data[i],
                            in.frames * data->audio_size);
    
    while (data->excess_frames[0].size >= frame_size_bytes) {
        for (size_t i = 0; i < data->audio_planes; i++)
            circlebuf_pop_front(&data->excess_frames[i],
                                data->samples[i], frame_size_bytes);
        
        encode_audio(data, context, data->audio_size);
    }
}


void ffmepg_write_avpacket(struct ffmpeg_data   *data , AVPacket packet){
    int ret= av_interleaved_write_frame(data->output, &packet);
    if (ret < 0) {
        av_log(NULL, AV_LOG_WARNING, "send: Error writing packet:  %s", av_err2str(ret));
        av_packet_unref(&packet);
        return ;
    }

}

