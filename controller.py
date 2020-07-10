#!/usr/bin/python3

import os
import errno
import subprocess
from threading import Thread, Event
import threading
import time
import json

#VARIABLE SPACE
FIFO1CTA = 'cta1'
FIFO1ATC = 'atc1'

FIFO2CTA = 'cta2'
FIFO2ATC = 'atc2'

#FIFO1 - (R & W)
try:
    os.mkfifo(FIFO1CTA)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise
try:
    os.mkfifo(FIFO1ATC)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise

#FIFO2 - (R & W)        
try:
    os.mkfifo(FIFO2CTA)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise
try:
    os.mkfifo(FIFO2ATC)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise

p1 = subprocess.Popen("./ai1.py", shell=False)
p2 = subprocess.Popen("./ai2.py", shell=False)

data="junk data"
rounds=0
winner=""
errMsg = "ERROR: ai timed out"

name={"a":1, "b":2}

stop_broadcast1=Event()
stop_broadcast2=Event()

#FUNCTION SPACE
def killProcess(pipe):
    print("KP: at killProcess")
    count = 0
    while True:
        time.sleep(.01)
        count += 1
        if pipe=='atc1' and stop_broadcast1.is_set():
            print("KP: stop_broadcast1 is set")
            break
        if pipe=='atc2' and stop_broadcast2.is_set():
            print("KP: stop_broadcast2 is set")
            break
        if(count%100==0):
            print("KP: KillProcess count: " + str(count) + " " + pipe)
        if count>=500:
            with open(pipe, 'w') as fifoatc:
                print("KP: killProcess is writing")
                fifoatc.write(errMsg)
            break

#MAIN SPACE
while (rounds<10 and winner==""):
    # This starts the first Ai
    # watchdog1 thread here
    watchdog1=Thread(target=killProcess, args=(FIFO1ATC,))
    watchdog1.start()

    with open(FIFO1CTA, 'w') as fifo1cta:
        print("CONT: FIFO1CTA opened")
        fifo1cta.write(json.dumps(name))
        print("CONT: Message sent")
    
    with open(FIFO1ATC, 'r') as fifo1atc:
        print("CONT: FIFO1ATC opened")
        print("DATA OPENING_____DATA OPENING_____DATA OPENING")
        data = fifo1atc.read()
        print("Data: "+str(data))
        print("DATA CLOSING_____DATA CLOSING_____DATA CLOSING")
        print('CONT: Read: "{0}"'.format(data))
    
    if (data==errMsg):
        p1.kill()
        winner = "p2"
        break
    else:
        stop_broadcast1.set()
    # watchdog1 thread ends by here, if not before
    
    # This starts the second Ai
    # watchdog2 thread here
    watchdog2=Thread(target=killProcess, args=(FIFO2ATC,))
    watchdog2.start()
    
    with open(FIFO2CTA, 'w') as fifo2cta:
        print("CONT: FIFO2CTA opened")
        fifo2cta.write("cmd " + str(rounds))
        print("CONT: Message sent")
    
    with open(FIFO2ATC, 'r') as fifo2atc:
        print("CONT: FIFO2ATC opened")
        print("DATA OPENING_____DATA OPENING_____DATA OPENING")
        data = fifo2atc.read()
        print("Data: "+str(data))
        print("DATA CLOSING_____DATA CLOSING_____DATA CLOSING")
        print('CONT: Read: "{0}"'.format(data))
    
    if (data==errMsg):
        p2.kill()
        winner = "p1"
        break
    else:
        stop_broadcast2.set()
    # watchdog2 thread ends by here, if not before
    
    rounds+=1
    time.sleep(.5)
    print("Thread Count: " + str(threading.active_count()))

    stop_broadcast1.clear()
    stop_broadcast2.clear()

print("winner: " + winner)
p1.kill()
p2.kill()


