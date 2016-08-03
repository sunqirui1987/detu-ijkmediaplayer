//
//  IJKPlayerMovieDecoder.m
//  IJKMediaDemo
//
//  Created by qiruisun on 16/5/20.
//  Copyright © 2016年 bilibili. All rights reserved.
//

//
//  IJKPlayerMovieDecoder.m
//  PanoPlayer
//
//  Created by qiruisun on 15/12/25.
//  Copyright © 2015年 apple. All rights reserved.
//

#import "IJKPlayerMovieDecoder.h"
#import <IJKMediaFramework/IJKMediaFramework.h>




@interface IJKPlayerMovieDecoder ()
{
    NSRecursiveLock *_lock;
    void *_framedata;
    int _videoWidth;
    int _videoHeight;
    int _channel;
    IJKFFMoviePlayerController *_player;
}

@end
@implementation IJKPlayerMovieDecoder
-(id)initWithMovie:(NSString*)path{
    self = [super init];
    
    _lock = [[NSRecursiveLock alloc] init];
    [self loadMovie:path];
    return self;
}


-(BOOL)loadMovie:(NSString*)path
{
    
    [IJKFFMoviePlayerController setLogReport:NO];
    [IJKFFMoviePlayerController setLogLevel:k_IJK_LOG_SILENT];
    
    
    IJKFFOptions *options =  [[IJKFFOptions alloc] init];
    
    [options setPlayerOptionIntValue:self.is_hardware?1:0      forKey:@"videotoolbox"];
    [options setPlayerOptionIntValue:2048    forKey:@"videotoolbox-max-frame-width"];
    

    if(self.is_hardware){
        [options setPlayerOptionValue:@"fcc-_vtb"          forKey:@"overlay-format"];
    }else{
        //     [options setPlayerOptionValue:@"fcc-rv24"          forKey:@"overlay-format"];
        [options setPlayerOptionValue:@"fcc-i420"          forKey:@"overlay-format"];
    }
    
    
    
    [options setPlayerOptionValue:0        forKey:@"start-on-prepared"];
    
    if(  [path hasPrefix:@"rtsp://"] ){

        [options setPlayerOptionIntValue:0 forKey:@"packet-buffering"];
        [options setPlayerOptionIntValue:25 forKey:@"limit_packets"];
   
    }
    
   
    _player = [[IJKFFMoviePlayerController alloc] initWithContentURLString:path withOptions:options];
    // _player.ishardWare = self.is_hardware;
    [_player prepareToPlay];
    [self start];
    return TRUE;
}

+(id)movieDecoderWithMovie:(NSString*)path isHardWare:(BOOL)isHardWare{
    IJKPlayerMovieDecoder *decoder =  [IJKPlayerMovieDecoder alloc];
    decoder.is_hardware = isHardWare;
    return [decoder initWithMovie:path];
}

+(id)movieDecoderWithMovie:(NSString*)path{
    return [[IJKPlayerMovieDecoder alloc] initWithMovie:path];
}


-(void)decodeFrame{
    //    if(self.is_hardware){
    //        SDL_VoutOverlay* frame = [_player getCurrentFrame3];
    //        if (frame == NULL) {
    //            return;
    //        }
    //        [self.delegate movieDecoderDidDecodeFrameSDL: frame];
    //
    //    }else{
    //        sframebuffer frame = [_player getCurrentFrame2];
    //        if (frame.frame == NULL) {
    //            return;
    //        }
    //
    //        _videoWidth = frame.width;
    //        _videoHeight = frame.height;
    //        _framedata = frame.frame;
    //        _channel   = frame.channel;
    //        [self.delegate movieDecoderDidDecodeFrameBuffer: _framedata width:_videoWidth height:_videoHeight channel:_channel];
    //    }
   
    SDL_VoutOverlay* frame = [_player getCurrentFrame3];
    if (frame == NULL) {
        return;
    }
    
    //   dispatch_async(dispatch_get_global_queue(0, 0), ^{
    [self.delegate movieDecoderDidDecodeFrameSDL: frame];
    //   });
    
}

-(void)captureNext{
    [_lock lock];
    [self decodeFrame];
    [_lock unlock];
}
-(void)start{
    [_player play];
    [self installMovieNotificationObservers];
}
-(void)pause{
    [_player pause];
}
-(void)stop{
    [_player stop];
    [self removeMovieNotificationObservers];
}

-(double)currentTime{
    
    return _player.currentPlaybackTime ;
    // return [[_player time] intValue] / 1000;
}

-(void)setCurrentTime:(double)currentTime{
    [_lock lock];
    [_player setCurrentPlaybackTime:currentTime];
    [self.delegate movieDecoderDidSeeked];
    [_lock unlock];
}

-(float)duration{
    return _player.duration;
}

-(void)cleargc{
    NSLog(@"alc cleargc");
    [_lock lock];
    if (_player != nil) {
        [_player shutdown];
    }
    _player = nil;
    [_lock unlock];
    
}
-(void)dealloc{
    NSLog(@"dealloc");
    [self cleargc];
    
}

-(NSString*)getDescriptMedta:(NSString *)key{
    if (_player == nil) {
        return nil;
    }
    
    NSDictionary *dic =  [_player.monitor mediaMeta];
    if (dic == nil) {
        return nil;
    }
    for (id skey in dic)
    {
        NSLog(@"key: %@ ,value: %@",skey,[dic objectForKey:skey]);
    }
    return [dic objectForKey:key];
    
}


