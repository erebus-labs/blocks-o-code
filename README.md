blocks-o-code
=============
Blocks of Code Main Project

#A Block of Code
+ *Title:* Physical manipulators for developing and implementing shot programing routines 
+ *Project Short Name:* A Block of Code (ABC)
+ *Sponsors:* 
  + [Dr. Mike Borowczak](https://www.linkedin.com/in/mborowczak) @ [Erebus Labs](www.erebuslabs.com)
  + [Dr. Andrea Burrows] (http://uwyo.academia.edu/AndreaBurrows) @ [University of Wyoming] (http://www.uwyo.edu/seced/faculty-staff/andrea-burrows.html)
+ *Senior Design Team:* 
  + Jacob Mickiewicz 
  + Tyler Hart
  + Daniel Frister
  + Greg Stromire
  + Nathan Bryant
+ *PSU Academic Advisor:* [Roy Kravitz](http://www.pdx.edu/directory/name/roy_kravitz) 

+ *Targeted Users:* Infants to High School aged children, parents and their educators


##Problem/Task

Your task is to design the basic hardware / software structure for ultra-low cost, connectable “blocks” that mimic a simple-grammar programming language. 

The fundamental grammar should contain 7 types of blocks:
+ numbers (ints/doubles) – stringing these blocks together forms longer numbers [0,1,2,3,4,5]; [6,7,8,9,.]
+ variables  
+ assignment operators 
+ simple binary operators (*,/,-,+,%?): 
+ equivalence operators (<,>,==, !=,<=,>=)
+ control block start/end
+ program start/end

While a majority of the logic for the entire operation could be housed in the program start/end or control block start/end – there are communication and connectivity challenges; especially without exposing connectors. 

##Background

Algorithms, logical constructs and the foundations of programming are generally limited to computer-based instances. No physical manipulators exist that expose children to programming as they do for number and alphabet systems, mechanics, art, etc.  Build a prototype for building blocks that can be attached through internal magnets, these magnets perform two actions 1) physical connect and 2) logical circuit connections.

##Skill Requirements (Order of importance):
1. System Design 
2.Ultra-small form factor design
3. Ultra-low power/heat design (the final design would be incased in a resin
4. Power and signal transmission through traditional wire & inductance

##Other Details: 
+ IP Ownership: Joint/”none”: The goal is to release an open hardware device and software; joint ownership of the original design, publicly available for reuse under similar attribution
+ Extension after End of Project: 50-100 parts run to distribute to high-needs schools as a mechanism to enhance/drive STEM as a college degree option.
+ Publication Opportunities: Conference and Journal Publication both in K20 Education, IEEE Spectrum, and/or others (joint ownership: ECE team and Sponsors) [resume/CV builder!]
+ No NDA required

##Requirements (Wish List):
###Must Have

- [ ] Hardware cost <$3 per unit (average cost for a set 20 = $60)
- [ ] Open Source Hardware Design & Board (can use “closed source” components: ASICs, uC, etc.)
- [ ] Must Have a Multi-Chip solution – e.g. no single SoC;
- [ ] Magnetic  + Inductive coupling between blocks
- [ ] Fundamental grammar functioning; assignment; binary operators; 
- [ ] Open Software Repository (github)
- [ ] Power: Ultra Low power operation
- [ ] Built in power source / power pack
- [ ] Separate “power” blocks?
- [ ] Set of blocks for each team member and two sponsors

###Like to Have
- [ ] Hardware cost <$2 per unit (average cost for a set 20 = $40)
- [ ] Hardware:
  - [ ] Multi-function blocks – e.g. 6 (or 4) binary operators on a single block; depending on connections/which side “faces up” operation changes 
  - [ ] Control Structure Blocks
- [ ] Set of blocks for each team member and two sponsors + 5 extra sets

###Nice to Have
- [ ] Hardware cost <$1 per unit (average cost for a set 20 = $20)
- [ ] Power
  - [ ] Built in power source w/inductance recharge
- [ ] Computer GUI/Phone App: simulate a set of blocks (pre build) or take a picture of a set of blocks and execute code
- [ ] Set of blocks for each team member and two sponsors + 10 extra sets (5 to sponsors; 5 to PSU)

##Milestones:

###Hardware Evaluation to satisfy given constraints 
- [ ] Main System: Modification of existing Open Hardware Design to create part or creation of custom PCB?
- [ ] Power System: Battery capacity + recharge?
- [ ] Communication mechanisms: Effective low cost communication and sync (without taxing power source)?

###Implementation
- [ ] Development of full hardware solution
- [ ] Development of power system
- [ ] Development of communication system

