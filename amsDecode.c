#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "ngr2ll.h"
#include "licenseProcessing.h"


#define VERSION 0.1
#define GNUPLOT "gnuplot -persist"

#define LICENSE_DB_EN 0
#define GNUPLOT_EN 0
#define HITS_DB_LEN 5000000
#define HIT_THRESHOLD -80
#define CHANNEL_DB_LEN 100000
#define CHANNEL_THRESHOLD -90
#define RAISED_NOISE_FLOOR_THRESHOLD -90
#define MAX_SPECTRA 20000
#define WBFM_CHANNEL_HALF 100000



unsigned int xml=0;
unsigned int gnuplot=0; 
unsigned int occupancy=0; 
unsigned int spectrumNumber=0; // spectrum number to capture. 0 = don't bother

typedef struct {
	double 		frequency;
	uint16_t 	occ;
	double		peak;
	double		min;
	double  	ave;
	double		level;
 } binStruct ;

typedef struct {
	double 		latitude;
	double 		longitude;
	double 		frequency;
	uint16_t 	power;
	uint16_t	height;
	uint16_t	type;
	uint16_t	level;
 } licenseStruct ;
 
 

 typedef struct {
	double 		frequency;
	double 		startFx; 	// Hz
	double 		stopFx;	  	// Hz
	double 		centreFx;	// Hz
 } channelStruct;

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




/**************************************************************************** 
* NAME:        writeOcc                                   
* DESCRIPTION: 
* ARGUMENTS:    array contains the points, aSize is the number of bins, and spectra is the number of sweeps
* RETURNS:     0
***************************************************************************/
int writeOcc ( binStruct array[], uint32_t aSize, uint32_t spectra)
{
	unsigned int fcount;
	
	printf("writeOcc: First point %0.1fMHz, bins %d, spectra %d\n", array[0].frequency/1000000, aSize, spectra  );
		
	FILE *fout = fopen("occupancy.csv","w");
	if (!fout) 
	{
		printf("Can't open file occupancy.csv\n");
		exit(1);
	}
	

	fprintf(fout, "Frequency, Occupancy\n");
	for ( fcount=1; fcount < aSize; fcount++ )
	{
		fprintf( fout, "%fMHz, %f, %f, %f\n", array[fcount].frequency/1000000, (((float)(array[fcount].occ )/spectra)*100)-150, (array[fcount].peak ), (array[fcount].min ) );
		//printf( "Spectra:%d, %fMHz, %d\n",spectra, array[fcount].frequency, array[fcount].occ );  
	}

	fclose(fout);
	return (0);
}


