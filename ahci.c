#include "ahci.h"
#include "std.h"
#include "vm.h"
#include "print.h"

int is_known_ahci(uint16_t vendor_id, uint16_t device_id);
// Function to construct PCI address
uint32_t pci_config_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	return (1 << 31) | ((bus & 0xFF) << 16) | ((slot & 0x1F) << 11) | ((func & 0x07) << 8) | (offset & 0xFC); // Offset aligned to 4 bytes
}

// Function to read a word from the PCI configuration space
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	outl(PCI_CONFIG_ADDRESS, pci_config_address(bus, slot, func, offset));
	return inw(PCI_CONFIG_DATA + (offset & 2));
}

// Function to read a 32-bit value from the PCI configuration space
uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	outl(PCI_CONFIG_ADDRESS, pci_config_address(bus, slot, func, offset));
	return inl(PCI_CONFIG_DATA);
}
uint32_t abar;

char str[8];
char str1[8];
char str2[8];
char t[32];
uint16_t device_id;
uint16_t class_code;
// Scan for AHCI controller and get ABAR
uint32_t *find_ahci_controller()
{
	uint8_t bus, slot, func = 0;
	asm volatile("cli");
	for (bus = 0; bus < 256; bus++)
	{

		for (slot = 0; slot < 32; slot++)
		{

			for (func = 0; func < 8; func++)
			{
				uint16_t vendor_id = pci_read_word(bus, slot, func, 0x00);
				// Device is present
				device_id = pci_read_word(bus, slot, func, 0x02);
				class_code = pci_read_word(bus, slot, func, 0x0A);

				uint8_t class_id = (class_code >> 8) & 0xFF;
				uint8_t subclass_id = class_code & 0xFF;

				int_to_string(bus, str, 10);
				int_to_string(slot, str1, 10);
				int_to_string(func, str2, 10);

				print(str);
				print(", ");
				print(str1);
				print(", ");
				print(str2);
				print("\n");
				// check if its an ahci controller (class ID 1, subclass ID 6)
				if (class_id == 0x01 && subclass_id == 0x06)
				{
					// check Vendor ID and Device ID for specific ahci

					// test
					if (is_known_ahci(vendor_id, device_id))
					{
						abar = pci_read_dword(bus, slot, func, 0x24); // BAR[5]

						// Check if BAR[5] is memory-mapped (bits [0-3] should be 0 for MMIO)
						if ((abar & 0x1) == 0)
						{
							abar &= 0xFFFFFFF0; // Mask to get the address

							uint32_t *ptr = abar;
							return ptr;
						}
					}
				}
			}
		}
	}

	return 0;
}
static int check_type(HBA_PORT *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT) // Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}
void probe_port(HBA_MEM *abar)
{
	char num[8];
	uint32_t pi = abar->pi;
	int i = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				int_to_string(i, num, 10);
				print("SATA drive found at port: ");
				print(num);
				print("\n");
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				int_to_string(i, num, 10);
				print("SATAPI drive found at port: ");
				print(num);
				print("\n");
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				int_to_string(i, num, 10);
				print("SEMB drive found at port: ");
				print(num);
				print("\n");
			}
			else if (dt == AHCI_DEV_PM)
			{
				int_to_string(i, num, 10);
				print("PM drive found at port: ");
				print(num);
				print("\n");
			}
			else
			{
				int_to_string(i, num, 10);
				print("No drive found at port: ");
				print(num);
				print("\n");
			}
		}

		pi >>= 1;
		i++;
	}
}

void start_cmd(HBA_PORT *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR)
		;

	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(HBA_PORT *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;

	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;

	// Wait until FR (bit14), CR (bit15) are cleared
	while (1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void port_rebase(HBA_PORT *port, int portno)
{
	stop_cmd(port); // Stop command engine

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno << 10);
	port->clbu = 0;
	memset((void *)(port->clb), 0, 1024);

	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
	port->fbu = 0;
	memset((void *)(port->fb), 0, 256);

	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)(port->clb);
	int i;

	for (i = 0; i < 32; i++)
	{
		cmdheader[i].prdtl = 8; // 8 prdt entries per command table
								// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40 << 10) + (portno << 13) + (i << 8);
		cmdheader[i].ctbau = 0;
		memset((void *)cmdheader[i].ctba, 0, 256);
	}

	start_cmd(port); // Start command engine
}
int find_cmdslot(HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = (port->sact | port->ci);
	int i;
	for (i = 0; i < 32; i++)
	{
		if ((slots & 1) == 0)
			return i;
		slots >>= 1;
	}
	print("Cannot find free command list entry\n");
	return -1;
}

int read_ahci(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
	port->is = (uint32_t)-1; // Clear pending interrupt bits
	int spin = 0;			 // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 0;
	// down
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)port->clb;
	cmdheader += slot;

	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
	cmdheader->w = 0;										 // Read from device
	cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;	 // PRDT entries count

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *)(cmdheader->ctba);

	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

	// up
	//  8K bytes (16 sectors) per PRDT
	int i;
	for (i = 0; i < cmdheader->prdtl - 1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)virt_to_phys(buf);
		cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4 * 1024; // 4K words
		count -= 16;	 // 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)virt_to_phys(buf);
	cmdtbl->prdt_entry[i].dbc = (count << 9) - 1; // 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1; // Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6; // LBA mode

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// up

	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{

		spin++;
	}
	if (spin == 1000000)
	{
		print("Port is hung\n");
		return 0;
	}

	port->ci = 1 << slot;

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0)
		{
			break;
		}
		if (port->is & (1 << 12)) // Task file error
		{
			print("read disk error");

			return 0;
		}
	}

	// Check again
	if (port->is & (1 << 12))
	{
		print("Read disk error\n");
		return 0;
	}

	return 1;
}
int write_ahci(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
	port->is = (uint32_t)-1; // Clear pending interrupt bits
	int spin = 0;			 // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 0;
	// down
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
	cmdheader->w = 1;										 // Read from device
	cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;	 // PRDT entries count

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *)(cmdheader->ctba);

	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

	// up
	//  8K bytes (16 sectors) per PRDT
	int i;
	for (i = 0; i < cmdheader->prdtl - 1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)virt_to_phys(buf);
		cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4 * 1024; // 4K words
		count -= 16;	 // 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)virt_to_phys(buf);
	cmdtbl->prdt_entry[i].dbc = (count << 9) - 1; // 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1; // Command
	cmdfis->command = ATA_CMD_WRITE_DMA_EX;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6; // LBA mode

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;
	// up

	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{

		spin++;
	}
	if (spin == 1000000)
	{
		print("Port is hung\n");
		return 0;
	}

	port->ci = 1 << slot;

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0)
		{
			break;
		}
		if (port->is & (1 << 12)) // Task file error
		{
			print("write disk error");

			return 0;
		}
	}

	// Check again
	if (port->is & (1 << 12))
	{
		print("write disk error\n");
		return 0;
	}

	return 1;
}
// Find a free command list slot

// Start command engine

// Function to verify known AHCI-compatible Vendor and Device IDs
int is_known_ahci(uint16_t vendor_id, uint16_t device_id)
{
	// Example: You may add more known Vendor/Device IDs here
	if ((vendor_id == 0x8086 && (device_id == 0x2922 || device_id == 0x2923)) ||
		(vendor_id == 0x1022 && device_id == 0x7801))
	{
		return 1; // AHCI device found
	}
	return 0;
}
