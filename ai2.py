import os
import errno

FIFO2 = 'mypipe2'

try:
	os.mkfifo(FIFO2)
except OSError as oe:
	if oe.errno != errno.EEXIST:
		raise


print("Opening FIFO2...")
while ( True ):
	with open(FIFO2, 'r') as fifo2:
		print("FIFO2 opened")
		while True:
			data = fifo2.read()
			if len(data) == 0:
				print("Writer closed")
				break
			print('Read: "{0}"'.format(data))


	with open(FIFO2, 'w') as fifo2:
		print("FIFO2 opened")
		fifo2.write("My name is Ai2")
		print("Message sent")


