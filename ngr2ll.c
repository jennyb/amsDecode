#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define PI 3.141592

/**
 * Converts standard grid reference ('SU387148') to fully numeric ref ([438700,114800]);
 *   returned co-ordinates are in metres, centred on supplied grid square;
 *
 * @param {String} gridref: Standard format OS grid reference
 * @returns {OsGridRef}     Numeric version of grid reference in metres from false origin
 */
void _ngr2en(char *ngr, long *eastings, long *northings)  
{
	char buff[64];
	char buff2[64];
	char *padding;

	// get numeric values of letter references, mapping A->0, B->1, C->2, etc:
	char c1 = toupper(ngr[0]);
	int l1 = c1 - 'A';
	char c2 = toupper(ngr[1]);
	int l2 = c2 - 'A';
	// shuffle down letters after 'I' since 'I' is not used in grid:
	if (l1 > 7) l1--;
	if (l2 > 7) l2--;

	// convert grid letters into 100km-square indexes from false origin (grid square SV):
	long e = ((l1-2)%5)*5 + (l2%5);
	long n = (19-floor(l1/5)*5) - floor(l2/5);
	if (e<0 || e>6 || n<0 || n>12) {
		printf("Invalid NGR in ngr2ew(): %s\n", ngr);
		exit;
	}
	// skip grid letters to get numeric part of ref, stripping any spaces:
	ngr += 2;

  // normalise to 1m grid, rounding up to centre of grid square:
  switch (strlen(ngr)) {
    case 0: padding = "50000"; break;
    case 2: padding = "5000"; break;
    case 4: padding = "500"; break;
    case 6: padding = "50"; break;
    case 8: padding = "5"; break;
    case 10: padding = ""; break; // 10-digit refs are already 1m
    default: printf("invalid length: %s (%d)\n", ngr, strlen(ngr)); exit(1);;
  }

	// append numeric part of references to grid index:
	memset(buff, 0, 64);
	strncpy(buff,ngr,strlen(ngr)/2);
	sprintf(buff2, "%ld%s%s", e, buff, padding);
	*eastings = atol(buff2);

	strcpy(buff, ngr + strlen(ngr)/2);
	sprintf(buff2, "%ld%s%s", n, buff, padding);
	*northings = atol(buff2);
}


int ngr2ll(char *ngr, double *latitude, double *longitude)
{
	long E;
	long N;
	_ngr2en(ngr, &E, &N);

  double a = 6377563.396, b = 6356256.910;              // Airy 1830 major & minor semi-axes
  double F0 = 0.9996012717;                             // NatGrid scale factor on central meridian
  double lat0 = 49*PI/180, lon0 = -2*PI/180;  // NatGrid true origin
  double N0 = -100000, E0 = 400000;                     // northing & easting of true origin, metres
  double e2 = 1 - (b*b)/(a*a);                          // eccentricity squared
  double n = (a-b)/(a+b), n2 = n*n, n3 = n*n*n;
  double M=0;
	double lat, lon;

  lat=lat0;
  do {
    lat = (N-N0-M)/(a*F0) + lat;

    double Ma = (1 + n + (5/4)*n2 + (5/4)*n3) * (lat-lat0);
    double Mb = (3*n + 3*n*n + (21/8)*n3) * sin(lat-lat0) * cos(lat+lat0);
    double Mc = ((15/8)*n2 + (15/8)*n3) * sin(2*(lat-lat0)) * cos(2*(lat+lat0));
    double Md = (35/24)*n3 * sin(3*(lat-lat0)) * cos(3*(lat+lat0));
    M = b * F0 * (Ma - Mb + Mc - Md);                // meridional arc

  } while (N-N0-M >= 0.00001);  // ie until < 0.01mm

  double cosLat = cos(lat), sinLat = sin(lat);
  double nu = a*F0/sqrt(1-e2*sinLat*sinLat);              // transverse radius of curvature
  double rho = a*F0*(1-e2)/pow(1-e2*sinLat*sinLat, 1.5);  // meridional radius of curvature
  double eta2 = nu/rho-1;

  double tanLat = tan(lat);
  double tan2lat = tanLat*tanLat, tan4lat = tan2lat*tan2lat, tan6lat = tan4lat*tan2lat;
  double secLat = 1/cosLat;
  double nu3 = nu*nu*nu, nu5 = nu3*nu*nu, nu7 = nu5*nu*nu;
  double VII = tanLat/(2*rho*nu);
  double VIII = tanLat/(24*rho*nu3)*(5+3*tan2lat+eta2-9*tan2lat*eta2);
  double IX = tanLat/(720*rho*nu5)*(61+90*tan2lat+45*tan4lat);
  double X = secLat/nu;
  double XI = secLat/(6*nu3)*(nu/rho+2*tan2lat);
  double XII = secLat/(120*nu5)*(5+28*tan2lat+24*tan4lat);
  double XIIA = secLat/(5040*nu7)*(61+662*tan2lat+1320*tan4lat+720*tan6lat);

  double dE = (E-E0), dE2 = dE*dE, dE3 = dE2*dE, dE4 = dE2*dE2, dE5 = dE3*dE2, dE6 = dE4*dE2, dE7 = dE5*dE2;
  lat = lat - VII*dE2 + VIII*dE4 - IX*dE6;
  lon = lon0 + X*dE - XI*dE3 + XII*dE5 - XIIA*dE7;
  
//  return new LatLon(lat.toDeg(), lon.toDeg());
	*latitude = lat * 180 / PI;
	*longitude = lon * 180 / PI;

	return 1;
}

