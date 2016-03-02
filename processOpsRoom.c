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
#include <unistd.h>


#define MAX_NUMBER_OF_FREQUENCIES 400000
#define MAX_TIME 24 * 60 * 60 
#define TIME_SLICE 60 * 15
#define HIT_LINE_LEN 100
#define VERSION 0.5

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



int fxCmpFunc ( const void *arg1, const void *arg2)
{
	const frequencyCount *a = arg1;
	const frequencyCount *b = arg2;
	if (a->frequency < b->frequency) return -1;
	if (a->frequency > b->frequency) return 1;
	return 0;
}


void displayHelp(void)
{
	printf("processOpsRoom Version:%2.1f\n", VERSION);
	printf("Display the file exceptions.csv in a format that is compatible with a spreadsheet\n");
	printf("Usage: processOpsRoom [-t]\n");
	printf("Where the options are :\n");  
	printf("-t minimum percentage to display\n");
	exit (0);	
}


int main(int argc, char **argv)
{
	FILE *fh, *results;
	char str[HIT_LINE_LEN];
	frequencyCount *testFx; 
	frequencyStr *frequencies;	
	double binFrequency;
	uint32_t frequencyCounter = 0, maxFrequency = 0, finalMaxFrequency=0; 
	uint16_t frequencyFound=0;
	uint16_t slot, maxSlot=0;
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
	float  slotOccupancy;
	float  minOccupancy;
	int c;
	 

    opterr = 0;
     
    while ((c = getopt (argc, argv, "h?t:")) != -1)
		switch (c)
	{
		case 't':  // minimum percentage threshold
			minOccupancy = atof(optarg);
				printf("Minimum occupancy percentage displayed:%2.1f\n",minOccupancy);
			break;
							
		case '?':
		case 'h':	
			displayHelp();
			break;
			
		default:
			abort ();
	}
     
     
	printf("processOpsRoom Version:%2.1f\n", VERSION);

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
	
	// malloc for database for frequency storage
	testFx=malloc(sizeof(frequencyCount) *  MAX_NUMBER_OF_FREQUENCIES);
	if ( ! testFx )
	{
		printf("Failed to allocate testFx frequency storage.");
		exit(1);
	}
	
	// first pass to find every distinct frequency and time span
	while ( fgets (str, HIT_LINE_LEN, fh) !=NULL ) 
	{
		//89.675000, 37.460000, 52.658178, 1.721563, Wed Sep 25 18:29:30 2013
		
		ptr = strtok(str, ",");
		//printf( "Frequency:  %s\n", ptr );
		
		// compare the latest frequency to all the frequencies found. If it is not in the list then add a new one
		binFrequency = 1000000 * atof ( ptr );
		for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
		{
			if ( binFrequency == testFx[frequencyCounter].frequency )
			{
				testFx[frequencyCounter].count++;
				//printf("Found Frequency:%fMHz again,hits so far:%d \n",binFrequency/1000000,testFx[frequencyCounter].count);
				frequencyFound = 1;
				//break; 
			}
		}
		// add a new frequency 
		if ( ! frequencyFound )
		{
			testFx[maxFrequency].frequency = binFrequency;
			testFx[maxFrequency].count = 1;			
			//printf("Found New Frequency number:%d Frequency:%fMHz\n",maxFrequency,testFx[maxFrequency].frequency/1000000);
			maxFrequency++;	
		}
		frequencyFound = 0;

		//Ignore all the fields on the first pass apart from frequency and time
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
		
		// work out the earliest reading and latest reading	
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


	qsort ( testFx, maxFrequency, sizeof(frequencyCount) , fxCmpFunc); 
	

	//create an array to write the hits to the appropriate frequency and time slot. Ignore those frequencies with less than 10 hits. 
	for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter ++ )
	{

		if ( testFx[frequencyCounter].count )
		{
			//printf( "Frequency: %f,  Count:  %d\n", testFx[frequencyCounter].frequency/1000000, testFx[frequencyCounter].count );
			frequencies[finalMaxFrequency].frequency = testFx[frequencyCounter].frequency;
			finalMaxFrequency++;
		}
	} 

	rewind(fh);

	for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
	{
		for ( slot = 0; slot < maxSlot; slot++ )
		{
			frequencies[frequencyCounter].reading[slot].level = -127 ;
		}
	}	

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
		//printf( "Lon %s\n", ptr); 
		lng = atof ( ptr );
		ptr = strtok(NULL, ",");
		
		strptime(ptr+1, "%a %b %d %H:%M:%S %Y", &tm);
		readingTime=mktime(&tm);
		
		slot = (difftime(readingTime, minTime)/(60*15));

	
		
		
		// wander up the frequency list until we get to the frequency we have found 
		for ( frequencyCounter = 0; frequencyCounter < maxFrequency; frequencyCounter++ )
		{
			if ( binFrequency == frequencies[frequencyCounter].frequency )
			{
				//printf( "Frequency Found; rssi Level:%f, existing rssi:%f, frequency:%f\n", rssi, frequencies[frequencyCounter].reading[slot].level, frequencies[frequencyCounter].frequency );
				frequencyFound = 1;
				// work out time for this reading and assign it to the correct time slice
				frequencies[frequencyCounter].reading[slot].occ++;
				// take the lat/long and add it to the slot info
				slotLat[slot] = lat;
				slotLng[slot] = lng;					
				if ( rssi > frequencies[frequencyCounter].reading[slot].level )
				{
					// printf( "Frequency Found; rssi Level:%f, existing rssi:%f, frequency:%f\n", rssi, frequencies[frequencyCounter].reading[slot].level, frequencies[frequencyCounter].frequency );
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
	// time header
	fprintf(results, "0,");
	for ( slot = 0; slot < maxSlot; slot++ )
	{
		tmpTime = minTime+(slot*60*15);
		info = localtime( &tmpTime );
		strftime(str,80,"%H:%M", info);
		fprintf(results, "%s,", str);
	}
	fprintf(results, "\n");
	
	// Location header
	fprintf(results, "0,");
	for ( slot = 0; slot < maxSlot; slot++ )
	{
		fprintf(results, "%f %f,",slotLat[slot],slotLng[slot] );
	}
	fprintf(results, "\n");
	
	for ( frequencyCounter = 0; frequencyCounter < finalMaxFrequency; frequencyCounter++)
	{
		slotOccupancy = 0;
		for ( slot = 0; slot < maxSlot; slot++ )
		{
			//check each slot for occupancy. If none are above the occupancy threshold then throw away the whole line
			occupancy = 100 * frequencies[frequencyCounter].reading[slot].occ/900;
			if ( occupancy > slotOccupancy )
			{
				slotOccupancy = occupancy;
			}
		}
		
		
		if ( slotOccupancy > minOccupancy )
		{
			// print the first field : Frequency
			fprintf(results, "%fMHz,",frequencies[frequencyCounter].frequency/1000000);
			for ( slot = 0; slot < maxSlot; slot++ )
			{
				// we have a problem working out percentage occupancy here
				// assuming channelise reduces the results to 1 per second
				// we have 900 possible seconds to find hits
				// and not let value go over 100. This is a fudge. 
				occupancy = 100 * frequencies[frequencyCounter].reading[slot].occ/900;
				if ( occupancy > 100 )
				{
					occupancy = 100;
				}
				else if ( occupancy == 0 )
				{
					if ( frequencies[frequencyCounter].reading[slot].occ )
					{
						occupancy = 0;  // so that we don't have a row of zeros
					}
				}
				fprintf(results,"%2.1f%%,",occupancy );
			} 
			fprintf(results,"\n");
			
			fprintf(results," ,");
			// now print signal strengths
			for ( slot = 0; slot < maxSlot; slot++ )
			{
				if (frequencies[frequencyCounter].reading[slot].level > -120 )
				{
					fprintf(results,"%2.1fdBm,", frequencies[frequencyCounter].reading[slot].level);
				}
				else
				{
					fprintf(results,",");
				}
			}
			fprintf(results,"\n");
		}
	}
	free(testFx);
	free(frequencies);
		
	return 0;
}

