//
//  GLPlayerView.h
//  IJKMediaDemo
//
//  Created by qiruisun on 16/5/20.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKMath.h>
#import <GLKit/GLKit.h>
#import <UIKit/UIKit.h>

#import "GLESRenderer.h"
#import "IJKPlayerMovieDecoder.h"



@interface GLPlayerView : UIView

@property  GLESRenderer* renderer;

-(void)setFrameSDL:(SDL_VoutOverlay*)frame;
//add for liveplayer by hcm
-(void)stepFrame:(uint8_t*)frame:(int)w:(int)h;
-(void)render;
- (UIImage *)getScreenShot;


@end