#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <ctype.h>
#include "common.h"

#define CHUNK   255
#define MAXCPUS 6
#define MAXBUF  1024

void parse_args(char **cmd_line, int *status)
{
	while (*cmd_line)
	{
		cmd_line++;
	}
}

Status query_cpu_stats()
{
	char   buf[CHUNK];
	int    onlineCpus[MAXCPUS];
	FILE   *fp_online, *fp_offline;
	int    bufferIndex = 0, onlineCpuIndex = 0;
	
	memset(buf, 0, CHUNK);
	memset(onlineCpus, 0, MAXCPUS);

	fp_online = fopen("/sys/devices/system/cpu/online", "r");
	
	if (!fp_online)
	{
		printf("%s: Error opening file\n", __FUNCTION__);
		return GENERIC_ERROR;		
	}
	
	// Output the online Cpus
	if (fgets(buf, CHUNK, fp_online) == NULL)
	{
		printf("%s: Error reading file\n", __FUNCTION__);
		fclose(fp_online);
		return GENERIC_ERROR;
	}
	
	// Really hacky, should find a better way
	while (buf[bufferIndex])
	{
		char cpuNum = buf[bufferIndex];
		if (isdigit(cpuNum))
		{
			onlineCpus[onlineCpuIndex] = cpuNum - '0';
		}
		else if (cpuNum == '-')
		{
			while (onlineCpus[onlineCpuIndex-1] < (buf[bufferIndex+1] - '0'))
			{
				onlineCpus[onlineCpuIndex] = onlineCpus[onlineCpuIndex-1] + 1;
				onlineCpuIndex++;
			}
			onlineCpuIndex--;
			bufferIndex++;
		}
		else
		{
			onlineCpuIndex--;
		}
		onlineCpuIndex++;
		bufferIndex++;
	}
	
	printf("Online CPUs: %s", buf);
	memset(buf, 0, CHUNK);
	fclose(fp_online);
	
	// Output the clock values of the Cpus
	for (int i = 0; i < onlineCpuIndex; i++)
	{
		FILE *cpuFile;
		char cpuDirBuffer[MAXBUF];
		
		if (snprintf(cpuDirBuffer, sizeof(cpuDirBuffer),
			"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq",
			onlineCpus[i]) < 0)
		{
			return INSUFFICIENT_RESOURCES;
		}
		
		cpuFile = fopen(cpuDirBuffer, "r");
		
		if (fgets(buf, CHUNK, cpuFile) == NULL)
		{
			printf("%s: Error reading file\n", __FUNCTION__);
			fclose(cpuFile);
			return GENERIC_ERROR;
		}
		printf("Cpu%d running at %s Khz\n", onlineCpus[i], strtok(buf, "\n"));
		fclose(cpuFile);
	}
	
	fp_offline = fopen("/sys/devices/system/cpu/offline", "r");
	memset(buf, 0, CHUNK);

	if (!fp_offline)
	{
		printf("%s: Error opening file\n", __FUNCTION__);
		return GENERIC_ERROR;
	}
	
	// Output the offline Cpus
	if (fgets(buf, CHUNK, fp_offline) == NULL)
	{
		printf("%s: Error reading file\n", __FUNCTION__);
		fclose(fp_offline );
		return GENERIC_ERROR;
	}
	printf("Offline CPUs: %s", buf);
	fclose(fp_offline);
	
	return SUCCESS;
}

int main(int argc, char **argv)
{
	Status status;
	//if (argc > 1)
	//{
	//	parse_args(&argv[0], &status);
	//}
	status = query_cpu_stats();
	
	return 0;
}
