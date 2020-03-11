# MonDSs:  Monitoring Distributed Systems

Gathering data from distributed systems are the main focus of this work. Percentage of CPU usage, Read and Write rate Usage and percentage of memory usage are calculated. Agent has five threats, first four are for gathering four above metrics and the other one is responsible to receive the massages. Each metric thread send its value, value name, agent name and time to middle server with a send message function. The middle server function has two threats for handling the agents and server.
