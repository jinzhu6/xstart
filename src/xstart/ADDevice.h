#ifdef _WIN32


#ifndef _ADDEVICE_H_
#define _ADDEIVCE_H_

#include "ScriptObject.h"

#define DIGITAL_BITS(d1,d2,d3,d4,d5,d6,d7,d8) (d1 + (d2<<1) + (d3<<2) + (d4<<3) + (d5<<4) + (d6<<5) + (d7<<6) + (d8<<7))

#define AD_CHA_TYPE_ANALOG_IN       0x01000000
#define AD_CHA_TYPE_ANALOG_OUT      0x02000000
#define AD_CHA_TYPE_DIGITAL_IO      0x03000000

typedef int (*fn_ad_analog_in) (int adh, int cha, int range, float* data);
typedef int (*fn_ad_digital_in) (int adh, int cha, unsigned int* data);
typedef int (*fn_ad_analog_out) (int adh, int cha, int range, float data);
typedef int (*fn_ad_digital_out) (int adh, int cha, unsigned int data);
typedef int (*fn_ad_open) (const char* name);
typedef int (*fn_ad_close) (int adh);
typedef int (*fn_ad_discrete_in) (int adh, int cha, int range, unsigned int* data);
typedef int (*fn_ad_set_line_direction) (int adh, int cha, unsigned int mask);
typedef int (*fn_ad_find_best_range) (int adh, int cha, int* range, double min, double max);


class ADDevice : public ScriptObject {
public:
	ADDevice() {
		id = "ADDevice";
		ctor = "((optional) {string} deviceId)";
		help = "USB-AD and USB-PIO device interface. <b>Requires the libad4.dll in the working directory or in path to work!</b>";

		adh = -1;
		connected = false;
		deviceId = "";

		ad_open = (fn_ad_open)DLLGetFunctionPtr("libad4.dll", "ad_open");
		if(ad_open) {
			ad_close = (fn_ad_close)DLLGetFunctionPtr("libad4.dll", "ad_close");
			ad_analog_in = (fn_ad_analog_in)DLLGetFunctionPtr("libad4.dll", "ad_analog_in");
			ad_analog_out = (fn_ad_analog_out)DLLGetFunctionPtr("libad4.dll", "ad_analog_out");
			ad_digital_in = (fn_ad_digital_in)DLLGetFunctionPtr("libad4.dll", "ad_digital_in");
			ad_digital_out = (fn_ad_digital_out)DLLGetFunctionPtr("libad4.dll", "ad_digital_out");
			ad_discrete_in = (fn_ad_discrete_in)DLLGetFunctionPtr("libad4.dll", "ad_discrete_in");
			ad_set_line_direction = (fn_ad_set_line_direction)DLLGetFunctionPtr("libad4.dll", "ad_set_line_direction");
			ad_find_best_range = (fn_ad_find_best_range)DLLGetFunctionPtr("libad4.dll", "ad_find_best_range");
		}

		BindMember("handle", &adh, TYPE_INT, 0, "{int} handle", "Internal libAD device handle, if the device is not connected the handle is -1.");
		BindMember("device", &deviceId, TYPE_STRING, 0, "{string} device", "The device id, should look like \"usb-pio\", \"usb-ad:1\", \"usb-pio:@012345\" or similar.");
		BindMember("connected", &connected, TYPE_INT, 0, "{int} connected", "Non-zero if the device is currently successfully connected.");

		BindFunction("connect", (SCRIPT_FUNCTION)&ADDevice::gm_connect, "{int} connect((optional) {string} deviceId)", "Connect to a AD device by device id. Returns the non-zero on success.");
		BindFunction("disconnect", (SCRIPT_FUNCTION)&ADDevice::gm_disconnect, "[this] disconnect()", "Disconnect currently connected AD device.");
		BindFunction("findAnalogRange", (SCRIPT_FUNCTION)&ADDevice::gm_findAnalogRange, "{int} findAnalogRange({int} channel, {float} volt)", "Finds the best volt range match for the given volt.");
		BindFunction("getAnalogValue", (SCRIPT_FUNCTION)&ADDevice::gm_getAnalogValue, "{int} getAnalogValue({int} channel, {int} range)", "Gets the current raw analog value of the given channel.");
		BindFunction("getAnalogVolt", (SCRIPT_FUNCTION)&ADDevice::gm_getAnalogVolt, "{float} getAnalogVolt({int} channel, {int} range)", "Gets the current analog voltage of the given channel.");
		BindFunction("setAnalogVolt", (SCRIPT_FUNCTION)&ADDevice::gm_setAnalogVolt, "[this] setAnalogVolt({int} channel, {float} volt)", "Sets the voltage of the given channel.");
		BindFunction("getDigitalBits", (SCRIPT_FUNCTION)&ADDevice::gm_getDigitalBits, "{int} getDigitalBits({int} channel)", "Get the bitmask of the given digital channel.");
		BindFunction("setDigitalBits", (SCRIPT_FUNCTION)&ADDevice::gm_setDigitalBits, "[this] setDigitalBits({int} channel, {int} bits)", "Set the bitmask of the given digital channel.");
	}

