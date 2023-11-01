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

void SetIRQEnabled(uint8_t irq, bool enabled)
{
    if (irq > 15)
	{
		_ASSERTF(false, "Invalid IRQ");
        return;  // Invalid IRQ
    }

    uint16_t port = 0x21;  // PIC1 command port
    if (irq >= 8)
	{
        port = 0xA1;  // PIC2 command port
        irq -= 8;
    }

    uint8_t mask = InPort(port);

    if (enabled)
	{
        mask &= ~(1 << irq);
    }
	else
	{
        mask |= (1 << irq);
    }

    OutPort(port, mask);
}

void InitPIC()
{
	RemapPIC();

	SetIRQEnabled(0, true);
}

void DisablePIC()
{
	OutPort(0x21, 0xFF);
    OutPort(0xA1, 0xFF);
}
