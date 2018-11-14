//
//  FfmpegDecoder.hpp
//  MutilRtspPlay_Lib
//
//  Created by chao on 2017/9/13.
//  Copyright © 2017年 detu. All rights reserved.
//

#import"VideoToolBoxDecoder.h"
#include <libavformat/avio.h>
#include <libavformat/avc.h>
#import "H264Utils.h"

typedef enum {
    SGFFVideoToolBoxErrorCodeExtradataSize,
    SGFFVideoToolBoxErrorCodeExtradataData,
    SGFFVideoToolBoxErrorCodeCreateFormatDescription,
    SGFFVideoToolBoxErrorCodeCreateSession,
    SGFFVideoToolBoxErrorCodeNotH264,
}SGFFVideoToolBoxErrorCode;

@interface VideoToolBoxDecoder() {
    CVImageBufferRef imageBuffer;
    VTDecompressionSessionRef _vt_session;
    CMFormatDescriptionRef _format_description;
    bool vtSessionToken;
    bool needConvertNALSize3To4;
    bool convert_bytestream;
    bool isWrite;
//    NSError* setupVTSession();
//    void cleanVTSession();
//    bool trySetupVTSession();
    H264Utils* h264Utils;
    int64_t frameIndex;
    AVCodecContext * context;
    int identityId;
}

@end

@implementation VideoToolBoxDecoder

-(void)init:(AVCodecContext *)contextIn :(int)inIdentityId {
    isWrite = false;
    vtSessionToken = false;
    _format_description = nil;
    _vt_session = nil;
    convert_bytestream = false;
    h264Utils = [[H264Utils alloc]init];
    frameIndex = 0;
    context = contextIn;
    identityId = inIdentityId;
}

-(bool)sendPacket:(AVPacket*) packet {
    BOOL setupResult = [self trySetupVTSession];
    if (!setupResult) return false;
    BOOL result = NO;
    CMBlockBufferRef blockBuffer = NULL;
    OSStatus status = noErr;
    
    if(convert_bytestream) {
        unsigned char* outData = NULL;
        int outSize = 0;
        bool ret = [h264Utils annexbToAvcPacket:packet->data size:packet->size outData:&outData outSize:&outSize];
        if(ret) {
            packet->data = outData;
            packet->size = outSize;
        }
    }
    
    if (needConvertNALSize3To4) {
//        AVIOContext * io_context = NULL;
//        if (avio_open_dyn_buf(&io_context) < 0) {
//            status = -1900;
//        } else {
//            uint32_t nal_size;
//            uint8_t * end = packet.data + packet.size;
//            uint8_t * nal_start = packet.data;
//            while (nal_start < end) {
//                nal_size = (nal_start[0] << 16) | (nal_start[1] << 8) | nal_start[2];
//                avio_wb32(io_context, nal_size);
//                nal_start += 3;
//                avio_write(io_context, nal_start, nal_size);
//                nal_start += nal_size;
//            }
//            uint8_t * demux_buffer = NULL;
//            int demux_size = avio_close_dyn_buf(io_context, &demux_buffer);
//            status = CMBlockBufferCreateWithMemoryBlock(NULL, demux_buffer, demux_size, kCFAllocatorNull, NULL, 0, packet.size, FALSE, &blockBuffer);
//        }
    } else {
        status = CMBlockBufferCreateWithMemoryBlock(NULL, packet->data, packet->size, kCFAllocatorNull, NULL, 0, packet->size, FALSE, &blockBuffer);
    }
    
    if (status == noErr) {
        CMSampleBufferRef sampleBuffer = NULL;
        status = CMSampleBufferCreate( NULL, blockBuffer, TRUE, 0, 0, _format_description, 1, 0, NULL, 0, NULL, &sampleBuffer);
        
        if (status == noErr) {
            long* timeStamp = (long*)calloc(1, sizeof(long));
            *timeStamp = packet->pts;
            status = VTDecompressionSessionDecodeFrame(_vt_session, sampleBuffer, kVTDecodeFrame_EnableAsynchronousDecompression, (void*)timeStamp, 0);
            //NSLog(@"sendPacket the session, status:%d", status);
            if (status != noErr) {
                //free(timeStamp);
            }
            if (status == noErr) {
                //status = VTDecompressionSessionWaitForAsynchronousFrames(_vt_session);
                result = YES;
            } else if (status == kVTInvalidSessionErr) {
                //* needFlush = YES;
                //trySetupVTSession();
            } else if(status == kVTParameterErr) {
                NSLog(@"send packet video toolbox param error, try ");
                [self cleanVTSession];
                [self trySetupVTSession];
                [self sendPacket:packet];
            }
        }
        if (sampleBuffer) {
            CFRelease(sampleBuffer);
        }
    }
    if (blockBuffer) {
        CFRelease(blockBuffer);
    }
    return result;
}

