#ifndef FFPLAY__FF_FFPIPELINE_WIN_H
#define FFPLAY__FF_FFPIPELINE_WIN_H


#include <stdbool.h>
#include "ijkplayer/ff_ffpipeline.h"
#include "ijksdl/ijksdl_vout.h"


struct  FFPlayer;
typedef struct IJKFF_Pipeline IJKFF_Pipeline;

IJKFF_Pipeline *ffpipeline_create_from_win(struct FFPlayer *ffp);


void    ffpipeline_set_vout(IJKFF_Pipeline* pipeline, SDL_Vout *vout);

int     ffpipeline_set_surface(JNIEnv *env, IJKFF_Pipeline* pipeline, jobject surface);

int     ffpipeline_lock_surface(IJKFF_Pipeline* pipeline);
int     ffpipeline_unlock_surface(IJKFF_Pipeline* pipeline);
jobject ffpipeline_get_surface_as_global_ref_l(JNIEnv *env, IJKFF_Pipeline* pipeline);
jobject ffpipeline_get_surface_as_global_ref(JNIEnv *env, IJKFF_Pipeline* pipeline);

bool    ffpipeline_is_surface_need_reconfigure_l(IJKFF_Pipeline* pipeline);
void    ffpipeline_set_surface_need_reconfigure_l(IJKFF_Pipeline* pipeline, bool need_reconfigure);

void    ffpipeline_set_mediacodec_select_callback(IJKFF_Pipeline* pipeline, bool(*callback)(void *opaque, ijkmp_mediacodecinfo_context *mcc), void *opaque);
bool    ffpipeline_select_mediacodec_l(IJKFF_Pipeline* pipeline, ijkmp_mediacodecinfo_context *mcc);

void    ffpipeline_set_volume(IJKFF_Pipeline* pipeline, float left, float right);

#endif
