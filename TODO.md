# Missing Features 
- [ ] TCP Backhaul  
  - [ ] Time Sync  
  - [ ] Key Negotiation  
- [ ] Session Concepts  
  - [ ] Sym Key Sharing  
  - [ ] UDP Reflector has no session concept  
- [ ] Session Join hooks
  - [ ] A good abstraction for the interface between bristlecone and your session manager.
  - [ ] A good STANDARD session approach that allows the edge reflectors to remain mostly stateless.
- [ ] Quiet Reflectors
  - [ ] In production envs, we likely need fewer clones transmitted by the reflector to the client
- [ ] Encryption  
  - [ ] Basic Payload Encryption  
  - [ ] Data access zoning by key
- [ ] Multicast\Broadcast Listen
  - [ ] the client to server connection's probably never in broadcast mode but...
  - [ ] the server to client connection may be in broadcast or multicast mode.
  - [ ] the potential bandwidth savings are too great for us to skip this.
     
**Nice to Have**
- [ ] Example of asymmetric up/down datagrams  
- [ ] Variable windowing  

**Areas of Work**  
- [ ] Configuration   
  - [ ] Many many values are hard-coded
- [ ] Reference Control Input Integration  
  - [ ] Right now, the sender just dumps an empty control packet and timestamps it.  
  - [ ] Until a ref input implementation exists, we just send at 100hz poll rate.  
- [ ] Fast Logging
  - [ ] 360hz is the very definition of log spam. We need to aggregate.
  - [ ] UE Logging is not intended for millisecond precision perf analysis.  
  - [ ] A library that provides a log-writer thread is probably needed.  
- [ ] Multiway Receive (Many threads per single socket)  
  - [ ] I hope we don't need this, but we might.
