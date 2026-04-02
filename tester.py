import os
import subprocess
import time

# Configuration
EXECUTABLE = "./mainV4"  # Use "mainV4.exe" on Windows
EXAMPLE_FOLDER = "./Example"
TIMEOUT_LIMIT = 10.0

def run_test(file_path):
    file_name = os.path.basename(file_path)
    start_time = time.time()
    
    try:
        # Run the C++ program
        # stdout is captured, stderr is redirected to DEVNULL to hide execution time print from C++
        process = subprocess.run(
            [EXECUTABLE, file_path],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_LIMIT
        )
        
        elapsed = time.time() - start_time
        
        # Parse the output
        output = process.stdout
        result_str = "-1"
        min_plants = "-1"
        
        for line in output.split('\n'):
            if "Final Result :" in line:
                result_str = line.split(":")[1].strip()
            if "Number of Plants :" in line:
                min_plants = line.split(":")[1].strip()
        
        print(f"{file_name:<20} | Result: {result_str:<10} | Plants: {min_plants:<3} | Time: {elapsed:.4f}s")

    except subprocess.TimeoutExpired:
        print(f"{file_name:<20} | Result: -1         | Plants: -1  | Time: >10s (TIMEOUT)")
    except Exception as e:
        print(f"{file_name:<20} | Error: {str(e)}")

def main():
    if not os.path.exists(EXAMPLE_FOLDER):
        print(f"Error: Folder {EXAMPLE_FOLDER} not found.")
        return

    # Filter for files (ignoring directories)
    files = [f for f in os.listdir(EXAMPLE_FOLDER) if os.path.isfile(os.path.join(EXAMPLE_FOLDER, f))]
    files.sort()

    print(f"{'File Name':<20} | {'Result':<10} | {'Plants':<7} | {'Exec Time'}")
    print("-" * 65)

    for file in files:
        run_test(os.path.join(EXAMPLE_FOLDER, file))

if __name__ == "__main__":
    main()