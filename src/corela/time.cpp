#include "corela.h"
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

static double g_baseTime;

double TimeGet() {
#ifdef _WIN32
	return (double)GetTickCount() / 1000.0 - g_baseTime;
#else
	// SOLUTION 1: Does not work correctly on Udoo.
	//return (double)clock() / (double)CLOCKS_PER_SEC - g_baseTime;

	// SOLUTION 2: 
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)(ts.tv_sec) + ((double)(ts.tv_nsec) / 1000000000.0) - g_baseTime;

	// SOLUTION 3:
	//struct timeval ttime;
	//gettimeofday(&ttime, NULL);
	//return (double)ttime.tv_sec + (double)ttime.tv_usec / 1000000.0 - g_baseTime;
#endif
}

void TimeSet(double t) {
	g_baseTime = t;
}

#ifdef WIN32
#define _WINSOCKAPI_
#include <windows.h>
void TimeSleep(double seconds) {
	if(seconds <= 0.0) {
		return;
	}
	if(seconds >= 60.0) {
		Log(LOG_WARNING, "TimeSleep was called with %s seconds!", seconds);
	}
	Sleep(unsigned(seconds*1000.0));
}
#else
#include <unistd.h>
void TimeSleep(double seconds) {
	if(seconds <= 0.0) {
		return;
	}
	if(seconds >= 60.0) {
		Log(LOG_WARNING, "TimeSleep was called with %s seconds!", seconds);
	}
	usleep(unsigned(seconds*1000.0) * 1000);
}
#endif
