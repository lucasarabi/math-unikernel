from scapy.all import sendp, Ether, Raw
import struct
import time
import os

def get_ethernet_interface():
    for iface in os.listdir('/sys/class/net/'):
        if iface == 'lo':
            continue
        if any(iface.startswith(p) for p in ('docker', 'br-', 'veth', 'wlan', 'wlo', 'virbr', 'eno', 'eth')):
            continue
        try:
            with open(f'/sys/class/net/{iface}/operstate') as f:
                if f.read().strip() == 'up':
                    return iface
        except:
            continue
    return None

def get_mac(iface):
    with open(f'/sys/class/net/{iface}/address') as f:
        return f.read().strip()

TARGET_MAC = "4c:36:4e:a6:31:69"
ETHERTYPE  = 0x88B5
MAGIC      = 0x474F2121
CHUNK_SIZE = 1486

IFACE = get_ethernet_interface()
if not IFACE:
    print("ERROR: No active Ethernet interface found. Is the cable plugged in?")
    exit(1)

SRC_MAC = get_mac(IFACE)
print(f"Using interface: {IFACE} ({SRC_MAC})")

def send_frame(data):
    frame = Ether(src=SRC_MAC, dst=TARGET_MAC, type=ETHERTYPE) / Raw(load=data)
    sendp(frame, iface=IFACE, verbose=False)

with open("workload.bin", "rb") as f:
    payload = f.read()

header = struct.pack(">I", MAGIC) + struct.pack("<Q", len(payload))
send_frame(header)
time.sleep(0.5)

for i in range(0, len(payload), CHUNK_SIZE):
    send_frame(payload[i:i + CHUNK_SIZE])
    time.sleep(0.001)

print(f"Sent {len(payload)} bytes in {(len(payload) // CHUNK_SIZE) + 2} frames.")