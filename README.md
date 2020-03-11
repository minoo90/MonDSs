# MonDSs:  Monitoring Distributed Systems

Gathering data from distributed systems are the main focus of this work. The responsibility of middle server is to pass the information to main server. 
Percentage of CPU usage, Read and Write rate Usage and percentage of memory usage are calculated. Agent has five threats, first four are for gathering the above metrics and the other one is responsible to receive the massages. Each metric thread send its value, value name, agent name and time to middle server with a send message function. In addition, the middle server function has two threats for handling the agents and server. 

