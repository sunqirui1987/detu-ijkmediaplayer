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
#import "IJKPlayerMovieDecoder.h"


@interface ViewController ()<AVCaptureVideoDataOutputSampleBufferDelegate, MovieDecoderDelegate>
@property (weak) IBOutlet NSTextField *timelabel;
@property (weak) IBOutlet NSButton *timeBut;

@property (weak) IBOutlet NSImageView *cameraView;
@property (weak) IBOutlet NSTextField *fpsLabel;
@property (weak) IBOutlet VideoGLView *openGLView;
@end

@implementation ViewController
{
    IJKPlayerMovieDecoder* decoder;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.view setWantsLayer:YES];
    // Do any additional setup after loading the view.
    NSString* path = @"/Users/chao/Downloads/wedding.mp4";
    //path = @"/Users/chao/Downloads/IMG_6551.MP4";
    path = @"/Users/chao/Downloads/xihu.mp4";
    path = @"http://media.detu.com/@/41020711-1591-C3CD-78FA-FB2F67437049/2017-06-05/593590081a66b-2048x1024.m3u8";
    decoder = [IJKPlayerMovieDecoder movieDecoderWithMovie:path isHardWare:false];
    decoder.delegate = self;
    [decoder start];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    

}

- (IBAction)addTimeBut:(NSButton *)sender {
}


-(void)movieDecoderDidFinishDecoding {
}
-(void)movieDecoderDidSeeked {
}
-(void)movieDecoderError:(NSError *)error {
}
-(void)moviceDecoderPlayItemState:(MovieDecoderPlayItemState)state {
}

-(void)movieDecoderDidDecodeFrameSDL:(SDL_VoutOverlay*)frame {
    //NSLog(@"movieDecoderDidDecodeFrameSDL");
    //[_openGLView setImage:frame];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    
    // Update the view, if already loaded.
}

@end
