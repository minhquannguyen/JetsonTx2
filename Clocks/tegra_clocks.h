#ifndef TEGRA_CLOCKS_H
#define TEGRA_CLOCKS_H

#include "../Common/inc/jetsonStatus.h"

#define MAXBUF     64
#define SYSTEM_CPU_DIR "/sys/devices/system/cpu"

Status get_num_cpus(int *totalCpus);
Status set_cpu_clock();
Status get_cpu_clocks(int gpuID, char buf[]);
Status set_gpu_clock();
Status get_gpu_clocks();
Status query_cpu_stats();
Status query_gpu_stats();

#endif
