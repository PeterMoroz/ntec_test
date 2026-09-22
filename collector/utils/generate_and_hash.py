import os
import json
import hashlib
import random
import string
import shutil
from pathlib import Path

# --- Configuration ---
BASE_DIR_NAME = "observed_directory"
OUTPUT_JSON = "baseline.json"

def generate_random_text(length=100):
    """Generates a random string of letters and numbers."""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def main():
    base_dir = Path(BASE_DIR_NAME)
    
    # Clean up previous runs to avoid mixing old and new files
    if base_dir.exists():
        shutil.rmtree(base_dir)
        
    # 1. Create the top-level directory
    base_dir.mkdir(parents=True)
    
    # 2. Create a few subdirectories (depth exactly 1) and random files
    num_subdirs = random.randint(3, 5)
    print(f"Creating {num_subdirs} subdirectories and random files...")
    
    for i in range(num_subdirs):
        subdir = base_dir / f"subdir_{i+1}"
        subdir.mkdir()
        
        num_files = random.randint(2, 4)
        for j in range(num_files):
            file_path = subdir / f"file_{j+1}.txt"
            file_path.write_text(generate_random_text())
            
    # 3. Traverse the directory and calculate SHA256 hashes
    hashes = {}
    
    # os.walk traverses the directory tree
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            file_path = Path(root) / file
            
            # Use relative paths for cleaner JSON keys (e.g., "subdir_1/file_1.txt")
            # .as_posix() ensures forward slashes are used even on Windows
            rel_path = file_path.relative_to(base_dir).as_posix()
            
            # Calculate SHA256
            sha256_hash = hashlib.sha256()
            with open(file_path, "rb") as f:
                # Read in chunks for memory efficiency
                for byte_block in iter(lambda: f.read(4096), b""):
                    sha256_hash.update(byte_block)
            
            hashes[rel_path] = sha256_hash.hexdigest()
            
    # 4. Generate the JSON file
    with open(OUTPUT_JSON, "w") as json_file:
        json.dump(hashes, json_file, indent=4)
        
    print(f"Successfully created directory structure in '{BASE_DIR_NAME}'")
    print(f"Generated {len(hashes)} file hashes and saved to '{OUTPUT_JSON}'")

if __name__ == "__main__":
    main()
