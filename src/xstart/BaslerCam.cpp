#if 0

#ifdef _WIN32


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <pylonc/PylonC.h>
#include <corela.h>

#pragma comment(lib, "..\\..\\..\\src\\PylonC_MD_VC100.lib")

#define CHECK(err) {if(err!=GENAPI_E_OK){_BaslerPrintError(err);}}

typedef struct BASLERCAM {
	PYLON_DEVICE_HANDLE hDev;
	PYLON_EVENTGRABBER_HANDLE hGrabber;
} BASLERCAM;



/***************************************************************************************************************************
/ _BaslerPrintError
/**************************************************************************************************************************/
void _BaslerPrintError(GENAPIC_RESULT errc) {
	char* errMsg;
	size_t length;

	/* Retrieve the error message.
	... First find out how big the buffer must be, */
	GenApiGetLastErrorMessage( NULL, &length );
	errMsg = (char*) malloc( length );
	/* ... and retrieve the message. */
	GenApiGetLastErrorMessage( errMsg, &length );

	fprintf( stderr, "%s (%#08x).\n", errMsg, errc);
	free( errMsg);

	/* Retrieve more details about the error
	... First find out how big the buffer must be, */
	GenApiGetLastErrorDetail( NULL, &length );
	errMsg = (char*) malloc( length );
	/* ... and retrieve the message. */
	GenApiGetLastErrorDetail( errMsg, &length );

	fprintf( stderr, "%s\n", errMsg);
	free( errMsg);

	fflush( stderr );
}



/***************************************************************************************************************************
/ BaslerInit
/**************************************************************************************************************************/
int BaslerInit() {
	GENAPIC_RESULT res;
	size_t numDevices;

	PylonInitialize();
	res = PylonEnumerateDevices(&numDevices);
	CHECK(res);
	if(!numDevices) {
		Log(LOG_ERROR, "No basler camera devices found!"); 
		PylonTerminate();
	}

	return numDevices;
}



/***************************************************************************************************************************
/ BaslerDestroy
/**************************************************************************************************************************/
void BaslerDestroy() {
	PylonTerminate();
}



/***************************************************************************************************************************
/ BaslerOpen
/**************************************************************************************************************************/
BASLERCAM* BaslerOpen(int index) {
	PYLON_DEVICE_HANDLE hDev = 0;
	GENAPIC_RESULT res;
	char buf[256];
	_Bool isReadable;
	size_t size;

	Log(LOG_NOBREAK, "Creating BaslerCam (device-index %d) ...", index);
	res = PylonCreateDeviceByIndex(index, &hDev);
	CHECK(res);
	size = sizeof(buf);
	if(res != GENAPI_E_OK) {
		Log(LOG_ERROR, "FAILED!");
		return 0;
	}
	Log(LOG_INFO, "ok.  Opening now ...");

	res = PylonDeviceOpen(hDev, PYLONC_ACCESS_MODE_CONTROL | PYLONC_ACCESS_MODE_STREAM | PYLONC_ACCESS_MODE_EVENT);
	CHECK(res);

	isReadable = PylonDeviceFeatureIsReadable(hDev, "DeviceModelName");
	if(isReadable) {
		res = PylonDeviceFeatureToString(hDev, "DeviceModelName", buf, &size );
		CHECK(res);
		printf("Using camera %s\n", buf);
	}

	BASLERCAM* camera = (BASLERCAM*)malloc(sizeof(BASLERCAM));
	memset(camera, 0, sizeof(BASLERCAM));
	camera->hDev = hDev;

	return camera;
}



/***************************************************************************************************************************
/ BaslerClose
/**************************************************************************************************************************/
void BaslerClose(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	res = PylonDeviceClose(cam->hDev);
	CHECK(res);
	res = PylonDestroyDevice(cam->hDev);
	CHECK(res);
	free(cam);
}



/***************************************************************************************************************************
/ BaslerStop
/**************************************************************************************************************************/
void BaslerStop(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	res = PylonDeviceExecuteCommandFeature(cam->hDev, "AcquisitionStop");
	CHECK(res);
}



