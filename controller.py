#!/usr/bin/python3

import os
import errno
import subprocess
import time

FIFO1 = 'mypipe1'

try:
    os.mkfifo(FIFO1)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise

FIFO2 = 'mypipe2'

try:
    os.mkfifo(FIFO2)
except OSError as oe: 
    if oe.errno != errno.EEXIST:
        raise


p1 = subprocess.Popen("./ai1.py", shell=False)
p2 = subprocess.Popen("./ai2.py", shell=False)

pid1=p1.pid
pid2=p2.pid

rounds=0

while (rounds<10):
    time.sleep(2)
    # This starts the first Ai
    with open(FIFO1, 'w') as fifo1:
        print("CONT: FIFO1 opened")
        fifo1.write("cmd " + str(rounds))
        print("CONT: Message sent")

    with open(FIFO1, 'r') as fifo1:
        print("CONT: FIFO1 opened")
        while True:
            data = fifo1.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    
    # This starts the second Ai
    with open(FIFO2, 'w') as fifo2:
        print("CONT: FIFO2 opened")
        fifo2.write("cmd " + str(rounds))
        print("Message sent")

    with open(FIFO2, 'r') as fifo2:
        print("CONT: FIFO2 opened")
        while True:
            data = fifo2.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    rounds+=1
 
p1.kill()
p2.kill()
