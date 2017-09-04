/*************************************************************************
	> File Name: thepublic.h
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Thu 17 Aug 2017 11:35:28 AM CST
 ************************************************************************/
#ifndef _THEPUBLIC_H_
#define _THEPUBLIC_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#include <sys/types.h> //定义了 unit 等类型的数据

#define _IN
#define _OUT
#define _IN_OUT

#ifndef INVALID_VALUE
#define INVALID_VALUE 0
#endif

#define ERR_PMUTEX_CREATE 1001
#define ERR_PMUTEX_CLOSE 1002
#define ERR_PMUTEX_INIT 1003
#define ERR_PMUTEX_LOCK 1004
#define ERR_PMUTEX_UNLOCK 1005

#define ERR_SHM_INVALID_ID  3001
#define ERR_SHM_GET         3002
#define ERR_SHM_STATUS      3003
#define ERR_SHMM_FORMAT     3004
#define ERR_SHMM_FULL       3005
#define ERR_SHMM_LOCK       3006
#define ERR_SHMM_UNLOCK     3007
#define ERR_SHMM_BUF_LESS   3008
#define ERR_SHMM_CLOSE      3009
#define ERR_SHM_CREATE      3010
#define ERR_SHM_BIND        3011


#define LOGS(format,...) fprintf(stdout,"[pos=%d]",__LINE__ ); fprintf(stdout,format,##__VA_ARGS__)
#define ErrorLog(f1,...)  fprintf(stdout,"[file=%s][line=%d]"#f1,  __FILE__,__LINE__,##__VA_ARGS__)

//#define INVALID_VALUE 0
#endif