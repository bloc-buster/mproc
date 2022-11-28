import sys
import time

# print("transpose.py")
# print(sys.argv)
if len(sys.argv) < 5:
	print("Usage: python3 transpose.py inputfile 'delimiter' outputfile printout (true or false)");
	sys.exit()
t1 = time.time()
inputfile = sys.argv[1]
delimiter = sys.argv[2]
delimiter = delimiter.lower()
if delimiter == "none" or delimiter == "false" or delimiter == "blank":
	delimiter = ' '
outputfile = sys.argv[3]
printout = sys.argv[4]
print_out = False
if printout=="true":
	print_out = True
if print_out:
	print("input {0} output {1}\n".format(inputfile,outputfile))
lines = []
with open(inputfile,"r") as f:
	for line in f.readlines():
		words = []
		splits = line.split(delimiter)
		for s in splits:
			words.append(s.lstrip().rstrip())
		lines.append(words)
if print_out:
	for x in range(0,len(lines)):
		s = ""
		for y in range(0,len(lines[0])):
			s += "{0} ".format(lines[x][y])
		print("{0}".format(s))
	print("\n")
t = []
for column in range(0,len(lines[0])):
	tt = []
	for row in range(0,len(lines)):
		#t[column][row] = lines[row][column]
		tt.append(lines[row][column])
	t.append(tt)
with open(outputfile,"w") as f:
	for x in range(0,len(t)):
		s = ""
		for y in range(0,len(t[0])):
			if y < len(t[0]) - 1:
				s += "{0}{1}".format(t[x][y],delimiter)
			else:
				s += "{0}".format(t[x][y])
		f.write("{0}\n".format(s))
if print_out:
	for x in range(0,len(t)):
		s = ""
		for y in range(0,len(t[0])):
			s += "{0} ".format(t[x][y])
		print("{0}".format(s))
	print("\n")
t2 = time.time()
elapsed = t2 - t1
# print("elapsed %s seconds" % (elapsed))
