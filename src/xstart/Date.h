//#ifdef _WIN32

#ifndef _DATE_H_
#define _DATE_H_

#include "ScriptObject.h"
#include <ctime>

void GetGregorianFromHijri(int& day, int& month, int& year, int offset);
void GetHijriFromGregorian(int& day, int& month, int& year, int offset);

class Date : public ScriptObject {
public:
	Date() : ScriptObject() {
		id = "Date";
		help = "Date and time conversion.";

		now();

		BindMember("year", &year, TYPE_INT);
		BindMember("month", &month, TYPE_INT);
		BindMember("day", &day, TYPE_INT);
		BindMember("hour", &hour, TYPE_INT);
		BindMember("minute", &minute, TYPE_INT);
		BindMember("second", &second, TYPE_INT);
		BindMember("_weekday", &weekday, TYPE_INT);
		BindMember("_yearday", &yearday, TYPE_INT);

		BindFunction("now", (SCRIPT_FUNCTION)&Date::gm_now);
		BindFunction("today", (SCRIPT_FUNCTION)&Date::gm_today);
		BindFunction("isExpired", (SCRIPT_FUNCTION)&Date::gm_isExpired);
		BindFunction("getUnix", (SCRIPT_FUNCTION)&Date::gm_unix);
		BindFunction("setUnix", (SCRIPT_FUNCTION)&Date::gm_set);
		BindFunction("adjust", (SCRIPT_FUNCTION)&Date::gm_adjust);
		BindFunction("format", (SCRIPT_FUNCTION)&Date::gm_format);
		BindFunction("parse", (SCRIPT_FUNCTION)&Date::gm_parse);

		BindFunction("toHijri", (SCRIPT_FUNCTION)&Date::gm_toHijri);
		BindFunction("toGregorian", (SCRIPT_FUNCTION)&Date::gm_toGregorian);
	}

	virtual int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_FLOAT) {
				gmfloat ts;
				a_thread->ParamFloat(0, ts, 0.0);
				set((unsigned long) ts);
			} else if(a_thread->ParamType(0) == GM_INT) {
				int ts;
				a_thread->ParamInt(0, ts, 0);
				set(ts);
			}
		}
		return GM_OK;
	}

	void now() {
		time_t now = time(0);
		tm* utc = gmtime(&now);
		year = utc->tm_year+1900;
		month = utc->tm_mon+1;
		day = utc->tm_mday;
		hour = utc->tm_hour;
		minute = utc->tm_min;
		second = utc->tm_sec;
		weekday = utc->tm_wday;
		yearday = utc->tm_yday;
	}
	int gm_now(gmThread* a_thread) {
		now();
		return ReturnThis(a_thread);
	}

	void today() {
		time_t now = time(0);
		tm* utc = gmtime(&now);
		year = utc->tm_year+1900;
		month = utc->tm_mon+1;
		day = utc->tm_mday;
		hour = 0;
		minute = 0;
		second = 0;
		weekday = utc->tm_wday;
		yearday = utc->tm_yday;
	}
	int gm_today(gmThread* a_thread) {
		today();
		return ReturnThis(a_thread);
	}

	unsigned long long getUnix() {
		tm date;
		date.tm_year = year - 1900;
		date.tm_mon = month - 1;
		date.tm_mday = day;
		date.tm_hour = hour;
		date.tm_min = minute;
		date.tm_sec = second;
#ifdef _WIN32
		time_t unixt = _mkgmtime(&date);
#else
		time_t unixt = timegm(&date);
#endif
		return unixt;
	}
	int gm_unix(gmThread* a_thread) {
		a_thread->PushInt(getUnix());
		return GM_OK;
	}

	void set(unsigned long ts) {
		time_t unixt = ts;
		tm* utc = gmtime(&unixt);
		year = utc->tm_year+1900;
		month = utc->tm_mon+1;
		day = utc->tm_mday;
		hour = utc->tm_hour;
		minute = utc->tm_min;
		second = utc->tm_sec;
		weekday = utc->tm_wday;
		yearday = utc->tm_yday;
	}
	int gm_set(gmThread* a_thread) {
		if(a_thread->GetNumParams() >= 1) {
			if(a_thread->ParamType(0) == GM_FLOAT) {
				gmfloat ts;
				a_thread->ParamFloat(0, ts, 0.0);
				set((unsigned long) ts);
			} else if(a_thread->ParamType(0) == GM_INT) {
				int ts;
				a_thread->ParamInt(0, ts, 0);
				set(ts);
			}
		}
		return ReturnThis(a_thread);
	}

	void adjust(signed long offset) {
		unsigned long long ts = getUnix();
		set(ts + offset);
	}
	int gm_adjust(gmThread* a_thread) {
		int offset; a_thread->ParamInt(0, offset, 0.0);
		adjust(offset);
		return ReturnThis(a_thread);
	}

	bool isExpired() {
		// get current time
		time_t ts = time(0);
		tm* now = gmtime(&ts);
		now->tm_year += 1900;
		now->tm_mon += 1;

		// check
		if(now->tm_year > year) { return true; }
		if(now->tm_year < year) { return false; }
		if(now->tm_mon  > month) { return true; }
		if(now->tm_mon  < month) { return false; }
		if(now->tm_mday > day) { return true; }
		if(now->tm_mday < day) { return false; }
		if(now->tm_hour > hour) { return true; }
		if(now->tm_hour < hour) { return false; }
		if(now->tm_min  > minute) { return true; }
		if(now->tm_min  < minute) { return false; }
		if(now->tm_sec  > second) { return true; }
		if(now->tm_sec  < second) { return false; }

		return false;
	}
	int gm_isExpired(gmThread* a_thread) {
		a_thread->PushInt(isExpired());
		return GM_OK;
	}

	void fromGregorianToHijri(int offset) {
		GetHijriFromGregorian(day, month, year, offset);
	}
	int gm_toHijri(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(offset, 0);
		fromGregorianToHijri(offset);
		return ReturnThis(a_thread);
	}

	void fromHijriToGregorian(int offset) {
		GetGregorianFromHijri(day, month, year, offset);
	}
	int gm_toGregorian(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(offset, 0);
		fromHijriToGregorian(offset);
		return ReturnThis(a_thread);
	}

	std::string format(const char* frmt) {
		tm t;
		t.tm_year = year - 1900;
		t.tm_mon = month - 1;
		t.tm_mday = day;
		t.tm_hour = hour;
		t.tm_isdst = 0;
		t.tm_min = minute;
		t.tm_sec = second;
		t.tm_wday = weekday;
		t.tm_yday = yearday;
		char buffer[1024];
		strftime(buffer, 1023, frmt, &t);
		return std::string(buffer);
	}
	int gm_format(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(frmt, 0);
		a_thread->PushNewString(format(frmt).c_str());
		return GM_OK;
	}

	bool parse(std::string datetime, const char* format = "%04d-%02d-%02d %02d:%02d:%02d") {
		if (sscanf(datetime.c_str(), format, &year, &month, &day, &hour, &minute, &second)) return true;
		else return false;
	}
	int gm_parse(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(datetime, 0);
		a_thread->PushInt(parse(datetime));
		return GM_OK;
	}

public:
	// date
	int year;
	int month;
	int day;

	// date extra (may not always be accurate)
	int weekday;
	int yearday;

	// time
	int hour;
	int minute;
	int second;
};


#endif
