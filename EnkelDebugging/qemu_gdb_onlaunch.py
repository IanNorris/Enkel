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

def MainScript():
    print("Enkel gdb scripts starting...")
    gdb.execute("interrupt")

    print("Setting GIsDebuggerPresent")
    gdb.parse_and_eval("GIsDebuggerPresent = true")

    print("Setting debugger hook")
    DebuggerHook("DebuggerHook")

    print("Setting halt hook")
    DebuggerHook("HaltPermanently")
    
    print("Setting hooks")
    AsmBreakpointWithError("DebugHook_ISR_GeneralProtectionFault")
    AsmBreakpoint("DebugHook_ISR_InvalidOpcode")
    AsmBreakpoint("DebugHook_ISR_Breakpoint")

    # Any page fault right now is bad news because we can't handle them
    print("Setting ISR PF hook")
    AsmBreakpointWithError("DebugHook_ISR_PageFault")
    
    print("Setup complete!")
    gdb.execute("continue")
    
    print("Continued!")

gdb.execute("set logging overwrite on")
gdb.execute("set logging enabled")
gdb.execute("set output-radix 16")
gdb.execute("add-symbol-file ~/Enkel/boot_iso/boot_part/kernel/enkel.elf")
OnKernelMainHook("OnKernelMainHook", temporary=True)
