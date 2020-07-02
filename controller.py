import os
import errno
import subprocess

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


p1 = subprocess.Popen("python ai1.py", shell=True)
p2 = subprocess.Popen("python ai2.py", shell=True)

pid1=p1.pid
pid2=p2.pid

rounds=0
print("Opening FIFO1...")
while (rounds<10):

    # This starts the first Ai
    with open(FIFO1, 'w') as fifo1:
        print("FIFO1 opened")
        fifo1.write("Hi")
        print("Message sent")

    with open(FIFO1, 'r') as fifo1:
        print("FIFO1 opened")
        while True:
            data = fifo1.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    
    # This starts the second Ai
    with open(FIFO2, 'w') as fifo2:
        print("FIFO2 opened")
        fifo2.write("Hi")
        print("Message sent")

    with open(FIFO2, 'r') as fifo2:
        print("FIFO2 opened")
        while True:
            data = fifo2.read()
            if len(data) == 0:
                print("Writer closed")
                break
            print('Read: "{0}"'.format(data))
    rounds+=1
 
subprocess.run(("kill "+pid1), shell=True)
subprocess.run(("kill "+pid2), shell=True)
