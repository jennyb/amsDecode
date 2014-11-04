 #ifndef __AMSDECODE__
#define AMSDECODE

typedef struct {
	double 		frequency;
	double 		latitude;
	double 		longitude;
	double 		level;
	uint32_t	hitCounter;
	time_t		time;
 } hitsStruct;
 
 #define pi 3.14159265358979323846 
 #define CHANNEL_SPACE 12500

#endif
 
 
 
