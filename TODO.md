# Missing Features 
- [ ] TCP Backhaul  
  - [ ] Time Sync  
  - [ ] Key Negotiation
- [ ] FlatBuff\Protobuff
  - [ ] Right now our datagrams are hardcoded.
  - [ ] We'd like to switch to schema-driven datagrams.
- [ ] Defactor datagram construction
  - [ ] We'd like to provide a clean interface for datagram building outside of the core of bristlecone.
  - [ ] We should still provide something similar to the current datagram building using templates + schemas when legal.
  - [ ] This will follow the provider strategy pattern.
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
     
# Nice to Have
- [ ] Example of asymmetric up/down datagrams
  - [ ] This one is pretty important for the same reason as mcast listen
  - [ ] we'd like the reflector to, when it can, send inputs from multiple players as single updates
- [ ] Variable windowing  

**Areas of Work**  
- [ ] Configuration   
  - [ ] Many many values are hard-coded
- [ ] Timestamping
  - [ ] Right now, for testing, our timestamp is really wide.
  - [ ] Eventually, we'll need to switch back to the single-byte rolling stamp.
  - [ ] This stamp's unit is poll-rate/clone-set, so a stamp of 45 is the 45th polling, not the 45th millisecond.
- [ ] Reference Control Input Integration  
  - [ ] Right now, the sender just dumps an empty control packet and timestamps it.  
  - [ ] Until a ref input implementation exists, we just send at 100hz poll rate.  
- [ ] Fast Logging
  - [ ] 360hz is the very definition of log spam. We need to aggregate.
  - [ ] UE Logging is not intended for millisecond precision perf analysis.  
  - [ ] A library that provides a log-writer thread is probably needed.  
- [ ] Multiway Receive (Many threads per single socket)  
  - [ ] I hope we don't need this, but we might.
