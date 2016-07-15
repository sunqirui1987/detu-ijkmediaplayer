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
#import "ijkplayer/ijkremux.h"
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
    
    struct ffmpeg_data ffmdata;
    struct ffmpeg_cfg sconfig;
    
    NSTimer *timer_save;
    Boolean isSavingNow;
    UIButton *btn_save;
    
    ALAssetsLibrary *lib;
    NSString *path;
    
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
    
    btn_save = [[UIButton alloc] initWithFrame:CGRectMake(0, 100, 80, 30)];
    [btn_save setTitle:@"保存" forState:UIControlStateNormal];
    [self.view addSubview:btn_save];
    [btn_save addTarget:self action:@selector(save) forControlEvents:UIControlEventTouchUpInside];
    
}

-(void)checkImage{
//    UIImage *image = [self snapshot:self.view];
//    UIImage *image2 = [self snapshot:_panoplayer];
//    
//    UIImage *image3 = [UIImage imageNamed:@"wechat"];
//    
//    UIImage *image4 = [self snapshot:btn_save];
//    
//    NSString *path2 = [NSString stringWithFormat:@"%@/aaa.png",[[NSBundle mainBundle] bundlePath]];
//    
//    NSString *path1 = [[NSBundle mainBundle] pathForResource:@"aaa.png" ofType:@""];
//    UIImage *image5 = [UIImage imageWithContentsOfFile:path1];
//    UIImage *image6 = [self imageWithScreenContents:self.view];
//    UIImage *image70 = [self imageWithScreenContents:_panoplayer];
//    
//    UIImage *image7 = [UIImage imageWithContentsOfFile:path2];
//    
//    NSString *path11 = [[NSBundle mainBundle] pathForResource:@"aaa" ofType:nil];
//    NSString *path12 = [[NSBundle mainBundle] pathForResource:@"IJKMediaDemo/aaa.png" ofType:nil];
//    
//    UIImage *image44 = [_panoplayer getScreenShot];
//    
//    NSURL *url = [NSURL URLWithString:@"http://media.qicdn.detu.com/pano681464157885493377656/oper/panofile_preview_detunew.jpg"];
//    
//    
//    NSURLRequest *request = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:10];
//    // 创建同步链接
//    NSURLResponse *response = nil;
//    NSError *error = nil;
//    UIImage *imageO;
//    NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
//    if (error) {
//        
//    } else {
//       imageO  = [UIImage imageWithData:data];
//    }
//
//    
//    NSLog(@"");
}

//-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event{
//    [self checkImage];
//}

-(UIImage *)imageWithScreenContents:(UIView*)view
{
//    UIGraphicsBeginImageContextWithOptions(self.view.bounds.size, YES, 1);
//    [self.view.layer renderInContext:UIGraphicsGetCurrentContext()];
//    UIImage *uiImage = UIGraphicsGetImageFromCurrentImageContext();
//    UIGraphicsEndImageContext();
    //开启位图上下文
    UIGraphicsBeginImageContextWithOptions(view.bounds.size, NO, 0);
    //创建大小等于剪切区域大小的封闭路径
    UIBezierPath *path = [UIBezierPath bezierPathWithRect:view.frame];
    //设置超出的内容不显示，
    [path addClip];
    //获取绘图上下文
    CGContextRef context = UIGraphicsGetCurrentContext();
    //将图片渲染的上下文中
    [view.layer renderInContext:context];
    //获取上下文中的图片
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    //关闭位图上下文
    UIGraphicsEndImageContext();
    return image;
}


