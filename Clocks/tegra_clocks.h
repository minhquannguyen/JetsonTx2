#ifndef TEGRA_CLOCKS_H
#define TEGRA_CLOCKS_H

#include "../Common/inc/jetsonStatus.h"

#define MAXBUF     1024
#define CHUNK      255
#define MAXTX2CPUS 11

Status queryCpuStats();
Status queryGpuStats();
Status setCpuClock();
Status getCpuClocks();
Status setGpuClock();
Status getGpuClocks();

#endif
