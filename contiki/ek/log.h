#ifndef __LOG_H__
#define __LOG_H__

#include "log-conf.h"

#if LOG_CONF_ENABLED
void log_message(const char *part1, const char *part2);
#else /* LOG_CONF_ENABLED */
#define log_message(p1, p2)
#endif /* LOG_CONF_ENABLED */

#endif /* __LOG_H__ */
