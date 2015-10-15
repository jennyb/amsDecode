/*
 * processOpsRoom.c
 * 
 * 
 * 
 */
#define _XOPEN_SOURCE /* glibc2 needs this */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#define MAX_NUMBER_OF_FREQUENCIES 400000
#define MAX_TIME 24 * 60 * 60 
#define TIME_SLICE 60 * 15
#define HIT_LINE_LEN 100

typedef struct {
	float  		level;
	uint16_t 	occ;
	double 		lat;
	double		lng;
} readingStr ;

typedef struct {
	double 		frequency;
	readingStr 	reading[MAX_TIME];
} frequencyStr ;

typedef struct {
	double 		frequency;
	uint16_t 	count;
} frequencyCount ;



//int fxCmpFunc (const void * a, const void * b)
//{
//   return ( *(double*)a - *(double*)b );
//}

int fxCmpFunc ( const void *arg1, const void *arg2)
{
	const frequencyCount *a = arg1;
	const frequencyCount *b = arg2;
	if (a->frequency < b->frequency) return -1;
	if (a->frequency > b->frequency) return 1;
	return 0;
}



int main(int argc, char **argv)
{
	FILE *fh, *results;
	char str[HIT_LINE_LEN];
	frequencyCount testFx[MAX_NUMBER_OF_FREQUENCIES]; 
	double binFrequency;
	uint32_t frequencyCounter = 0, maxFrequency = 1, finalMaxFrequency=0; 
	uint16_t frequencyFound=0;
	uint16_t slot, maxSlot=0;
	frequencyStr *frequencies;
	char *ptr;
	struct tm tm;
	struct tm *info;
	time_t readingTime=0;
	time_t minTime=9999999999, maxTime=0, tmpTime;
	double occupancy;
	double rssi;
	double lat, lng;
	double slotLat[MAX_TIME/TIME_SLICE];
	double slotLng[MAX_TIME/TIME_SLICE];
	 

	fh = fopen("exceptions.csv","rb");
	if (!fh) 
	{
		printf("Can't open file exceptions.csv\n");
		exit(1);
	}

	results=fopen("results.csv","w");
	if (results==NULL)
	{
		printf("Error opening results.csv\n");
		exit(1);
	}
	
	
	// first pass to find every distinct frequency and time span
	while ( fgets (str, HIT_LINE_LEN, fh) !=NULL ) 
	{
		//89.675000, 37.460000, 52.658178, 1.721563, Wed Sep 25 18:29:30 2013
		
		ptr = strtok(str, ",");
		//printf( "Frequency:  %s\n", ptr );
		
		binFrequency = 1000000 * atof ( ptr );
		for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
		{
			if ( binFrequency == testFx[frequencyCounter].frequency )
			{
				testFx[frequencyCounter].count++;
				frequencyFound = 1;
				//break; 
			}
		}
		if ( ! frequencyFound )
		{
			testFx[maxFrequency].frequency = binFrequency;
			testFx[maxFrequency].count = 1;			
			maxFrequency++;	
		}
		frequencyFound = 0;

		ptr = strtok(NULL, ",");
		//printf( "Level:  %s\n", ptr );	
		ptr = strtok(NULL, ",");	
		//printf( "Lat %s\n", ptr );
		//lat = atof ( ptr );
		 ptr = strtok(NULL, ",");	
		//printf( "Lon %s\n", ptr ); 
		//lng = atof ( ptr );
		ptr = strtok(NULL, ",");
		//printf("string: %s\n",ptr);
		strptime(ptr+1, "%a %b %d %H:%M:%S %Y", &tm);
		readingTime=mktime(&tm);

		if ( readingTime < minTime )
		{
			minTime = readingTime;
		}
		if ( readingTime > maxTime )
		{
			maxTime = readingTime;	
		} 
	}

	printf("Found %d unique frequencies\n",maxFrequency);
	printf("Earliest reading is %s",ctime(&minTime));
	printf("Last reading is at %s", ctime(&maxTime));	
	printf("Number of 15 minute slots is %f\n", difftime(maxTime, minTime)/(60*15));
		
	// malloc for database for frequency storage 
	frequencies=malloc(sizeof(frequencyStr) *  maxFrequency);
	if ( ! frequencies )
	{
		printf("Failed to allocate frequency storage\n");
		exit(1);
	}
	
	//initialise it 
	for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter ++ )
	{
		for ( slot = 0; slot < maxSlot; slot ++ )
		{
			frequencies[frequencyCounter].reading[slot].level = -255;
		}
	}
	
	maxSlot = difftime(maxTime, minTime)/(60*15);

	//for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter ++ )
	//{
	//	printf( "Frequency: %f,  Count:  %d\n", testFx[frequencyCounter].frequency/1000000, testFx[frequencyCounter].count );
	//} 

	
	// sort the exclusion database
	//qsort ( testFx, maxFrequency, sizeof(double), fxCmpFunc); 
	qsort ( testFx, maxFrequency, sizeof(frequencyCount) , fxCmpFunc); 
	

	//create an array to write the hits to the appropriate frequency and time slot. Ignore those frequencies with less than 10 hits. 
	for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter ++ )
	{

		if ( testFx[frequencyCounter].count > 10 )
		{
			//printf( "Frequency: %f,  Count:  %d\n", testFx[frequencyCounter].frequency/1000000, testFx[frequencyCounter].count );
			finalMaxFrequency++;
			frequencies[finalMaxFrequency].frequency = testFx[frequencyCounter].frequency;
		}
	} 

	rewind(fh);


	//for each line, add this to the approriate slot
	while ( fgets (str, HIT_LINE_LEN, fh) !=NULL ) 
	{
		//89.675000, 37.460000, 52.658178, 1.721563, Wed Sep 25 18:29:30 2013
		
		ptr = strtok(str, ",");
		//printf( "Frequency:  %s\n", ptr );
		
		binFrequency = 1000000 * atof ( ptr );


		ptr = strtok(NULL, ",");
		//printf( "Level:  %s\n", ptr );
		rssi = atof ( ptr );	
		ptr = strtok(NULL, ",");	
		//printf( "Lat %s\n", ptr );
		lat = atof ( ptr );
		 ptr = strtok(NULL, ",");	
		//printf( "Lon %s\n", ptr ); 
		lng = atof ( ptr );
		ptr = strtok(NULL, ",");
		
		strptime(ptr+1, "%a %b %d %H:%M:%S %Y", &tm);
		readingTime=mktime(&tm);
		
		slot = (difftime(readingTime, minTime)/(60*15));

		//for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
		//{
		//	frequencies[frequencyCounter].reading[slot].level = -127 ;
		//}		
		
		
		// wander up the frequency list until we get to the frequency we have found 
		for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
		{
			if ( binFrequency == frequencies[frequencyCounter].frequency )
			{
				frequencyFound = 1;
				// work out time for this reading and assign it to the correct time slice
				frequencies[frequencyCounter].reading[slot].occ++;
				// take the lat/long at add it to the slot info
				slotLat[slot] = lat;
				slotLng[slot] = lng;					
				if ( rssi > frequencies[frequencyCounter].reading[slot].level )
				{
					//printf( "Frequency Found; rssi Level:%f, existing rssi:%f, frequency:%f\n", rssi, frequencies[frequencyCounter].reading[slot].level, frequencies[frequencyCounter].frequency );
					frequencies[frequencyCounter].reading[slot].level = rssi ;
				}
				break; 
			}
		}
		if ( ! frequencyFound )
		{
			//frequencies[frequencyCounter++].frequency = binFrequency;
			//maxFrequency++;	
			//frequencies[frequencyCounter].reading[slot].occ++;
			//printf( "Frequency %f Not found\n", binFrequency );
		}
		frequencyFound = 0;
	}
	
	// now print out the results
	// header
	fprintf(results, "0,");
	for ( slot = 0; slot < maxSlot; slot++ )
	{
		tmpTime = minTime+(slot*60*15);
		info = localtime( &tmpTime );
		strftime(str,80,"%H:%M", info);
		fprintf(results, "%s,", str);
	}
	fprintf(results, "\n");
	
	fprintf(results, "0,");
	for ( slot = 0; slot < maxSlot; slot++ )
	{
		fprintf(results, "%f %f,",slotLat[slot],slotLng[slot] );
	}
	fprintf(results, "\n");
	
	for ( frequencyCounter = 1; frequencyCounter < finalMaxFrequency; frequencyCounter++)
	{
		fprintf(results, "%f,",frequencies[frequencyCounter].frequency/1000000);
		for ( slot = 0; slot < maxSlot; slot++ )
		{
			// we have a problem working out percentage occupancy here
			// we don't know how many bins make a channel ( 7-8 on current setup ), and how many scans per 15 minutes
			// The test system scans about once every 8 seconds - this will vary depending on scanned bandwidth
			// so the max could be 675 - depending on modulation 
			// so, lets assume 2 bins would get the majority, over 90 scans = 180
			// and not let value go over 100. This is a fudge. 
			occupancy = 100 * frequencies[frequencyCounter].reading[slot].occ/180;
			if ( occupancy > 100 )
			{
				occupancy = 100;
			}
			else if ( occupancy == 0 )
			{
				if ( frequencies[frequencyCounter].reading[slot].occ )
				{
					occupancy = 1;  // so that we don't have a row of zeros
				}
			}
			fprintf(results,"%2.1f,",occupancy );
		} 
		fprintf(results,"\n");
		
		fprintf(results," ,");
		// now print signal strengths
		for ( slot = 0; slot < maxSlot; slot++ )
		{
			fprintf(results,"%2.1f,", frequencies[frequencyCounter].reading[slot].level);
		}
		fprintf(results,"\n");
	}
	
	
	return 0;
}

