#!/usr/bin/python3

import os, errno, time

FIFO1='mypipe1'

with open(FIFO1, 'r') as fifo1:
    print("OthrTst: FIFO1 opened")
    time.sleep(10)
