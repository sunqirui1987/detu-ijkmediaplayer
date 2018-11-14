//
//  H264Utils.m
//  MutilRtspPlay_Lib
//
//  Created by chao on 2017/9/14.
//  Copyright © 2017年 detu. All rights reserved.
//

#import "H264Utils.h"

#include <libavformat/avio.h>
#include <libavformat/avc.h>

@implementation H264Utils

-(bool)annexbToAvcExtraSize:(unsigned char**)data size:(int*)size {
    AVIOContext *pb;
    if (avio_open_dyn_buf(&pb) < 0) {
        return false;
    }
    ff_isom_write_avcc(pb, *data, *size);
    *data = NULL;
    
    *size = avio_close_dyn_buf(pb, data);
    return true;
}

-(bool)annexbToAvcPacket:(unsigned char*)data size:(int)size outData:(unsigned char**)outData outSize:(int*)outSize {
    AVIOContext *pb = NULL;
    if(avio_open_dyn_buf(&pb) < 0) {
        return false;
    }
    ff_avc_parse_nal_units(pb, data, size);
    *outSize = avio_close_dyn_buf(pb, outData);
    if (*outSize == 0) {
        return false;
    }
    return true;
}




@end
