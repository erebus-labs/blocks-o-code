blocks-o-code
=============

#A Block of Code
+ *Title:* Physical manipulators for developing and implementing short programing routines 
+ *Project Short Name:* A Block of Code (ABC)
+ *Sponsors:* 
  + [Dr. Mike Borowczak](https://www.linkedin.com/in/mborowczak) @ [Erebus Labs](http://www.erebuslabs.com)
  + [Dr. Andrea Burrows] (http://uwyo.academia.edu/AndreaBurrows) @ [University of Wyoming] (http://www.uwyo.edu/seced/faculty-staff/andrea-burrows.html)
+ *Senior Design Team:* 
  + Jacob Mickiewicz 
  + Tyler Hart
  + Daniel Frister
  + Greg Stromire
  + Nathan Bryant
+ *PSU Academic Advisor:* [Roy Kravitz](http://www.pdx.edu/directory/name/roy_kravitz) 
+ *Targeted Users:* Infants to High School aged children, parents and their educators


##About A Block of Code

Blocks of Code are connectable “blocks” that mimic a simple-grammar programming language. 

The fundamental grammar contain 4 types of blocks:
+ Value Blocks
  + numbers (ints/doubles) – stringing these blocks together forms longer numbers [0,1,2,3,4,5]; [6,7,8,9,.]
  + variables
+ Operator Blocks
  + simple binary operators (*,/,-,+,%?): 
  + equivalence operators (<,>,==, !=,<=,>=)
+ Control Blocks (start and end)
  + if structures
  + while structures
  + explicit encapsulation ( )
+ Output Blocks

##Background

Algorithms, logical constructs and the foundations of programming are generally limited to computer-based instances. No physical manipulators exist that expose children to programming as they do for number and alphabet systems, mechanics, art, etc.  Build a prototype for building blocks that can be attached through internal magnets, these magnets perform two actions 
1. physical connection and 
2. logical circuit connections.

##Current Feature List
- [x] Open Source Hardware Design & Board (can use “closed source” components: ASICs, uC, etc.)
- [x] Must Have a Multi-Chip solution – e.g. no single SoC;
- [x] Fundamental grammar functioning; assignment; binary operators; 
- [x] Open Software Repository (github)
- [x] Multi-function blocks – e.g. 6 (or 4) binary operators on a single block; depending on connections/which side “faces up” operation changes 
- [x] Separate “power” blocks

##Wish List
- [ ] Hardware cost <$3 per unit (average cost for a set 20 = $60)
- [ ] Unexposed Connectors (e.g. Magnetic  + Inductive coupling between blocks?)
- [ ] Power: Ultra Low power operation
- [ ] Power
  - [ ] Built in power source / power pack
  - [ ] Built in power source w/inductance recharge
- [ ] Computer GUI/Phone App:
  - [ ] simulate a set of blocks (pre build) 
  - [ ] take a picture of a set of blocks and execute code (they don't even have to be REAL blocks - could be paper!)
