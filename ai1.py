#!/usr/bin/python3

import os
import errno

FIFO1 = 'mypipe1'

try:
	os.mkfifo(FIFO1)
except OSError as oe:
	if oe.errno != errno.EEXIST:
		raise


while ( True ):
    with open(FIFO1, 'r') as fifo1:
        print("AI1: FIFO1 opened")
        while True:
            data = fifo1.read()
            if len(data) == 0:
                print("AI1: Writer closed")
                break
            print('AI1: Read: "{0}"'.format(data))


    with open(FIFO1, 'w') as fifo1:
        print("AI1: FIFO1 opened")
        fifo1.write("My name is Ai1")
        print("AI1: Message sent")
print("Never got here")
