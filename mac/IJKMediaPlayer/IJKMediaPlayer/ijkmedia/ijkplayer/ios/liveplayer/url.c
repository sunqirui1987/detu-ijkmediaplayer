/*_
 * Copyright 2010-2011 Scyphus Solutions Co. Ltd.  All rights reserved.
 *
 * Authors:
 *      Hirochika Asai
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "url.h"

/*
 * Prototype declarations
 */
static __inline__ int _is_scheme_char(int);

/*
 * Check whether the character is permitted in scheme string
 */
static __inline__ int
_is_scheme_char(int c)
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/*
 * See RFC 1738, 3986
 */
UrlData *UrlParse(const char *url)
{
    UrlData *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;

    /* Allocate the parsed url storage */
    purl = malloc(sizeof(UrlData));
    if (!purl)
        return NULL;
    memset(purl, 0, sizeof(UrlData));

    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if (NULL == tmpstr) {
        /* Not found the character */
        UrlFree(purl);
        return NULL;
    }
    /* Get the scheme length */
    len = tmpstr - curstr;
    /* Check restrictions */
    for (i = 0; i < len; i++) {
        if (!_is_scheme_char(curstr[i])) {
            /* Invalid format */
            UrlFree(purl);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = malloc(sizeof(char) * (len + 1));
    if (NULL == purl->scheme) {
        UrlFree(purl);
        return NULL;
    }
    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';
    /* Make the character to lower if it is upper case. */
    for (i = 0; i < len; i++) {
        purl->scheme[i] = tolower(purl->scheme[i]);
    }
    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for (i = 0; i < 2; i++) {
        if ('/' != *curstr) {
            UrlFree(purl);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ('\0' != *tmpstr) {
        if ('@' == *tmpstr) {
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        } else if ('/' == *tmpstr) {
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if (userpass_flag) {
        /* Read username */
        while ('\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->username = malloc(sizeof(char) * (len + 1));
        if (NULL == purl->username) {
            UrlFree(purl);
            return NULL;
        }
        (void)strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';
        /* Proceed current pointer */
        curstr = tmpstr;
        if (':' == *curstr) {
            /* Skip ':' */
            curstr++;
            /* Read password */
            tmpstr = curstr;
            while ('\0' != *tmpstr && '@' != *tmpstr) {
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = malloc(sizeof(char) * (len + 1));
            if (NULL == purl->password) {
                UrlFree(purl);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }
        /* Skip '@' */
        if ('@' != *curstr) {
            UrlFree(purl);
            return NULL;
        }
        curstr++;
    }

    if ('[' == *curstr) {
        bracket_flag = 1;
    } else {
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ('\0' != *tmpstr) {
        if (bracket_flag && ']' == *tmpstr) {
            /* End of IPv6 address. */
            tmpstr++;
            break;
        } else if (!bracket_flag && (':' == *tmpstr || '/' == *tmpstr)) {
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = malloc(sizeof(char) * (len + 1));
    if (NULL == purl->host || len <= 0) {
        UrlFree(purl);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if (':' == *curstr) {
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ('\0' != *tmpstr && '/' != *tmpstr) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->port = malloc(sizeof(char) * (len + 1));
        if (NULL == purl->port) {
            UrlFree(purl);
            return NULL;
        }
        (void)strncpy(purl->port, curstr, len);
        purl->port[len] = '\0';
        curstr = tmpstr;
    }

    /* End of the string */
    if ('\0' == *curstr) {
        return purl;
    }

    /* Skip '/' */
    if ('/' != *curstr) {
        UrlFree(purl);
        return NULL;
    }
//    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ('\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr) {
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->path = malloc(sizeof(char) * (len + 1));
    if (NULL == purl->path) {
        UrlFree(purl);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    /* Is query specified? */
    if ('?' == *curstr) {
        /* Skip '?' */
        curstr++;
        /* Read query */
        tmpstr = curstr;
        while ('\0' != *tmpstr && '#' != *tmpstr) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->query = malloc(sizeof(char) * (len + 1));
        if (NULL == purl->query) {
            UrlFree(purl);
            return NULL;
        }
        (void)strncpy(purl->query, curstr, len);
        purl->query[len] = '\0';
        curstr = tmpstr;
    }

    /* Is fragment specified? */
    if ('#' == *curstr) {
        /* Skip '#' */
        curstr++;
        /* Read fragment */
        tmpstr = curstr;
        while ('\0' != *tmpstr) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->fragment = malloc(sizeof(char) * (len + 1));
        if (NULL == purl->fragment) {
            UrlFree(purl);
            return NULL;
        }
        (void)strncpy(purl->fragment, curstr, len);
        purl->fragment[len] = '\0';
        curstr = tmpstr;
    }

    return purl;
}

/*
 * Free memory of parsed url
 */
void UrlFree(UrlData *purl)
{
    if (purl) {
        if (purl->scheme)
            free(purl->scheme);
        if (purl->host)
            free(purl->host);
        if (purl->port)
            free(purl->port);
        if (purl->path)
            free(purl->path);
        if (purl->query)
            free(purl->query);
        if (purl->fragment)
            free(purl->fragment);
        if (purl->username)
            free(purl->username);
        if (purl->password)
            free(purl->password);
        free(purl);
    }
}
