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
#define HIT_LINE_LEN 100
#define AMS_LOCATION_MOVE_DISTANCE 5



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

		fprintf( of, "%f, %f, %f, %f, %s", array[fcount].frequency/1000000, array[fcount].level+107, array[fcount].latitude, array[fcount].longitude, ctime(&array[fcount].time) ) ;
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
* ARGUMENTS:   	-h  returns help
* 				
* RETURNS:     	0
***************************************************************************/
int main(int argc, char *argv[])
{
	double lat, lng, startLat, startLng, distance ;
	double binFrequency;
	char str[HIT_LINE_LEN], resultStr[HIT_LINE_LEN];
	char *ptr;
	FILE *fh;
	uint32_t validSignalsTotal;
	signalListStruct *validSignals;
	hitsStruct *targetFrequencies;
	FILE *results;	
    int c;
    uint32_t validHits=0, inValidHits=0;
     
    opterr = 0;
     
    while ((c = getopt (argc, argv, "h?")) != -1)
		switch (c)
	{
			
		case '?':
		case 'h':	
			displayHelp();
			break;
			
		default:
			abort ();
	}

	
	fh = fopen("specData.csv","rb");
	if (!fh) 
	{
		printf("Can't open file specData.csv\n");
		exit(1);
	}


	results=fopen("exceptions.csv","w");
	if (results==NULL)
	{
		printf("Error opening exceptions.csv\n");
		exit(1);
	}
	
	// work out all signals within MAX_DISTANCE and put them into list[]

	// create array for frequency 
	targetFrequencies=malloc(sizeof(hitsStruct) * 1000000);
	if ( ! targetFrequencies )
	{
		printf("Failed to allocate target signals array\n");
		exit(1);
	}	

	// create array for exclusion database
	validSignals=malloc(sizeof(signalListStruct) * LICENSE_FILE_LEN);
	if ( ! validSignals )
	{
		printf("Failed to allocate valid signals array\n");
		exit(1);
	}		

	// from the first line of the specData file, extract the starting Latitude and Longitude
	while  ( fgets (str, HIT_LINE_LEN, fh) != NULL )
	{
		ptr = strtok(str, ",");
		//printf( "Frequency:  %s\n", ptr );
		ptr = strtok(NULL, ",");
		//printf( "Level:  %s\n", ptr );	
		ptr = strtok(NULL, ",");	
		//printf( "Lat %s\n", ptr );
		lat = atof ( ptr );
		ptr = strtok(NULL, ",");	
		//printf( "Lon %s\n", ptr ); 
		lng = atof ( ptr );
		if ( lat != 0 )
		{
			break; // we are rather UK centric here. Ignore latitude zero, but of course accept longitude of zero )
		}
		//printf("zero lat lat%f, long%f\n",lat,lng);
	}		
	  
	//set this lat/long as the starting position, then compare each line and recalculate licenses when we have moved. 
	startLat = lat;
	startLng = lng; 
	  
	
	validSignalsTotal = openLicenseCsv( validSignals, lat, lng );
	printf("Found  %d license records within %dkm of the sensor location at %f and %f\n",validSignalsTotal,LICENSE_DISTANCE, lat, lng);	


	while ( fgets (str, HIT_LINE_LEN, fh) !=NULL  ) 
	{
		//89.675000, 37.460000, 52.658178, 1.721563, Wed Sep 25 18:29:30 2013
		strcpy ( resultStr, str );	
		ptr = strtok(str, ",");
		binFrequency = 1000000 * atof ( ptr );
		ptr = strtok(NULL, ",");
		//printf( "Level:  %s\n", ptr );	
		ptr = strtok(NULL, ",");	
		//printf( "Lat %s\n", ptr );
		lat = atof ( ptr );
		ptr = strtok(NULL, ",");	
		//printf( "Lon %s\n", ptr ); 
		lng = atof ( ptr);
		
		if ( lat == 0 )
		{
			//printf("zero lat lat%f, long%f\n",lat,lng);
			continue; // ignore, get the next record
		}
		
		distance =  geoDistance(startLat, startLng, lat, lng, 'K');
		if ( distance > AMS_LOCATION_MOVE_DISTANCE )
		{
			startLat = lat;
			startLng = lng; 
			validSignalsTotal = openLicenseCsv( validSignals, lat, lng );
			printf("Location moved by %.1fkm, Found %d license records within %dkm of the sensor location at %f and %f\n",distance, validSignalsTotal,LICENSE_DISTANCE, lat, lng);
		}

		if ( testFrequency ( binFrequency, validSignals, validSignalsTotal ) )
		{
			//printf("****Valid : %s\n",str);
			validHits++;
		}
		else
		{
			//printf("--Invalid : %s\n",str);
			fputs(resultStr,results);
			inValidHits++;
		}
	}
		
	printf("Found %d hits with corresponding license and %d without a license\n",validHits,inValidHits);		
		
	
	fclose(fh);
	fclose(results);

	exit(0);
}
