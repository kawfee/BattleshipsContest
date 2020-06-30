import os
import errno

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


print("Opening FIFO1...")
while ( True ):

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
