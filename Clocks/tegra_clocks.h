#ifndef TEGRA_CLOCKS_H
#define TEGRA_CLOCKS_H

#include "../Common/inc/jetsonStatus.h"

#define MAXBUF     64
#define SYSTEM_CPU_DIR "/sys/devices/system/cpu"

Status getNumCpus(int *totalCpus);
Status setCpuClock();
Status getCpuClocks(int gpuID, char buf[]);
Status setGpuClock();
Status getGpuClocks();
Status queryCpuStats();
Status queryGpuStats();

#endif
