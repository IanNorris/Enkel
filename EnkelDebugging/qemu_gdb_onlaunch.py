import gdb

class AsmBreakpoint(gdb.Breakpoint):
    def __init__(self, functionName):
        self.functionName = functionName
        address = gdb.parse_and_eval("&{}".format(functionName))
        address = int(address.cast(gdb.lookup_type('int'))) # Magic incantation to get the address only
        super(AsmBreakpoint, self).__init__("*{}".format(address), gdb.BP_BREAKPOINT)

    def stop(self):
        print("Top of stack for {}:".format(self.functionName))
        gdb.execute("x/16gx $rsp")
        #gdb.execute("set $eip = $rsp")
        print("PAGE Stopped!")
        return True


class AsmBreakpointWithError(gdb.Breakpoint):
    def __init__(self, functionName):
        self.functionName = functionName
        address = gdb.parse_and_eval("&{}".format(functionName))
        address = int(address.cast(gdb.lookup_type('int'))) # Magic incantation to get the address only
        super(AsmBreakpointWithError, self).__init__("*{}".format(address), gdb.BP_BREAKPOINT)

    def stop(self):
        print("Top of stack for {}:".format(self.functionName))
        gdb.execute("info break")
        gdb.execute("x/16gx $rsp")
        #gdb.execute("set $eip = $rsp + 8")
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
    AsmBreakpointWithError("ISR_GeneralProtectionFault")
    AsmBreakpoint("ISR_InvalidOpcode")

    # Any page fault right now is bad news because we can't handle them
    print("Setting ISR PF hook")
    AsmBreakpointWithError("ISR_PageFault")

    gdb.execute("continue")

gdb.execute("set logging overwrite on")
gdb.execute("set logging on")
gdb.execute("set output-radix 16")
gdb.execute("add-symbol-file ~/Enkel/boot_iso/boot_part/kernel/enkel.elf")
OnKernelMainHook("OnKernelMainHook")
