#ifndef __RTSPDEC_H__
#define __RTSPDEC_H__

#include <stdint.h>

typedef struct RtspDec RtspDec;

int RtspDec_Init(void);
int RtspDec_Quit(void);

RtspDec *RtspDec_OpenStream(char *url);
int RtspDec_CloseStream(RtspDec *dec);

typedef struct {
    int      codec;     /* 0 - h.264 */
    int      keyframe;
    int64_t  pts;
    uint8_t *data;
    int      size;
    int 	pkt_num;
} RtspFrame;

int RtspDec_ReadStream(RtspDec *dec, RtspFrame *frame, int time_ms);

int RtspDec_HasKeyFrameBuffered(RtspDec *dec);
int RtspDec_NumberFrameBuffered(RtspDec *dec);

#endif /* __RTSPDEC_H__ */
