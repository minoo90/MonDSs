# MonDSs:  Monitoring Distributed Systems

Gathering data from distributed systems is the main focus of this work. The responsibility of the middle server is to pass the information to the main server.
Percentage of CPU usage, Read and Write rate Usage, and percentage of memory usage are calculated. The agent has five threads; the first four are for gathering the above metrics, and the other one is responsible for receiving the massages. Each metric thread sends its value, value name, agent name, and time to the middle server with a Send Message function. In addition, the middle server function has two threads for handling the agents and the server.
