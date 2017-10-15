//
//  ViewController.m
//  AVCapturePreview2
//
//  Created by annidy on 16/4/16.
//  Copyright © 2016年 annidy. All rights reserved.
//

#import "ViewController.h"
#import "VideoGLView.h"
@import AVFoundation;

//#define QUARTZ
//#define LAYER

#include <assert.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include "ijk_ffplay_decoder.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavfilter/avfilter.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"


@interface ViewController ()<AVCaptureVideoDataOutputSampleBufferDelegate>
@property (weak) IBOutlet NSTextField *timelabel;
@property (weak) IBOutlet NSButton *timeBut;

@property (weak) IBOutlet NSImageView *cameraView;
@property (weak) IBOutlet NSTextField *fpsLabel;
@property (weak) IBOutlet VideoGLView *openGLView;
@end

@implementation ViewController
{
    IjkFfplayDecoder* decoder;
    RcFrame frame;
}


void func_get_frame(void* opaque, IjkVideoFrame *frame_callback) {
    ViewController* controller = (__bridge ViewController*)opaque;
    RcFrame* frame = &controller->frame;
    frame->data[0] = frame_callback->data[0];
    frame->data[1] = frame_callback->data[1];
    frame->data[2] = frame_callback->data[2];
    frame->linesize[0] = frame_callback->linesize[0];
    frame->linesize[1] = frame_callback->linesize[1];
    frame->linesize[2] = frame_callback->linesize[2];
    frame->width = frame_callback->w;
    frame->height = frame_callback->h;
    switch(frame_callback->format) {
        case PIX_FMT_YUV420P:
            frame->format = FMT_YUV420P;
            frame->planes = 3;
            break;
        case PIX_FMT_NV12:
            frame->format = FMT_NV12;
            frame->planes = 2;
            break;
        default:
            break;
    }
    [controller.openGLView setImage:frame];
}

void func_state_change(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2) {
    ViewController* controller = (__bridge ViewController*)opaque;
    switch (ijk_msgint) {
        case IJK_MSG_PREPARED:
            ijkFfplayDecoder_setVolume(controller->decoder, 0.2f);
            break;
            
        default:
            break;
    }
}



- (void)viewDidLoad {
    [super viewDidLoad];
    [self.view setWantsLayer:YES];
    // Do any additional setup after loading the view.
    NSString* path = @"/Users/chao/Downloads/wedding.mp4";
    //path = @"/Users/chao/Downloads/IMG_6551.MP4";
    path = @"/Users/chao/Downloads/xihu.mp4";
    path = @"http://media.detu.com/@/41020711-1591-C3CD-78FA-FB2F67437049/2017-06-05/593590081a66b-2048x1024.m3u8";
    path = @"/Users/chao/Desktop/穿梭在法国小镇_injected.mp4";
    path = @"/Users/chao/Downloads/xihu_cut.mp4";
    //path = @"/Users/chao/Downloads/IMG_0728.MP4";
    ijkFfplayDecoder_init();
    decoder = ijkFfplayDecoder_create();
    IjkFfplayDecoderCallBack callBack = {0};
    callBack.func_get_frame = &func_get_frame;
    callBack.func_state_change = &func_state_change;
    ijkFfplayDecoder_setDecoderCallBack(decoder, (__bridge void*) self, &callBack);
    ijkFfplayDecoder_setHwDecoderName(decoder, "h264_vtb");
    ijkFfplayDecoder_setDataSource(decoder, [path UTF8String]);
    ijkFfplayDecoder_prepare(decoder);
    ijkFfplayDecoder_start(decoder);
    
    //[self testCutVideoPacketNumber];
}

-(void)testCutVideoPacketNumber{
    const char* path = "/Users/chao/Downloads/xihu_cut.mp4";
    av_register_all();
    AVFormatContext *ic = avformat_alloc_context();
    avformat_open_input(&ic, path, NULL, NULL);
    avformat_find_stream_info(ic, NULL);
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    
    int posVideoStream = -1;
    int posAudioStream = -1;
    const int NUMBER_STREAM = ic->nb_streams;
    for (int i = 0; i < NUMBER_STREAM; i++) {
        switch (ic->streams[i]->codec->codec_type) {
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
    
    int numAudio = 0;
    int numVideoNum = 0;
    while (true) {
        int resultCode = av_read_frame(ic, pPacket);
        if (resultCode < 0) {
            break;
        }
        if(pPacket->stream_index == posAudioStream) {
            numAudio++;
        } else if(pPacket->stream_index == posVideoStream) {
            numVideoNum++;
        }
    }
    NSLog(@"pakcet size:%d, %d", numAudio, numVideoNum);
}

- (void)viewDidAppear
{
    [super viewDidAppear];
}

- (IBAction)addTimeBut:(NSButton *)sender {
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    
    // Update the view, if already loaded.
}

@end
