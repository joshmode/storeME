import subprocess
import random
import string
import os

DB_PATH = os.path.abspath("./db")

def run_command(input_str):
    # ensure db exists
    if not os.path.exists(DB_PATH):
        raise FileNotFoundError(f"database executable not found at {DB_PATH}")

    process = subprocess.Popen(
        [DB_PATH],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    stdout, stderr = process.communicate(input_str)
    return stdout, stderr, process.returncode

def test_fuzz():
    print("starting fuzz test...")
    if os.path.exists("wal.log"):
        os.remove("wal.log")

    # compile
    print("compiling...")
    subprocess.run(["make"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    commands = []

    # generate random commands
    print("generating random commands...")
    for _ in range(1000):
        op = random.choice(["SET", "GET", "DELETE"])
        # keys restricted to no spaces
        key = ''.join(random.choices(string.ascii_letters + string.digits, k=random.randint(1, 10)))

        if op == "SET":
            value = ''.join(random.choices(string.ascii_letters + string.digits + " ", k=random.randint(1, 20)))
            commands.append(f"SET {key} {value}")
        elif op == "GET":
            commands.append(f"GET {key}")
        elif op == "DELETE":
            commands.append(f"DELETE {key}")

    commands.append("EXIT")
    input_str = "\n".join(commands)

    print("running database...")
    stdout, stderr, ret = run_command(input_str)

    if ret != 0:
        print("crash detected!")
        print("stderr:", stderr)
        exit(1)

    print("fuzz test passed.")

    verify_simple()

def verify_simple():
    print("running logic verification...")
    # clean start
    if os.path.exists("wal.log"):
        os.remove("wal.log")

    # phase 1: set data
    commands = [
        "SET user1 Alice",
        "SET user2 Bob Builder",
        "DELETE user1",
        "EXIT"
    ]
    run_command("\n".join(commands))

    # phase 2: verify persistence
    check_commands = [
        "GET user1", # should be (nil)
        "GET user2", # should be Bob Builder
        "EXIT"
    ]
    stdout, _, _ = run_command("\n".join(check_commands))

    if "(nil)" in stdout and "Bob Builder" in stdout:
        print("logic and persistence verification passed.")
    else:
        print("logic and persistence verification failed.")
        print("output received:\n", stdout)
        exit(1)

if __name__ == "__main__":
    test_fuzz()
