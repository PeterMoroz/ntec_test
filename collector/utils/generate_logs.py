import json
import random

process_names = [
    "chrome.exe", "msedge.exe", "firefox.exe", "explorer.exe", "svchost.exe",
    "cmd.exe", "powershell.exe", "conhost.exe", "notepad.exe", "taskmgr.exe",
    "slack.exe", "teams.exe", "outlook.exe", "excel.exe", "winword.exe",
    "code.exe", "node.exe", "python.exe", "git.exe", "bash.exe"
]

# Track active processes: pid -> (ppid, name)
active_processes = {
    4: (0, "System"),
    812: (4, "explorer.exe")
}

next_pid = 1000
ts = 1790053650
events = []
N = 100000

for _ in range(N):
    # 60% chance to start a process, 40% chance to write file
    if random.random() < 0.4:
        # End a random active process (excluding base System/explorer)
        path = random.choice(["/tmp/765f6940.txt", "/tmp/c865.txt", "/tmp/4f07.txt", "/tmp/8e4e.txt", "/tmp/d42a70ea4375.txt"])
        events.append({
            "type": "FileWritten",
            "path": path,
            "ts": ts
        })
    else:
        # Start a new process
        ppid = random.choice(list(active_processes.keys()))
        ppid_name = active_processes[ppid][1]
        
        # Context-aware process spawning for realism
        if ppid_name in ["cmd.exe", "powershell.exe"]:
            name = random.choice(["conhost.exe", "net.exe", "ipconfig.exe", "ping.exe", "curl.exe"])
        elif ppid_name == "explorer.exe":
            name = random.choice(["chrome.exe", "msedge.exe", "notepad.exe", "excel.exe", "taskmgr.exe"])
        else:
            name = random.choice(process_names)
            
        pid = next_pid
        next_pid += 1
        active_processes[pid] = (ppid, name)
        events.append({
            "type": "ProcessStarted",
            "pid": pid,
            "ppid": ppid,
            "name": name,
            "ts": ts
        })
    
    # Increment timestamp by 1 to 15 units
    ts += random.randint(1, 15)

# Write to file
with open(f"events_log_{N}.json", "w") as f:
    for event in events:
        f.write(json.dumps(event) + "\n")

print(f"Successfully generated events_log_{N}.json with {N} records!")
