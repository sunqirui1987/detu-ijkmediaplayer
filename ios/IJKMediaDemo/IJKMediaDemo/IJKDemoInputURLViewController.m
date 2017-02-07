/*
 * Copyright (C) 2015 Gdier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "IJKDemoInputURLViewController.h"
#import "GLPlayerView.h"
#import "ALAssetsLibrary+CustomPhotoAlbum.h"

#define downloadBasePath ({\
NSString *sandboxPath=NSHomeDirectory();\
NSString *basePath=[sandboxPath stringByAppendingString:@"/Documents"];\
[[NSFileManager defaultManager] createDirectoryAtPath:[basePath stringByAppendingString:@"/transeFile"] withIntermediateDirectories:YES attributes:nil error:nil];\
basePath;\
})

#define kDownloadPath [downloadBasePath stringByAppendingString:@"/transeFile"]

@interface IJKDemoInputURLViewController () <UITextViewDelegate>{
    
    IJKPlayerMovieDecoder* decoder;
    NSTimer *timer;
    bool isloaded;
    
    int fixed_timer_count ;
    bool fixed_timer;
    bool isrstp;
    
    GLPlayerView *_panoplayer;
    
    SDL_VoutOverlay* cur_overlay;
    
    NSRecursiveLock *_lock;
    long long index;
}



@end

@implementation IJKDemoInputURLViewController

- (instancetype)init {
    self = [super init];
    if (self) {
        self.title = @"Input URL";
        
        [self.navigationItem setRightBarButtonItem:[[UIBarButtonItem alloc] initWithTitle:@"Play" style:UIBarButtonItemStyleDone target:self action:@selector(onClickPlayButton)]];
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor redColor];
    
    
    
    _panoplayer = [[GLPlayerView alloc] init];
    _panoplayer.backgroundColor = [UIColor blueColor];
    _panoplayer.frame = CGRectMake(0, 30, self.view.frame.size.width, self.view.frame.size.height);
    NSLog(@" frame size %f,%f",_panoplayer.frame.size.width,_panoplayer.frame.size.height);
    [self.view addSubview:_panoplayer];
    
    
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    NSLog(@"video plugin deallo");
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self innerstop];
    [decoder stop];
    [decoder cleargc];
    decoder.delegate = nil;
    decoder = nil;
    timer = nil;
}


//---------------

- (void)onClickPlayButton {
    NSLog(@"onClickPlayButton>>>>>>>>>>>>>>>>>");
    
    //rtsp://192.168.1.254/xxx.mov http://192.168.1.254:8192 http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8
    
//    NSString *path =  [[NSBundle mainBundle] pathForResource:@"test" ofType:@"MP4"];
    
    NSString *path =  @"rtsp://192.168.42.1/live";
   // path = @"rtmp://pili-live-rtmp.live.detu.com/detulive/123";
    
  // path = @"rtsp://192.168.42.1/tmp/SD0/DCIM/161216000/150303AB.MP4";
    
  //  path =  [[NSBundle mainBundle] pathForResource:@"IMG_1060" ofType:@"MP4"];
//   path = [NSString stringWithFormat:@"file://%@",path];
    
//    path =  @"http://192.168.1.254:8192";
    
    
    //  path = @"rtsp://192.168.42.1/tmp/SD0/DCIM/161228000/220330AB.MP4";
    //path = @"rtmp://pili-live-rtmp.live.detu.com/detulive/4444";
    
  //  path = @"http://media.qicdn.detu.com/@/11952648-8057-79C8-112C-3359F38671974/2016-09-06/57ce57417e25e-2048x1024.m3u8";//容易cup爆表，掉针的视频地址，用6或者5s测试比较明显，6s比较少见
    
 //path = @"http://cache.utovr.com/s1rtqwszpusjowqhui/L2_dphr42xukiougcgk.m3u8";//utovr地址，经测试秒出
    
 // path = @"http://detu-static.oss-cn-hangzhou.aliyuncs.com/static/app/version/resource/video.mp4";//该视频没有画面，只有声音，vlc播放器可播
    
//    path=@"http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8";
    
 //   path=@"http://media.qicdn.detu.com/@/31957945-57B6-63AE-B02B-95F74469962/2016-10-07/57f714f005dbf-2880x1440.m3u8";
    
  //  path = @"http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8";
    
//    path = @"rtmp://live.yueyetv.com/JTChatRoomFuns/funs10170/7e8d290e-8c35-4818-8a2d-45d487f3bc33";
//    
//    path = @"rtmp://live.yueyetv.com/JTChatRoomFuns/funs10170/7e8d290e-8c35-4818-8a2d-45d487f3bc33";
    
// path = @"rtmp://13306.lssplay.aodianyun.com/hundun001/stream";
    
//    path =  [[NSBundle mainBundle] pathForResource:@"aaa" ofType:@"mp4"];
    
    decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:path isHardWare:_panoplayer.isHardDecoder];

 //  decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:@"http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8" isHardWare:false];
    
    _lock = [[NSRecursiveLock alloc] init];
    index = -1;


    decoder.delegate=self;
    [self innerstart];
    
    }


-(void)_startDraw{
    if (!timer) {
        timer=[NSTimer scheduledTimerWithTimeInterval:(1.0/60) target:_panoplayer selector:@selector(render) userInfo:nil repeats:YES];
    }
    
}

-(void)captureNext{
    return;
    [decoder captureNext];
//    if(cur_overlay && cur_overlay->w>0){
//        index = cur_overlay->sar_num;
//   //     dispatch_async(dispatch_get_main_queue(), ^{
//            [_panoplayer setFrameSDL:cur_overlay];
//   //     });
//    }
    [_panoplayer render];
}


-(void)_stopDraw{
    if (timer) {
        [timer invalidate];
        timer=nil;
    }
    
}

-(void)innerstart{
    [self _startDraw];
    [decoder start];
    
}

-(void)start{
    [self innerstart];
    
}

-(void)_pauseAndStopDraw{
    
    if(self != nil && decoder != nil)
    {
        [self _stopDraw];
        [self pause];
    }
    
}

-(void)innerpause{
    [decoder pause];
}

-(void)pause{
    [self innerpause];
    
}

-(void)innerstop{
    [self _stopDraw];
    if (!decoder) {
        return;
    }
    [decoder stop];
    
}

-(void)stop{
    [self innerstop];
    
}



-(void)setCurrentTime:(float)currentTime{
    decoder.currentTime=currentTime;
}

-(float)currentTime{
    return decoder.currentTime;
}

-(float)duration{
    return decoder.duration;
}

-(void)movieDecoderDidDecodeFrameSDL:(SDL_VoutOverlay*)frame{
//    cur_overlay = frame;
//    return;
    if (frame->w > 0) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [_panoplayer setFrameSDL:frame];
        });
  //      [self innerstop];
    }
}

-(void)movieDecoderDidDecodeFrameRawbuf:(uint8_t *)frame :(int)w :(int)h{
    if (frame) {
        [_panoplayer stepFrame:frame :w :h];
        [_panoplayer render];
    }
}

-(void)movieDecoderDidDecodeFrame:(CVPixelBufferRef)buffer{
    NSLog(@"movieDecoderDidDecodeFrame");
}
-(void)movieDecoderDidDecodeFrameBuffer:(void*)buffer width:(int)width height:(int)height channel:(int)channel{
    
    NSLog(@"movieDecoderDidDecodeFrameBuffer");
}


-(void)movieDecoderDidFinishDecoding{
}
-(void)movieDecoderDidSeeked{
}
-(void)movieDecoderError:(NSError *)error{
}
-(void)moviceDecoderPlayItemState:(MovieDecoderPlayItemState)state{
    
}




-(void)dealloc{
    NSLog(@"video plugin deallo");
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self innerstop];
    [decoder stop];
    [decoder cleargc];
    decoder.delegate = nil;
    decoder = nil;
    timer = nil;
}


@end
