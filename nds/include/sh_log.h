#ifndef SH_NDS_LOG_H
#define SH_NDS_LOG_H

#include <stdio.h>

/* Some original translation units flush the PC diagnostic stream directly.
 * Keep a real, always-NULL symbol so those sources compile and link unchanged
 * while the DS build performs no filesystem logging. */
extern FILE* g_ShDebugLog;

#define SH_DBG(...) ((void)0)
#define SH_DBG_ECHO(...) ((void)0)
#define SH_LOG(...) ((void)0)
#define SH_WARN(...) ((void)0)
#define SH_ERR(...) ((void)0)

#endif
