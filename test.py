import os

def run():
  os.system("./controller > tmp.txt")
  #print("Waited?")
  # Read line by line and hold the last 2 lines in last_lines list.
  with open("tmp.txt", "r") as file:
    i=0
    lines_size = 2
    last_lines = []
    for line in file:
      if i < lines_size:
        last_lines.append(line)
      else:
        last_lines[i%lines_size] = line
      i = i + 1
  
  # Shift the last line index to the end of list:
  #   The last line index can be in the middle of the list.
  last_lines = last_lines[(i%lines_size):] + last_lines[:(i%lines_size)]
  
  # Display the last 5 lines.
  #   Will display the 5th line from the end, 4th line from the end,
  #       and so on.
  for line in last_lines:
    if line != 'PLAYER[2] RECORD W-L-T: 10 0 0\n' and line != 'PLAYER[0] RECORD W-L-T: 0 10 0\n':
      print(last_lines)
      return 1
  return 0


count = 0
for i in range(1000):
  count += run()
print("The amount of bad runs =", count)