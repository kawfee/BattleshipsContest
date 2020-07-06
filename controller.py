#!/usr/bin/python3

import os
import errno
import subprocess
import time

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

rounds=0

while (rounds<10):
    # This starts the first Ai
    with open(FIFO1CTA, 'w') as fifo1cta:
        print("CONT: FIFO1CTA opened")
        fifo1cta.write("cmd " + str(rounds))
        print("CONT: Message sent")

    with open(FIFO1ATC, 'r') as fifo1atc:
        print("CONT: FIFO1ATC opened")
        while True:
            data = fifo1atc.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    
    # This starts the second Ai
    with open(FIFO2CTA, 'w') as fifo2cta:
        print("CONT: FIFO2CTA opened")
        fifo2cta.write("cmd " + str(rounds))
        print("Message sent")

    with open(FIFO2ATC, 'r') as fifo2atc:
        print("CONT: FIFO2atc opened")
        while True:
            data = fifo2atc.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    rounds+=1
    time.sleep(0.5)
 
p1.kill()
p2.kill()