/**************************************************************************** 
* NAME:        plotSpectrum                                   
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
int plotSpectrum ( binStruct array[], uint32_t aSize, double strtFx,double stpFx )
{
	FILE *gp;
	uint32_t fcount;

	//printf("First point %d, size %d, Freq %f, stopFx %f, randomFx %f\n", array[0].occ, aSize, strtFx, stpFx, array[0].frequency  );	

	gp=popen(GNUPLOT,"w");
	if (gp==NULL)
	{
		printf("Error opening pipe into GNUPLOT\n");
		exit(1);
	}
	
	fprintf( gp, "set terminal png\n");         	// gnuplot recommends setting terminal before output
	fprintf( gp, "set output \"spectrum.png\"\n"); 
	fprintf( gp, "plot '-' with lines\n");
	
	for ( fcount=0; fcount < aSize ; fcount++ )
	{ 
		if (( array[fcount].frequency > (strtFx * 1000000) ) && (array[fcount].frequency < (stpFx*1000000)))
		{
			fprintf( gp, "%f %f\n", array[fcount].frequency, array[fcount].level ) ;
			//printf("%f %f\n", array[fcount].frequency, array[fcount].level ) ;
		}
	}
	fprintf(gp, "e");
	fflush(gp);
	fclose(gp);

	return (0);
}

/**************************************************************************** 
* NAME:        calcNoiseFloor                                   
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
float calcNoiseFloor ( binStruct array[], uint32_t aSize, double strtFx,double stpFx )
{
	float noiseAccumulator=0;
	uint32_t fcount;
	
	for ( fcount=0; fcount < aSize ; fcount++ )
	{ 
		if (( array[fcount].frequency > (strtFx * 1000000) ) && (array[fcount].frequency < (stpFx * 1000000)))
		{	
			noiseAccumulator += ( array[fcount].min * -1);
		}
	}
	return ( noiseAccumulator );
}

/**************************************************************************** 
* NAME:        saveSpectrum                                    
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
int saveSpectrum ( binStruct array[], uint32_t aSize, double strtFx,double stpFx )
{
	FILE *spc;
	uint32_t fcount;
	
	spc=fopen("spectrum.csv","w");
	if (spc==NULL)
	{
		printf("Error opening spectrum.csv\n");
		exit(1);
	}
	
	fprintf( spc, "Frequency, Value\n");
	for ( fcount=0; fcount < aSize ; fcount++ )
	{ 
		if (( array[fcount].frequency > (strtFx * 1000000) ) && (array[fcount].frequency < (stpFx * 1000000)))
		{
			fprintf( spc, "%f, %f\n", array[fcount].frequency/1000000, array[fcount].level ) ;
			//printf("%f %f\n", array[fcount].frequency, array[fcount].level ) ;
		}
	}
	fflush(spc);
	fclose(spc);
	return (0);
}

/**************************************************************************** 
* NAME:        saveHits                                    
* DESCRIPTION: writes the hits array to 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
int saveHits ( hitsStruct array[], uint64_t aSize )
{
	FILE *of;
	uint64_t fcount;
	
	of=fopen("specData.csv","a");
	if (of==NULL)
	{
		printf("Error opening specdata.csv\n");
		exit(1);
	}
	//fprintf( of, "Frequency, Value, Latitude, Longitude, Time\n");
	for ( fcount=0; fcount < aSize ; fcount++ )
	{ 

		fprintf( of, "%f, %3.1f, %f, %f, %s", array[fcount].frequency/1000000, array[fcount].level+107, array[fcount].latitude, array[fcount].longitude, ctime(&array[fcount].time) ) ;
		//fprintf( of, "%f\n", (array[fcount+1].frequency - array[fcount].frequency)/1000);
		//printf("%f %f\n", array[fcount].frequency, array[fcount].level ) ;
	}
	fflush(of);
	fclose(of);
	return (0);
}



/**************************************************************************** 
* NAME:        saveSingleFrequency                                    
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/
int saveSingleFrequency ( hitsStruct array[] )
{
	FILE *of;
	uint64_t fcount;
	
	of=fopen("singleFrequency.csv","w");
	if (of==NULL)
	{
		printf("Error opening singleFrequency.csv\n");
		exit(1);
	}
	
	fprintf( of, "Frequency, Value\n");
	for ( fcount=1; fcount < MAX_SPECTRA ; fcount++ )
	{ 
		if ( array[fcount].frequency == 0 )
		{
			break; 
		}
		fprintf( of, "%f, %f\n", array[fcount].frequency, array[fcount].level+107 ) ;

	}
	fflush(of);
	fclose(of);
	return (0);
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
	uint32_t overload=0, spanOverload=0, throwAwayOverload=1;
	int16_t ampValue;
	unsigned int i=0 ;
	uint32_t binCounter=0, arraySize =0, maxBinCounter=0, binsPerSpan=0;
	double startFx=20000000; 	// botton of band of interest in Hz
	double stopFx=3000000000;	// top of band of interest in Hz
	char *filename;
	binStruct *freqOcc;
	uint32_t spectrumCount=0;
	double tempValue; 
	hitsStruct *hitsDb;
	hitsStruct *channelResults;
	hitsStruct *singleFrequencyArray;
	hitsStruct *frequencyArray;
	double hitThreshold = HIT_THRESHOLD;
	channelStruct *channelLimits;
	FILE *fh;
	//FILE *outSpecFile;
	uint64_t hitCounter = 0; 		// If we find a signal above a threshold, count it and add it to the Hit array
	double noiseFloor=255;
	double raisedNoiseFloorThreshold = RAISED_NOISE_FLOOR_THRESHOLD;
    int index;
    int c;
    float singleFrequency = 0;

     
    opterr = 0;
     
    while ((c = getopt (argc, argv, "h?gt:s:r:bf:kolx:y:")) != -1)
		switch (c)
	{
		case 's':  // scan number
			spectrumNumber = atoi(optarg);
			break;
		case 'r':  // raised noise threshold
			raisedNoiseFloorThreshold = atof(optarg);
			break;
		case 't':  // signal detection threshold
			hitThreshold = atof(optarg);
			break;
		case 'b':  // overcome a bug where a duplicated field is absent in the .bin file 
			binBug =1;
			break;	
		case 'f':  // output a file containing the magnitude of a single frequency 
			singleFrequency = atof(optarg) * 1000000;
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
     
    printf ("Scan number = %d, raised noise threshold = %g, Signal detection threshold = %g \n", spectrumNumber, raisedNoiseFloorThreshold, hitThreshold );
     
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

	printf("Selected frequencies from %gMHz to %gMHz\n",(startFx/1000000),(stopFx/1000000));
	
	
	fh = fopen(filename,"rb");
	if (!fh) 
	{
		printf("Can't open file %s\n", filename);
		exit(1);
	}




	// open file for full spectrum storage
	//outSpecFile=fopen("fullSpecData.csv","w");
	//if (outSpecFile==NULL)
	//{
		//printf("Error opening fullSpecData.csv\n");
		//exit(1);
	//}
	
	
	// malloc for database for hits data storage 
	hitsDb=malloc(sizeof(hitsStruct) *  HITS_DB_LEN);
	if ( ! hitsDb )
	{
		printf("Failed to allocate hits array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < HITS_DB_LEN; i++ )
		{
			hitsDb[i].frequency = 0;
			hitsDb[i].latitude = 0;
			hitsDb[i].longitude = 0;
			hitsDb[i].level = -255;
			hitsDb[i].time = 0;
		} 
	}
	
	// malloc another database for channelised hits data storage 
	channelResults=malloc(sizeof(hitsStruct) *  CHANNEL_DB_LEN);
	if ( ! channelResults )
	{
		printf("Failed to allocate channel array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < CHANNEL_DB_LEN; i++ )
		{
			hitsDb[i].frequency = 0;
			hitsDb[i].latitude = 0;
			hitsDb[i].longitude = 0;
			hitsDb[i].level = -255;
			hitsDb[i].time = 0;
		} 
	}
	
	
	printf("\n");


	fread(&version,sizeof(version),1,fh);       //printf("Version, %d\n", version);
	fread(&startFreq,sizeof(startFreq),1,fh);   printf("Start Frequency, %gMHz\n", startFreq/1000000);
	fread(&stopFreq,sizeof(stopFreq),1,fh);     printf("Stop  Frequency, %gMHz\n", stopFreq/1000000);
	fread(&span,sizeof(span),1,fh);         	printf("Span %gMHz\n",span/1000000 );
	fread(&fftSize,sizeof(fftSize),1,fh);       printf("FFT Size, %d\n", fftSize ); 
	fread(&averages,sizeof(averages),1,fh);     //printf("Number of averages, %d\n", averages);
	fread(&gpsSource,sizeof(gpsSource),1,fh);   printf("GPS Source, %d\n", gpsSource );
	
	
	// set up an array for occupancy - only allocate for bins we are interested in
	if ( span == 10000000 ) 
	{
		freqRbw = (float)28000000/fftSize; 						// We seem to be calculating RBW given a 28MHz span rather than the reported 10MHz; Humph !
	}
	else
	{
		freqRbw = 56000000/fftSize; 						
	}
	
	arraySize = (float)((stopFx-startFx) /freqRbw);		// we can work out the array size from the freq range and bin size
	binsPerSpan = (float)( span / freqRbw );						// number of bins we expect per segment
	
	
	printf("RBW:%fKHz, Total number of bins:%d, bins per segment:%d\n", freqRbw/1000 ,arraySize, binsPerSpan);



	// write header for full spectrum storage
	//for ( i=0; i < arraySize ; i++ )
	//{ 
	//	fprintf( outSpecFile, "%g, ", (startFx+(i*freqRbw))/1000000);
	//}
	//fprintf( outSpecFile, "\n");
	
	
	
	// malloc for database for the single spectrum data storage 
	freqOcc=malloc(sizeof(binStruct)*arraySize);
	if ( ! freqOcc )
	{
		printf("Failed to allocate occupancy array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < arraySize; i++ )
		{
			freqOcc[i].frequency = 0;
			freqOcc[i].occ = 0;
			freqOcc[i].peak = -255;
			freqOcc[i].min = DBL_MAX;
			freqOcc[i].level = 0;
		} 
	}

	// malloc for database for channel extent
	channelLimits=malloc(sizeof(channelStruct)*CHANNEL_DB_LEN);
	if ( ! channelLimits )
	{
		printf("Failed to allocate channel limit array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < CHANNEL_DB_LEN; i++ )
		{
			channelLimits[i].frequency = 0;
			channelLimits[i].startFx = 0;
			channelLimits[i].stopFx = -255;
		} 
	}

	// malloc for database for single channel extraction
	singleFrequencyArray=malloc(sizeof(hitsStruct)*MAX_SPECTRA);
	if ( ! singleFrequencyArray )
	{
		printf("Failed to allocate single Frequency array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < MAX_SPECTRA; i++ )
		{
			singleFrequencyArray[i].frequency = 0;
		} 
	}

	// malloc for database for single trace 
	frequencyArray=malloc(sizeof(hitsStruct)*arraySize);
	if ( ! frequencyArray )
	{
		printf("Failed to allocate frequency array\n");
		exit(1);
	}
	else 
	{
		for ( i=0; i < arraySize; i++ )
		{
			frequencyArray[i].frequency = 0;
		} 
	}


	while ( !feof(fh)) 
	{
		fread(&LatPosition,sizeof(LatPosition),1,fh);     //printf("%f,", LatPosition);
		fread(&LonPosition,sizeof(LonPosition),1,fh);     //printf("%f\n", LonPosition);
		
		//xmlLocation ( LatPosition, LonPosition );
		
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
			fread(&ampPoints,sizeof(ampPoints2),1,fh);       	//printf("Amplitude Points: %d,",ampPoints2 );
		}
		fread(&numAve,sizeof(numAve),1,fh);         		//printf("Number of averages: %d\n",numAve );
		fread(&overload,sizeof(overload),1,fh);     		// printf("Overload: %d\n", overload);
		fread(&ampPoints,sizeof(ampPoints),1,fh);     		//printf("Amplitude Points(2): %d\n",ampPoints );

		//printf("ampPoints: %d, ampPoints2: %d, binsPerSegment %d\n",ampPoints, ampPoints2, binsPerSpan);
		
		if ( overload )
		{
			spanOverload = 1;
			printf("Overload flag from Agilent Sensor detected on spectrum number %d. Overload = %d\n",spectrumCount, spanOverload );
		}
		
		for ( i = 0; i < ampPoints ; i++ )
		{ 
			fread(&ampValue,sizeof(ampValue),1,fh); 
			ampFx = ( blkStartFx + ( i * freqRbw));
			
			//sort into 12.5kHz channel				
			//ampFx =  12500 * (uint32_t)(( ampFx + 6250 ) / 12500);
			
			if ((ampFx >= startFx ) && ( ampFx <= stopFx ))	
			{
					
				tempValue = ((double)ampValue/50)-174; 				// convert to dBm
				frequencyArray[binCounter].level = tempValue; 
				frequencyArray[binCounter].frequency = ampFx;

				if ( binCounter > arraySize )
				{
					printf("error binCounter greater than array size %d %d\n", binCounter, arraySize );
				}
				else
				{  
					binCounter++;
				}
				
				// detect raised noise floor - we want to know the value of the lowest bin. This is probably the noise floor
				if ( tempValue < noiseFloor )
				{
					//printf("Changed min level from %f to %f on scan number %d\n", noiseFloor, tempValue, spectrumCount);
					noiseFloor = tempValue;
				}

			}
			
			// printf("startFreq %f,blkstart %f, i %d; ampFx %f\n",startFreq, blkStartFx, i, ampFx);		
		
			if (ampFx == startFreq )	 
			{
				printf("processing spectrum number %d\r",spectrumCount );
				//printf("startFreq %f, ampFx %f\n",startFreq, ampFx);

				if ( binCounter > maxBinCounter )
				{
					maxBinCounter = binCounter;
				}

				
				if (  (noiseFloor > raisedNoiseFloorThreshold || spanOverload)  &&  throwAwayOverload ) 
				{
					//printf("Raised Noise Floor detected on spectrum number %d the lowest level during the scan was %f. Overload = %d\n",spectrumCount, noiseFloor, spanOverload );
					spanOverload = 0;
					singleFrequencyArray[spectrumCount].frequency = ampFx;
					singleFrequencyArray[spectrumCount].level = -130;
				}
				else
				{
					// finished reading the spectrum, it is not overloaded so now process it
					for ( binCounter = 0; binCounter < maxBinCounter; binCounter++ )
					{
						ampFx = frequencyArray[binCounter].frequency;
						tempValue = frequencyArray[binCounter].level; 
						//printf("Freq %fMHz,  %f\n", ampFx/1000000, tempValue );
						
						if ( singleFrequency )
						{
							if ( ampFx >= (singleFrequency - (freqRbw/2))  && (ampFx <= (singleFrequency + (freqRbw/2) ) ) )   
							{
									//printf("Freq %fMHz,  %f\n", ampFx/1000000, tempValue );
									singleFrequencyArray[spectrumCount].frequency = ampFx;
									singleFrequencyArray[spectrumCount].level = tempValue;
							}
						}
						

						freqOcc[binCounter].frequency = frequencyArray[binCounter].frequency;
	
					

						
						if ( frequencyArray[binCounter].level > hitThreshold )
						{
							// occupancy
							freqOcc[binCounter].occ++ ;
							
							hitsDb[hitCounter].frequency =  12500 * (uint32_t)(( frequencyArray[binCounter].frequency + 6250 ) / 12500);
							hitsDb[hitCounter].latitude = LatPosition;
							hitsDb[hitCounter].longitude = LonPosition;
							hitsDb[hitCounter].level = frequencyArray[binCounter].level; 
							hitsDb[hitCounter].time = time;
							if ( hitCounter < HITS_DB_LEN )
							{
								hitCounter++;
							}
							else
							{
								 printf("Too many signal hits. Either raise the threshold, lower the band of interest or ask Jenny to extend the hit table length to more than %d\n",HITS_DB_LEN );
								 exit (1);
							}
							//printf("Freq %fMHz,  %f\n", ampFx/1000000, tempValue );
	
						}


						// capture  single spectra
						if ( spectrumNumber )
						{
							if ( spectrumCount == spectrumNumber )
							{
								freqOcc[binCounter].level = tempValue;
							}
						}
						
							
						// peak hold for entire file 
						if ( tempValue > freqOcc[binCounter].peak )
						{
							freqOcc[binCounter].peak = tempValue;
						}

						// min for entire file 
						if ( tempValue < freqOcc[binCounter].min )
						{
							freqOcc[binCounter].min = tempValue;
						}	
						
						// ave for entire file 
						freqOcc[binCounter].ave += tempValue;
					}  // for bincounter..
					
					// full spectrum output
					//fprintf( outSpecFile, "%03d, ",spectrumCount);
					//for ( binCounter = 0; binCounter < maxBinCounter; binCounter++ )
					//{
					//	fprintf( outSpecFile, "%03.3f, ", frequencyArray[binCounter].level+130);
					//}	
					//fprintf( outSpecFile, "\n");
				}  // if noisefloor or overload

				binCounter = 0;
				spectrumCount++;
				noiseFloor = 255;
			} // if startFx
		}  // for all ampPoints
	}  // while not eof

	printf("Processed %ld hits above threshold\n", hitCounter);

	saveHits ( hitsDb, hitCounter );

	writeOcc ( freqOcc, maxBinCounter, spectrumCount );
	
	printf("Hits above threshold %ld\n",hitCounter);
	printf("Processed %d spectra, ending at latitude:%f and longitude:%f and the last timestamp was %s\n",spectrumCount, LatPosition, LonPosition, ctime(&time));  

	exit(0);
}
