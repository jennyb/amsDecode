#ifndef __LIPRO__
#define LIPRO

#include "amsDecode.h"

#define LICENSE_DISTANCE 50
#define LICENSE_LINE_LEN 200
#define LICENSE_FILE_LEN 200000
#define WEST_MOST_LICENSE -9
#define EAST_MOST_LICENSE 4
#define NORTH_MOST_LICENSE 62
#define SOUTH_MOST_LICENSE 48

 typedef struct {
	double 		lowFrequency;
	double 		highFrequency;
 } signalListStruct ;

double deg2rad(double deg);
double rad2deg(double rad);
double geoDistance(double lat1, double lon1, double lat2, double lon2, char unit);
double ngrDistance ( double lat,double lng, char *country1, char *country2, char *ngr  );  
int signalCmpFunc ( const void *arg1, const void *arg2);
uint32_t openLicenseCsv ( signalListStruct list[], double lat, double lng);
int testFrequency ( double lookupFx, signalListStruct *list, uint32_t records );
uint64_t detectLicenseViolations ( hitsStruct *hits, uint64_t maxHit, double lat, double lng, uint32_t spectra );

#endif
