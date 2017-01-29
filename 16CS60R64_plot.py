#!/usr/bin/python
import matplotlib.pyplot as plt
import numpy as np
import sys
import csv

N = 0
sender = []
chars = []

# Reading the CSV file and getting the sender and number of chars they sent.

with open(sys.argv[1], 'rb') as csvfile:
    cLoad = csv.reader(csvfile)
    for row in cLoad:
		if N > 0 and len(row) > 0:
			sender.append(row[0])
			chars.append(int(row[1], 10))
		N += 1

# Makin the bar chart from the info about the sender from charLoad.csv
ind = np.arange(len(sender))
width = 0.8
fig = plt.figure()
ax = plt.subplot(111)
ax.bar(ind, chars, width, color='r')
ax.set_ylabel('Number of characters')
ax.set_xticks(ind + width / 2)
ax.set_xticklabels(sender)
plt.savefig("charLoad.png") # saving the image of the plot in charLoad.csv

N = 0
sender = []
receiver = []
message = []

# Reading the message load between the sender and the receiver

with open(sys.argv[2], 'rb') as csvfile:
	mLoad = csv.reader(csvfile)
	for row in mLoad:
		if N == 0 and len(row) > 0:
			receiver = row[1:]
		elif N > 0 and len(row) > 0:
			sender.append(row[0])
			message.append(row[1:])
		N += 1

# making the heat map from the information read.

fig = plt.figure()
ax = plt.subplot(2, 1, 2, aspect = 1)
heatmap = ax.pcolor(np.array(message).astype(np.float), cmap=plt.cm.Blues, alpha=0.8)
indr = np.arange(len(receiver))
indc = np.arange(len(sender))
fig = plt.gcf()
ax.set_frame_on(False)
fig.set_size_inches(8, 8)
ax.set_xticklabels(receiver, minor=False)
ax.set_yticklabels(sender, minor=False)
plt.xticks(indr + 0.5, rotation=90)
plt.yticks(indc + 0.5)
ax.set_xlabel("Receiver")
ax.set_ylabel("Sender")
ax.grid(False)
plt.savefig("messageLoag.png") # saving the image of the plot in messageLoad.csv