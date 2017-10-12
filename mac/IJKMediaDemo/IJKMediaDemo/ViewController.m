//
//  ViewController.m
//  IJKMediaDemo
//
//  Created by chao on 2017/6/10.
//  Copyright © 2017年 detu. All rights reserved.
//

#import "ViewController.h"

#import "GLEssentialsGLView.h"
#import "GlRender.h"
#import "GlFlatRender.h"
#import "GLView.h"
#include "ijk_ffplay_decoder.h"

@interface ViewController() {
    bool haveExecute;
    IjkFfplayDecoder* ijkDecoder;
}
@property(nonatomic, strong)GLEssentialsGLView* glView;
@property(nonatomic, strong)GLView* glView0;
@property(nonatomic, strong)GlRender* render;
@end

@implementation ViewController


- (void)viewDidLoad {
    [super viewDidLoad];
   
    CGRect rect = [[NSScreen mainScreen] frame];
    CGSize size = rect.size;
    CGFloat windowWidth = size.width;
    CGFloat windowHeight = size.height;
    self.render = [[GlFlatRender alloc]init];
    //self.glView = [[GLEssentialsGLView alloc]initWithFrame:CGRectMake(0, 0, windowWidth, windowHeight) render:self.render];
//    self.glView0 = [[GLView alloc]initWithFrame:CGRectMake(0, 0, windowWidth, windowHeight) render:self.render];
//    [self.view addSubview:self.glView0];
    //[self testDecoder];
    [self testDecoderWrapper];
}

static void func_get_frame(void* opaque, IjkVideoFrame *frame_callback) {
    NSLog(@"this is func_get_frame!");
}

static void func_state_change(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2) {
    NSLog(@"this is func_state_change, %d!", ijk_msgint);
}

-(void)testDecoderWrapper{
    NSLog(@"testDecoderWrappers!");
    ijkFfplayDecoder_init();
    ijkDecoder = ijkFfplayDecoder_create();
    IjkFfplayDecoderCallBack callBack;
    callBack.func_get_frame = func_get_frame;
    callBack.func_state_change = func_state_change;
    ijkFfplayDecoder_setDecoderCallBack(ijkDecoder, (__bridge void *) self, &callBack);
    const char* path = "http://media.qicdn.detu.com//13468647-9359-C8B5-8844-6AD3F83942488/2017-10-12/59df08a9f78b7-test.m3u8";
    path = "/Users/chao/Downloads/pano/tb402.MP4";
    ijkFfplayDecoder_setHwDecoderName(ijkDecoder, "h264_vtb");
    ijkFfplayDecoder_setDataSource(ijkDecoder, path);
    ijkFfplayDecoder_prepare(ijkDecoder);
    ijkFfplayDecoder_start(ijkDecoder);
}

- (void) awakeFromNib {
    NSLog(@"awakeFromNib ---------------");
    
//    if(!haveExecute) {
//        CGRect rect = [[NSScreen mainScreen] frame];
//        CGSize size = rect.size;
//        CGFloat windowWidth = size.width;
//        CGFloat windowHeight = size.height;
//        self.render = [[GlFlatRender alloc]init];
//        self.glView = [[GLEssentialsGLView alloc]initWithFrame:CGRectMake(0, 0, windowWidth, windowHeight) render:self.render];
//        [self.glView awakeFromNib];
//        haveExecute = true;
//    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

@end