-(void)save{
    if(isSavingNow){
        [self endSave];
        return;
    }
    path = [NSString stringWithFormat:@"%@/aa.mp4",kDownloadPath];
    
    memset(&ffmdata, 0, sizeof(struct ffmpeg_data));
    memset(&sconfig, 0, sizeof(struct ffmpeg_cfg));
    
    sconfig.url = [path UTF8String];
    sconfig.format_name = [@"mp4" UTF8String];
    sconfig.format_mime_type = NULL;
    sconfig.muxer_settings = NULL;
    sconfig.video_bitrate = 2500;
    sconfig.audio_bitrate = 128;

    sconfig.video_encoder_id = AV_CODEC_ID_H264;
    
    sconfig.audio_encoder_id = AV_CODEC_ID_AAC;
    sconfig.audio_encoder = "";
    sconfig.audio_settings = "";
    sconfig.format = AV_PIX_FMT_BGR24;//  AV_PIX_FMT_BGR24;
    sconfig.color_range = AVCOL_RANGE_JPEG;
    sconfig.color_space = AVCOL_SPC_BT709;
    sconfig.muxer_settings = "";
    sconfig.writeflag = 1;
    
    sconfig.scale_width = 400;
    sconfig.scale_height = 300;
    sconfig.width = 400;
    sconfig.height = 300;
    
    sconfig.fps = 30;
    sconfig.audio_samples_per_sec = 44100;
    sconfig.audio_speakers = SPEAKERS_STEREO;
    sconfig.audio_format = AUDIO_FORMAT_FLOAT_PLANAR;
    ffmdata.config = sconfig;
    
    
    bool initResult = ffmpeg_data_init(&(ffmdata));
    
    if(initResult){
        [self startSave];
    }else{
        NSLog(@"init ffmpeg data error>>>>>>>>>>>>>>>>>>>>");
        return;
    }
    
    isSavingNow = YES;
    [btn_save setTitle:isSavingNow?@"停止":@"保存" forState:UIControlStateNormal];
    
    
}

-(void) startSave{
    if (!timer_save) {
        timer_save=[NSTimer scheduledTimerWithTimeInterval:(1.0/30) target:self selector:@selector(saveFrame) userInfo:nil repeats:YES];
    }
}

-(void)endSave{
    if (timer_save) {
        [timer_save invalidate];
        timer_save=nil;
    }
    isSavingNow = NO;
    [btn_save setTitle:isSavingNow?@"停止":@"保存" forState:UIControlStateNormal];
    ffmpeg_data_free(&(ffmdata));
    
    if(!lib){
        lib = [ALAssetsLibrary new];
    }
    [lib saveVedio:path toAlbum:@"转存" withCompletionBlock:^(NSError *error) {
        NSLog(@"转存完毕");
    }];
}

- (UIImage *)snapshot:(UIView *)theView
{
    UIGraphicsBeginImageContext(theView.frame.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    [theView.layer renderInContext:context];
    UIImage *theImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return theImage;
}

-(void)saveFrame{
    struct video_data videoframe;
    
    NSLog(@"截图前》》》》》》》》》");
    
//    UIImage *image = [self snapshot:self.view];
//    if(image == nil)
//        return;
    
    UIImage *image = [UIImage imageNamed:@"qumengEnterPrise"];
    if(image == nil)
        return;
    
    
    NSLog(@"截图后《《《《《《《《《");
    
    
    
//    ffmdata.config.scale_width = _panoplayer.frame.size.width;
//    ffmdata.config.scale_height = _panoplayer.frame.size.height;
//    ffmdata.config.width = _panoplayer.frame.size.width;
//    ffmdata.config.height = _panoplayer.frame.size.height;
    
    videoframe.linesize[0] = 3;
    videoframe.data[0] = (uint8_t *)(UIImageJPEGRepresentation(image,1).bytes);
    
    
    ffmepg_write_video(&(ffmdata),&videoframe);
    
    
}

//---------------

- (void)onClickPlayButton {
  //  http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8
    //rtsp://192.168.1.254/xxx.mov
//    decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:@"http://media.qicdn.detu.com/@/70955075-5571-986D-9DC4-450F13866573/2016-05-19/573d15dfa19f3-2048x1024.m3u8" isHardWare:false];

 //   decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:@"http://192.168.1.254:8192" isHardWare:false];
    
    NSString *inputS = [NSString stringWithFormat:@"%@",[[NSBundle mainBundle] pathForResource:@"2016_0714_110943_008" ofType:@"MOV"]];
    decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:inputS isHardWare:false];

    decoder.delegate=self;
    [self innerstart];
}


-(void)_startDraw{
    if (!timer) {
        timer=[NSTimer scheduledTimerWithTimeInterval:(1.0/60) target:decoder selector:@selector(captureNext) userInfo:nil repeats:YES];
    }
    
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
    if (frame->w > 0) {
        [_panoplayer setFrameSDL:frame];
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
    [decoder stop];
    [decoder cleargc];
    decoder.delegate = nil;
    decoder = nil;
    timer = nil;
}

@end