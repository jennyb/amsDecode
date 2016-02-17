#!/usr/bin/python
import os, subprocess

amsDecode = "/usr/local/bin/amsDecode"
path = "/usr/local/bin"
specDataFile = "specData.csv"
licenseCheck = "/usr/local/bin/licenseCheck"
processOpsRoom = "/usr/local/bin/processOpsRoom"
channelise = "/usr/local/bin/channelise.py"


if os.path.exists(specDataFile):
    os.remove(specDataFile)

for fileName in os.listdir('.'):
    if fileName.endswith('.bin'):
		print 'file :' + fileName

		cmnd = [amsDecode,
				fileName,
				"-t -95",
				"-b",
				"68",
				"468" ]

		subprocess.call(cmnd)

subprocess.call(channelise)
subprocess.call(licenseCheck)
subprocess.call(processOpsRoom)


