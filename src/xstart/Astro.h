// TODO: libnova needs some minor Linux porting...
#ifdef _WIN32

#undef _USE_32BIT_TIME_T

#include "ScriptObject.h"
#include "Date.h"
#include "Vector.h"
#include <libnova/libnova.h>

int _monthdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};

typedef enum PLANETS {
	MOON = -1,
	SUN = 0,
	MERCURY = 1,
	VENUS = 2,
	EARTH = 3,
	MARS = 4,
	JUPITER = 5,
	SATURN = 6,
	URANUS = 7,
	NEPTUNE = 8
} PLANETS;

class Astro : public ScriptObject {
public:
	Astro() : ScriptObject() {
		id = "Astro";
		ctor = "((optional) [Date] Date)";
		help = "Astronomical mathmatical functions. Angles are in degrees.";

		jd = ln_get_julian_from_sys();
		CONST_MOON = -1;
		CONST_SUN = 0;
		CONST_MERCURY = 1;
		CONST_VENUS = 2;
		CONST_EARTH = 3;
		CONST_MARS = 4;
		CONST_JUPITER = 5;
		CONST_SATURN = 6;
		CONST_URANUS = 7;
		CONST_NEPTUNE = 8;

		BindFunction("setDate", (SCRIPT_FUNCTION)&Astro::gm_setDate, "[this] setDate([Date] date)", "Sets the date for further computations. Dates before Oct 4th 1582 are in the Julian Calender, otherwise in the gregorian calender.");
		BindFunction("getJulianDay", (SCRIPT_FUNCTION)&Astro::gm_getJulianDay, "{float} getJulianDay()", "Gets the currently set julian day as a float value. This number is used for internal computations.");
		BindFunction("getSunDistance", (SCRIPT_FUNCTION)&Astro::gm_getSunDistance, "{float} getSunDistance()", "Gets the sun to earth distance.");
		BindFunction("getMoonDistance", (SCRIPT_FUNCTION)&Astro::gm_getMoonDistance, "{float} getMoonDistance()", "Gets the moon to earth distance.");
		BindFunction("getMoonPhase", (SCRIPT_FUNCTION)&Astro::gm_getMoonPhase, "{float} getMoonPhase()", "Gets the moon phase on earth.");
		BindFunction("getMoonDisk", (SCRIPT_FUNCTION)&Astro::gm_getMoonDisk, "{float} getMoonDisk()", "Gets the visible moon disk (percentage?).");
		BindFunction("getAxialTilt", (SCRIPT_FUNCTION)&Astro::gm_getAxialTilt, "{float} getAxialTilt((optional) {int} day)", "Get the axial tilt/inclination of the earth for the given day of the year or the currently set date.");
		BindFunction("getPlanetDisk", (SCRIPT_FUNCTION)&Astro::gm_getPlanetDisk, "{float} getPlanetDisk({int} day, {float} offset", "Get the visible disk of the given planet.");
		BindFunction("getPlanetEarthDistance", (SCRIPT_FUNCTION)&Astro::gm_getPlanetEarthDistance, "{float} getPlanetEarthDistance({int} planet)", "Get distance of the given planet to earth.");
		BindFunction("getPlanetSunDistance", (SCRIPT_FUNCTION)&Astro::gm_getPlanetSunDistance, "{float} getPlanetSunDistance({int} planet)", "Get distance of the given planet to the sun.");
		BindFunction("getPlanetRST", (SCRIPT_FUNCTION)&Astro::gm_getPlanetRST, "[Vector] getPlanetRST({int} planetId, {float} offset, {float} longitude, {float} latitude)", "Get rise (x), set (y) and fall (z) for the given planet at the given coordinates. The offset affects the currently set julian day for computations.");
		BindFunction("getPlanetHorizontalPosition", (SCRIPT_FUNCTION)&Astro::gm_getPlanetHorizontalPosition, "[Vector] getPlanetHorizontalPosition({int} planetId, {float} offset, {float} longitude, {float} latitude)", "Get the position (x=Azimuth, y=Altitude) of the given planet on the earths horizon. The position on earth is set by the longitude and latitude and the offset affects the julian day for the computation.");
//		BindFunction("getHeliocentricCoords", (SCRIPT_FUNCTION)&Astro::gm_getHeliocentricCoords, (SCRIPT_FUNCTION)&Astro::gm_getHeliocentricCoordinates, "[Vector] getHeliocentricCoordinates({int} planetId), {float} offset", "Get the heliocentric coordinates of the given planet.");

		// planet constants
		BindMember("MOON", &CONST_MOON, TYPE_INT, 0, "Moon ID");
		BindMember("SUN", &CONST_SUN, TYPE_INT, 0, "Sun ID");
		BindMember("MERCURY", &CONST_MERCURY, TYPE_INT, 0, "Mercury ID");
		BindMember("VENUS", &CONST_VENUS, TYPE_INT, 0, "Venus ID");
		BindMember("EARTH", &CONST_EARTH, TYPE_INT, 0, "Earth ID");
		BindMember("MARS", &CONST_MARS, TYPE_INT, 0, "Mars ID");
		BindMember("JUPITER", &CONST_JUPITER, TYPE_INT, 0, "Jupiter ID");
		BindMember("SATURN", &CONST_SATURN, TYPE_INT, 0, "Saturn ID");
		BindMember("URANUS", &CONST_URANUS, TYPE_INT, 0, "Uranus ID");
		BindMember("NEPTUNE", &CONST_NEPTUNE, TYPE_INT, 0, "Neptune ID");
	}