/***************************************************************************************************************************
/ BaslerStart
/**************************************************************************************************************************/
void BaslerStart(BASLERCAM* cam, int continuous) {
	GENAPIC_RESULT res;
	BaslerStop(cam);
	res = PylonDeviceFeatureFromString(cam->hDev, "AcquisitionMode", continuous ? "Continuous" : "SingleFrame");
	CHECK(res);
	res = PylonDeviceExecuteCommandFeature(cam->hDev, "AcquisitionStart");
	CHECK(res);
}




/***************************************************************************************************************************
/ BaslerTrigger
/**************************************************************************************************************************/
void BaslerTrigger(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	res = PylonDeviceExecuteCommandFeature(cam->hDev, "TriggerSoftware");
	CHECK(res);
}



/***************************************************************************************************************************
/ BaslerTrigger
/**************************************************************************************************************************/
int BaslerSetOutputLine(BASLERCAM* cam, const char* source) {
	GENAPIC_RESULT res;
	res = PylonDeviceFeatureFromString(cam->hDev, "LineSelector", "Out1");
	CHECK(res);
	res = PylonDeviceFeatureFromString(cam->hDev, "LineSource", source);
	CHECK(res);
	return 1;
}



/***************************************************************************************************************************
/ BaslerTrigger
/**************************************************************************************************************************/
int BaslerSetTrigger(BASLERCAM* cam, const char* trigger, int enable, int software) {
	GENAPIC_RESULT res;
	res = PylonDeviceFeatureFromString(cam->hDev, "TriggerSelector", trigger);
	CHECK(res);
	res = PylonDeviceFeatureFromString(cam->hDev, "TriggerMode", enable ? "On" : "Off");
	CHECK(res);
//	res = PylonDeviceFeatureFromString(cam->hDev, "TriggerSelector", trigger);
//	CHECK(res);
	res = PylonDeviceFeatureFromString(cam->hDev, "TriggerSource", software ? "Software" : "Line1");
	CHECK(res);
	return 1;
}


/***************************************************************************************************************************
/ BaslerSetFeature
/**************************************************************************************************************************/
int BaslerSetFeature(BASLERCAM* cam, const char* key, const char* value) {
	GENAPIC_RESULT res;
	if(PylonDeviceFeatureIsAvailable(cam->hDev, key)) {
		res = PylonDeviceFeatureFromString(cam->hDev, key, value);
		CHECK(res);
		if(res != GENAPI_E_OK) {
			Log(LOG_ERROR, "Error while setting camera feature '%s' to '%s'!", key ,value);
			return 0;
		}
	}
	return 1;
}



/***************************************************************************************************************************
/ BaslerSetFeatureInt
/**************************************************************************************************************************/
int BaslerSetFeatureInt(BASLERCAM* cam, const char* key, int value) {
	GENAPIC_RESULT res;
	if(PylonDeviceFeatureIsWritable(cam->hDev, key)) {
		res = PylonDeviceSetIntegerFeature(cam->hDev, key, value);
		CHECK(res);
		if(res != GENAPI_E_OK) {
			Log(LOG_ERROR, "Error while setting camera feature '%s' to '%d'!", key ,value);
			return 0;
		}
	}
	return 1;
}


/***************************************************************************************************************************
/ BaslerSetFeatureFloat
/**************************************************************************************************************************/
int BaslerSetFeatureFloat(BASLERCAM* cam, const char* key, float value) {
	GENAPIC_RESULT res;
	if(PylonDeviceFeatureIsWritable(cam->hDev, key)) {
		res = PylonDeviceSetFloatFeature(cam->hDev, key, value);
		CHECK(res);
		if(res != GENAPI_E_OK) {
			Log(LOG_ERROR, "Error while setting camera feature '%s' to '%.2f'!", key ,value);
			return 0;
		}
	}
	return 1;
}


