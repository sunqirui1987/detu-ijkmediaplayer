//
//  ALAssetsLibrary+CustomPhotoAlbum.h
//  DeTuZZN
//
//  Created by shenxing on 15/10/9.
//  Copyright (c) 2015年 DETU. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import <UIKit/UIKit.h>
#import <ImageIO/ImageIO.h>

typedef NS_ENUM(NSInteger, SAVE_TO_SYSTEM_ALBUM_PIC_TYPE)
{
    SAVE_TO_SYSTEM_ALBUM_PIC_TYPE_ORIGINAL,
    SAVE_TO_SYSTEM_ALBUM_PIC_TYPE_2to1,
    SAVE_TO_SYSTEM_ALBUM_PIC_TYPE_FISHEYE
    
};

@interface ALAssetsLibrary(CustomPhotoAlbum)
-(void)saveImage:(NSString*)path toAlbum:(NSString*)albumName picType:(SAVE_TO_SYSTEM_ALBUM_PIC_TYPE)picType withCompletionBlock:(void(^)(NSError* error))completionBlock;
-(void)saveVedio:(NSString*)path toAlbum:(NSString*)albumName withCompletionBlock:(void(^)(NSError* error))completionBlock;
@end
