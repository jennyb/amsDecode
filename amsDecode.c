#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "licenseProcessing.h"


#define VERSION 0.6
#define GNUPLOT "gnuplot -persist"

#define LICENSE_DB_EN 0
#define GNUPLOT_EN 0
#define HITS_DB_LEN 10000000
#define HIT_THRESHOLD -80
#define CHANNEL_DB_LEN 100000
#define CHANNEL_THRESHOLD -90
#define RAISED_NOISE_FLOOR_THRESHOLD -90
#define MAX_SPECTRA 20000
#define WBFM_CHANNEL_HALF 100000



/**************************************************************************** 
* NAME:        calcTime                                    
* DESCRIPTION: converts from windows time ( seconds since Bill Gates's Birthday )
*   		   to Unix time  
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
time_t calcTime ( time_t inTime )
{
	// realtime 	000000014804562C
	// rtichardTime 23416F4E08D55000
	// internet 	000000001436227200
	
	return ((time_t)(((uint64_t)inTime - 621355968000000000)/10000000));
	//printf("Time: %016lX %s\n", (uint64_t)(tempTime), ctime(&(tempTime)));
}  



void displayHelp(void)
{
	printf("Decoder for AMSDrive .bin files. Usage: amsDecode [options] <binfile.bin>  [start Fx], [stop FX]\n");
	printf("Where the options are :\n");  
	printf("-s scan number to dump in occupancy.csv ( default 0 - not dumped )\n");
	printf("-r is the raised noise floor threshold in dBm ( default -90 )\n");	
	printf("-g output the spectrum to gnuplot\n");	
	printf("-t signal detection threshold in dBm\n");
	printf("-k output drive as kml\n");	
	printf("-f output a file containing the magnitude of a single frequency\n");
	printf("-b overcome a bug where a duplicated field is absent in the .bin file <sigh>\n");		
	printf("-o don't throw away overloaded spectra\n");	
	printf("-l compare the spectrum to the license database\n");		
	exit (0);	
}


/**************************************************************************** 
* NAME:        	main                                   
* DESCRIPTION: 
* ARGUMENTS:   	-s spectrum number to export
* 				-l compare to license database
* 				-k output kml
* 				-b bottom frequency
* 				-t top frequency 
* RETURNS:     	0
***************************************************************************/
int main(int argc, char *argv[])
{
	int32_t version, binBug=0;
	double startFreq; 	// Frequency of file start in Hz
	double stopFreq;	// Frequency of file stop in Hz
	double span;
	float ampValueDBm;
	int32_t fftSize;
	int32_t averages;
	unsigned char gpsSource;
	double LatPosition = 0, cmdLatPosition=0;
	double LonPosition = 0, cmdLonPosition=0;
	double elevation;
	double speed;
	double bearing;
	//int64_t tmpTime;
	time_t time; 
	int64_t richardTime; 
	double sampleRate;
	int32_t segIndex;
	int32_t segNumber;
	int32_t lastSeg;
	double freqRbw, tmpFreqRbw;		// bin size in Hz
	double blkStartFx; 	// start of the current block of bins in Hz
	double ampFx;		// frequency of current bin in Hz
	uint32_t ampPoints, ampPoints2;
	uint32_t numAve;
	uint32_t overload=0, segmentOverload=0, throwAwayOverload=1;
	int16_t ampValue;
	unsigned int i=0 ;
	uint32_t binCounter=0, arraySize =0, binsPerSpan=0;
	double startFx=20000000; 	// botton of band of interest in Hz
	double stopFx=3000000000;	// top of band of interest in Hz
	char *filename;
	uint32_t spectrumCount=0;
	double hitThreshold = HIT_THRESHOLD;
	FILE *fh;
	FILE *of;
	
	//FILE *outSpecFile;
	uint64_t hitCounter = 0; 		// If we find a signal above a threshold, count it and add it to the Hit array
	//double noiseFloor=255;
    int index;
    int c;
    //float singleFrequency = 0;
 	//uint64_t fcount;   

	
	of=fopen("specData.csv","a");
	if (of==NULL)
	{
		printf("Error opening specdata.csv\n");
		exit(1);
	}
	//fprintf( of, "Frequency, Value, Latitude, Longitude, Time\n");
     
    opterr = 0;
     
    while ((c = getopt (argc, argv, "h?gt:bf:kolx:y:")) != -1)
		switch (c)
	{
		case 't':  // signal detection threshold
			hitThreshold = atof(optarg);
			break;
		case 'b':  // overcome a bug where a duplicated field is absent in the .bin file 
			binBug =1;
			break;	
		case 'o':  // don't throw away overloaded spectra
			throwAwayOverload =0;			
			break;		
		case 'x':  // latitude
			cmdLatPosition = atof(optarg);
			break;
		case 'y':  // longitude
			cmdLonPosition = atof(optarg);
			break;			
							
		case '?':
		case 'h':	
			displayHelp();
			break;
			
		default:
			abort ();
	}
     
     
	index = optind;
	
	if (index  < argc ) 
	{
		filename = argv[index++];
	}
	else          
	{
		displayHelp();
		exit(0);
	}
	
	if (index < argc )
	{
		startFx = (atof(argv[index++])) * 1000000;
 	}
	if (index < argc )
	{
		stopFx = (atof(argv[index++])) * 1000000;
	}       

	printf("amsDecode Version %0.1f, Selected frequencies from %gMHz to %gMHz\n",VERSION, (startFx/1000000),(stopFx/1000000));
	
	
	fh = fopen(filename,"rb");
	if (!fh) 
	{
		printf("Can't open file %s\n", filename);
		exit(1);
	}


	//read header - executed only once per file
	fread(&version,sizeof(version),1,fh);       //printf("Version, %d\n", version);
	fread(&startFreq,sizeof(startFreq),1,fh);   //printf("Start Frequency, %gMHz\n", startFreq/1000000);
	fread(&stopFreq,sizeof(stopFreq),1,fh);     //printf("Stop  Frequency, %gMHz\n", stopFreq/1000000);
	fread(&span,sizeof(span),1,fh);         	//printf("Span %gMHz\n",span/1000000 );
	fread(&fftSize,sizeof(fftSize),1,fh);       //printf("FFT Size, %d\n", fftSize ); 
	fread(&averages,sizeof(averages),1,fh);     //printf("Number of averages, %d\n", averages);
	fread(&gpsSource,sizeof(gpsSource),1,fh);   //printf("GPS Source, %d\n", gpsSource );
	
	printf("bin file; Version:%d, StartFx:%gMHz, StopFx:%gMHz, SegmentSpan:%gMHz, fft:%d  \n", version, startFreq/1000000, stopFreq/1000000, span/1000000, fftSize );
	

	if ( span == 10000000 ) 
	{
		freqRbw = (float)28000000/fftSize; 						// We seem to be calculating RBW given a 28MHz span rather than the reported 10MHz; Humph !
	}
	else if (span == 5000000 )
	{
		freqRbw = (float)14000000/fftSize; 						
	}
	else
	{
		printf("Error, span is neither 5MHz nor 10MHz, something weird is going on here. I blame Agilent. Span : %gMHz", span );
		exit(1);
	}
	
	arraySize = (float)((stopFx-startFx) /freqRbw);		// we can work out the array size from the freq range and bin size
	binsPerSpan = (float)( span / freqRbw );			// number of bins we expect per segment
	
	
	printf("RBW:%fKHz, Total number of bins:%d, bins per segment:%d\n", freqRbw/1000 ,arraySize, binsPerSpan);


	// repeatedly read the next segment header of spectral data. Do this over and over again until file corruption or file end 
	while ( !feof(fh)) 
	{
		segmentOverload=0;
		fread(&LatPosition,sizeof(LatPosition),1,fh);     //printf("%f,", LatPosition);
		fread(&LonPosition,sizeof(LonPosition),1,fh);     //printf("%f\n", LonPosition);
		fread(&elevation,sizeof(elevation),1,fh);       // printf("Elevation: %f\n", elevation);
		fread(&speed,sizeof(speed),1,fh);           //printf("Speed: %f\n", speed );
		fread(&bearing,sizeof(bearing),1,fh);       // printf("Bearing: %f\n",bearing );
		fread(&richardTime,sizeof(richardTime),1,fh);         //printf("Time %s\n ", ctime (unEndian(&time)) ); 
		time = calcTime(richardTime);
		//printf("Time: %016lX %s\n", (uint64_t)(time), ctime(&(time)));
		fread(&sampleRate,sizeof(sampleRate),1,fh);     	//printf("sampleRate: %f,",sampleRate );
		fread(&segIndex,sizeof(segIndex),1,fh);         	//printf("SegmentIndex: %d,",segIndex );
		fread(&segNumber,sizeof(segNumber),1,fh);   		//printf("Segment Number: %d,",segNumber );
		fread(&lastSeg,sizeof(lastSeg),1,fh);       		//printf("Last Segment: %d,", lastSeg);
		fread(&tmpFreqRbw,sizeof(freqRbw),1,fh);       		//printf("Frequency RBW: %fkHz,",freqRbw/1000 );
		fread(&blkStartFx,sizeof(blkStartFx),1,fh);     	//printf("Block Start Frequency: %fMHz,",blkStartFx/1000000 );
		fread(&richardTime,sizeof(richardTime),1,fh);       //printf("Time %s", ctime(&time));
		if ( ! binBug )
		{
			fread(&ampPoints2,sizeof(ampPoints2),1,fh);       	//printf("Amplitude Points: %d,",ampPoints2 );
		}
		fread(&numAve,sizeof(numAve),1,fh);         		//printf("Number of averages: %d\n",numAve );
		fread(&overload,sizeof(overload),1,fh);     		 //printf("Overload: %d\n", overload);
		fread(&ampPoints,sizeof(ampPoints),1,fh);     		//printf("Amplitude Points(2): %d\n",ampPoints );
		
		//printf("Latitude: %f, Longitude: %f, ampPoints: %d, ampPoints2: %d, binsPerSegment %d, RBW: %fkHz \n",LatPosition, LonPosition, ampPoints, ampPoints2, binsPerSpan, freqRbw/1000);


		if ( blkStartFx < startFreq )  //This is the first segment on a new complete spectrum. Show progress by telling the user 
		{
			printf("Spectrum number %d starting at Lat/Long %f,%f on %s\n",spectrumCount++,LatPosition,LonPosition, ctime(&(time)));
		}

		if ( ampPoints == 11703 )
		{
			if ( (blkStartFx > 10000000) || (blkStartFx < 4000000000) ) // make sure the start frequency of the block is between 10MHz and 4GHz  
			{
				if ( overload )
				{
					segmentOverload=1;
					printf("Overload flag from Agilent Sensor detected on spectrum number %d, frequency Segment %f at location Lat/Long %f,%f\n",spectrumCount,blkStartFx/1000000,LatPosition,LonPosition );
				}	
				// now we have the number of amplitude points, read them in until we reach the end of the expected number. We should then have a new spectral header 
				for ( i = 0; i < ampPoints ; i++ )
				{ 
					fread(&ampValue,sizeof(ampValue),1,fh); 
					ampFx = ( blkStartFx + ( i * freqRbw));
					
					//sort into 12.5kHz channel				
					//ampFx =  12500 * (uint32_t)(( ampFx + 6250 ) / 12500);
					
					if ((ampFx >= startFx ) && ( ampFx <= stopFx ))	
					{
						if ( cmdLatPosition || cmdLonPosition )  // we can pick up a lat/lon from the command line and overide the reported position
						{
							LatPosition = cmdLatPosition;
							LonPosition = cmdLonPosition;
						}

						ampValueDBm = (double)ampValue/50-174;

						if ( (ampValueDBm > hitThreshold)  && ( LatPosition || LonPosition ) && (!segmentOverload) )
						{
							fprintf( of, "%f, %3.1f, %f, %f, %s", ampFx/1000000, ampValueDBm, LatPosition, LonPosition, ctime(&time) ) ;
						}
					}
					else
					{  
						binCounter++;
					} // Frequency within the limits set from the command line
				} // for all ampPoints
			} // blockstart or stop frequency incorrect
			else
			{ 
				printf("Block start or stop frequency incorrect. This probably means a corrupt .bin file \n" );
				exit(1);
			} 
		}  // ampPoint value incorrect
		else
		{
			printf("Number of bins ( amplitude values )  unexpected with a value of %d. This probably means a corrupt .bin file \n", ampPoints );
			exit(1);
		} 
	}  // while not eof



	fclose(of);
	printf("Processed %ld hits above threshold\n", hitCounter);

	
	printf("Hits above threshold %ld\n",hitCounter);
	printf("Processed %d spectra, ending at latitude:%f and longitude:%f and the last timestamp was %s\n",spectrumCount, LatPosition, LonPosition, ctime(&time));  

	exit(0);
}
