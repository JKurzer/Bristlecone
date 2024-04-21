**Missing Features**  
- [ ] TCP Backhaul  
  - [ ] Time Sync  
  -Key Negotiation  
-Session Concepts  
--Sym Key Sharing  
--UDP Reflector has no session concept  
--Session Join  
-Encryption  
--Basic Payload Encryption  
--Data access zoning by key  

**Areas of Work**  
-Configuration   
--Many many values are hard-coded
-Reference Control Input Integration  
--Right now, the sender just dumps an empty control packet and timestamps it.  
--Until a ref input implementation exists, we just send at 100hz poll rate.  
-Fast Logging  
--UE Logging is not intended for millisecond precision perf analysis.  
-Multiway Receive (Many threads per single socket)  
--I hope we don't need this, but we might.
