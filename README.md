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
 |                  Events collector                 |
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

 