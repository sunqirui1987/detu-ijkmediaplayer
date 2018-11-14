//
//  ALAssetsLibrary+CustomPhotoAlbum.m
//  DeTuZZN
//
//  Created by shenxing on 15/10/9.
//  Copyright (c) 2015年 DETU. All rights reserved.
//

#import "ALAssetsLibrary+CustomPhotoAlbum.h"
//#import "GLProjectiveImage.h"


@implementation ALAssetsLibrary(CustomPhotoAlbum)



-(void)saveImage:(NSString*)path toAlbum:(NSString*)albumName picType:(SAVE_TO_SYSTEM_ALBUM_PIC_TYPE)picType withCompletionBlock:(void(^)(NSError* error))completionBlock
{
    UIImage* image = [[UIImage alloc] initWithContentsOfFile:path];
    UIImage *resimg;
    
    if(picType == SAVE_TO_SYSTEM_ALBUM_PIC_TYPE_2to1){
//        GLProjectiveImage *prejectimage = [[GLProjectiveImage alloc]init];
//        resimg= [prejectimage convertPanoUIImage:image deviceid:1];
    }else if(picType == SAVE_TO_SYSTEM_ALBUM_PIC_TYPE_FISHEYE){
//        GLProjectiveImage *prejectimage = [[GLProjectiveImage alloc]init];
//        resimg = [prejectimage convertFisheyeToRect:image deviceid:1];
    }else{
        resimg = image;
    }
    
    
    if(resimg)
        [self writeImageToSavedPhotosAlbum:resimg.CGImage metadata:nil completionBlock:^(NSURL *assetURL, NSError *error) {
            
            //error handling
            if (error!=nil) {
                completionBlock(error);
                return;
            }
            
            //add the asset to the custom photo album
            [self addAssetURL: assetURL
                      toAlbum:albumName
          withCompletionBlock:completionBlock];
            
        } ];
}

-(void)saveVedio:(NSString*)path toAlbum:(NSString*)albumName withCompletionBlock:(void(^)(NSError* error))completionBlock
{
    [self writeVideoAtPathToSavedPhotosAlbum:[NSURL URLWithString:path] completionBlock:^(NSURL *assetURL, NSError *error) {
        if (error!=nil) {
            completionBlock(error);
            return;
        }
        
        //add the asset to the custom photo album
        [self addAssetURL: assetURL
                  toAlbum:albumName
      withCompletionBlock:completionBlock];
    }];
}

-(void)addAssetURL:(NSURL*)assetURL toAlbum:(NSString*)albumName withCompletionBlock:(void(^)(NSError* error))completionBlock
{
    __block BOOL albumWasFound = NO;
    
    //search all photo albums in the library
    [self enumerateGroupsWithTypes:ALAssetsGroupAlbum
                        usingBlock:^(ALAssetsGroup *group, BOOL *stop) {
                            
                            //compare the names of the albums
                            if ([albumName compare: [group valueForProperty:ALAssetsGroupPropertyName]]==NSOrderedSame) {
                                
                                //target album is found
                                albumWasFound = YES;
                                
                                //get a hold of the photo's asset instance
                                [self assetForURL: assetURL
                                      resultBlock:^(ALAsset *asset) {
                                          
                                          //add photo to the target album
                                          [group addAsset: asset];
                                          
                                          //run the completion block
                                          completionBlock(nil);
                                          
                                      } failureBlock: completionBlock];
                                
                                //album was found, bail out of the method
                                return;
                            }
                            
                            if (group==nil && albumWasFound==NO) {
                                //photo albums are over, target album does not exist, thus create it
                                
                                __weak ALAssetsLibrary* weakSelf = self;
                                
                                //create new assets album
                                [self addAssetsGroupAlbumWithName:albumName
                                                      resultBlock:^(ALAssetsGroup *group) {
                                                          
                                                          //get the photo's instance
                                                          [weakSelf assetForURL: assetURL
                                                                    resultBlock:^(ALAsset *asset) {
                                                                        
                                                                        //add photo to the newly created album
                                                                        [group addAsset: asset];
                                                                        
                                                                        //call the completion block
                                                                        completionBlock(nil);
                                                                        
                                                                    } failureBlock: completionBlock];
                                                          
                                                      } failureBlock: completionBlock];
                                
                                //should be the last iteration anyway, but just in case
                                return;
                            }
                            
                        } failureBlock: completionBlock];
    
}


@end
