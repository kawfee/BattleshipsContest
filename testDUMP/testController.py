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
    counter=0
    while(True):
        time.sleep(counter)
        if(counter>=5):
            with open(FIFO2, 'r') as fifo2:
                print("KP1: FIFO1 opened")
                while True:
                    data = fifo1.read()
                    print('KP1: process timed out--no data read')
            p1.kill()
            break
        else:
            counter+=1

        if stop_broadcast.is_set():
            break


with open(FIFO1, 'w') as fifo1:
    print("TSTCONT: FIFO1 opened")
    
    watchdog1=Thread(target=killProcess)
    
    watchdog1.start()
    watchdog1.join(timeout=10)
    fifo1.write("cmd " + str(rounds))
    stop_broadcast.set()
    
    print("TSTCONT: Message either sent or was killed")

p1.kill()
