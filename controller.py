#!/usr/bin/python3

import os
import errno
import subprocess
from threading import Thread, Event
import time


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
msg = "ERROR: ai timed out"

stop_broadcast=Event()

#FUNCTION SPACE
def killProcess():
    print("at killProcess")
    count = 0
    while True:
        time.sleep(1)
        count += 1
        print("KillProcess count: " + str(count))
        if(count>=5):
            with open(FIFO1ATC, 'w') as fifo1atc:
                print("killProcess is writing")
                fifo1atc.write(msg)
            break
        if stop_broadcast.is_set():
            break

#MAIN SPACE
while (rounds<10 and winner==""):
    # This starts the first Ai
    # watchdog1 thread here
    watchdog1=Thread(target=killProcess)
    watchdog1.start()

    with open(FIFO1CTA, 'w') as fifo1cta:
        print("CONT: FIFO1CTA opened")
        fifo1cta.write("cmd " + str(rounds))
        print("CONT: Message sent")
    
    with open(FIFO1ATC, 'r') as fifo1atc:
        print("CONT: FIFO1ATC opened")
        print("DATA OPENING_____DATA OPENING_____DATA OPENING")
        data = fifo1atc.read()
        print("Data: "+str(data))
        print("DATA CLOSING_____DATA CLOSING_____DATA CLOSING")
        print('Read: "{0}"'.format(data))
    
    if (data==msg):
        p1.kill()
        winner = "p2"
        break
    else:
        stop_broadcast.set()
    # watchdog1 thread ends by here, if not before
    
    # This starts the second Ai
    with open(FIFO2CTA, 'w') as fifo2cta:
        print("CONT: FIFO2CTA opened")
        fifo2cta.write("cmd " + str(rounds))
        print("Message sent")

    with open(FIFO2ATC, 'r') as fifo2atc:
        print("CONT: FIFO2atc opened")
        data = fifo2atc.read()
        print('Read: "{0}"'.format(data))
    rounds+=1
    time.sleep(0.5)

print("winner: " + winner)
p1.kill()
p2.kill()


