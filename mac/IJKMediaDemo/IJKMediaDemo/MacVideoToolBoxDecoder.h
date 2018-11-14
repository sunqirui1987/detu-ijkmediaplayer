//
//  MacVideoToolBoxDecoder.h
//  IJkMediaDemo
//
//  Created by chao on 2017/10/17.
//  Copyright © 2017年 annidy. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VideoToolBoxDecoder.h"

@interface MacVideoToolBoxDecoder : NSObject

@property(nonatomic, weak)id<VideoToolBoxDecoderDelegate> delegate;

-(void)setDataSource:(NSString*) url;

-(void)start;

@end
