import socket
import time

HOST = "127.0.0.1"   # or your server IP
PORT = 6667          # your IRC port
N_CLIENTS = 100000

sockets = []

for i in range(N_CLIENTS):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    sockets.append(s)
    print(f"Connected client {i+1}")

print(f"All {N_CLIENTS} clients connected. Press Ctrl+C to exit and close them.")
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    for s in sockets:
        s.close()

