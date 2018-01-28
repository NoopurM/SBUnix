#include <sys/ahci.h>
int find_cmdslot(hba_mem_t *abar, hba_port_t *port);
int read_ahci(hba_mem_t *abar, hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf);
//int read(hba_mem_t *abar, hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, char *buf);
int write_ahci(hba_mem_t *abar, hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf);
//int write(hba_mem_t *abar, hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, char *buf);