-(bool) trySetupVTSession {
    if (!vtSessionToken) {
        NSError * error = [self setupVTSession];
        if (!error) {
            vtSessionToken = YES;
            NSLog(@"trySetupVTSession  scuess ");
        }else{
            NSLog(@"trySetupVTSession  failed %d ",[error code]);
        }
    }
    return vtSessionToken;
}

-(void)flush {
    [self cleanVTSession];
}

static void cf_dict_set_data(CFMutableDictionaryRef dict, CFStringRef key, uint8_t * value, uint64_t length)
{
    CFDataRef data;
    data = CFDataCreate(NULL, value, (CFIndex)length);
    CFDictionarySetValue(dict, key, data);
    CFRelease(data);
}

static void cf_dict_set_int32(CFMutableDictionaryRef dict, CFStringRef key, int32_t value)
{
    CFNumberRef number;
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
    CFDictionarySetValue(dict, key, number);
    CFRelease(number);
}

static void cf_dict_set_string(CFMutableDictionaryRef dict, CFStringRef key, const char * value)
{
    CFStringRef string;
    string = CFStringCreateWithCString(NULL, value, kCFStringEncodingASCII);
    CFDictionarySetValue(dict, key, string);
    CFRelease(string);
}

static void cf_dict_set_boolean(CFMutableDictionaryRef dict, CFStringRef key, BOOL value)
{
    CFDictionarySetValue(dict, key, value ? kCFBooleanTrue: kCFBooleanFalse);
}

static void cf_dict_set_object(CFMutableDictionaryRef dict, CFStringRef key, CFTypeRef *value)
{
    CFDictionarySetValue(dict, key, value);
}

static double countTime = 0;

static void outputCallback(void * decompressionOutputRefCon, void * sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime presentationDuration)
{
    @autoreleasepool
    {
        VideoToolBoxDecoder * videoToolBox = (__bridge VideoToolBoxDecoder *)decompressionOutputRefCon;
        //CVBufferRef pixelBuffer = CVBufferRetain(imageBuffer);
        //CVBufferRef pixelBuffer = nil;
        //NSLog(@"this is outputCallback, index:%d", videoToolBox->identityId);
        double end = CFAbsoluteTimeGetCurrent()*1000;
        NSLog(@"this is outputCallback use time%f millisecond ", (end-countTime));
        countTime = end;
        
//        DecodeFrame frame;
//        memset(&frame, 0, sizeof(frame));
//        frame.width = videoToolBox->context->width;
//        frame.height = videoToolBox->context->height;
//        frame.opaque = pixelBuffer;
//        frame.format = AV_PIX_FMT_VDA;
//        long* timeStamp = (long*)sourceFrameRefCon;
//        frame.pts = *timeStamp;
//        frame.canRelease = true;
//        //NSLog(@"videtool box input pts:%" PRIu64 "", frame.pts);
//        free(timeStamp);
//        if(videoToolBox.delegate != nil) {
//            [videoToolBox.delegate videoToolBoxDecoderFrame:&frame identity:videoToolBox->identityId];
//        }
    }
}

static CMFormatDescriptionRef CreateFormatDescription(CMVideoCodecType codec_type, int width, int height, const uint8_t * extradata, int extradata_size)
{
    CMFormatDescriptionRef format_description = NULL;
    OSStatus status;
    
    CFMutableDictionaryRef par = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    // CVPixelAspectRatio
    cf_dict_set_int32(par, CFSTR("HorizontalSpacing"), 0);
    cf_dict_set_int32(par, CFSTR("VerticalSpacing"), 0);
    
    // SampleDescriptionExtensionAtoms
    cf_dict_set_data(atoms, CFSTR("avcC"), (uint8_t *)extradata, extradata_size);
    
    // Extensions
    //cf_dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationBottomField"), "left");
    //cf_dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationTopField"), "left");
    cf_dict_set_boolean(extensions, CFSTR("FullRangeVideo"), TRUE);
    cf_dict_set_object(extensions, CFSTR ("CVPixelAspectRatio"), (CFTypeRef *)par);
    cf_dict_set_object(extensions, CFSTR ("SampleDescriptionExtensionAtoms"), (CFTypeRef *)atoms);
    
    status = CMVideoFormatDescriptionCreate(NULL, codec_type, width, height, extensions, &format_description);
    
    CFRelease(extensions);
    CFRelease(atoms);
    CFRelease(par);
    
    if (status != noErr) {
        return NULL;
    }
    return format_description;
}

