#!/usr/bin/python3

import os
import errno

FIFO2 = 'mypipe2'

try:
	os.mkfifo(FIFO2)
except OSError as oe:
	if oe.errno != errno.EEXIST:
		raise


while ( True ):
	with open(FIFO2, 'r') as fifo2:
		print("AI2: FIFO2 opened")
		while True:
			data = fifo2.read()
			if len(data) == 0:
				print("AI2: Writer closed")
				break
			print('AI2: Read: "{0}"'.format(data))

	with open(FIFO2, 'w') as fifo2:
		print("AI2: FIFO2 opened")
		fifo2.write("My name is Ai2")
		print("AI2: Message sent")
