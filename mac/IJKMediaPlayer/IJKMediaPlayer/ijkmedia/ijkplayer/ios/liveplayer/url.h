/*
 * Copyright 2010 Scyphus Solutions Co. Ltd.  All rights reserved.
 *
 * Authors:
 *      Hirochika Asai
 */

#ifndef _URL_PARSER_H
#define _URL_PARSER_H

/*
 * URL storage
 */
typedef struct UrlData {
	char *scheme;               /* mandatory */
	char *host;                 /* mandatory */
	char *port;                 /* optional */
	char *path;                 /* optional */
	char *query;                /* optional */
	char *fragment;             /* optional */
	char *username;             /* optional */
	char *password;             /* optional */
} UrlData;

#ifdef __cplusplus
extern "C" {
#endif

	/*
	 * Declaration of function prototypes
	 */
	UrlData *UrlParse(const char *url);
	void UrlFree(UrlData *ud);

#ifdef __cplusplus
}
#endif

#endif /* _URL_PARSER_H */