-(NSError*)setupVTSession
{
    NSError * error;
    
    enum AVCodecID codec_id = context->codec_id;
    uint8_t * extradata = context->extradata;
    int extradata_size = context->extradata_size;
    
    if (codec_id == AV_CODEC_ID_H264) {
        if (extradata_size < 7 || extradata == NULL) {
            error = [NSError errorWithDomain:@"extradata error" code:SGFFVideoToolBoxErrorCodeExtradataSize userInfo:nil];
            return error;
        }
        if ((extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 0 && extradata[3] == 1) ||
            (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 1)) {
//            AVIOContext *pb;
//            if (avio_open_dyn_buf(&pb) < 0) {
//                return [NSError errorWithDomain:@"create session error" code:SGFFVideoToolBoxErrorCodeCreateSession userInfo:nil];
//            }
//            
//            this->convert_bytestream = true;
//            ff_isom_write_avcc(pb, extradata, extradata_size);
//            extradata = NULL;
//            
//            extradata_size = avio_close_dyn_buf(pb, &extradata);
            
//            if (!validate_avcC_spc(extradata, extradata_size, &fmt_desc->max_ref_frames, &sps_level, &sps_profile)) {
//                av_free(extradata);
//                goto fail;
//            }
            bool ret = [h264Utils annexbToAvcExtraSize:&extradata size:&extradata_size];
            if(ret) {
                convert_bytestream = true;
            }
            NSLog(@"annexb to avc:%d", ret);
        }

        if (extradata[0] == 1) {
            if (extradata[4] == 0xFE) {
                extradata[4] = 0xFF;
                needConvertNALSize3To4 = YES;
            }
            _format_description = CreateFormatDescription(kCMVideoCodecType_H264, context->width, context->height, extradata, extradata_size);
            if (_format_description == NULL) {
                error = [NSError errorWithDomain:@"create format description error" code:SGFFVideoToolBoxErrorCodeCreateFormatDescription userInfo:nil];
                return error;
            }
            
            CFMutableDictionaryRef destinationPixelBufferAttributes = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            cf_dict_set_int32(destinationPixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
            cf_dict_set_int32(destinationPixelBufferAttributes, kCVPixelBufferWidthKey, context->width);
            cf_dict_set_int32(destinationPixelBufferAttributes, kCVPixelBufferHeightKey, context->height);
            
#if SGPLATFORM_TARGET_OS_MAC
            cf_dict_set_boolean(destinationPixelBufferAttributes, kCVPixelBufferOpenGLCompatibilityKey, YES);
            cf_dict_set_boolean(destinationPixelBufferAttributes, kCVPixelBufferOpenGLTextureCacheCompatibilityKey, YES);
#elif SGPLATFORM_TARGET_OS_IPHONE
            //            cf_dict_set_boolean(destinationPixelBufferAttributes, kCVPixelBufferOpenGLESCompatibilityKey, YES);
            //            cf_dict_set_boolean(destinationPixelBufferAttributes, kCVPixelBufferOpenGLESTextureCacheCompatibilityKey, YES);
#endif
            
            VTDecompressionOutputCallbackRecord outputCallbackRecord;
            outputCallbackRecord.decompressionOutputCallback = outputCallback;
            outputCallbackRecord.decompressionOutputRefCon = (__bridge void*)self;
            
            OSStatus status = VTDecompressionSessionCreate(kCFAllocatorDefault, _format_description, NULL, destinationPixelBufferAttributes, &outputCallbackRecord, &_vt_session);
            if (status != noErr) {
                error = [NSError errorWithDomain:@"create session error" code:SGFFVideoToolBoxErrorCodeCreateSession userInfo:nil];
                return error;
            }
            CFRelease(destinationPixelBufferAttributes);
            return nil;
        } else {
            error = [NSError errorWithDomain:@"deal extradata error" code:SGFFVideoToolBoxErrorCodeExtradataData userInfo:nil];
            return error;
        }
    } else {
        error = [NSError errorWithDomain:@"not h264 error" code:SGFFVideoToolBoxErrorCodeNotH264 userInfo:nil];
        return error;
    }
    
    return error;
}

-(void)cleanVTSession
{
    if (_format_description) {
        CFRelease(_format_description);
        _format_description = NULL;
    }
    if (_vt_session) {
        //VTDecompressionSessionWaitForAsynchronousFrames(this->_vt_session);
        VTDecompressionSessionInvalidate(_vt_session);
        CFRelease(_vt_session);
        _vt_session = NULL;
    }
    needConvertNALSize3To4 = NO;
    vtSessionToken = NO;
    NSLog(@"video toolbox cleanVTSession!");
}

-(void) unrefFrame:(DecodeFrame*) frame {
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef)frame->opaque;
    if(pixelBuffer != nil) {
        //NSLog(@"send packet unrefFrame");
        CVBufferRelease(pixelBuffer);
    }
}

@end
