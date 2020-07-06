#!/usr/bin/python3

import os
import errno

FIFO1CTA = 'cta1'
FIFO1ATC = 'atc1'

while ( True ):
    with open(FIFO1CTA, 'r') as fifo1cta:
        print("AI1: FIFO1CTA opened")
        while True:
            data = fifo1cta.read()
            if len(data) == 0:
                print("AI1: Writer closed")
                break
            print('AI1: Read: "{0}"'.format(data))


    with open(FIFO1ATC, 'w') as fifo1atc:
        print("AI1: FIFO1ATC opened")
        fifo1atc.write("My name is Ai1")
        print("AI1: Message sent")
