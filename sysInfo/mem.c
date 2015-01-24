#include <stdio.h>
#include <sys/sysinfo.h>



sysinfo getSysInfo()
{
    struct sysinfo si;
    sysinfo(&si);
    printf("Totalram:       %d kb\n", si.totalram>>10);
    printf("Available:      %d kb\n", si.freeram>>10);
    return si;

}

int main()
{
    struct sysinfo si;
    sysinfo(&si);
    printf("Totalram:       %d kb\n", si.totalram>>10);
    printf("Available:      %d kb\n", si.freeram>>10);
    return 0;
}


/*
struct sysinfo {
    long uptime;             // Seconds since boot
    unsigned long loads[3];  // 1, 5, and 15 minute load averages
    unsigned long totalram;  // Total usable main memory size
    unsigned long freeram;   // Available memory size
    unsigned long sharedram; // Amount of shared memory
    unsigned long bufferram; // Memory used by buffers
    unsigned long totalswap; // Total swap space size
    unsigned long freeswap;  // swap space still available
    unsigned short procs;    // Number of current processes
    unsigned long totalhigh; // Total high memory size
    unsigned long freehigh;  // Available high memory size
    unsigned int mem_unit;   // Memory unit size in bytes
    char _f[20-2*sizeof(long)-sizeof(int)]; // Padding for libc5
    };
*/



