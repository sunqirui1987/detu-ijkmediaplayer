//
//  ijksdl_data.h
//  IJKMediaPlayer
//
//  Created by chao on 2017/9/30.
//  Copyright © 2017年 bilibili. All rights reserved.
//

#ifndef ijksdl_data_h
#define ijksdl_data_h

typedef struct SDL_VoutOverlay_Opaque SDL_VoutOverlay_Opaque;
typedef struct SDL_VoutOverlay SDL_VoutOverlay;
struct SDL_VoutOverlay {
    int w; /**< Read-only */
    int h; /**< Read-only */
    Uint32 format; /**< Read-only */
    int planes; /**< Read-only */
    Uint16 *pitches; /**< in bytes, Read-only */
    Uint8 **pixels; /**< Read-write */
    
    int is_private;
    
    int sar_num;
    int sar_den;
    
    SDL_Class               *opaque_class;
    SDL_VoutOverlay_Opaque  *opaque;
    
    void    (*free_l)(SDL_VoutOverlay *overlay);
    int     (*lock)(SDL_VoutOverlay *overlay);
    int     (*unlock)(SDL_VoutOverlay *overlay);
    void    (*unref)(SDL_VoutOverlay *overlay);
    
    int     (*func_fill_frame)(SDL_VoutOverlay *overlay, const void *frame);
};



#endif /* ijksdl_data_h */