/***************************************************************************************************************************
/ BaslerSetFeatureBool
/**************************************************************************************************************************/
int BaslerSetFeatureBool(BASLERCAM* cam, const char* key, bool value) {
	GENAPIC_RESULT res;
	if(PylonDeviceFeatureIsWritable(cam->hDev, key)) {
		res = PylonDeviceSetBooleanFeature(cam->hDev, key, value);
		CHECK(res);
		if(res != GENAPI_E_OK) {
			Log(LOG_ERROR, "Error while setting camera feature '%s' to '%s'!", key, value ? "true" : "false");
			return 0;
		}
	}
	return 1;
}



/***************************************************************************************************************************
/ BaslerGetFeatureInt
/**************************************************************************************************************************/
int BaslerGetFeatureInt(BASLERCAM* cam, const char* key) {
	GENAPIC_RESULT res;
	int32_t value;
	res = PylonDeviceGetIntegerFeatureInt32(cam->hDev, key, &value);
	CHECK(res);
	return value;
}



/***************************************************************************************************************************
/ BaslerGetRequiredBufferSize
/**************************************************************************************************************************/
unsigned int BaslerGetRequiredBufferSize(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	int32_t payloadSize = 0;
	res = PylonDeviceGetIntegerFeatureInt32(cam->hDev, "PayloadSize", &payloadSize);
	CHECK(res);
	return payloadSize;
}



/***************************************************************************************************************************
/ BaslerGrabSingleFrame
/**************************************************************************************************************************/
int BaslerGrabSingleFrame(BASLERCAM* cam, void* buffer, unsigned int size, int msTimeout) {
	GENAPIC_RESULT res;
	PylonGrabResult_t grabRes;
	_Bool ready;

	res = PylonDeviceGrabSingleFrame(cam->hDev, 0, buffer, size, &grabRes, &ready, msTimeout);
	CHECK(res);

	if(res != GENAPI_E_OK) { return 0; }

	if(grabRes.Status == Grabbed) {
		//res = PylonImageWindowDisplayImageGrabResult(0, &grabRes);
	}

	return grabRes.Status == Grabbed;
}



/***************************************************************************************************************************
/ BaslerSetHeartbeatTimeout
/**************************************************************************************************************************/
void BaslerSetHeartbeatTimeout(BASLERCAM* cam, unsigned int msTimeout) {
	GENAPIC_RESULT res;
	NODEMAP_HANDLE              hNodemap;   /* Handle to the node map */
	NODE_HANDLE                 hNode;      /* Handle to a node, i.e., a feature */
	int64_t                     oldTimeout; /* The current timeout value */

	/* Get the node map for the transport layer parameters. */
	res = PylonDeviceGetTLNodeMap(cam->hDev, &hNodemap);
	CHECK(res);
	if(hNodemap == GENAPIC_INVALID_HANDLE) {
		/* The device doesn't provide a transport layer node map. Nothing to do. */
		return;
	}

	/* Get the node for the heartbeat timeout parameter. */
	res = GenApiNodeMapGetNode(hNodemap, "HeartbeatTimeout", &hNode );
	CHECK(res);
	if(hNode == GENAPIC_INVALID_HANDLE) {
		/* There is no heartbeat timeout parameter. Nothing to do. */
		return;
	}

	/* Set the new value. */
	res = GenApiIntegerSetValue(hNode, msTimeout);
	CHECK(res);
}



/***************************************************************************************************************************
/ BaslerCreateEventHandler
/**************************************************************************************************************************/
int BaslerCreateEventHandler(BASLERCAM* cam, int numBuffers) {
	GENAPIC_RESULT res;

	// get event grabber object
	res = PylonDeviceGetEventGrabber(cam->hDev, &cam->hGrabber);
	CHECK(res);

	// check for errors
	if(cam->hGrabber == PYLONC_INVALID_HANDLE) {
		fprintf(stderr, "No event grabber supported.\n");
		return 0;
	}

	// set number of event buffers
	res = PylonEventGrabberSetNumBuffers(cam->hGrabber, numBuffers);
	CHECK(res);

	// Enable camera event reporting
	res = PylonDeviceFeatureFromString(cam->hDev, "EventSelector", "ExposureEnd");
	CHECK(res);

	// start event grabber
	res = PylonEventGrabberOpen(cam->hGrabber);
	CHECK(res);

	return 1;
}



