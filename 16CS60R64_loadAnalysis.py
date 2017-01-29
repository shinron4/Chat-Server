#!/usr/bin/python

import math
import sys

# DEPQ
MAXSIZE = 1023

#Compare two elements

def compare(a, b):
	line1 = a.split(" ");
	line2 = b.split(" ");
	if line1[0] < line2[0]:
		return -1;
	elif line1[0] == line2[0] and line1[1] <= line2[1]:
		return -1;
	else:
		return 1;

# Exchange two elements

def exchange(a, b):
	temp = a
	a = b
	b = temp
	return a, b

# computing the length of the file

def length(filename):
	f = open(filename, "r")
	line = "\n"
	while line != "":
		line = f.readline()
	length = f.tell()
	f.close()
	return length
	
# Parent of an element 

def Parent(i):
	return int(i / 2)

# Left of an element

def Left(i):
	return int(2 * i)

# Right of an element

def Right(i):
	return int(2 * i + 1)


# Check if interval heap is empty

def isEmpty(X):
	if X[0] == 0:
		return True
	else:
		return False

#size of the interval heap

def Size(X):
	return X[0]

# Minimum element in the interval heap

def GetMin(X):
	if X[0] == 0:
		return -1
	return X[1][0]

# Maximum element in the interval heap

def GetMax(X):
	if X[0] == 0:
		return -1
	elif len(X[1]) == 1:
		return X[1][0]
	return X[1][1]

#Inserting an element in the interval heap

def Insert(X, e):
	if X[0] % 2 == 0:
		X.append([e])
		j = 0
	else:
		i = int(math.ceil(X[0]/2.0))
		if compare(X[i][0], e) > 0:
			X[i].insert(0,e)
			j = 0
		else:
			X[i].insert(1,e)
			j = 1
	X[0] += 1
	i = int(math.ceil(X[0]/2.0))
	y = Parent(i)
	if y > 0 and compare(X[i][j], X[y][0]) < 0:
		while y > 0 and compare(X[i][0], X[y][0]) < 0:
			X[i][0], X[y][0] = exchange(X[i][0], X[y][0])
			i = y
			y = Parent(i)
	elif y > 0 and compare(X[i][j], X[y][1]) > 0:
		while y > 0 and (compare(X[i][1], X[y][1]) > 0 if len(X[i]) == 2 else compare(X[i][0], X[y][1]) > 0):
			if len(X[i]) == 2:
				X[i][1], X[y][1] = exchange(X[i][1], X[y][1])
			else:
				X[i][0], X[y][1] = exchange(X[i][0], X[y][1])
			i = y
			y = Parent(i)

# Building an interval heap from file

def BuiltDEPQ(f, offset, length):	#'f' set to the starting point of reading and 'length' gives the length of the block in which we have to check
	A = [0]	
	if f:
		numlines = 0
		size = 0
		line = "\n"
		while numlines < MAXSIZE and (f.tell() - offset) < length:
			line = f.readline()
			if line == "\n":
				continue
			elif line == "":
				break
			line = line.split("\n")[0]
			if A[0] % 2 == 0:
				A.append([line])
			else:
				size = int(math.ceil(A[0]/2.0))
				A[size].append(line)
			A[0] = numlines = numlines + 1
		if numlines > 0:
			Init(A)
	return A

#deleting the minimum element form the interval heap

def DeleteMin(X):
	size = int(math.ceil(X[0]/2.0))
	if size < 1:
		return -1
	X[1][0], X[size][0] = exchange(X[1][0], X[size][0])
	if len(X[size]) == 2:
		minimum = X[size].pop(0)
	elif len(X[size]) == 1:
		minimum = X.pop(size)[0]
	X[0] -= 1
	size = int(math.ceil(X[0]/2.0))
	if size > 1:
		ReinsertLow(X, 1)
	return minimum

#deleting the maximum element form the interval heap

def DeleteMax(X):
	size = int(math.ceil(X[0]/2.0))
	if size < 1:
		return -1
	i = len(X[1]) - 1
	j = len(X[size]) - 1 
	X[1][i], X[size][j] = exchange(X[1][i], X[size][j])
	if j == 1:
		maximum = X[size].pop(1)
	elif j == 0:
		maximum = X.pop(size)[0]
	X[0] -= 1
	size = int(math.ceil(X[0]/2.0))
	if size > 1:
		ReinsertHigh(X, 1)
	return maximum

# Reinsert an element at lower end

def ReinsertLow(X, i):
	left = Left(i)
	right = Right(i)
	if len(X[i]) == 2 and compare(X[i][0], X[i][1]) > 0:
		X[i][0], X[i][1] = exchange(X[i][0], X[i][1])
	minimum = i
	if left <= int(math.ceil(X[0]/2.0)) and compare(X[left][0], X[i][0]) < 0:
		minimum = left
	if right <= int(math.ceil(X[0]/2.0)) and compare(X[right][0], X[minimum][0]) < 0:
		minimum = right
	if minimum != i:
		X[i][0], X[minimum][0] = exchange(X[i][0], X[minimum][0])
		ReinsertLow(X, minimum)

# Reinsert an element at higher end

