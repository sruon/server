#!/usr/bin/python

import platform
import subprocess
import time
import signal
import threading
from queue import Queue, Empty
import datetime

TEN_MINUTES_IN_SECONDS = 600
CHECK_INTERVAL_SECONDS = 2.5

processes = []


def debug_log(message):
    """Print debug message with timestamp"""
    timestamp = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"[DEBUG {timestamp}] {message}")


def kill(process):
    """Send SIGTERM to a running process."""
    debug_log(f"Attempting to kill process {process.args[0]} (PID: {process.pid})")
    if process.poll() is None:  # still running
        debug_log(f"Process {process.args[0]} is still running, sending SIGTERM")
        process.send_signal(signal.SIGTERM)
    else:
        debug_log(f"Process {process.args[0]} already terminated with code {process.returncode}")


def kill_all():
    """Send SIGTERM to all running processes."""
    debug_log("Killing all processes...")
    for proc in processes:
        kill(proc)


def reader_thread(proc, output_queue):
    """
    Reads lines from proc.stdout and puts them into the output_queue
    along with a reference to the proc.

    When the process ends (stdout is closed), push a (proc, None)
    to indicate it's done.
    """
    debug_log(f"Reader thread started for {proc.args[0]} (PID: {proc.pid})")

    try:
        with proc.stdout:
            line_count = 0
            for line in proc.stdout:
                line_count += 1
                # 'line' already in string form since we use text=True
                output_queue.put((proc, line))

                # Debug every 10 lines to avoid spam
                if line_count % 10 == 0:
                    debug_log(f"Reader thread for {proc.args[0]} read {line_count} lines so far")

            debug_log(f"Reader thread for {proc.args[0]} finished reading stdout after {line_count} lines")

    except Exception as e:
        debug_log(f"Exception in reader thread for {proc.args[0]}: {e}")

    # Check process status before signaling end
    poll_result = proc.poll()
    debug_log(f"Process {proc.args[0]} poll result: {poll_result} (None=still running)")

    # Signal that this proc has ended
    debug_log(f"Reader thread signaling end for {proc.args[0]}")
    output_queue.put((proc, None))


def main():
    global processes
    print("Running exe startup checks...({})".format(platform.system()))

    # Start the processes
    debug_log("Starting processes...")
    processes = [
        subprocess.Popen(
            ["xi_connect", "--log", "connect-server.log"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        ),
        subprocess.Popen(
            ["xi_search", "--log", "search-server.log"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        ),
        subprocess.Popen(
            ["xi_map", "--ci", "--log", "map-server.log"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        ),
        subprocess.Popen(
            ["xi_world", "--log", "world-server.log"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        ),
    ]

    # Log process PIDs
    for proc in processes:
        debug_log(f"Started {proc.args[0]} with PID {proc.pid}")

    # Keep track of which processes have reported "ready to work"
    ready_status = {proc: False for proc in processes}

    # Sleep for a moment to give the processes time to start up
    time.sleep(1)

    # Check initial process status
    debug_log("Checking initial process status...")
    for proc in processes:
        poll_result = proc.poll()
        debug_log(f"{proc.args[0]} (PID: {proc.pid}) poll result: {poll_result}")

    # Create a queue to receive stdout lines from all processes
    output_queue = Queue()

    # Start a reading thread for each process
    threads = []
    for proc in processes:
        t = threading.Thread(
            target=reader_thread, args=(proc, output_queue), daemon=True
        )
        t.start()
        threads.append(t)
        debug_log(f"Started reader thread for {proc.args[0]}")

    print(
        f"Polling process output every {CHECK_INTERVAL_SECONDS}s for up to {TEN_MINUTES_IN_SECONDS}s..."
    )

    start_time = time.time()
    error_strs = ["error", "warning", "crash", "critical"]
    loop_count = 0

    while True:
        loop_count += 1

        # Debug every 10 loops to avoid spam
        if loop_count % 10 == 0:
            debug_log(f"Main loop iteration {loop_count}")
            # Check process status
            for proc in processes:
                poll_result = proc.poll()
                debug_log(f"{proc.args[0]} status: poll={poll_result}, ready={ready_status[proc]}")

        # If we've hit the timeout (10 minutes), fail
        if time.time() - start_time > TEN_MINUTES_IN_SECONDS:
            debug_log("Timeout reached")
            print("Timed out waiting for all processes to become ready.")
            kill_all()
            exit(-1)

        # Poll the queue for new lines
        # We'll keep pulling until it's empty (non-blocking)
        lines_processed = 0
        while True:
            try:
                proc, line = output_queue.get_nowait()
                lines_processed += 1
            except Empty:
                break  # No more lines at the moment

            # If line is None, that means this proc ended
            if line is None:
                debug_log(f"Received end signal for {proc.args[0]}")

                # CRITICAL DEBUG: Check if process is actually dead
                actual_poll = proc.poll()
                debug_log(f"Process {proc.args[0]} actual poll result: {actual_poll}")

                if actual_poll is None:
                    debug_log(f"WARNING: Process {proc.args[0]} is still running but reader thread thinks it ended!")
                    # Process is still running, this might be a false positive
                    # Continue without treating this as an error
                    continue

                # If the process ended but wasn't marked ready => error
                if not ready_status[proc]:
                    pid = proc.pid
                    return_code = proc.returncode
                    debug_log(f"ERROR: Process {proc.args[0]} actually exited with code {return_code}")
                    print(
                        f"ERROR: {proc.args[0]} (PID: {pid}) exited before it was 'ready to work' with code {return_code}."
                    )
                    kill_all()
                    exit(-1)
                else:
                    debug_log(f"Process {proc.args[0]} ended after being marked ready - this is OK")
            else:
                # We have an actual line of output
                line_str = line.strip()
                print(f"[{proc.args[0]}] {line_str}")

                # Check for error or warning text
                lower_line = line_str.lower()
                if any(x in lower_line for x in error_strs):
                    debug_log(f"Found error/warning in output from {proc.args[0]}: {line_str}")
                    print("^^^ Found error or warning in output.")
                    kill_all()
                    print("Killing all processes and exiting with error.")
                    exit(-1)

                # Check for "ready to work"
                if "ready to work" in lower_line:
                    debug_log(f"Process {proc.args[0]} reported ready to work!")
                    print(f"==> {proc.args[0]} is ready!")
                    ready_status[proc] = True
                    kill(proc)

        if lines_processed > 0:
            debug_log(f"Processed {lines_processed} lines from queue")

        # Check if all processes are marked ready
        if all(ready_status.values()):
            debug_log("All processes are ready!")
            print("All processes reached 'ready to work'! Exiting successfully.")
            kill_all()
            exit(0)

        # Sleep until next poll
        time.sleep(CHECK_INTERVAL_SECONDS)


if __name__ == "__main__":
    main()