	virtual int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) { gm_setDate(a_thread); }
		return GM_OK;
	}

	double getJulianDay() {
		return jd;
	}
	int gm_getJulianDay(gmThread* a_thread) {
		a_thread->PushFloat( (float) getJulianDay() );
		return GM_OK;
	}

	void setDate(Date* date) {
		ln_date d;
		d.years = date->year;
		d.months = date->month;
		d.days = date->day;
		d.hours = date->hour;
		d.minutes = date->minute;
		d.seconds = date->second;
		jd = ln_get_julian_day(&d);
	}
	int gm_setDate(gmThread* a_thread) {
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_USER_PARAM(Date*, GM_TYPE_OBJECT, date, 0);
		setDate(date);
		return ReturnThis(a_thread);
	}

	double getSunDistance() {
		return ln_get_earth_solar_dist(jd);
	}
	int gm_getSunDistance(gmThread* a_thread) {
		a_thread->PushFloat(getSunDistance());
		return GM_OK;
	}

	double getMoonDistance() {
		return ln_get_lunar_earth_dist(jd);
	}
	int gm_getMoonDistance(gmThread* a_thread) {
		a_thread->PushFloat(getMoonDistance());
		return GM_OK;
	}

	double getMoonPhase() {
		return ln_get_lunar_phase(jd);
	}
	int gm_getMoonPhase(gmThread* a_thread) {
		a_thread->PushFloat(getMoonPhase());
		return GM_OK;
	}

	double getMoonDisk() {
		return ln_get_lunar_disk(jd);
	}
	int gm_getMoonDisk(gmThread* a_thread) {
		a_thread->PushFloat(getMoonDisk());
		return GM_OK;
	}

	double getAxialTilt(int day) {  // Approximation
		ln_date date;
		ln_get_date(jd, &date);
		double yearday = date.days;
		for(int n=1; n<date.months; n++) {
			yearday += _monthdays[n-1];
		}
		if(day >= 0) { yearday = day; }
		double tilt = cos(  (yearday+11) / 182.6211 * 2.0 * 1.5708  ) * 23.5;
		return tilt;
	}
	int gm_getAxialTilt(gmThread* a_thread) {
		int day;
		a_thread->ParamInt(0, day, -1);
		a_thread->PushFloat(getAxialTilt(day));
		return GM_OK;
	}

	Vector* getPlanetRST(int planet, double delta, float lng, float lat) {
		ln_rst_time rst;
		ln_lnlat_posn observer;
		observer.lng = lng;
		observer.lat = lat;
		int circumpolar;
		switch(planet) {
		case MOON:
			circumpolar = ln_get_lunar_rst(jd + delta, &observer, &rst);
			break;
		case SUN:
			circumpolar = ln_get_solar_rst(jd + delta, &observer, &rst);
			break;
		case MERCURY:
			circumpolar = ln_get_mercury_rst(jd + delta, &observer, &rst);
			break;
		case VENUS:
			circumpolar = ln_get_venus_rst(jd + delta, &observer, &rst);
			break;
		case EARTH:
			return 0;
		case MARS:
			circumpolar = ln_get_mars_rst(jd + delta, &observer, &rst);
			break;
		case JUPITER:
			circumpolar = ln_get_jupiter_rst(jd + delta, &observer, &rst);
			break;
		case SATURN:
			circumpolar = ln_get_saturn_rst(jd + delta, &observer, &rst);
			break;
		case URANUS:
			circumpolar = ln_get_uranus_rst(jd + delta, &observer, &rst);
			break;
		case NEPTUNE:
			circumpolar = ln_get_neptune_rst(jd + delta, &observer, &rst);
			break;
		}
		if(circumpolar != 1) {
			Vector* v = new Vector();
			v->x = rst.rise - (jd + delta);
			v->y = rst.set - (jd + delta);
			v->z = rst.transit - (jd + delta);
			return v;
		}
		return 0;
	}
	int gm_getPlanetRST(gmThread* a_thread) {
		int planet;  a_thread->ParamInt(0, planet, 0);
		gmfloat delta;  a_thread->ParamFloatOrInt(1, delta, 0.0);
		gmfloat lng;  a_thread->ParamFloatOrInt(2, lng, 0.0);
		gmfloat lat;  a_thread->ParamFloatOrInt(3, lat, 0.0);
		Vector* v = getPlanetRST(planet, delta, lng, lat);
		if(v) { return v->ReturnThis(a_thread, false); }
		return ReturnNull(a_thread);
	}

	Vector* getPlanetHorizontalPosition(int planet, double delta, float lng, float lat) {
		ln_hrz_posn hrz;
		ln_equ_posn equ;
		ln_lnlat_posn observer;
		observer.lng = lng;
		observer.lat = lat;

		switch(planet) {
		case MOON:
			ln_get_lunar_equ_coords(jd + delta, &equ);
			break;
		case SUN:
			ln_get_solar_equ_coords(jd + delta, &equ);
			break;
		case MERCURY:
			ln_get_mercury_equ_coords(jd + delta, &equ);
			break;
		case VENUS:
			ln_get_venus_equ_coords(jd + delta, &equ);
			break;
		case EARTH:
			return 0;
		case MARS:
			ln_get_mars_equ_coords(jd + delta, &equ);
			break;
		case JUPITER:
			ln_get_jupiter_equ_coords(jd + delta, &equ);
			break;
		case SATURN:
			ln_get_saturn_equ_coords(jd + delta, &equ);
			break;
		case URANUS:
			ln_get_uranus_equ_coords(jd + delta, &equ);
			break;
		case NEPTUNE:
			ln_get_neptune_equ_coords(jd + delta, &equ);
			break;
		}

		ln_get_hrz_from_equ(&equ, &observer, jd + delta, &hrz);

		Vector* v = new Vector();
		v->x = hrz.az;
		v->y = hrz.alt;
		v->z = 0.0;
		return v;
	}
	int gm_getPlanetHorizontalPosition(gmThread* a_thread) {
		int planet;  a_thread->ParamInt(0, planet, 0);
		gmfloat delta;  a_thread->ParamFloatOrInt(1, delta, 0.0);
		gmfloat lng;  a_thread->ParamFloatOrInt(2, lng, 0.0);
		gmfloat lat;  a_thread->ParamFloatOrInt(3, lat, 0.0);
		Vector* v = getPlanetHorizontalPosition(planet, delta, lng, lat);
		if(v) { return v->ReturnThis(a_thread, false); }
		return ReturnNull(a_thread);
	}

	double getPlanetDisk(int planet, double delta) {
		switch(planet) {
		case MOON:
			return ln_get_lunar_disk(jd + delta);
		case SUN:
			return 1.0;
		case MERCURY:
			return ln_get_mercury_disk(jd + delta);
		case VENUS:
			return ln_get_venus_disk(jd + delta);
		case EARTH:
			break;
		case MARS:
			return ln_get_mars_disk(jd + delta);
		case JUPITER:
			return ln_get_jupiter_disk(jd + delta);
		case SATURN:
			return ln_get_saturn_disk(jd + delta);
		case URANUS:
			return ln_get_uranus_disk(jd + delta);
		case NEPTUNE:
			return ln_get_neptune_disk(jd + delta);
		}
		return 0.0;
	}
	int gm_getPlanetDisk(gmThread* a_thread) {
		int planet;  a_thread->ParamInt(0, planet, 0);
		gmfloat delta;  a_thread->ParamFloatOrInt(1, delta, 0.0);
		a_thread->PushFloat(getPlanetDisk(planet, delta));
		return GM_OK;
	}

	double getPlanetEarthDistance(int planet, double delta) {
		switch(planet) {
		case MOON:
			return ln_get_lunar_earth_dist(jd + delta);
		case SUN:
			return ln_get_earth_solar_dist(jd + delta);
		case MERCURY:
			return ln_get_mercury_earth_dist(jd + delta);
		case VENUS:
			return ln_get_venus_earth_dist(jd + delta);
		case EARTH:
			return 0.0;
		case MARS:
			return ln_get_mars_earth_dist(jd + delta);
		case JUPITER:
			return ln_get_jupiter_earth_dist(jd + delta);
		case SATURN:
			return ln_get_saturn_earth_dist(jd + delta);
		case URANUS:
			return ln_get_uranus_earth_dist(jd + delta);
		case NEPTUNE:
			return ln_get_neptune_earth_dist(jd + delta);
		}
		return 0.0;
	}
	int gm_getPlanetEarthDistance(gmThread* a_thread) {
		int planet;  a_thread->ParamInt(0, planet, 0);
		gmfloat delta;  a_thread->ParamFloatOrInt(1, delta, 0.0);
		a_thread->PushFloat(getPlanetEarthDistance(planet, delta));
		return GM_OK;
	}

	double getPlanetSunDistance(int planet, double delta) {
		switch(planet) {
		case MOON:
			return ln_get_earth_solar_dist(jd + delta);
		case SUN:
			return 0.0;
		case MERCURY:
			return ln_get_mercury_solar_dist(jd + delta);
		case VENUS:
			return ln_get_venus_solar_dist(jd + delta);
		case EARTH:
			return ln_get_earth_solar_dist(jd + delta);
		case MARS:
			return ln_get_mars_solar_dist(jd + delta);
		case JUPITER:
			return ln_get_jupiter_solar_dist(jd + delta);
		case SATURN:
			return ln_get_saturn_solar_dist(jd + delta);
		case URANUS:
			return ln_get_uranus_solar_dist(jd + delta);
		case NEPTUNE:
			return ln_get_neptune_solar_dist(jd + delta);
		}
		return 0.0;
	}
	int gm_getPlanetSunDistance(gmThread* a_thread) {
		int planet;  a_thread->ParamInt(0, planet, 0);
		gmfloat delta;  a_thread->ParamFloatOrInt(1, delta, 0.0);
		a_thread->PushFloat(getPlanetSunDistance(planet, delta));
		return GM_OK;
	}




	/*double getDeclination(double delta) {
		ln_equ_posn pos;
		ln_get_solar_equ_coords(jd + delta, &pos);
		return pos.dec;
	}
	int gm_getDeclination(gmThread* a_thread) {
		float delta;
		a_thread->ParamFloatOrInt(0, delta, 0.0f);
		a_thread->PushFloat(getDeclination(delta));
		return GM_OK;
	}*/

	/*	double getDeclination(double delta) {
			ln_equ_posn pos;
			ln_get_solar_equ_coords(jd + delta, &pos);
			return pos.ra;
		}
		int gm_getDeclination(gmThread* a_thread) {
			float delta;
			a_thread->ParamFloatOrInt(0, delta, 0.0f);
			a_thread->PushFloat(getDeclination(delta));
			return GM_OK;
		}*/

	/*double getSunSet(double delta) {
		ln_rst_time rst;
		ln_get_solar_rst(jd + delta,
	}
	int gm_getSunSet(gmThread* a_thread) {

	}*/



	/*void getSunRise(double lat, double lng) {

	}
	void getSunTransit(double lat, double lng) {

	}
	void getSunSet(double lat, double lng) {

	}*/

	/*void getEarthHeliosCoords(Vector* vOut) {

	}*/


public:
	double jd;
	int CONST_MOON;
	int CONST_SUN;
	int CONST_MERCURY;
	int CONST_VENUS;
	int CONST_EARTH;
	int CONST_MARS;
	int CONST_JUPITER;
	int CONST_SATURN;
	int CONST_URANUS;
	int CONST_NEPTUNE;
};


#endif
