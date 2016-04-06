#!/usr/bin/python
import os, subprocess

amsDecode = "/usr/local/bin/amsDecode"
path = "/usr/local/bin"
specDataFile = "specData.csv"
f = open("processFile.log", "w")

if os.path.exists(specDataFile):
    os.remove(specDataFile)

for fileName in os.listdir('.'):
    if fileName.endswith('.bin'):
		#print 'file :' + fileName

		cmnd = [amsDecode,
				fileName,
				"-t -95",
				"-b",
				"68",
				"468" ]

		subprocess.call(cmnd,stdout=f)
f.close

