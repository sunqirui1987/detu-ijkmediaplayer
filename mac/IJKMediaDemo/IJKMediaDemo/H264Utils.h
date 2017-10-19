//
//  H264Utils.h
//  MutilRtspPlay_Lib
//
//  Created by chao on 2017/9/14.
//  Copyright © 2017年 detu. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface H264Utils : NSObject

-(bool)annexbToAvcExtraSize:(unsigned char**)data size:(int*)size;

-(bool)annexbToAvcPacket:(unsigned char*)data size:(int)size outData:(unsigned char**)outData outSize:(int*)outSize;

@end
