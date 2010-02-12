#ifndef FLYSWATTER_H
#define FLYSWATTER_H		$Id: flyswatterclient.h,v 1.1.1.1 2010-02-12 10:11:55 bits Exp $

#include <windows.h>

typedef int (*flyswatter_init_func_ptr)(TCHAR *dumpPath, TCHAR *reportUrl);
typedef int (*flyswatter_enable_func_ptr)();
typedef int (*flyswatter_disable_func_ptr)();
typedef int (*flyswatter_isenabled_func_ptr)();

#endif
