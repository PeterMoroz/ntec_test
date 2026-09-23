import os
import random
import string
import argparse
import time

def get_random_filename():
    """Generates a random filename with a random extension."""
    # Random length between 5 and 12 characters
    length = random.randint(5, 12)
    # Random alphanumeric string
    name = ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))
    # Randomly pick an extension (including the chance of no extension)
    extensions = ['.txt', '.log', '.dat', '.bin', '.tmp', '.csv', '']
    ext = random.choice(extensions)
    return name + ext

def main():
    # Set up command-line arguments
    parser = argparse.ArgumentParser(description="Generate dummy files in a specified directory randomly.")
    parser.add_argument("directory", help="Target directory to create files in.")
    parser.add_argument("-n", "--num-files", type=int, default=10, help="Number of files to generate (default: 10).")
    parser.add_argument("--min-size", type=int, default=1024, help="Minimum file size in bytes (default: 1 KB).")
    parser.add_argument("--max-size", type=int, default=1048576, help="Maximum file size in bytes (default: 1 MB).")
    parser.add_argument("--max-delay", type=float, default=0.0, help="Maximum random delay in seconds between file creations (default: 0).")
    
    args = parser.parse_args()

    # Ensure min size is not greater than max size
    if args.min_size > args.max_size:
        args.min_size, args.max_size = args.max_size, args.min_size

    # Create the target directory if it doesn't already exist
    os.makedirs(args.directory, exist_ok=True)

    print(f"Generating {args.num_files} dummy files in '{os.path.abspath(args.directory)}'...")
    
    for i in range(args.num_files):
        filename = get_random_filename()
        filepath = os.path.join(args.directory, filename)
        
        # Pick a random size between the defined min and max
        size = random.randint(args.min_size, args.max_size)
        
        # Write random binary data to the file
        with open(filepath, 'wb') as f:
            f.write(os.urandom(size))
            
        print(f"[{i+1}/{args.num_files}] Created: {filename} ({size:,} bytes)")
        
        # Optional random delay to simulate real-world file creation timing
        if args.max_delay > 0:
            delay = random.uniform(0, args.max_delay)
            time.sleep(delay)
            
    print("\nDone!")

if __name__ == "__main__":
    main()