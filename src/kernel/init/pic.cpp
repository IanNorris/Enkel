#include "kernel/init/apic.h"
#include "kernel/init/msr.h"

void RemapPIC()
{
    // Save masks
    uint8_t a1 = InPort(0xA1);
    uint8_t a21 = InPort(0x21);

    // Start initialization sequence
    OutPort(0x20, 0x11); // Initialize Master PIC
    OutPort(0xA0, 0x11); // Initialize Slave PIC

    // Remap IRQs
    OutPort(0x21, 0x20); // Map Master PIC's IRQs to 0x20-0x27
    OutPort(0xA1, 0x28); // Map Slave PIC's IRQs to 0x28-0x2F

    // Set up cascading
    OutPort(0x21, 0x04); // Tell Master PIC there is a slave PIC at IRQ2 (0000 0100)
    OutPort(0xA1, 0x02); // Tell Slave PIC its cascade identity (0000 0010)

    // Set PICs to 8086 mode
    OutPort(0x21, 0x01);
    OutPort(0xA1, 0x01);

    // Restore masks
    OutPort(0x21, a21);
    OutPort(0xA1, a1);
}

void EnableIRQ0()
{
    // Read the current IMR state
    uint8_t mask = InPort(0x21);

    // Clear the bit corresponding to IRQ0
    mask &= ~(1 << 0);

    // Write the new IMR state
    OutPort(0x21, mask);
}

void InitPIC()
{
	RemapPIC();

	EnableIRQ0();
}