from re import S
import socket
import time
import sys
# import fcntl, os
import errno
import keyboard

HOST = "192.168.0.104"  # The server's hostname or IP address
PORT = 3333  # The port used by the server
MAX_BYTE = 1024

print(f"Połączenie z {HOST}:{PORT}")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.settimeout(1)
    while True:
        if keyboard.read_key() != 0:
            dataToSend = str(input("->> "))
            s.send(str.encode(dataToSend))
            if dataToSend == "close":
                sys.exit(0)
        
        msg = ''
        try:
            dataRec = s.recv(1024) 
            msg += dataRec.decode("utf-8")
        except socket.error as e:
            a=0
        finally:
            if msg != '':
                print("[server] "+msg)