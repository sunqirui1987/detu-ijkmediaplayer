# detu-ijkmediaplayer
Android/iOS/MAC/WiN video player base ijkplayer


 Platform | Build Status
 -------- | ------------
Android | YES
iOS | YES
WIN | YES
MAC | YES

## code project

#### exec code
IJKMediaDemo 
#### static libary
IJKMediaPlayer 


## ios

detu-ijkmediaplayer/ios/


## Android

detu-ijkmediaplayer/Android/


## Mac

detu-ijkmediaplayer/mac/

## WIN

detu-ijkmediaplayer/win/



-----------------------------------------------------------------------------------
### 配置信息：
1.控制使用软解还是硬解(改配置仅为建议，若设备无硬解能力，配置硬解走软解)
目前只在android端实现
>
type:OPT_CATEGORY_CODEC key:hw_decoder value:[true|false]

2.控制是否输出统计信息 time:2017-6-26
通用配置，不分平台
>
type:OPT_CATEGORY_PLAYER key:detu_show_statistics value:[true|false]



-----------------------------------------------------------------------------------
### 状态回调：
ios:按照解码流程划分

- 1.FFP_MSG_VIDEO_DECODER_OPEN(图像解码器打开事件,此时可以通过IJKFFMoviePlayerController的_isVideoToolboxOpen判断是硬解还是软解)
>
    1.key:@"IJKMPMoviePlayerVideoDecoderOpenNotification" userInfo:(IJKFFMoviePlayerController*)
    
- 2.FFP_MSG_VIDEO_SIZE_CHANGED
>
	1.key:@"IJKMPMovieNaturalSizeAvailableNotification" userInfo:(IJKFFMoviePlayerController*)
	
- 3.FFP_MSG_SAR_CHANGED
>
    1.key:@"IJKMPMovieNaturalSizeAvailableNotification" userInfo:(IJKFFMoviePlayerController*)
    
- 4.FFP_MSG_PREPARED（可以获取流元信息，通过_monitor.mediaMeta获取MediaMeta结构体）
>
    1.key:@"IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    2.key:@"IJKMPMoviePlayerLoadStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    
- 5.FFP_MSG_BUFFERING_START(开始拉流缓存数据，可能会回调多次)
>
    1.key:@"IJKMPMoviePlayerLoadStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    2.key:@"IJKMPMoviePlayerPlaybackStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    
- 6.FFP_MSG_SEEK_COMPLETE(拖动进完成回调)
>
    1.key:@"IJKMPMoviePlayerDidSeekCompleteNotification" 
      userInfo:
          (int)seekMilliseconds 拖动进度毫秒值
          (int)ret              seek方法返回值，ret >=0 on success, error code otherwise


- 7.FFP_MSG_AUDIO_RENDERING_START(声音首帧播放事件)
>
    1.key:@"IJKMPMoviePlayerFirstAudioFrameRenderedNotification" userInfo:(IJKFFMoviePlayerController*)

- 8.FFP_MSG_VIDEO_RENDERING_START(图像首帧渲染事件)
>
	1.key:@"IJKMPMoviePlayerFirstVideoFrameRenderedNotification" userInfo:(IJKFFMoviePlayerController*)

- 9.FFP_MSG_DETU_STATISTICS_DATA(拉流统计事件，每秒回调一次，通过播放器配置"detu_show_statistics"控制开关)
>
    1.key:@"IJKMPMoviePlayerDetuStatisticsNotification" 
      userInfo:(NSDictionary *)
         集合内容:
         1.每秒接收码率值， key:@"detu_video_bitrate" value:Integer
         2.每秒接收帧数，其中h264编码为B、P帧总和，mjpeg编码为全部I帧综合，key:@"detu_gop_size" value:Integer

- 10.FFP_MSG_BUFFERING_END（拉流完成，遇到错误可能会回调多次）
>
    1.key:@"IJKMPMoviePlayerLoadStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    2.key:@"IJKMPMoviePlayerPlaybackStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)

- 11.FFP_MSG_ERROR(错误回调)
>
    1.key:@"IJKMPMoviePlayerPlaybackStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
    2.key:@"IJKMPMoviePlayerPlaybackDidFinishNotification"
      userInfo:@{IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(IJKMPMovieFinishReasonPlaybackError), @"error": 错误ret}
      这里的错误回调ret基本都是ijk拉流产生的ffmpeg函数错误，ijk不关心解码部分错误
      
- 12.FFP_MSG_PLAYBACK_STATE_CHANGED（各种状态回调）
>
    1.key:@"IJKMPMoviePlayerPlaybackStateDidChangeNotification" userInfo:(IJKFFMoviePlayerController*)
       这时候可以通过IJKFFMoviePlayerController->mp->mp_state获取当前播放器状态
       其中:详情见ijkplayer.h
           #define MP_STATE_INITIALIZED      1       初始化完成
           #define MP_STATE_ASYNC_PREPARING  2       开始prepare状态
           #define MP_STATE_PREPARED         3       prepare状态
           #define MP_STATE_PAUSED           5       暂停
           #define MP_STATE_COMPLETED        6       完成





