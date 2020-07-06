#!/usr/bin/python3

import os
import errno
import subprocess
from threading import Thread, Event
import time

FIFO1 = 'mypipe1'
p1 = subprocess.Popen("./otherTest.py", shell=False)
pid1=p1.pid
stop_broadcast=Event()

try:
    os.mkfifo(FIFO1)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise

def killProcess():
    print("5: got to killProcess def")
    counter=0
    while(True):
        print("5: counter = "+str(counter))
        time.sleep(1)
        
        if(counter>=5):
            with open(FIFO1, 'r') as fifo1:
                print("5.1: KP1: FIFO1 opened")
                data = fifo1.read()
                print('5.2: KP1: process timed out--no data read')
            p1.kill()
            break
        else:
            counter+=1
        
        if stop_broadcast.is_set():
            break

#main
print("1: got to MAIN def")            
with open(FIFO1, 'w') as fifo1:
    print("2: TSTCONT: FIFO1 opened")
    
    watchdog1=Thread(target=killProcess)
    
    print("3: watchdog started")
    watchdog1.start()

    print("4: watchdog joined")
    #watchdog1.join(timeout=10)
    
    print("6: fifo1 written to")
    fifo1.write("Hello! ")
    
    print("7: broadcast stopped(?)")
    stop_broadcast.set()
    
    print("8: TSTCONT: Message either sent or was killed")
p1.kill()















































