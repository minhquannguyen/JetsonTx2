#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <ctype.h>
#include "tegra_clocks.h"

void
parse_args
(
    char **cmd_line,
    int *status
)
{
    while (*cmd_line)
    {
        cmd_line++;
    }
}

Status
get_num_cpus
(
    int *totalCpus
)
{
    FILE    *pFile;
    char    readBuf[BUF_SIZE];
    Status  status = SUCCESS;
    int     cpuCount = 0;

    pFile = fopen("/proc/cpuinfo", "r");
    if (!pFile)
    {
        printf("%s: Error opening file\n", __FUNCTION__);
        status = FILE_HANDLING_ERROR;
        goto end;
    }

    while (fscanf(pFile, "%s", readBuf) != EOF)
    {
        if (strstr(readBuf, "processor"))
        {
            cpuCount++;
        }
    }

    *totalCpus = cpuCount;
    fclose(pFile);

end:
    return status;
}

Status
get_cpu_clocks
(
    int gpuID,
    char buf[]
)
{
    FILE *cpuFile;
    char cpuDirBuffer[BUF_SIZE];
    Status status = SUCCESS;

    if (snprintf(cpuDirBuffer, sizeof(cpuDirBuffer),
        SYSTEM_CPU_DIR "/cpu%d/cpufreq/scaling_cur_freq",
        gpuID) < 0)
    {
        status = INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    cpuFile = fopen(cpuDirBuffer, "r");
    if (!cpuFile)
    {
        printf("%s: Error opening file\n", __FUNCTION__);
        status = FILE_HANDLING_ERROR;
        goto end;
    }

    // copy cpu clock value back into buf
    if (fgets(buf, BUF_SIZE, cpuFile) == NULL)
    {
        printf("%s: Error reading file\n", __FUNCTION__);
        status = FILE_HANDLING_ERROR;
        goto cleanup;
    }

cleanup:
    fclose(cpuFile);
end:
    return status;
}

Status
query_cpu_stats()
{
    char   readBuf[BUF_SIZE];
    char   tempBuf[BUF_SIZE];
    FILE   *fp_online, *fp_offline;
    int    *onlineCpus = NULL;
    int    readBufIndex = 0, onlineCpuIndex = 0, tempBufIndex = 0, numCpus;
    Status status = SUCCESS;

    memset(readBuf, 0, sizeof(readBuf));

    status = get_num_cpus(&numCpus);
    if (status != SUCCESS)
    {
        printf("%s: Error getting number of CPUs status = \n",
                __FUNCTION__, status);
        return status;
    }

    onlineCpus = malloc(sizeof(int) * numCpus);
    if (!onlineCpus)
    {
        printf("%s: Error allocating memory\n", __FUNCTION__);
        return INSUFFICIENT_RESOURCES;
    }

    fp_online = fopen(SYSTEM_CPU_DIR "/online", "r");
    if (!fp_online)
    {
        printf("%s: Error opening file\n", __FUNCTION__);
        return FILE_HANDLING_ERROR;
    }

    // Output the online Cpus
    if (fgets(readBuf, sizeof(readBuf), fp_online) == NULL)
    {
        printf("%s: Error reading file\n", __FUNCTION__);
        fclose(fp_online);
        return FILE_HANDLING_ERROR;
    }

    memset(tempBuf, 0, sizeof(tempBuf));
    fclose(fp_online);

    // Really hacky, should find a better way
    while (1)
    {
        char ch = readBuf[readBufIndex];
        if (ch == '\0')
        {
            onlineCpus[onlineCpuIndex] = atoi(tempBuf);
            break;
        }
        else if (ch == '-')
        {
            onlineCpus[onlineCpuIndex] = atoi(tempBuf);
            tempBufIndex = 0;
            readBufIndex++;
            memset(tempBuf, 0, sizeof(tempBuf));

            // get the full numeric value into the buffer
            for (ch = readBuf[readBufIndex]; ch != ',' && ch != '\0' && ch != '\n';
                    ch = readBuf[++readBufIndex])
            {
                tempBuf[tempBufIndex] = ch;
                tempBufIndex++;
            }

            while (onlineCpus[onlineCpuIndex] <= atoi(tempBuf))
            {
                onlineCpuIndex++;
                onlineCpus[onlineCpuIndex] =
                    onlineCpus[onlineCpuIndex-1] + 1;
            }

            tempBufIndex = 0;
            memset(tempBuf, 0, sizeof(tempBuf));
        }
        else if (ch == ',')
        {
            onlineCpus[onlineCpuIndex] = atoi(tempBuf);
            onlineCpuIndex++;
            memset(tempBuf, 0, sizeof(tempBuf));
            tempBufIndex = 0;
            readBufIndex++;
            continue;
        }
        else
        {
            tempBuf[tempBufIndex] = ch;
            tempBufIndex++;
        }
        readBufIndex++;
    }

    printf("Online CPUs: %s", readBuf);
    memset(readBuf, 0, sizeof(readBuf));

    // Output the clock values of the Cpus
    for (int i = 0; i < onlineCpuIndex; i++)
    {
        get_cpu_clocks(onlineCpus[i], readBuf);
        printf("Cpu%d running at %s Khz\n", onlineCpus[i], strtok(readBuf, "\n"));
    }

    fp_offline = fopen(SYSTEM_CPU_DIR "/offline", "r");
    memset(readBuf, 0, sizeof(readBuf));

    if (!fp_offline)
    {
        printf("%s: Error opening file\n", __FUNCTION__);
        return FILE_HANDLING_ERROR;
    }

    // Output the offline Cpus
    if (fgets(readBuf, sizeof(readBuf), fp_offline) == NULL)
    {
        printf("%s: Error reading file\n", __FUNCTION__);
        fclose(fp_offline );
        return FILE_HANDLING_ERROR;
    }

    printf("\nOffline CPUs: %s", readBuf);
    fclose(fp_offline);
    free(onlineCpus);

    return SUCCESS;
}

// Testing
int main(int argc, char **argv)
{
    Status status;
    //if (argc > 1)
    //{
    //  parse_args(&argv[0], &status);
    //}
    status = query_cpu_stats();

    return 0;
}
