#ifndef _PASCO_H_
#define _PASCO_H_


#include "ScriptObject.h"
#include "pasco/pasco_api.h"


#define MAX_PASCO_NAME 64
typedef struct PASCO_MEASUREMENT_RAW_DIGITAL_WORD {
	unsigned char length;
};

typedef struct PASCO_MEASUREMENT {
	unsigned short id;					// unique id within the sensor
	char name[MAX_PASCO_NAME];			// human readable name of the measurement
	char units[MAX_PASCO_NAME];			// units for display
	unsigned char type;					// 0 = Direct Value; 1 = Raw Analog ADC Value; 2 = Raw Digital Word; 3 = Simple Calibration (2-Point Linear); 4 = Macro Calculation; 5 = Table Calibrated Value (N-Points); 6 = Equation Calculated Value
	unsigned char type_desc_length;		// type descript length (may be 0)
	unsigned char type_descriptor;		// based on type ...
	unsigned char visible;				// (v1) Bit 0: 1 = visible to user by default.   (v2) Bit 1: 1 = internal measurement, always hide from user. Bit 2: 1 = choose this measurement for default display if only 1 measurement can be displayed. Bit 3: 1 = applicable for digital readout. Bit 6: 1 = measurement is an accumulator that is zeroed when read. Bit 7: 1 = measurement should be zeroed in software by reading an initial value and offsetting all other values.
	unsigned char accurancy[4];			// (fixed-point) This defines the measurement’s repeatability or the width of the window in which repeated measurements fall.
	unsigned char display_format;		// 0 = no preference.  1 = digital readout.  2 = graph.  3 = table.  4 = meter.
	unsigned char minimum_value;		// (fixed-point) the lowest possible value
	unsigned char maximum_value;		// (fixed-point) the highest possible value
	unsigned char minimum_typical;		// (fixed-point) the lowest typical value
	unsigned char maximum_typical;		// (fixed-point) the highest typical value
};

typedef struct PASCO_DATASHEET {
	unsigned short maximum_memory;		// maximum memory for the datasheet (may not equal the size of the actual data)
	unsigned short length;				// bytes used by the data sheet
	unsigned short length_ex;			// (v1) reserved; (v2) extended data beyond the data sheet
	unsigned short class_code;			// special sensor groups
	char version[4];					// data sheet version string
	char model_number[10];				// pasco model number
	unsigned short measurement_count;	// the count of measurement records that follows the general sensor information
	unsigned short icon[16];			// sensor icon bitmap, 2 bytes per row
	char name[MAX_PASCO_NAME];			// human readable sensor name
	unsigned int min_rate;				// lowest reasonable sample rate (MSB Flag: 0==Hz; 1==sec)
	unsigned int max_rate;				// highest sample rate (MSB Flag: 0==Hz; 1==sec)
	unsigned int default_rate;			// default sample rate (MSB Flag: 0==Hz; 1==sec)
	unsigned int latency;				// maxium latency between sample command and result (in us)
	unsigned int warmup;				// power warmup till the sensor is ready (in ms)
	PASCO_MEASUREMENT measurements[64];	// measurements data
	unsigned int checksum;				// sum of all bytes in the datasheet
};


Pasco_ParseDataSheet() {

}



class PascoDevice : public ScriptObject {
public:

public:
	std::string name;
	int numChannels;
	int sampleSize;
	int rateMin;
	int rateMax;
	int rateDefault;

};


class Pasco : public ScriptObject {

public:

	Pasco() : ScriptObject() {
		id = "Pasco";
		help = "";


	}

	getDeviceName() {}

	get

};


#endif
