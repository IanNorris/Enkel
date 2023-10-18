import gdb

def GetFunctionAddress(functionName):
	address = gdb.parse_and_eval("&{}".format(functionName))
	address = int(address.cast(gdb.lookup_type('int'))) # Magic incantation to get the address only
	return address

class AsmBreakpoint(gdb.Breakpoint):
    def __init__(self, functionName):
        print("Creating breakpoint for asm for {}".format(functionName))
        self.functionName = functionName
        address = GetFunctionAddress(functionName)
        super(AsmBreakpoint, self).__init__("*{}".format(address), gdb.BP_BREAKPOINT)

    def stop(self):
        print("Breakpoint at {}".format(self.functionName))
        print("Top of stack:")
        gdb.execute("x/16gx $rsp")
        print("PAGE Stopped!")
        return True


class AsmBreakpointWithError(gdb.Breakpoint):
    def __init__(self, functionName):
        print("Creating breakpoint for asm with error for {}".format(functionName))
        self.functionName = functionName
        address = GetFunctionAddress(functionName)
        super(AsmBreakpointWithError, self).__init__("*{}".format(address), gdb.BP_BREAKPOINT)

    def stop(self):
        print("Breakpoint at {}".format(self.functionName))
        print("Top of stack")
        gdb.execute("x/16gx $rsp")
        print("PAGE Stopped!")
        return True

class DebuggerHook(gdb.Breakpoint):       
    def stop (self):
        print("DebuggerHook hit")
        return True

class OnKernelMainHook(gdb.Breakpoint):
    def stop (self):
        print("Debugger attached")
        MainScript()
        return False

def get_pml4_base():
    # Execute the "info registers cr3" command and capture its output
    output = gdb.execute("info registers cr3", to_string=True)

    # Extract the PDBR value from the output string
    pml4_base = int(output.split()[1], 16)

    return pml4_base

def translate_address(virtual_address):
    # Get the base address of the PML4 from the CR3 register
    pml4_base = get_pml4_base()

    # Calculate the address of the PML4 entry
    pml4_entry = pml4_base + ((virtual_address >> 39) & 0x1FF) * 8

    # Get the PML4 entry
    pml4_value = gdb.parse_and_eval("*((unsigned long long *)%s)" % pml4_entry)

    # Check if the PML4 entry is present
    if not pml4_value & 1:
        raise Exception("Address not mapped (PML4 entry not present)")

    # Print flags for PML4
    print_flags("PML4", pml4_value)

    # Get the base address of the PDPT from the PML4 entry
    pdpt_base = pml4_value & 0x7ffffffffffff000
    pt_entry_value = None

    # Repeat the process for the PDPT, PDT, and PT
    for level, shift in [("PDPT", 30), ("PDT", 21), ("PT", 12)]:
        entry_address = pdpt_base + ((virtual_address >> shift) & 0x1FF) * 8
        entry_value = gdb.parse_and_eval("*((unsigned long long *)%s)" % entry_address)

        # Print flags for the current level
        print_flags(level, entry_value)

        # Check if the entry is present
        if not entry_value & 1:
            raise Exception("Address not mapped (%s entry not present)" % level)

        pdpt_base = entry_value & 0x7ffffffffffff000

        if level == "PT":
            pt_entry_value = entry_value

    # Calculate the physical address
    physical_address = pdpt_base + (virtual_address & 0xFFF)

    return physical_address

def print_flags(level, entry_value):
    p = entry_value & 1
    rw = (entry_value >> 1) & 1
    us = (entry_value >> 2) & 1
    pwt = (entry_value >> 3) & 1
    pcd = (entry_value >> 4) & 1
    a = (entry_value >> 5) & 1
    d = (entry_value >> 6) & 1
    pat = (entry_value >> 7) & 1
    g = (entry_value >> 8) & 1
    nx = (entry_value >> 63) & 1
    print("%s value: %x, Present: %d, R/W: %d, User/Supervisor: %d, PageWriteThrough: %d, PageCacheDisable: %d, Accessed: %d, Dirty: %d, PageAttributeTable: %d, Global: %d, NoExecute: %d" % (level, entry_value, p, rw, us, pwt, pcd, a, d, pat, g, nx))


class TranslateCommand(gdb.Command):
    def __init__(self):
        super(TranslateCommand, self).__init__("translate_address", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        virtual_address = int(arg, 0)
        try:
            physical_address = translate_address(virtual_address)
            print("Virtual address 0x%x translates to physical address 0x%x" % (virtual_address, physical_address))
        except Exception as e:
            print(str(e))

def MainScript():
    print("Enkel gdb scripts starting...")
    gdb.execute("interrupt")

    print("Setting GIsDebuggerPresent")
    gdb.parse_and_eval("GIsDebuggerPresent = true")
    
    TranslateCommand()

    print("Setting debugger hook")
    DebuggerHook("DebuggerHook")

    print("Setting halt hook")
    DebuggerHook("HaltPermanently")
    
    print("Setting hooks")
    AsmBreakpointWithError("DebugHook_ISR_GeneralProtectionFault")
    AsmBreakpoint("DebugHook_ISR_InvalidOpcode")
    AsmBreakpoint("DebugHook_ISR_Breakpoint")

    #AsmBreakpointWithError("AccessViolationException")
    AsmBreakpointWithError("DebugHook_ISR_PageFault")
    
    print("Setup complete!")
    gdb.execute("continue")
    
    print("Continued!")

gdb.execute("set logging overwrite on")
gdb.execute("set logging enabled")
gdb.execute("set output-radix 16")
gdb.execute("set disassembly-flavor intel")
gdb.execute("set print asm-demangle on")
gdb.execute("add-symbol-file ~/Enkel/boot_iso/boot_part/kernel/enkel.elf")
OnKernelMainHook("OnKernelMainHook", temporary=True)
