# Quiz 4

## Question 1

```text
VA: Virtual Address
PA: Physical Address
VPN: Virtual Page Number
VPO: Virtual Page Offset
PPN: Physical Page Number
PPO: Physical Page Offset
TLB: Translation Lookaside Buffer
TLBI: TLB Index
TLBT: TLB Tag
CT: Cache Tag
CI: Cache Index
CO: Cache Offset
```

## Question 2

```text
a) 2^14 bytes
b) 2^12 bytes
c) VPO = PPO = 6 bits, VPN = 8 bits, PFN = 6 bits
d) 2^8 = 256 pages
e) 2^6 = 64 pages
f) 4 sets
g) TLBI: 2 bits, TLBT: 6 bits 
h) CT: 6 bits, CI: 4 bits, CO: 2 bits 
i) cached values are only valid for a single process, homonyms and synonyms
j) same as above, but CT: 8 bits, because tags is now bigger
k) first look up virtually addressed cache, if not in there, lookup physical address with MMU
```

## Question 3

```text
a)
00 0001 1110 0101

b)
VPN: 00000111 = 0x7
TLBI: 0x3
TLBT: 0x1
TLB hit: yes
Page fault: no
PPN: 0x22

c)
1000 1010 0101

d)
offset: 0x1
cache index: 0x9
cache tag: 0x22
hit: yes
cache byte returned: 0x1A
```
