from scapy.all import sendp, Ether, Raw
import struct

TARGET_MAC = "4c:36:4e:a6:31:69"    # MAC printed by your kernel
SRC_MAC    = "10:7c:61:b6:94:55"    # your desktop's MAC
IFACE      = "eno1"                 # your interface name
ETHERTYPE  = 0x88B5
MAGIC      = 0x474F2121             # "GO!!"

data  = struct.pack(">I", MAGIC)

frame = Ether(src=SRC_MAC, dst=TARGET_MAC, type=ETHERTYPE) / Raw(load=data)
sendp(frame, iface=IFACE)