- (void)loadStateDidChange:(NSNotification*)notification
{
    //    MPMovieLoadStateUnknown        = 0,
    //    MPMovieLoadStatePlayable       = 1 << 0,
    //    MPMovieLoadStatePlaythroughOK  = 1 << 1, // Playback will be automatically started in this state when shouldAutoplay is YES
    //    MPMovieLoadStateStalled        = 1 << 2, // Playback will be automatically paused in this state, if started
    
    MPMovieLoadState loadState = _player.loadState;
    
    if ((loadState & MPMovieLoadStatePlaythroughOK) != 0) {
        [self.delegate moviceDecoderPlayItemState:MOVICE_STATE_PLAYING];
        NSLog(@"loadStateDidChange: MPMovieLoadStatePlaythroughOK: %d\n", (int)loadState);
    } else if ((loadState & MPMovieLoadStateStalled) != 0) {
        //加载中
        [self.delegate moviceDecoderPlayItemState:MOVICE_STATE_BUFFER_EMPTY];
        NSLog(@"loadStateDidChange: MPMovieLoadStateStalled: %d\n", (int)loadState);
    } else {
        NSLog(@"loadStateDidChange: ???: %d\n", (int)loadState);
    }
}

- (void)moviePlayBackDidFinish:(NSNotification*)notification
{
    //    MPMovieFinishReasonPlaybackEnded,
    //    MPMovieFinishReasonPlaybackError,
    //    MPMovieFinishReasonUserExited
    int reason = [[[notification userInfo] valueForKey:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey] intValue];
    
    switch (reason)
    {
        case MPMovieFinishReasonPlaybackEnded:
            [self.delegate movieDecoderDidFinishDecoding];
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonPlaybackEnded: %d\n", reason);
            break;
            
        case MPMovieFinishReasonUserExited:
            [self.delegate movieDecoderError:[NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier] code:0 userInfo:@{NSLocalizedDescriptionKey:@"播放失败，用户强制退出"}]];
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonUserExited: %d\n", reason);
            break;
            
        case MPMovieFinishReasonPlaybackError:
            [self.delegate movieDecoderError:[NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier] code:0 userInfo:@{NSLocalizedDescriptionKey:@"播放文件格式错误或网络异常"}]];
            
            NSLog(@"playbackStateDidChange: MPMovieFinishReasonPlaybackError: %d\n", reason);
            break;
            
        default:
            [self.delegate movieDecoderError:[NSError errorWithDomain:[[NSBundle mainBundle] bundleIdentifier] code:0 userInfo:@{NSLocalizedDescriptionKey:@"未知错误"}]];
            NSLog(@"playbackPlayBackDidFinish: ???: %d\n", reason);
            break;
    }
}

- (void)mediaIsPreparedToPlayDidChange:(NSNotification*)notification
{
    NSLog(@"mediaIsPreparedToPlayDidChange\n");
}

- (void)moviePlayBackStateDidChange:(NSNotification*)notification
{
    //    MPMoviePlaybackStateStopped,
    //    MPMoviePlaybackStatePlaying,
    //    MPMoviePlaybackStatePaused,
    //    MPMoviePlaybackStateInterrupted,
    //    MPMoviePlaybackStateSeekingForward,
    //    MPMoviePlaybackStateSeekingBackward
    
    switch (_player.playbackState)
    {
        case MPMoviePlaybackStateStopped: {
            [self.delegate moviceDecoderPlayItemState:MOVICE_STATE_STOP];
            NSLog(@"moviePlayBackStateDidChange %d: stoped", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStatePlaying: {
            [self.delegate moviceDecoderPlayItemState:MOVICE_STATE_PLAYING];
            NSLog(@"moviePlayBackStateDidChange %d: playing", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStatePaused: {
            [self.delegate moviceDecoderPlayItemState:MOVICE_STATE_PAUSE];
            NSLog(@"moviePlayBackStateDidChange %d: paused", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStateInterrupted: {
            NSLog(@"moviePlayBackStateDidChange %d: interrupted", (int)_player.playbackState);
            break;
        }
        case MPMoviePlaybackStateSeekingForward:
        case MPMoviePlaybackStateSeekingBackward: {
            NSLog(@"moviePlayBackStateDidChange %d: seeking", (int)_player.playbackState);
            break;
        }
        default: {
            NSLog(@"moviePlayBackStateDidChange %d: unknown", (int)_player.playbackState);
            break;
        }
    }
}

#pragma mark Install Movie Notifications

/* Register observers for the various movie object notifications. */
-(void)installMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(loadStateDidChange:)
                                                 name:IJKMPMoviePlayerLoadStateDidChangeNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(moviePlayBackDidFinish:)
                                                 name:IJKMPMoviePlayerPlaybackDidFinishNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(mediaIsPreparedToPlayDidChange:)
                                                 name:IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification
                                               object:_player];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(moviePlayBackStateDidChange:)
                                                 name:IJKMPMoviePlayerPlaybackStateDidChangeNotification
                                               object:_player];
    
}

#pragma mark Remove Movie Notification Handlers

/* Remove the movie notification observers from the movie object. */
-(void)removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMPMoviePlayerLoadStateDidChangeNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMPMoviePlayerPlaybackDidFinishNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification object:_player];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:IJKMPMoviePlayerPlaybackStateDidChangeNotification object:_player];
}


@end