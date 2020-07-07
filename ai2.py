#!/usr/bin/python3

import os
import errno

FIFO2CTA = 'cta2'
FIFO2ATC = 'atc2'

while ( True ):
    with open(FIFO2CTA, 'r') as fifo2cta:
        print("AI2: FIFO2CTA opened")
        data = fifo2cta.read()
        print('AI2: Read: "{0}"'.format(data))


    with open(FIFO2ATC, 'w') as fifo2atc:
        print("AI2: FIFO2ATC opened")
        fifo2atc.write("My name is Ai2")
        print("AI2: Message sent")
