import gdb

class OnKernelMainHook(gdb.Breakpoint):
    def stop (self):
        print("Debugger attached")
        gdb.parse_and_eval("GIsDebuggerPresent = true")
        return False

OnKernelMainHook("OnKernelMainHook")
