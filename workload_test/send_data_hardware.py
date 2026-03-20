from scapy.all import sendp, Ether, Raw
import struct
import time

TARGET_MAC = "4c:36:4e:a6:31:69"
SRC_MAC    = "9c:69:d3:40:ab:97"
IFACE      = "enx9c69d340ab97"
ETHERTYPE  = 0x88B5
MAGIC      = 0x474F2121
CHUNK_SIZE = 1486

def send_frame(data):
    frame = Ether(src=SRC_MAC, dst=TARGET_MAC, type=ETHERTYPE) / Raw(load=data)
    sendp(frame, iface=IFACE, verbose=False)

# Frame 1: magic + size only
with open("workload.bin", "rb") as f:
    payload = f.read()

header = struct.pack(">I", MAGIC) + struct.pack("<Q", len(payload))
send_frame(header)
time.sleep(0.5)  # Give kernel time to process header

# Remaining frames: binary chunks
for i in range(0, len(payload), CHUNK_SIZE):
    send_frame(payload[i:i + CHUNK_SIZE])
    time.sleep(0.001)

print(f"Sent {len(payload)} bytes in {(len(payload) // CHUNK_SIZE) + 2} frames.")
