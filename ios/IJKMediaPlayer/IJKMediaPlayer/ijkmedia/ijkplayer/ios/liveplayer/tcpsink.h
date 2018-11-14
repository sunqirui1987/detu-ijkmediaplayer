#ifndef __TCPSINK_H__
#define __TCPSINK_H__

#include "viddec.h"

typedef struct TcpSink TcpSink;

TcpSink *TcpSink_OpenStream(char *ipaddr, int port);
int TcpSink_CloseStream(TcpSink *sink);
int TcpSink_ReadStream(TcpSink *sink, VideoPkt *pkt, int time_ms);

int TcpSink_HasKeyFrameBuffered(TcpSink *sink);
int TcpSink_NumberFrameBuffered(TcpSink *sink);

#endif
