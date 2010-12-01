/* Uncomment to enable debugging messages from all source files */
//#define DEBUG_ALL

#ifndef SENSORS_H
#define SENSORS_H
#define _XOPEN_SOURCE 600

#ifdef HAVE_CONFIG_H
#include "config.h"
#define HAVE_PCI	HAVE_LIBPCI
#endif

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#ifdef HAVE_PCI
#include <pci/pci.h>
#endif
#include <time.h>

#ifdef DEBUG_ALL
#define DEBUG
#define DEBUG_AMD
#define DEBUG_COMMON
#define DEBUG_INTEL
#define DEBUG_PCI
#define DEBUG_XML
#endif

/* MSR related stuff */
#define EMSROK                  0
#define EMSRNOCPU               1000
#define EMSRNOSUP               1001
#define EMSRNODATA              1002
#define EMSRNOPERM              1003
#define EMSRUERROR              1099

/* Intel related stuff */
#define IA32_THERM_STATUS       0x19c
#define IA32_PLATFORM_ID        0x17
#define IA32_TEMPERATURE_TARGET 0x1a2

/* AMD related PCI stuff */
#define AMD_PCI_VENDOR_ID               0x1022
#define AMD_PCI_NORTHBRIDGE_TEMP_REG    0xa4
#define AMD_PCI_REQUIRED_CLASS          PCI_CLASS_BRIDGE_HOST
#define AMD_PCI_REQUIRED_DEVFN          3

/* Variable types */
#define VTYPE_UINT64_T		1
#define VTYPE_FLAG		2
#define VTYPE_INT		4

/* Structs */
typedef struct {
        char *name;
        char *value;
        int type;
} tVariables;

typedef struct {
        char *function;
	int iValue;
	uint64_t uValue;
	int type;
	int subval;
} tFunctionResult;

typedef struct {
	char *type;
	char *value;
	char *name;
} tXmlSubst;

typedef struct {
	char *name;
	struct pci_dev *dev;
	int valid;
} tPCIDevices;

/* Variables */
char *cpuVendor;
tVariables *vars;
tFunctionResult *gFuncResults;
int gFuncResCnt;
tPCIDevices *gPCIDevs;
int gPCIDevCnt;
tVariables *gVars;
int gNumVars;
int actionRun;
time_t lastTime;

/* Function prototypes */
/* Common */
int CPUGetModel(int *bIntel);
int CPUGetCount();
char *CPUGetVendor(int allowSubst);
int MSRRead(int cpu, uint32_t reg, uint64_t *value);
int CPUIDRead(unsigned int ax, unsigned int *p);
char *MSRGetError(int err);
/* PCI */
struct pci_dev **pci_getDeviceList(int *numDevices, int class, int vendor_id, int func);
struct pci_dev **pci_getDevices(int *numDevices, int vendor_id, int device_id, int func);
struct pci_dev *pci_getDevice(int vendor_id, int device_id, int func);
void pci_free_device(struct pci_dev *p);
void pci_dcleanup();
/* CPU Types getTemp() prototypes */
int iAMDGetTempK10(int average);
int iIntelGetTemp(int average);
/* XML stuff */
int processXml(char *xmlFile);
#endif
