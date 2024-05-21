#ifndef INC_NMEA_H
#define INC_NMEA_H

typedef struct{
	int hour;
	int min;
	int sec;
}TIME;

typedef struct{
	float latitude;
	char NS;
	float longitude;
	char EW;
}LOCATION;

typedef struct{
	float altitude;
	char unit;
}ALTITUDE;

typedef struct{
	int Day;
	int Mon;
	int Yr;
}DATE;

typedef struct{
	LOCATION location;
	TIME tim;
	int isfixValid;
	ALTITUDE alt;
	int numofsat;
}GGASTRUCT;

typedef struct{
	DATE date;
	float speed;
	float course;
	int isValid;
}RMCSTRUCT;

typedef struct{
	GGASTRUCT ggastruct;
	RMCSTRUCT rmcstruct;
}GPSSTRUCT;

int decodeGGA(char *GGAbuffer, GGASTRUCT *gga);

int decodeRMC (char *RMCbuffer, RMCSTRUCT *rmc);

#endif
