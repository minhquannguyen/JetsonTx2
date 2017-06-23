#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <ctype.h>
#include "tegra_clocks.h"

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
	int    onlineCpus[MAXTX2CPUS];
	FILE   *fp_online, *fp_offline;
	int    bufferIndex = 0, onlineCpuIndex = 0;
	
	memset(buf, 0, sizeof(buf));
	memset(onlineCpus, 0, sizeof(onlineCpus));

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

	int bufIndex = 0;
	char tempBuf[CHUNK];
	memset(tempBuf, 0, sizeof(tempBuf));

	// Really hacky, should find a better way
	while (1)
	{
		char ch = buf[bufferIndex];
		if (ch == '\0')
		{
			onlineCpus[onlineCpuIndex] = atoi(tempBuf);
			break;
		}
		else if (ch == '-')
		{
			onlineCpus[onlineCpuIndex] = atoi(tempBuf);
			bufIndex = 0;
			bufferIndex++;
			memset(tempBuf, 0, sizeof(tempBuf));

			for (ch = buf[bufferIndex]; ch != ',' && ch != '\0'; ch = buf[++bufferIndex])
			{
				tempBuf[bufIndex] = ch;
				bufIndex++;
			}

			while (onlineCpus[onlineCpuIndex] <= atoi(tempBuf))
			{
				onlineCpuIndex++;
				onlineCpus[onlineCpuIndex] =
					onlineCpus[onlineCpuIndex-1] + 1;
			}
			bufIndex = 0;
			memset(tempBuf, 0, sizeof(tempBuf));
		}
		else if (ch == ',')
		{
			onlineCpus[onlineCpuIndex] = atoi(tempBuf);
			onlineCpuIndex++;
			memset(tempBuf, 0, sizeof(tempBuf));
			bufIndex = 0;
			bufferIndex++;
			continue;
		}
		else
		{
			tempBuf[bufIndex] = ch;
			bufIndex++;
		}
		bufferIndex++;
	}

	printf("Online CPUs: %s", buf);
	memset(buf, 0, sizeof(buf));
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
	memset(buf, 0, sizeof(buf));

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