	~ADDevice() {
		disconnect();
	}

	int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() > 0) {
			return gm_connect(a_thread);
		}
		return GM_OK;
	}

	bool connect(const char* deviceId) {
		if(this->deviceId == deviceId && connected) { return true; }

		if(connected) { disconnect(); }

		this->deviceId = deviceId;
		connected = false;

		if(!ad_open) { return false; }
		adh = ad_open(deviceId);
		if(adh < 0) {
			Log(LOG_ERROR, "ADDevice '%s' failed to connect!", deviceId);
			return false;
		}

		connected = true;
		return true;
	}
	int gm_connect(gmThread* a_thread) {
		if(a_thread->GetNumParams() == 0) {
			if(this->deviceId == "") {
				a_thread->PushInt(connect("usb-pio"));
			} else {
				a_thread->PushInt(connect(this->deviceId.c_str()));
			}
		} else {
			GM_CHECK_STRING_PARAM(deviceId, 0);
			a_thread->PushInt(connect(deviceId));
		}
		return GM_OK;
	}

	void disconnect() {
		if(adh < 0) { return; }
		ad_close(adh);
		adh = -1;
		connected = false;
	}
	int gm_disconnect(gmThread* a_thread) {
		disconnect();
		return ReturnThis(a_thread);
	}

	int findAnalogRange(int cha, double volt) {
		int rc, range;
		rc = ad_find_best_range(adh, AD_CHA_TYPE_ANALOG_IN | cha, &range, -volt, volt);
		if(!rc) { return -1; }
		return range;
	}
	int gm_findAnalogRange(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(cha, 0);
		GM_CHECK_FLOAT_PARAM(volt, 1);
		a_thread->PushInt( findAnalogRange(cha, volt) );
		return GM_OK;
	}

	unsigned int getAnalogValue(int cha, int range) {
		unsigned int st;
		unsigned int data;
		ad_set_line_direction(adh, cha, 0x00); // ??
		st = ad_discrete_in(adh, cha | AD_CHA_TYPE_ANALOG_IN, range, &data);
		return data;
	}
	int gm_getAnalogValue(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(cha, 0);
		GM_CHECK_INT_PARAM(range, 0);
		a_thread->PushInt( getAnalogValue(cha, range) );
		return GM_OK;
	}

	float getAnalogVolt(int cha, int range) {
		float data;
		ad_analog_in(adh, cha, range, &data);
		return data;
	}
	int gm_getAnalogVolt(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(cha, 0);
		GM_CHECK_INT_PARAM(range, 0);
		a_thread->PushFloat( getAnalogVolt(cha, range) );
		return GM_OK;
	}

	void setAnalogVolt(int cha, float v) {
		ad_analog_out(adh, cha, 0, v);
	}
	int gm_setAnalogVolt(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(cha, 0);
		GM_CHECK_FLOAT_PARAM(volt, 1);
		setAnalogVolt(cha, volt);
		return ReturnThis(a_thread);
	}

	unsigned int getDigitalBits(int cha) {
		unsigned int data;
		ad_set_line_direction(adh, cha, 0xffff);
		ad_digital_in(adh, cha, &data);
//		ad_set_line_direction(adh, cha, 0x0000);
		return data;
	}
	int gm_getDigitalBits(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(channel, 0);
		a_thread->PushInt( getDigitalBits(channel) );
		return GM_OK;
	}

	void setDigitalBits(int cha, unsigned int v) {
		ad_set_line_direction(adh, cha, 0x0000);
		ad_digital_out(adh, cha, v);
	}
	int gm_setDigitalBits(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(channel, 0);
		GM_CHECK_INT_PARAM(bits, 1);
		setDigitalBits(channel, bits);
		return ReturnThis(a_thread);
	}

private:
	int adh;
	int connected;
	std::string deviceId;
	fn_ad_analog_in ad_analog_in;
	fn_ad_digital_in ad_digital_in;
	fn_ad_analog_out ad_analog_out;
	fn_ad_digital_out ad_digital_out;
	fn_ad_open ad_open;
	fn_ad_close ad_close;
	fn_ad_discrete_in ad_discrete_in;
	fn_ad_set_line_direction ad_set_line_direction;
	fn_ad_find_best_range ad_find_best_range;
};


#endif



#endif