/***************************************************************************************************************************
/ BaslerStartEventHandler
/**************************************************************************************************************************/
void BaslerStartEventHandler(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	int32_t sfncVersionMajor;

	// Determine the major number of the SFNC version used by the camera device
	if(PylonDeviceGetIntegerFeatureInt32(cam->hDev, "DeviceSFNCVersionMajor", &sfncVersionMajor) != GENAPI_E_OK) {
		sfncVersionMajor = 0;
	}

	// Enable camera event reporting
	res = PylonDeviceFeatureFromString(cam->hDev, "EventSelector", "ExposureEnd");
	CHECK(res);

	// Enable the event reporting.
	if (sfncVersionMajor >= 2) {
		res = PylonDeviceFeatureFromString(cam->hDev, "EventNotification", "On");
	} else {
		res = PylonDeviceFeatureFromString(cam->hDev, "EventNotification", "GenICamEvent");
	}
	CHECK(res);
}



/***************************************************************************************************************************
/ BaslerPollEvent
/**************************************************************************************************************************/
int BaslerPollEvent(BASLERCAM* cam) {
	GENAPIC_RESULT res;
	PylonEventResult_t eventMsg;
	_Bool isReady;
	{
		// get wait object
		PYLON_WAITOBJECT_HANDLE hWait;
		res = PylonEventGrabberGetWaitObject(cam->hGrabber, &hWait);
		CHECK(res);

		// For extracting the event data from an event message, an event adapter is used.
		PYLON_EVENTADAPTER_HANDLE hAdapter;
		res = PylonDeviceCreateEventAdapter(cam->hDev, &hAdapter);
		CHECK(res);
		if(cam->hGrabber == PYLONC_INVALID_HANDLE) {
			// The transport layer doesn't support event grabbers.
			fprintf(stderr, "No event adapter supported.\n");
			return 0;
		}

		// wait for event
		_Bool waitOk;
		PylonWaitObjectWait(hWait, 1000, &waitOk);
		if(!waitOk) {
			fprintf(stderr, "Wait object time out.\n");
			return 0;

		}
	}

	// retreive event
	res = PylonEventGrabberRetrieveEvent(cam->hGrabber, &eventMsg, &isReady);
	CHECK(res);

	if(!isReady) {
		/* Oops. No event message available? We should never have reached this point.
		Since the wait operation above returned without a timeout, an event message
		should be available. */
		fprintf(stderr, "Failed to retrieve an event\n");
		return 0;
	}
	/* Check to see if the event was successfully received. */
	if (eventMsg.ErrorCode == 0) {
		/* Successfully received an event message. */
		/* Pass the event message to the event adapter. The event adapter will
		update the parameters related to events and will fire the callbacks
		registered to event related parameters. */
//		res = PylonEventAdapterDeliverMessage(hEventAdapter, &eventMsg );
//		CHECK(res);
		return 1;
	} else {
		fprintf(stderr, "Error when receiving an event: 0x%08x\n", eventMsg.ErrorCode);
		return 0;
	}

	return 0;
}


/*int BaslerCreateStreamGrabber(BASLERCAM* cam) {
	PYLON_STREAMGRABBER_HANDLE  hStreamGrabber;
	PYLON_WAITOBJECT_HANDLE     hWaitStream;
	PYLON_STREAMBUFFER_HANDLE   bufHandles[NUM_IMAGE_BUFFERS];

	// get number of stream channels
	res = PylonDeviceGetNumStreamGrabberChannels(cam->hDev, &nStreams);
	CHECK(res);
	if(nStreams < 1) {
		fprintf(stderr, "The transport layer doesn't support image streams\n");
		return 0;
	}

	// create grabber for the first channel
	res = PylonDeviceGetStreamGrabber( hDev, 0, &hStreamGrabber );
	CHECK(res);
	res = PylonStreamGrabberOpen( hStreamGrabber );
	CHECK(res);

	// create wait object
	res = PylonStreamGrabberGetWaitObject( hStreamGrabber, &hWaitStream );
	CHECK(res);


}*/


#endif

#endif