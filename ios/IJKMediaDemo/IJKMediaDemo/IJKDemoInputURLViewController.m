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

@interface IJKDemoInputURLViewController () <UITextViewDelegate>{
    
    IJKPlayerMovieDecoder* decoder;
    NSTimer *timer;
    bool isloaded;
    
    int fixed_timer_count ;
    bool fixed_timer;
    bool isrstp;
    
    GLPlayerView *_panoplayer;
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
    
    
    
    _panoplayer = [[GLPlayerView alloc] init];
    _panoplayer.frame = CGRectMake(0, 30, self.view.frame.size.width, self.view.frame.size.height);
    NSLog(@" frame size %f,%f",_panoplayer.frame.size.width,_panoplayer.frame.size.height);
    [self.view addSubview:_panoplayer];
    
}


- (void)onClickPlayButton {
    
    decoder=[IJKPlayerMovieDecoder movieDecoderWithMovie:@"http://192.168.1.254:8192" isHardWare:false];
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