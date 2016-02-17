#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  channelise.py
#  
#  
#  
import csv

def channelise(frequency,channel):
	#newFreq = ((frequency/channel) + channel/2)
	channelCentre = format(((frequency + (channel/2)) //channel) * channel,'.4f')
	return channelCentre
	
def channelSize ( frequency ):
	if (frequency > 88) & (frequency < 108):
		return 0.1
	elif (frequency > 194) & (frequency < 230):
		return 5
	elif (frequency > 390) & (frequency < 400):
		return 0.025
	else:  
		return 0.0125  


def main():
	workingFrequency = 0
	workingDatestring = 0
	workingLevel = 0
	workingLat = 0
	workingLon = 0
	workingDateString = 0	
	with open ( 'specData.csv', 'rb') as csvFile:
		with open ( 'channelise.csv','wb' ) as outFile:
			channelisedData = csv.writer(outFile,delimiter=',')
			specRecord = csv.reader(csvFile, delimiter=',')
			for row in specRecord :
				fileFrequency,level,lat,lon,dateString = row
				channel = channelSize ( float(fileFrequency) )
				newFrequency = channelise (float(fileFrequency), channel)
				#print fileFrequency, newFrequency, channel, workingFrequency, workingLevel, workingLat, workingLon, dateString ,workingDateString,'\n'
				if newFrequency == workingFrequency:
					# Another measurement within the same channel as before
					if dateString == workingDateString:
						#same channel and same dateTime
						if float(level) > float(workingLevel):
							workingLevel = level
					else:
						#same channel, new date. Write the record to the output file
						channelisedData.writerow([workingFrequency,workingLevel,workingLat,workingLon,workingDateString])
						workingFrequency = newFrequency
						workingLevel = level
						workingLat = lat
						workingLon = lon
						workingDateString = dateString	
					
				# This is a new channel
				elif ( workingFrequency ):
					# if the channel is non-zero ( i.e. not the first one ) then write the old one to the output csv file
					channelisedData.writerow([workingFrequency,workingLevel,workingLat,workingLon,workingDateString])
					workingFrequency = newFrequency
					workingLevel = level
					workingLat = lat
					workingLon = lon
					workingDateString = dateString
				else:
					#The workingFrequency is zero. This must be the first time round the loop
					workingFrequency = newFrequency
					workingLevel = level
					workingLat = lat
					workingLon = lon
					workingDateString = dateString	
			 
	return 0

if __name__ == '__main__':
	main()