def ReinsertHigh(X, i):
	left = Left(i)
	right = Right(i)
	if len(X[i]) == 2 and compare(X[i][0], X[i][1]) > 0:
		X[i][0], X[i][1] = exchange(X[i][0], X[i][1])
	minimum = i
	if left <= int(math.ceil(X[0]/2.0)) and (compare(X[left][1], X[i][1]) > 0 if len(X[left]) == 2 else compare(X[left][0], X[i][1]) > 0):
		minimum = left
	if right <= int(math.ceil(X[0]/2.0)) and (compare(X[right][1], X[minimum][1]) > 0 if len(X[right]) == 2 else compare(X[minimum][0], X[i][1]) > 0):
		minimum = right
	if minimum != i:
		if len(X[minimum]) == 2:
			X[i][1], X[minimum][1] = exchange(X[i][1], X[minimum][1])
		elif len(X[minimum]) == 1:
			X[i][1], X[minimum][0] = exchange(X[i][1], X[minimum][0])
		ReinsertHigh(X, minimum)

# Initialization of an interval heap

def Init(X):
	size = int(math.ceil(X[0]/2.0))
	for i in range(int(size/2) + 1, size):
		if compare(X[i][0], X[i][1]) > 0:
			X[i][0], X[i][1] = exchange(X[i][0], X[i][1])

	if size > 0 and len(X[size]) == 2:
		 X[size][0], X[size][1] = exchange(X[size][0], X[size][1])
		
	for i in range(int(size/2), 0, -1):
		ReinsertLow(X, i)
		ReinsertHigh(X, i)

# Partitioning Procedure for Quick sort

def partition(name, offset, length):				#'name' of the input file 'offset' of the block in the file 'length' of the block starting from 'offset'
	filename = open(name, "r")
	filename.seek(offset)							# Setting up the files
	left = open("Left", "w")
	right = open("Right", "w")
	A = BuiltDEPQ(filename, offset, length)			# Reading the middle portion
	line = "\n"
	while (filename.tell() - offset) < length:
		line = filename.readline()					# Dividing the file into left and right file
		if line == "\n":
			continue
		elif line == "":
			break
		line = line.split("\n")[0]
		if compare(line, GetMin(A)) <= 0:
			left.write(line + "\n")
		elif compare(line, GetMax(A)) >= 0:
			right.write(line + "\n")
		else:
			Insert(A, line)
			line = DeleteMin(A)
			if line != -1:
				left.write(line + "\n")
	filename.close()
	filename = open(name, "r+")
	filename.seek(offset)					# Setting up the files
	lptr = offset							# Setting up the left file
	llength = left.tell()
	rlength = right.tell()					# Writing back the left file into parent 'filename'
	left.close()
	right.close()
	left = open("Left", "r")
	line = left.readline()
	while line != "":
		if line == "\n":
			line = left.readline()
			continue
		filename.write(line)
		line = left.readline()
	left.close()
	while A[0] > 0:							# Writing back the sorted DEPQ in parent 'filename'
		filename.write(DeleteMin(A) + "\n")
	rptr = filename.tell()					# Setting the pointer for right
	right = open("Right", "r")				# Writing back the right file into parent 'filename'
	line = right.readline()
	while line != "":
		if line == "\n":
			line = right.readline()
			continue
		filename.write(line)
		line = right.readline()
	right.close()
	filename.close()
	return [[lptr, llength], [rptr, rlength]]

#Making Load statistics File

def loadStatistics(filename):
	f = open(filename, "r")
	mLoad = open("messageLoad.csv", "w")
	cLoad = open("characterLoad.csv", "w")
	cLoad.write("Sender,Characters\n")
	chars = message = numM = []
	c = m = 0
	first = 1
	line = f.readline()
	while line != "":
		if line != "\n":
			if first == 1:
				chars = line.split(" ")
				c = int(chars[2], 10)
				receiver = [chars[1]]
				first = 0
			else:
				line = line.split(" ");
				receiver += [line[1]]
				if line[0] == chars[0]:
					c = c + int(line[2], 10)
				else:
					cLoad.write("%s,%d\n"%(chars[0], c))
					c = int(line[2], 10)
					chars = line
		line = f.readline()
	if len(chars) != 0:
		cLoad.write("%s,%d"%(chars[0], c))
	cLoad.close()
	receiver = sorted(set(receiver))
	mLoad.write("Sender \ Receiver,%s\n"% ','.join(receiver))
	f.close()
	f = open(filename, "r")
	line = f.readline()
	first = 1
	while line != "":
		if line != '\n':
			if first == 1:
				message = line.split(" ")
				m = 1
				numM = [0 for i in range(0, len(receiver))]
				first = 0
			else:
				line = line.split(" ");
				if line[0] == message[0] and line[1] == message[1]:
					m = m + 1
				elif line[0] == message[0]:
					numM[receiver.index(message[1])] = m
					message = line
					m = 1;
				else:
					numM[receiver.index(message[1])] = m
					mLoad.write("%s,%s\n"%(message[0], ','.join(str(e) for e in numM)))
					message = line
					m = 1;
					numM = [0 for i in range(0, len(receiver))]
		line = f.readline()
	if len(message) != 0:
		numM[receiver.index(message[1])] = m
		mLoad.write("%s,%s"%(message[0], ','.join(str(e) for e in numM)))
	mLoad.close()
	f.close()
# External Quick Sort

def ExtQkSrt(filename, offset, length):
	if length == 0:									# Return if the length of the file is 0
		return
	else:
		A = partition(filename, offset, length)		# Partition the file into left and right
		ExtQkSrt(filename, A[0][0], A[0][1])		# Recusively sort the left portion
		ExtQkSrt(filename, A[1][0], A[1][1])		# Recusively sort the right portion


l = length(sys.argv[1])

ExtQkSrt(sys.argv[1], 0, l)
loadStatistics(sys.argv[1])