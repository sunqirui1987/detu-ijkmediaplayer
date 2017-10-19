//
//  MacVideoToolBoxDecoder.m
//  IJkMediaDemo
//
//  Created by chao on 2017/10/17.
//  Copyright © 2017年 annidy. All rights reserved.
//

#import "MacVideoToolBoxDecoder.h"
#include "VideoToolBoxDecoder.h"

@interface MacVideoToolBoxDecoder(){
    
}

@property(nonatomic, strong)NSString* url;
@property(nonatomic, strong)VideoToolBoxDecoder* decoder;
@end

@implementation MacVideoToolBoxDecoder


-(void)setDataSource:(NSString*) url {
    self.url = url;
}

-(void)start {
    __weak MacVideoToolBoxDecoder* decoder = self;
    NSThread* thread = [[NSThread alloc]initWithBlock:^{
        [decoder decodeThread];
    }];
    [thread start];
}

-(void)decodeThread {
    _decoder = [[VideoToolBoxDecoder alloc] init];
    const char* fileAbsolutePath = [_url UTF8String];
    av_register_all();
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pFormatContext, fileAbsolutePath, NULL, NULL) != 0) {
        return;
    }
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        avformat_close_input(&pFormatContext);
        return;
    }
    int posVideoStream = -1;
    int posAudioStream = -1;
    
    //AVCodec* codec = avcodec_find_encoder_by_name("h264_videotoolbox");
    AVCodec* codec = avcodec_find_decoder_by_name("h264_vda");
    int a = AV_PIX_FMT_VDA;
    const int NUMBER_STREAM = pFormatContext->nb_streams;
    for (int i = 0; i < NUMBER_STREAM; i++) {
        switch (pFormatContext->streams[i]->codec->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                posVideoStream = i;
                break;
            case AVMEDIA_TYPE_AUDIO:
                posAudioStream = i;
                break;
            default:
                break;
        }
    }
    [_decoder init:pFormatContext->streams[posVideoStream]->codec :0];
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    while (true) {
        int resultCode = av_read_frame(pFormatContext, packet);
        if (resultCode < 0) {
            break;
        }
        if (packet->stream_index != posVideoStream) {
            continue;
        }
        [_decoder sendPacket:packet];
    }
}

@end
