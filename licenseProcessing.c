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


 

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double deg) 
{
  return (deg * pi / 180);
}

 
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double rad2deg(double rad) 
{
  return (rad * 180 / pi);
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::                                                                         :*/
/*::  This routine calculates the distance between two points (given the     :*/
/*::  latitude/longitude of those points). It is being used to calculate     :*/
/*::  the distance between two locations using GeoDataSource(TM) products.   :*/
/*::                                                                         :*/
/*::  Definitions:                                                           :*/
/*::    South latitudes are negative, east longitudes are positive           :*/
/*::                                                                         :*/
/*::  Passed to function:                                                    :*/
/*::    lat1, lon1 = Latitude and Longitude of point 1 (in decimal degrees)  :*/
/*::    lat2, lon2 = Latitude and Longitude of point 2 (in decimal degrees)  :*/
/*::    unit = the unit you desire for results                               :*/
/*::           where: 'M' is statute miles                                   :*/
/*::                  'K' is kilometers (default)                            :*/
/*::                  'N' is nautical miles                                  :*/
/*::  Worldwide cities and other features databases with latitude longitude  :*/
/*::  are available at http://www.geodatasource.com                          :*/
/*::                                                                         :*/
/*::  For enquiries, please contact sales@geodatasource.com                  :*/
/*::                                                                         :*/
/*::  Official Web site: http://www.geodatasource.com                        :*/
/*::                                                                         :*/
/*::           GeoDataSource.com (C) All Rights Reserved 2013                :*/
/*::                                                                         :*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double geoDistance(double lat1, double lon1, double lat2, double lon2, char unit) 
{
	double theta, dist;
	theta = lon1 - lon2;
	
	dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	
	switch(unit) 
	{
    case 'M':
      break;
    case 'K':
      dist = dist * 1.609344;
      break;
    case 'N':
      dist = dist * 0.8684;
      break;
	}
	return (dist);
}

/**************************************************************************** 
* NAME:        signalDistance                                    
* DESCRIPTION: works out a very rough distance from the horrible text of ngr 
* ARGUMENTS:    
* RETURNS:     signal distance in km
***************************************************************************/  
double ngrDistance ( double lat,double lng, char *country1, char *country2, char *ngr  )
{
	uint16_t i;
	
	static struct ngrLookupStruct
	{
		char 	ngr[5];
		char 	country1[18]; 
		char 	country2[10];
		double  lat;
		double  lon;
	}
	ngrLookup[] = { {"SV","England","", 50.246,-6.911},
	                  {"SW","England","",50.297,-5.511},
	                  {"SX","England","",50.330,-4.109},
	                  {"SY","England","",50.348,-2.704},
	                  {"SZ","England","",50.348,-1.3},
	                  {"TV","England","",50.331,0.106},
	                  {"SR","Wales","", 51.194388, -5.579542},
	                  {"SS","England","Wales",51.229421, -4.149717},
	                  {"ST","England","Wales",51.246969, -2.717735},
	                  {"SU","England","",51.246989, -1.285031},
	                  {"TQ","England","",51.229482, 0.146955},
	                  {"TR","England","",51.194489, 1.576786},
	                  {"SM","Wales","",52.091722, -5.651005},
	                  {"SN","Wales","",52.127894, -4.192711},
	                  {"SO","England","Wales",52.146013, -2.732105},
	                  {"SP","England","",52.146034, -1.270726},
	                  {"TL","England","",53.026267, 0.235122},
	                  {"TM","England","",52.091825, 1.648184},
	                  {"SH","Wales","",53.026202, -4.238019},
	                  {"SJ","England","Wales",53.044916, -2.747250},
	                  {"SK","England","",53.044937, -1.255651},
	                  {"TF","England","",53.026267, 0.235122},
	                  {"TG","England","",52.988951, 1.723418},
	                  {"SC","England","",53.924346, -4.285810},
	                  {"SD","England","",53.943679, -2.763226},
	                  {"SE","England","",53.943701, -1.239749}, 
	                  {"TA","England","",54.782439, -5.889575},
	                  {"NW","Northern_Ireland","",54.782439, -5.889575},
	                  {"NX","England","Scotland",54.822323, -4.336271},
	                  {"NY","England","Scotland",54.842304, -2.780095},
	                  {"NZ","England","Scotland",54.842327, -1.222957},
	                  {"OV","England","",54.822392, 0.333224},
	                  {"NR","Scotland","",55.678902, -5.978189},
	                  {"NS","Scotland","",55.720134, -4.389608},
	                  {"NT","Scotland","",55.740791, -2.797926},
	                  {"NU","Scotland","",55.740815, -1.205207},
	                  {"NL","Scotland","",56.511336, -7.692291},
	                  {"NM","Scotland","",56.575134, -6.071947},
	                  {"NN","Scotland","",56.617777, -4.446048 },
	                  {"NO","Scotland","",56.639142, -2.816797 },
	                  {"NF","Scotland","",57.405124, -7.830669},
	                  {"NG","Scotland","",57.471129, -6.171267},
	                  {"NH","Scotland","",57.515251, -4.505847},
	                  {"NJ","Scotland","",57.537358, -2.836792},
	                  {"NK","Scotland","",57.537383, -1.166519},
	                  {"NA","Scotland","",58.298558, -7.977409 },
	                  {"NB","Scotland","",58.366878, -6.276616 },
	                  {"NC","Scotland","",58.412554, -4.569285 },
	                  {"ND","Scotland","",58.435441, -2.858005}
	};
	
	printf( "The size of ngrLookup is %ld, lat %f, long %f \n", sizeof(ngrLookup)/sizeof(ngrLookup[0]), lat,lng );
	
	for ( i = 0; i < sizeof(ngrLookup)/sizeof(ngrLookup[0]); i++ )
	{
		if ( geoDistance( lat,lng, ngrLookup[i].lat, ngrLookup[i].lon,'K') < 60 )
		{
			printf( "You are located in square %s located in %s\n", ngrLookup[i].ngr, ngrLookup[i].country1 );
			strcpy ( ngr,  ngrLookup[i].ngr );
			strcpy ( country1,  ngrLookup[i].country1 );
			strcpy ( country2,  ngrLookup[i].country2 );						
			return 1;
		}
	}
	printf( "Location not found\n");
	
	return 0;	
	            
}


/**************************************************************************** 
* NAME:        signalCmpFunc                                    
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/ 
int signalCmpFunc ( const void *arg1, const void *arg2)
{
	const signalListStruct *a = arg1;
	const signalListStruct *b = arg2;
	if (a->lowFrequency < b->lowFrequency) return -1;
	if (a->lowFrequency > b->lowFrequency) return 1;
	return 0;
}


/**************************************************************************** 
* NAME:        openLicenseCsv                                    
* DESCRIPTION: Opens two .csv files : license1.csv containing fixed licenses and license2.csv containing area defined licenses. 
* ARGUMENTS:    
* RETURNS:     0
***************************************************************************/ 
uint32_t openLicenseCsv ( signalListStruct list[], double lat, double lng) 
{
	char str[LICENSE_LINE_LEN]; 
	char str1[LICENSE_LINE_LEN]; 
	char *ptr;
	double licenseLatitude, licenseLongitude, licensePower, licenseHeight, licenseFrequency;
	double licenseDistance;	
	uint32_t frequencyCounter=0;
	char locationString[100];
	char country1[20], country2[2];
	char ngr[10];
	
	// create a file with all the frequencies within 50km that might be busy and will therefore be excluded ( for debug only - not actually used )
	FILE *fDebug = fopen("debug_licenses_found.csv","w");
	if (!fDebug) 
	{
		printf("Can't open file debug_licenses_found.csv\n");
		exit(1);
	}
	
	
	//Firstly open the fixed license database
	//Licence number,Latitude (number),Longitude (number),Station Type,Antenna erp,Antenna height,Frequency,Licence state,Height asl,Product
	//"0000000/1","55.8123456","-3.84123456","Base station","25","8","141.825","Live","261","312345 - Coastal Station Radio (Hobbes)"," "
	FILE *fh = fopen("license1.csv","rb");
	if (!fh) 
	{
		printf("Can't open file license1.csv\n");
		exit(1);
	}


	if ( fgets (str, LICENSE_LINE_LEN, fh) != NULL ) 
	{
		if ( strstr(str, "Licence number") != NULL )
		{
			while ( fgets (str, LICENSE_LINE_LEN, fh) !=NULL ) 
			{
				if ( strlen ( str ) > 50 )
				{
					strcpy(str1,str);
					//printf("%s",str);
					ptr = strtok(str, ","); 
					ptr = strtok(NULL, ",");  // License Number
					licenseLatitude = atof(ptr); // Lat
					//printf( " %f", licenseLatitude );
					ptr = strtok(NULL, ","); // Long
					licenseLongitude = atof(ptr);
					//printf( "  %f", licenseLongitude );
					ptr = strtok(NULL, ","); // Type ( Base Station )
					ptr = strtok(NULL, ","); // Power ERP
					licensePower = atof(ptr);		
					ptr = strtok(NULL, ","); // Height
					licenseHeight = atof(ptr);	// 	
					ptr = strtok(NULL, ",");  // Frequency
					licenseFrequency = 1000000 * atof(ptr);	
					//printf( "  %fMHz\n", licenseFrequency/1000000 );

					//validate data
					if ( licenseLatitude < SOUTH_MOST_LICENSE || licenseLatitude > NORTH_MOST_LICENSE )
					{
						printf("Latitude Error:  licenseLatLong:%f,%f                License:%s", licenseLatitude, licenseLongitude, str1 );
					}
					if ( licenseLongitude < WEST_MOST_LICENSE || licenseLongitude > EAST_MOST_LICENSE )
					{
						printf("Longitude Error: licenseLatLong:%f,%f                License:%s", licenseLatitude, licenseLongitude, str1 );
					}

					if ( ! licenseHeight || ! licensePower )
					{
						//printf("License Database query Height %f, Power %f\n", licenseHeight, licensePower); 
					}
					
					licenseDistance = geoDistance( lat,lng, licenseLatitude, licenseLongitude,'K');
					// printf("license Distance: %f, lat %f,lng %f, licenseLatitude %f, licenseLongitude %f\n", licenseDistance, lat,lng, licenseLatitude, licenseLongitude );     
					
					if ( licenseDistance < LICENSE_DISTANCE )
					{
						list[frequencyCounter].lowFrequency = (licenseFrequency - (CHANNEL_SPACE/2)) ;
						list[frequencyCounter].highFrequency = (licenseFrequency + (CHANNEL_SPACE/2)) ;
						// printf("license Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
						fprintf(fDebug,"License Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
						frequencyCounter++;			
					}
					//else // Debug only 
					//{
						//fprintf(fDebug,"Rejected : license Distance:%f, amsLat:%f, amsLong:%f, licenseLatitude %f, licenseLongitude %f, Low Frequency:%f, high Frequency:%f\n", licenseDistance, lat, lng, licenseLatitude, licenseLongitude, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
					//}	
				}
				else
				{
					fclose(fh);
				}
			} // while there are more lines
		} // first line contains "License Number"
		else
		{
			printf("Incorrect first line in license1.csv. Was expecting lat/long based licenses starting with the first line of License Number, Instead got %s\n", str);
			exit(1); 
		} 
	}  // Able to get first line of file
	else
	{
		printf("Opened license1.csv but was unable to get the first line\n");
		exit(1);
	}	
	
	// then open the area licenses
	//"0038611/2","England","440.89500 MHz","409510 BR Area Assigned","Live"	
	fh = fopen("license2.csv","rb");
	if (!fh) 
	{
		printf("Can't open file license2.csv\n");
		exit(1);
	}





	if ( fgets (str, LICENSE_LINE_LEN, fh) !=NULL ) 
	{
		puts(str);
	}
	while ( fgets (str, LICENSE_LINE_LEN, fh) !=NULL ) 
	{
		if ( strlen ( str ) > 40 )
		{
			ptr = strtok(str, ",");  // License
			//printf( " %s\n", ptr );
			ptr = strtok(NULL, ","); // Location string 
			//printf( " %s\n", ptr ); 
			strcpy ( locationString, ptr );  
			ptr = strtok(NULL, ","); // Frequency 
			licenseFrequency = 1000000 * atof(ptr);
			ptr = strtok(NULL, ",");
			//printf("%s\n",locationString);
			if ( strlen ( locationString) == 3 )
			{
				//"NUa - BNG_Subsquares_Geo"
				//"NUc - BNG_Subsquares_Geo"
				if ( locationString[2] == 'a' )
				{
					sprintf( ngr, "%c%c250750",locationString[0],locationString[1] );
				}
				else if ( locationString[2] == 'b' )
				{
					sprintf( ngr, "%c%c750750",locationString[0],locationString[1] );
				}
				else if ( locationString[2] == 'c' )
				{
					sprintf( ngr, "%c%c250250",locationString[0],locationString[1] );
					
				}
				else if ( locationString[2] == 'd' )
				{
					sprintf( ngr, "%c%c250750",locationString[0],locationString[1] );
				}
				else
				{
					printf("ngr lookup failure char: %c string: %s\n",locationString[2], locationString );
				}	
				ngr2ll(ngr, &licenseLatitude, &licenseLongitude);
				licenseDistance = geoDistance( lat,lng, licenseLatitude, licenseLongitude,'K');
				//printf("New ngr: %s, %s, distance %f\n",ngr, locationString, licenseDistance );
				if ( licenseDistance < LICENSE_DISTANCE )
				{
					list[frequencyCounter].lowFrequency = (licenseFrequency - (CHANNEL_SPACE/2)) ;
					list[frequencyCounter].highFrequency = (licenseFrequency + (CHANNEL_SPACE/2)) ;
					//printf("license Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
					fprintf(fDebug, "license Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
					frequencyCounter++;			
				}
			}
			else if ( strstr ( locationString, "UK" ) != NULL )
			{
				list[frequencyCounter].lowFrequency = (licenseFrequency - (CHANNEL_SPACE/2)) ;
				list[frequencyCounter].highFrequency = (licenseFrequency + (CHANNEL_SPACE/2)) ;
				//printf("Matched UK on license number %d\n",frequencyCounter );
				fprintf(fDebug, "Matched UK  : license Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
				frequencyCounter++;	
			}
			else if (  strstr( locationString, "England" ) != NULL )
			{
				list[frequencyCounter].lowFrequency = (licenseFrequency - (CHANNEL_SPACE/2)) ;
				list[frequencyCounter].highFrequency = (licenseFrequency + (CHANNEL_SPACE/2)) ;
				// printf("Matched England on license number %d\n",frequencyCounter );	
				fprintf(fDebug, "Matched England  : license Distance: %f, Low Frequency: %f, high Frequency: %f\n", licenseDistance, list[frequencyCounter].lowFrequency/1000000, list[frequencyCounter].highFrequency/1000000 );     
				frequencyCounter++;	
			}
			
		}
		else
		{
			fclose(fh);
		}
	}
	
	
	// add other exclusions 
	list[frequencyCounter].lowFrequency = 1000000;
	list[frequencyCounter].highFrequency = 50000000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 88000000;		//VHF FM broadcast
	list[frequencyCounter].highFrequency = 108000000;	
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 108000000;	//air band
	list[frequencyCounter].highFrequency = 137000000;	
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 144000000;	//2m
	list[frequencyCounter].highFrequency = 147000000;
	frequencyCounter++;
	//list[frequencyCounter].lowFrequency = 153000000;	//Pager
	//list[frequencyCounter].highFrequency = 153500000;
	//frequencyCounter++;	
	list[frequencyCounter].lowFrequency = 156000000;	//Marine low
	list[frequencyCounter].highFrequency = 157500000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 160600000;	//Marine High
	list[frequencyCounter].highFrequency = 162000000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 210000000;  	// DAB
	list[frequencyCounter].highFrequency = 380000000; 	//military air band
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 390000000;	//Tetra
	list[frequencyCounter].highFrequency = 395000000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 430000000;	//70cm
	list[frequencyCounter].highFrequency = 440000000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 470000000;	//Digital TV
	list[frequencyCounter].highFrequency = 790000000;
	frequencyCounter++;		
	list[frequencyCounter].lowFrequency = 876000000;
	list[frequencyCounter].highFrequency = 915000000;	
	frequencyCounter++;		
	list[frequencyCounter].lowFrequency = 921000000;
	list[frequencyCounter].highFrequency = 960000000;
	frequencyCounter++;
	list[frequencyCounter].lowFrequency = 1805200000;
	list[frequencyCounter].highFrequency = 1879800000;
	frequencyCounter++;	
	list[frequencyCounter].lowFrequency = 2400000000;
	list[frequencyCounter].highFrequency = 2500000000;
	frequencyCounter++;	


	
		
	// sort the exclusion database
	qsort ( list, frequencyCounter, sizeof(signalListStruct), signalCmpFunc); 	

	fclose(fDebug);
	
	// debug
	//uint_fast64_t exclusionCounter;
	//for ( exclusionCounter = 0 ; exclusionCounter < frequencyCounter; exclusionCounter ++ )
	//{
		//printf("Low Frequency: %f, high Frequency: %f\n",list[exclusionCounter].lowFrequency/1000000, list[exclusionCounter].highFrequency/1000000 );     
	//}
	
	return(frequencyCounter);
}

/**************************************************************************** 
* NAME:        testFrequency                                    
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     	0 = no license match
* 				1 = license ok 
***************************************************************************/ 
int testFrequency ( double lookupFx, signalListStruct *list, uint32_t records )
{
	uint32_t i;
	for ( i = 0; i < records; i++)
	{
		//printf("looking up %fMHz and %fMHz\n", lookupFx/1000000, array[i].frequency/1000000 );
		if ( lookupFx  > list[i].lowFrequency )
		{
			if ( lookupFx  < list[i].highFrequency )
			{
				// signal is within channel. 
				//printf("Valid signal detected %fMHz between %fMHz and %fMHz\n", lookupFx/1000000, list[i].lowFrequency/1000000, list[i].highFrequency/1000000 );
				return(1);
			}
		}			
	}			
			
	//printf("Dodgy signal detected %fMHz. No channel match\n", lookupFx/1000000 );
	return(0);
}


/**************************************************************************** 
* NAME:        detectLicenseViolations                                   
* DESCRIPTION: 
* ARGUMENTS:    
* RETURNS:     	number of licese violations
* 			
***************************************************************************/ 
uint64_t detectLicenseViolations ( hitsStruct *hits, uint64_t maxHit, double lat, double lng, uint32_t spectra )
{
		uint32_t validSignalsTotal;
		uint32_t hitCounter=0, channelCounter=0, channelFound=0, maxChannel=0;
		signalListStruct *validSignals;
		hitsStruct *targetFrequencies;
		FILE *results;
		
		results=fopen("exceptions.csv","w");
		if (results==NULL)
		{
			printf("Error opening exceptions.csv\n");
			exit(1);
		}
		
		// work out all signals within MAX_DISTANCE and put them into list[]
		printf("Processing license records start\n");

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
		
		validSignalsTotal = openLicenseCsv( validSignals, lat, lng );
		
		printf("Found  %d license records within %dkm of the sensor location\n",validSignalsTotal,LICENSE_DISTANCE);		
		
		
		// compare all hits with 

		fprintf(results, "Target signal Frequency, Occupancy, Max Level\n");		
		
		
		for ( hitCounter = 0; hitCounter < maxHit; hitCounter++ )
		{
			if ( testFrequency ( hits[hitCounter].frequency, validSignals, validSignalsTotal ) )
			{
				// valid signal 
			}
			else
			{
				//invalid signal
				
				//fprintf(results, "%f, %f, %f, %f, %d:%d:%d\n", hits[hitCounter].frequency/1000000, hits[hitCounter].latitude, hits[hitCounter].longitude, hits[hitCounter].level, localtime( &hits[hitCounter].time)->tm_hour, localtime( &hits[hitCounter].time)->tm_min, localtime( &hits[hitCounter].time)->tm_sec   );
				
				channelFound = 0;
				// see if we already have this frequency
				for ( channelCounter =0; channelCounter < maxChannel; channelCounter ++ )
				{
					if ( targetFrequencies[channelCounter].frequency == hits[hitCounter].frequency )
					{
						// this frequency has already been seen
						if ( targetFrequencies[channelCounter].time != hits[hitCounter].time )
						{
							// this is a new time for this frequency
							targetFrequencies[channelCounter].hitCounter ++;
							targetFrequencies[channelCounter].time = hits[hitCounter].time;
						}
						if ( hits[hitCounter].level > targetFrequencies[channelCounter].level )
						{
							// this the same frequency with a greater level
							targetFrequencies[channelCounter].level = hits[hitCounter].level;
							targetFrequencies[channelCounter].latitude = hits[hitCounter].latitude;
							targetFrequencies[channelCounter].longitude = hits[hitCounter].longitude;
						}
						channelFound = 1;	
						break;
					}
				}
				if ( ! channelFound )
				// this is a new frequency
				{
					targetFrequencies[channelCounter].frequency = hits[hitCounter].frequency;
					targetFrequencies[channelCounter].hitCounter = 1;
					targetFrequencies[channelCounter].time = hits[hitCounter].time;
					targetFrequencies[channelCounter].level = hits[hitCounter].level;
					targetFrequencies[channelCounter].latitude = hits[hitCounter].latitude;
					targetFrequencies[channelCounter].longitude = hits[hitCounter].longitude;
					channelCounter ++;
					maxChannel = channelCounter;
				}				
			}
		}
		
		
		for ( channelCounter = 0; channelCounter < maxChannel; channelCounter ++ )
		{
			fprintf(results, "%4.4f, %2.1f, %2.1f, %f, %f, %s\n", targetFrequencies[channelCounter].frequency/1000000, 100 * (float)targetFrequencies[channelCounter].hitCounter/spectra, targetFrequencies[channelCounter].level, targetFrequencies[channelCounter].latitude, targetFrequencies[channelCounter].longitude, ctime(&targetFrequencies[channelCounter].time)   );
		}
		
		
		//printf("Processing license records end\n");
		fclose (results);
		return (0);
}
