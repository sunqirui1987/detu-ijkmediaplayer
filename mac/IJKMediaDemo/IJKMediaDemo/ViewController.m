//
//  ViewController.m
//  IJKMediaDemo
//
//  Created by chao on 2017/6/10.
//  Copyright © 2017年 detu. All rights reserved.
//

#import "ViewController.h"

#import "IJKPlayerMovieDecoder.h"
#import "GLEssentialsGLView.h"
#import "GlRender.h"
#import "GlFlatRender.h"
#import "GLView.h"

@interface ViewController()<MovieDecoderDelegate> {
    bool haveExecute;
}
@property(nonatomic, strong)IJKPlayerMovieDecoder* decoder;
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
    [self testDecoder];
    
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

-(void)testDecoder{
    NSString* path = @"http://media.detu.com/@/17717910-8057-4FDF-2F33-F8B1F68282395/2016-08-22/57baeda5920ea-similar.mp4";
    path = @"http://media.detu.com/@/13711706-6958-9D6C-5D4C-BB75F19844880/2017-07-09/5961f45b072d1-2048x1024.m3u8";
    self.decoder = [IJKPlayerMovieDecoder movieDecoderWithMovie:path isHardWare:false];
    self.decoder.delegate = self;
    [self.decoder start];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
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
    //[self.render setOverlay:frame];
    FILE* file = fopen("/Users/chao/Desktop/ijk.yuv", "wb+");
    int ySize = frame->w * frame->h;
    int uSize = ySize / 4;

    CVPixelBufferRef pixel = SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(frame);
    size_t count =  CVPixelBufferGetPlaneCount(pixel);
    for(int i = 0; i < count; i++) {
        //CVPixelBufferLockBaseAddress(pixel, i);
        size_t height = CVPixelBufferGetHeightOfPlane(pixel, i);
        size_t stride = CVPixelBufferGetBytesPerRowOfPlane(pixel, i);
        void * pb = CVPixelBufferGetBaseAddressOfPlane(pixel, i);
        fwrite(pb, stride * height, 1, file);
        //CVPixelBufferUnlockBaseAddress(pixel, i);
    }

    
    //fwrite(frame->pixels[0], ySize, 1, file);
    //fwrite(frame->pixels[1], uSize * 2, 1, file);
    //fwrite(frame->pixels[2], uSize, 1, file);
    fclose(file);
}

@end
