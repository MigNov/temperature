//#define DEBUG_AMD

#include "sensors.h"

#ifdef HAVE_PCI
#ifdef DEBUG_AMD
#define DPRINTF(fmt, ...) \
do { fprintf(stderr, "sensors [AMD]: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
do {} while(0)
#endif

int iPCINumHBDevs;
struct pci_dev **sPCIDevs;

int iGetAMDTempK10(struct pci_dev **devs, int numDevs, int average)
{
  struct pci_dev *p;
  long val, maxValue, sumVal;
  int i;

  maxValue = 0;
  sumVal = 0;
  val = 0;

  for (i = 0; i < numDevs; i++) {
    p = devs[i];
    val = pci_read_long(devs[i], AMD_PCI_NORTHBRIDGE_TEMP_REG);
    if (val > 0) {
        val = ((val >> 21) * 125) / 1000;
        DPRINTF("Read temperature from %02x:%02x.%02x: %d\n", p->bus, p->dev,
                 p->func, (int)val);
        if (!average && maxValue < val)
            maxValue = val;
        if (average)
            sumVal += val;
    }
  }

  return average ? (sumVal / numDevs) : maxValue;
}

int iAMDGetTempK10(int average)
{
  sPCIDevs = pci_getDeviceList(&iPCINumHBDevs, AMD_PCI_REQUIRED_CLASS,
                               AMD_PCI_VENDOR_ID, AMD_PCI_REQUIRED_DEVFN);
  return iPCINumHBDevs == 0 ? -1 : iGetAMDTempK10(sPCIDevs, iPCINumHBDevs, average);
}
#else
int iGetAMDTempK10(struct pci_dev **devs, int numDevs, int average) { return 0; }
int iAMDGetTempK10(int average) { return 0; }
#endif
