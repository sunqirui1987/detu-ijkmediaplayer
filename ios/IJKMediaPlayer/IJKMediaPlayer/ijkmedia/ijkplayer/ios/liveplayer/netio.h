#ifndef __UTILS_NETIO_H__
#define __UTILS_NETIO_H__

#include <ctype.h>

int j_net_wait_readable (int fd, int milliseconds);
int j_net_wait_writeable(int fd, int milliseconds);

ssize_t j_net_readn_timedwait (int fd, void *buf, size_t count);
ssize_t j_net_writen_timedwait(int fd, void *buf, size_t count);

int j_net_set_nonblock(int socket, int enable);

int j_net_tcp_server (char *ipaddr, int port);
int j_net_tcp_connect(char *ipaddr, int port);

int j_net_udp_server (char *ipaddr, int port);
int j_net_udp_connect(char *ipaddr, int port);

int j_net_tcp_accept(int sockfd);
int j_net_connect_state(int sockfd);

#endif /* __UTILS_NETIO_H__ */
