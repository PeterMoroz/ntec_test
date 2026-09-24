# ntec_test

The repo contains a simplified part of MDR-agent, which do:
- read stream of events from file 
- check integrity of specified directory comparing with baseline-manifest
- apply a simple rule to detect suspicious actvity
- send alerts via network to monitoring process


The repo contains 2 directories 
1. *collector*, contains application which collect events, apply rules and generates alerts
2. *monitor*, contains appliction which listen for connections from collectors, receive alerts serialized in JSON and print them to console

Both applications should be build independent. Each subdirectory contains `build.sh` which runs cmake to do all job.
No need to install dependencies on the build host, all needed will be pulled during build into local directory with cmake-module FetchContent. 
The implementation of SHA256 algorithm (files `sha256.h` and `sha256.cpp`) I borrowed here https://github.com/stbrumme/hash-library .

Subdirectory `collector/utils` contains auxiliary tools:
- to generate dummy events' log in JSON (file `generate_logs.py`)
- to create directory stuffed with dummy files and create baseline in JSON (file `generate_and_hash.py`)
- to generate dummy files in specified directory (file `generate_random_files.py`),
could be useful when checking directory's integrity feature. By default script requres only path to directory (`python3 utils/generate_random_files.py target_directory`), it creates 10 random files in that directory. But for advanced usage it accepts parameters, 
for example create 50 files, ranging from 10K to 500K in size and add random delay up to 0.5 seconds between each creation:
`python3 utils/generate_random_files.py target_directory -n 50 --min-size 10240 --max-size 512000 --max-delay 0.5`


##### overall architecture
 ____________________________________________________
 |                                                   |
 |                  Events Collector                 |
 |    ___________________      _____________         |                 
 |   |                   |    |             |        |                                   
 |   |  Events Observer  |    |  Directory  |        |                
 |   |___________________|    |  Integrity  |-- network connection --------
 |            |               |   Checker   |        |                    |
 |            |               |_____________|        |                    |
 |    ________V__________                            |                    |
 |   |                   |                           |           _________V__________
 |   |  Events Queue     |                           |          |                    |
 |   |___________________|                           |          |   Events Monitor   |
 |            |                                      |          |                    |
 |            |                                      |          |____________________|
 |   _________V__________                            |                   ^
 |  |                    |                           |                   |
 |  |  Events Processor  | ------ network connection ---------------------
 |  |____________________|                           |
 |___________________________________________________|

 Service **EventsCollector** consists of 
- *EventsObserver* - the module read events (by 2 threads) from supplied file and put these events into queue.
- *EventsQueue* - thread-safe bounded queue with blocking Put/Get operations
- *EventsProcessor* - the module recieve events from queue (by single thread), parse them and apply detection rule to events with type 'ProcessStarted'. All other events just resend to EventsMonitor service. The result of processing events by detection rule will be either list of childrens' PIDs of suspicious process or nothing. If rule returns non-empty list of PIDs processing flow generates a new event and send it to monitoring service.
- *DirectoryIntegrityChecker* - the module constantly scans directory with fixed time delay, compares directory's content with manifest file and generates events in case of differences. These events are sent to monitoring service. Beside the directory's content the scanning procedure calculated files' hashes (with SHA256) and compare resulting values with hashes in manifest. In case of comparison mismatch the event will be generated and sent to monitoring service.

Service **EventsMonitor** is a simple TCP-server listening for connections on the port 5000, every incoming connection creates a new session. During session's lifetime it reads messages from socket and print them on console. It's expectes that messages are delimited by symbol '\n'.

The modules *EventsProcessor* and *DirectoryIntegrityChecker* both has TCP-client which connects to endpoint 127.0.0.1:5000 at startup. As endpoint is hardcoded it's expected that  monitoring service is running on the same host.