#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  channelise.py
#  
#  
#  
import csv
import argparse
import datetime





#1414.306763, -101.1, 51.512334, -0.120718, Tue Dec  1 17:19:16 2015
	
def main():
	parser = argparse.ArgumentParser(description='Generate a specData.csv file containing a signal')
	parser.add_argument('--frequency','-f', type=float, default = 156.8, help='signal frequency in MHz(default: 156.800MHz)')
	parser.add_argument('--duration','-d', type=int, default=60,help='signal duration in seconds(default: 60 seconds)')
	parser.add_argument('--gap','-g', type=int, default=60,help='no signal duration in seconds(default: 60 seconds)')
	parser.add_argument('--repeats','-r', type=int, default=1,help='number of signal pulses(default: 1)')	
	parser.add_argument('--bandwidth','-b', type=float, default=0.5,help='signal bandwidth in kHz(default: 0.5kHz)')	
	parser.add_argument('--level','-l', type=float, default=-90,help='signal level in dBm(default: -90dBm)')
	parser.add_argument('--x','-x', type=float, default=-52.201284,help='latitude(default: 52.201284)')
	parser.add_argument('--y','-y', type=float, default=-0.437056,help='longitude(default:0.437056)')
				
	args = parser.parse_args()
	
	print args.frequency, args.duration, args.gap, args.repeats
	
	
	now = datetime.datetime.now()
	print now
	# create a specdata.csv with a new spectrum every second
	with open ( 'specData.csv', 'w') as outFile:
		specData = csv.writer(outFile,delimiter=',')
		for r in range (0,args.repeats):
			for g in range(0,args.gap):
				# signal off - no data lines
				now = now + datetime.timedelta(seconds=1)
			for d in range(0,args.duration): 
				specData.writerow([args.frequency,args.level,args.x,args.y,now.strftime(" %a %b  %d %H:%M:%S %Y")])
				now = now + datetime.timedelta(seconds=1) 	
		
if __name__ == '__main__':
	main()

