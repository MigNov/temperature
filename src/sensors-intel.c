//#define DEBUG_INTEL

#include "sensors.h"

#ifdef DEBUG_INTEL
#define DPRINTF(fmt, ...) \
do { fprintf(stderr, "sensors [Intel]: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
do {} while(0)
#endif

/* Use -ENOENT when not set yet */
int bIntel = -ENOENT;
int iModel = -ENOENT;

int iGetTjunctionMax(int cpu)
{
    uint64_t uMSRTemp;
    int err, val;
    int platformId = -1;
    int use_ee = 0;

    if ((bIntel == -ENOENT) || (iModel == -ENOENT))
        iModel = CPUGetModel(&bIntel);

    if (!bIntel) {
        DPRINTF("Not running on Intel !\n");
        return -ENOSYS;
    }

    err = MSRRead(cpu, 0xee, &uMSRTemp);
    if (err == EMSROK) {
        use_ee = (uMSRTemp & 0x40000000);
        DPRINTF("Use_ee %s set\n", use_ee ? "is" : "is not");
    }

    err = MSRRead(cpu, IA32_PLATFORM_ID, &uMSRTemp);
    if (err == EMSROK) {
	DPRINTF("CPU #%d, MSR Platform ID = %" PRIx64 "\n", cpu, uMSRTemp);
        platformId = (uMSRTemp >> 50) & 0x7;
	DPRINTF("Platform id: %d\n", platformId);
    }

    err = MSRRead(cpu, IA32_TEMPERATURE_TARGET, &uMSRTemp);
    if (err == EMSROK) {
	DPRINTF("CPU #%d, MSR Temp target = %" PRIx64 "\n", cpu, uMSRTemp);
        val = (uMSRTemp >> 16) & 0xff;
	DPRINTF("Temp target value: %d\n", val);

        if ((val > 80) && (val < 120)) {
            DPRINTF("Read TjunctionMax from MSR = %d\n", val);
            return val;
        }
    }

    DPRINTF("Read TjunctionMax for model = 0x%02x, platformId = 0x%02x\n",
             iModel, platformId);

    /* This is borrowed from coretemp driver code but adapted to Celsius */
    if ((iModel == 0x17) &&
        ((platformId == 5) || (platformId == 7)))
        return use_ee ? 90 : 105;

    return use_ee ? 85 : 100;
}


int iGetTemperature(int cpu)
{
    int tjmax, value, err;
    uint64_t tempMSRVal;

    if (bIntel == -ENOENT)
        CPUGetModel(&bIntel);

    if (!bIntel) {
        DPRINTF("CPU Type is not Intel\n");
        return -ENOTSUP;
    }

    err = MSRRead(cpu, IA32_THERM_STATUS, &tempMSRVal);
    if (err < EMSROK) {
        DPRINTF("Cannot access required register on CPU %d: %s\n", cpu, MSRGetError(err));
        return -ENOTSUP;
    }

    tjmax = iGetTjunctionMax(cpu);
    DPRINTF("Using TjunctionMax value = %d\n", tjmax);

    value = tjmax - ((tempMSRVal >> 16) & 0x7F);

    return value;
}

int iIntelGetTemp(int average)
{
    int val, i, sumVal, maxCPU;
    int maxVal = 0, numVals = 0;

    maxCPU = CPUGetCount();
    if (maxCPU <= 0)
        return -ENOTSUP;

    sumVal = 0;
    for (i = 0; i <= maxCPU; i++) {
        val = iGetTemperature(i);
        if (val == -ENOTSUP)
            return -ENOTSUP;
        if (val > 0) {
            if (!average && (maxVal < val))
                maxVal = val;
            else if (average && (val > 0)) {
                sumVal += val;
                numVals++;
            }
            DPRINTF("CPU #%d temp: %d °C\n", i, val);
        }
    }

    if (val < 0)
        return -ENOTSUP;

    if (numVals == 0)
        numVals++;

    return average ? (sumVal / numVals) : maxVal;
}

