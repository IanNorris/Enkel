#!/usr/bin/python3

import os
import sys
import subprocess
import threading

should_log = False
root_dir = ""

def runAndLogCommand(gdb_process, log_file, command):
    global should_log
    if "/mnt//INVALID_CWD" in command:
        log_file.write('# Before:' + command + '\n')
        command = command.replace("/mnt//INVALID_CWD", root_dir)
        log_file.write('# After:' + command + '\n')
    gdb_process.stdin.write(command + '\n')
    gdb_process.stdin.flush()

    # Log the response
    if should_log:
        log_file.write(f"< {command}\n")
        log_file.flush()

def handle_gdb_output(gdb_process, log_file, initial_commands):
    global should_log
    
    sent_initial = False
    binaryLoaded = False
    while True:
        output = gdb_process.stdout.readline()

        if not output:
            break

        # Print the output to stdout
        sys.stdout.write(output)
        sys.stdout.flush()

        # Log the response
        if should_log:
            log_file.write(f"< {output.strip()}\n")
            log_file.flush()

        if not sent_initial and "(gdb)" in output:
            sent_initial = True
            for command in initial_commands:
                runAndLogCommand(gdb_process, log_file, command)
        
        if "stopped,reason=\"breakpoint-hit" in output and "OnBinaryLoadHook_Inner" in output:
            binaryLoaded = True

        if "(gdb)" in output and binaryLoaded:
            runAndLogCommand(gdb_process, log_file, '-exec-continue')
            binaryLoaded = False

def main():
    global root_dir
    
    # Get the command line arguments for GDB
    gdb_args = sys.argv[1:]
    root_dir = os.path.expandvars("$HOME/Enkel/")
    
    # Open the log file
    with open("/tmp/vs_gdb_log.txt", "w+") as log_file:
        log_file.write('# ROOT:' + root_dir + '\n')

        qemu_process = subprocess.Popen(
            ["sh", "run.sh"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            cwd=root_dir
        )

        while qemu_process.poll() is None:
            # Start the GDB subprocess
            gdb_process = subprocess.Popen(
                ["gdb"] + gdb_args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                universal_newlines=True,
                cwd=root_dir
            )

            initial_commands = [
                '100-gdb-set mi-async on',
                '102-target-select remote localhost:5000',
                '101-interpreter-exec console "python exec(open(\'tools/qemu_gdb_onlaunch.py\').read())"'
            ]

            # Start a separate thread to handle GDB's output
            output_thread = threading.Thread(target=handle_gdb_output, args=(gdb_process, log_file, initial_commands))
            output_thread.start()

            try:
                while True:
                    # Read input from stdin
                    command = sys.stdin.readline()

                    if not command:
                        return

                    # Send the command to GDB
                    runAndLogCommand(gdb_process, log_file, command.strip())

            except KeyboardInterrupt:
                # Handle keyboard interrupt (Ctrl+C)
                break

            finally:
                # Terminate the GDB process
                gdb_process.terminate()
                gdb_process.wait()

            # Wait for the output thread to finish
            output_thread.join()
    
    qemu_process.wait()

if __name__ == "__main__":
    main()
