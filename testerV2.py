import os
import subprocess
import time

# Configuration
EXECUTABLE = "./mainV4"
EXAMPLE_FOLDER = "./Example"
TIMEOUT_LIMIT = 10.0

# Ground Truth Dictionary (Consolidated from your images)
OPTIMAL_VALUES = {
    "grid-6-7": 2, "grid-9-12": 3, "grid-12-17": 4, "grid-16-24": 4, "grid-20-31": 6,
    "grid-25-40": 7, "grid-30-49": 8, "grid-40-67": 11, "grid-49-84": 12,
    "grid-56-97": 14, "grid-60-104": 16, "grid-72-127": 20, "grid-81-144": 21, "grid-100-180": 27,
    "rand-5-7": 1, "rand-10-15": 3, "rand-15-28": 4, "rand-20-40": 5,
    "rand-25-50": 5, "rand-30-60": 7, "rand-30-120": 5, "rand-35-140": 6,
    "rand-40-80": 9, "rand-40-160": 6, "rand-45-180": 7, "rand-50-200": 7,
    "rand-60-250": 9, "rand-70-300": 10, "rand-80-350": 11,
    "ring-5-5": 2, "ring-10-10": 4, "ring-15-15": 5, "ring-20-20": 7,
    "ring-25-25": 9, "ring-30-30": 10, "ring-35-35": 12, "ring-40-40": 14,
    "ring-50-50": 17, "ring-60-60": 20, "ring-75-75": 25, "ring-100-100": 34,
    "tree-5-4": 2, "tree-10-9": 5, "tree-15-14": 4, "tree-20-19": 7,
    "tree-25-24": 8, "tree-30-29": 11, "tree-35-34": 13, "tree-40-39": 16,
    "tree-50-49": 18, "tree-60-59": 24, "tree-75-74": 30, "tree-100-99": 36,
    "spec-1.dat": 2, "spec-2.dat": 2, "spec-3.dat": 2, "spec-4.dat": 1,
    "spec-5.dat": 3, "spec-6.dat": 3, "spec-7.dat": 3,
}

def run_test(file_path):
    file_name = os.path.basename(file_path)
    try:
        start_time = time.time()
        process = subprocess.run(
            [EXECUTABLE, file_path],
            capture_output=True, text=True, timeout=TIMEOUT_LIMIT
        )
        elapsed = time.time() - start_time
        
        found_plants = -1
        result_config = "N/A"
        
        # Parse the output line by line
        for line in process.stdout.split('\n'):
            if "Number of Plants :" in line:
                found_plants = int(line.split(":")[1].strip())
            if "Final Result :" in line:
                result_config = line.split(":")[1].strip()
        
        status = "UNKNOWN"
        if file_name in OPTIMAL_VALUES:
            expected = OPTIMAL_VALUES[file_name]
            # Check if optimal (or better)
            if found_plants != -1 and found_plants <= expected:
                status = "PASS"
            else:
                status = f"FAIL (Exp {expected})"
        
        # Format: Config is truncated if it's too long for the terminal
        display_config = result_config if len(result_config) <= 20 else result_config
        
        print(f"{file_name:<18} | P: {found_plants:<2} | {elapsed}s | {status:<10} | {display_config}")

    except subprocess.TimeoutExpired:
        print(f"{file_name:<18} | TIMEOUT")
    except Exception as e:
        print(f"{file_name:<18} | Error: {str(e)}")

def main():
    if not os.path.exists(EXAMPLE_FOLDER): 
        print(f"Error: {EXAMPLE_FOLDER} folder not found.")
        return
        
    files = sorted([f for f in os.listdir(EXAMPLE_FOLDER) if os.path.isfile(os.path.join(EXAMPLE_FOLDER, f))])
    
    print(f"{'File Name':<18} | {'P':<2} | {'Time':<6} | {'Status':<10} | {'Result String'}")
    print("-" * 80)
    for file in files:
        run_test(os.path.join(EXAMPLE_FOLDER, file))

if __name__ == "__main__":
    main()