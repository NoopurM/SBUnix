#ifndef _PCI_H
#define _PCI_H

uint32_t pci_config_readword (uint16_t bus, uint16_t slot,uint16_t func, uint16_t offset);
void check_all_buses(void);
#endif
