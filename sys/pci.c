#include <sys/inout.h>
#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/ahci_rw.h>
#include <sys/rebase.h>
#include <sys/memset.h>

#define AHCI_DEV_NULL           0
#define HBA_PORT_DET_PRESENT    0x3
#define HBA_PORT_IPM_ACTIVE     0x1
#define	SATA_SIG_ATA	        0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	        0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	        0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	            0x96690101	// Port multiplier

uint16_t vendorID, deviceID;
uint64_t bar_addr = 0;
void *buff_addr = (uint16_t *)0x1000000;

//extern void *memset(void *s, int c, size_t n);
 
// Check device type
static int check_type(hba_port_t *port)
{
	uint32_t ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT) {	// Check drive status
		return AHCI_DEV_NULL;
    }
	if (ipm != HBA_PORT_IPM_ACTIVE) {
		return AHCI_DEV_NULL;
    }
 
	switch (port->sig) {
	    case SATA_SIG_ATAPI : return AHCI_DEV_SATAPI;
	    case SATA_SIG_SEMB  : return AHCI_DEV_SEMB;
	    case SATA_SIG_PM    :return AHCI_DEV_PM;
	    default             :return AHCI_DEV_SATA;
	}
}

void probe_port(hba_mem_t *abar_tmp)
{
	// Search disk in impelemented ports
	uint32_t pi = abar_tmp->pi;
    //kprintf("pi:%d\n",pi);
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar_tmp->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				kprintf("SATA drive found at port %d\n", i);
                port_rebase(abar_tmp, abar_tmp->ports,i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				kprintf("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				kprintf("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				kprintf("PM drive found at port %d\n", i);
			}
			else
			{
				//kprintf("No drive found at port %d\n", i);
			}
		}
 
		pi >>= 1;
		i++;
	}
}
uint32_t pci_config_readword (uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    tmp = (uint32_t)(inl(0xCFC));
	return (tmp);
}

uint32_t pci_config_readbar (uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;
    
    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    outl(0xCFC, 0x3ffff800); //- 27th bit to 29th bit working
    tmp = (uint32_t)(inl(0xCFC));
    return (tmp);
}


int get_ACI_controller(uint8_t *bus, uint8_t *device) {
    uint32_t config_space = 0;
    for(*bus = 0; *bus < 256; (*bus)++) {
        for(*device = 0; *device < 32; (*device)++) {
            config_space = pci_config_readword(*bus, *device, 0, 0x00);
            vendorID = config_space & 0x0000FFFF;
            config_space = config_space & 0xFFFF0000;
            deviceID = config_space >> 16;
            if (vendorID == 0x8086 && deviceID == 0x2922) {
                kprintf("AHCI found!!!");
                return 0;
            }
        }
    }
    return -1;
}

void check_all_buses(void) {
    uint32_t config_space = 0;
    uint64_t bar_addr = 0;
    uint16_t class = 0;
    uint8_t sub_class, base_class = 0;
    uint8_t bus = 0, device = 0;
    int flag = 0;
    int sector_no = 0;
    int i=0;
    hba_mem_t *abar = NULL;

    //NO need to check vendor id and device id, directly check for class and subclass.
    /*ret = get_ACI_controller(&bus, &device);
    if (ret == -1) {
        kprintf("\nNo ACI controller found!!!");
        return;
    }*/
    for (bus = 0; bus < 256 && flag!=1; bus++) {
        for(device = 0; device < 32 && flag!=1; device++) {
            for (i = 0; i <8 ; i++) {
                config_space = pci_config_readword(bus, device, i, 0x08);
                class = (config_space & 0xFFFF0000) >> 16;
                sub_class = class & 0x00FF;
                base_class = class >> 8;
                if (sub_class == 0x06 && base_class == 0x01) {
                    kprintf("\nSerial ATA found at bus :%d device :%d func :%d\n", bus, device, i);
                    flag = 1;
                    break;
                }
            }
        }
    }

    config_space = pci_config_readbar(bus-1, device-1, i, 0x24);
    bar_addr = 0xffffffff00000000 + (uint64_t)config_space;
    //kprintf("\nBAR 5 addr :%x\n", bar_addr);
    probe_port((hba_mem_t *)bar_addr);

    abar = (hba_mem_t *)bar_addr;
    //kprintf("\nbuff addr :%p", buff_addr);
    sector_no = 8; //Don't start from 0 to make gdb work.
    for (int i = 0; i<100; i++) {
        int no = 0 + i;
        memset(buff_addr, no, 4096);
        write_ahci(abar, &(abar->ports[0]), sector_no, 0, 4096/512, buff_addr);
        sector_no += (4096/512);
    }
    kprintf("\nData written successfully");
    memset(buff_addr, 0, 4096);
    sector_no = 8;
    for (i =0 ; i<10; i++) {
        memset(buff_addr, 0, 4096);
        read_ahci(abar, &abar->ports[0], sector_no, 0, 4096/512, buff_addr);
        kprintf("\ndata read at sector :%d is %d", sector_no, *((uint16_t *)buff_addr));
        sector_no += 4096/512;
    }
}

