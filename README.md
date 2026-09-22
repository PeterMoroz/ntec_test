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

Subdirectory `collector\utils` contains auxiliary tools:
- to generate dummy events' log in JSON (file `generate_logs.py`)
- to create directory stuffed with dummy files and create baseline in JSON (file `generate_and_hash.py`)

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

 