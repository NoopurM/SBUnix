#include <sys/ahci.h>
#include <sys/rebase.h>
#include <sys/kprintf.h>
#include <sys/memset.h>
/*void *memset(void *s, uint16_t c, uint32_t n)
{
    uint16_t *p = (uint16_t *)s;
    while(n--)
        *p++ = c;
    return s;
}*/

// Start command engine
void start_cmd(hba_port_t *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(hba_port_t *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
}

#define	AHCI_BASE	0x400000	// 4M
 
void port_rebase(hba_mem_t *abar, hba_port_t *port, int portno)
{
    abar->ghc = (uint32_t)(1<<31);
    abar->ghc = (uint32_t)(1<<0);
    abar->ghc = (uint32_t)(1<<31);
    abar->ghc = (uint32_t)(1<<1);
    stop_cmd(port);	// Stop command engine
 
    port->clb = AHCI_BASE + (portno<<10); //1k byte aligned.
	port->clb = port->clb & 0x00000000ffffffff; // to reset upper 32 bits
	memset((void*)(port->clb), 0, 1024);
 
	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	//port->fbu = 0;
	port->fb = port->fb & 0x00000000ffffffff;
	memset((void*)(port->fb), 0, 256);
 
	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctba = cmdheader[i].ctba & 0x00000000ffffffff;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
 
	start_cmd(port);	// Start command engine
}
