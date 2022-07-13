#include <stdio.h>
#include <stdint.h>

#define my_printf(LOG_LEVEL,...) QCLI_Printf(LOG_LEVEL,NULL,__VA_ARGS__)

