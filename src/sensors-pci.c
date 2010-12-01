//#define DEBUG_PCI

#include "sensors.h"

#ifdef HAVE_PCI
#ifdef DEBUG_PCI
#define DPRINTF(fmt, ...) \
do { fprintf(stderr, "sensors [PCI]: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
do {} while(0)
#endif

struct pci_access *sPCIAccess = NULL;
struct pci_dev **sPCIDevs = NULL;
int pciInitDone = 0;

void PCIInit()
{
  sPCIAccess = pci_alloc();
  pci_init(sPCIAccess);
}

void PCIDestroy()
{
  free(sPCIDevs);
  pci_cleanup(sPCIAccess);
}

struct pci_dev **pci_getDeviceList(int *numDevices, int class, int vendor_id, int func)
{
  struct pci_dev *p, **plist;
  int iPCINumDevs;

  if (!pciInitDone) {
    PCIInit();
    pciInitDone = 1;
  }

  iPCINumDevs = 0;
  plist = (struct pci_dev **)malloc( sizeof(struct pci_dev **) );
  pci_scan_bus( sPCIAccess );
  for ( p = sPCIAccess->devices; p != NULL; p = p->next ) {
    if ((p->device_class == class)
         && (p->vendor_id == vendor_id)
         && (p->func == func)) {
             DPRINTF("Adding PCI device: %02x:%02x.%02x\n", p->bus, p->dev, p->func);
             iPCINumDevs++;
             plist = (struct pci_dev **)realloc(plist, iPCINumDevs * sizeof(struct pci_dev **));
             plist[iPCINumDevs-1] = p;
    }
  }

  if (numDevices != NULL)
     *numDevices = iPCINumDevs;

  return plist;
}

struct pci_dev *pci_getDevice(int vendor_id, int device_id, int func)
{
  struct pci_dev *p;

  if (!pciInitDone) {
    PCIInit();
    pciInitDone = 1;
  }

  pci_scan_bus( sPCIAccess );
  for ( p = sPCIAccess->devices; p != NULL; p = p->next ) {
    if ((p->device_id == device_id)
         && (p->vendor_id == vendor_id)
         && (p->func == func)) {
             DPRINTF("Adding PCI device: %02x:%02x.%02x\n", p->bus, p->dev, p->func);
             return p;
    }
  }

  return NULL;
}

struct pci_dev **pci_getDevices(int *numDevices, int vendor_id, int device_id, int func)
{
	struct pci_dev *p, **plist = NULL;
	int iPCINumDevs;

	if (!pciInitDone) {
		PCIInit();
		pciInitDone = 1;
	}

	pci_scan_bus( sPCIAccess );
	iPCINumDevs = 0;
	for ( p = sPCIAccess->devices; p != NULL; p = p->next ) {
		if ((p->device_id == device_id)
		&& (p->vendor_id == vendor_id)
		&& (p->func == func)) {
			DPRINTF("Adding PCI device: %02x:%02x.%02x\n", p->bus, p->dev, p->func);
			iPCINumDevs++;
			plist = (struct pci_dev **)realloc(plist, iPCINumDevs * sizeof(struct pci_dev **));
			plist[iPCINumDevs-1] = p;
		}
	}

	if (numDevices != NULL)
		*numDevices = iPCINumDevs;

	return plist;
}

void pci_free_device(struct pci_dev *p)
{
	pci_free_dev(p);
}

void pci_dcleanup()
{
	if (sPCIAccess != NULL)
		pci_cleanup( sPCIAccess );
	pciInitDone = 0;
}
#endif
