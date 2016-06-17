#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "ScriptObject.h"
#include <corela/strtrim.h>

// Include for windows
#ifdef _WIN32
// Accessing to the serial port under Windows
#include <windows.h>
#endif

// Include for Linux
#ifdef __GNUC__
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <termios.h>
#include <string.h>
#include <iostream>
// File control definitions
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

class TimeOut {
public:
	void InitTimer() { prevTime = TimeGet(); }
	unsigned long int ElapsedTime_ms() { return (unsigned long int)((TimeGet() - prevTime) * 1000.0); }
private:
	double prevTime;
};

class SerialPort: public ScriptObject {
public:
	SerialPort() : ScriptObject() {
		id = "SerialPort";
		help = "SerialPort class";

		BindFunction("open", (SCRIPT_FUNCTION)&SerialPort::gm_open, "[this] open((optional) {string} port, (optional) {int} baudrate)");
		BindFunction("close", (SCRIPT_FUNCTION)&SerialPort::gm_close, "[this] close()");
		BindFunction("flush", (SCRIPT_FUNCTION)&SerialPort::gm_flush, "[this] flush()", "NOT SUPPORTED ON WINDOWS!");
		BindFunction("peek", (SCRIPT_FUNCTION)&SerialPort::gm_peek, "{int} peek()", "NOT SUPPORTED ON WINDOWS!");
		BindFunction("write", (SCRIPT_FUNCTION)&SerialPort::gm_write, "[this] write({string} data)");
		BindFunction("writeData", (SCRIPT_FUNCTION)&SerialPort::gm_writeData, "[this] writeData([Data] data)");
		BindFunction("read", (SCRIPT_FUNCTION)&SerialPort::gm_read, "{string} read()");
		BindFunction("readEx", (SCRIPT_FUNCTION)&SerialPort::gm_readEx, "{string} read({int} maxChar, {int} endChar, {int} timeout)");
		BindFunction("readData", (SCRIPT_FUNCTION)&SerialPort::gm_readData, "[Data] readData({int} maxChar, {int} timeout)");
	}

	~SerialPort() {
		_close();
	}


	int gm_open(gmThread* a_thread) {
		int np = a_thread->GetNumParams();
		if(np == 0) {
			Log(LOG_FATAL, "SerialPort: Please give serial port as first param.");
		}

		//char* port = "/dev/ttymxc3";
		//int baud = 9600;

		//a_thread->ParamString(0, port);
		//a_thread->ParamInt(1, baud, 9600);

		GM_CHECK_STRING_PARAM(port, 0);
		GM_CHECK_INT_PARAM(baud, 1);

		_open(port, baud);

		return ReturnThis(a_thread);
	}

	int gm_close(gmThread* a_thread) {
		_close();
		return ReturnThis(a_thread);
	}

	int gm_flush(gmThread* a_thread) {
		flushReceiver();
		return ReturnThis(a_thread);
	}

	int gm_peek(gmThread* a_thread) {
		a_thread->PushInt(_peek());
		return GM_OK;
	}

	int gm_write(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(data, 0);
		writeString(data);
		return ReturnThis(a_thread);
	}

	int gm_writeData(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Data*, GM_TYPE_OBJECT, data, 0);
		_write(data->data, data->size);
		return ReturnThis(a_thread);
	}

	int gm_read(gmThread* a_thread) {
		char buffer[4096];
		int timeout = 0;
		a_thread->ParamInt(0, timeout, 0);
		memset(buffer, 0, 4096);
//		_read(buffer, 4095, 1);
		readString(buffer, '\n', 4095, timeout);
		trim(buffer);
		a_thread->PushNewString(buffer);
		return GM_OK;
	}

	int gm_readEx(gmThread* a_thread) {
		char buffer[4096];
		GM_CHECK_INT_PARAM(maxChar,0);
		GM_CHECK_INT_PARAM(endChar,1);
		GM_CHECK_INT_PARAM(timeout,2);
		memset(buffer, 0, 4096);
//		_read(buffer, 4095, 1);
		readString(buffer, endChar, maxChar, timeout);
		trim(buffer);
		a_thread->PushNewString(buffer);
		return GM_OK;
	}

	int gm_readData(gmThread* a_thread) {
		char buffer[4096];
		GM_CHECK_INT_PARAM(maxChar, 0);
//		GM_CHECK_INT_PARAM(endChar, 1);
		GM_CHECK_INT_PARAM(timeout, 1);
		memset(buffer, 0, 4096);
		
		if (timeout == 0) { timeout = 1; }
		Data* data = new Data();
		data->resize(maxChar);
		int i=0; char d;
		while (i < maxChar) {
			if (_readChar(&d, timeout) <= 0) { // timeout
				break;
			}
			data->poke(i++, d);
		}
		data->resize(i);
		return data->ReturnThis(a_thread, false);
	}


	int _open(const char *device, const unsigned int bauds) {
#ifdef _WIN32

		// open serial port
		hSerial = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if(hSerial==INVALID_HANDLE_VALUE) {
			if(GetLastError()==ERROR_FILE_NOT_FOUND) {
				return -1;    // Device not found
			}
			return -2;                                                      // Error while opening the device
		}

		// Set parameters
		DCB dcbSerialParams = {0};                                          // Structure for the port parameters
		dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
		if (!GetCommState(hSerial, &dcbSerialParams)) {                     // Get the port parameters
			return -3;    // Error while getting port parameters
		}
		switch (bauds) {                                                    // Set the speed (Bauds)
		case 110  :
			dcbSerialParams.BaudRate=CBR_110;
			break;
		case 300  :
			dcbSerialParams.BaudRate=CBR_300;
			break;
		case 600  :
			dcbSerialParams.BaudRate=CBR_600;
			break;
		case 1200 :
			dcbSerialParams.BaudRate=CBR_1200;
			break;
		case 2400 :
			dcbSerialParams.BaudRate=CBR_2400;
			break;
		case 4800 :
			dcbSerialParams.BaudRate=CBR_4800;
			break;
		case 9600 :
			dcbSerialParams.BaudRate=CBR_9600;
			break;
		case 14400 :
			dcbSerialParams.BaudRate=CBR_14400;
			break;
		case 19200 :
			dcbSerialParams.BaudRate=CBR_19200;
			break;
		case 38400 :
			dcbSerialParams.BaudRate=CBR_38400;
			break;
		case 56000 :
			dcbSerialParams.BaudRate=CBR_56000;
			break;
		case 57600 :
			dcbSerialParams.BaudRate=CBR_57600;
			break;
		case 115200 :
			dcbSerialParams.BaudRate=CBR_115200;
			break;
		case 128000 :
			dcbSerialParams.BaudRate=CBR_128000;
			break;
		case 256000 :
			dcbSerialParams.BaudRate=CBR_256000;
			break;
		default :
			return -4;
		}
		dcbSerialParams.ByteSize=8;                                         // 8 bit data
		dcbSerialParams.StopBits=ONESTOPBIT;                                // One stop bit
		dcbSerialParams.Parity=NOPARITY;                                    // No parity
		if(!SetCommState(hSerial, &dcbSerialParams)) {                      // Write the parameters
			return -5;    // Error while writing
		}

		// Set TimeOut
		timeouts.ReadIntervalTimeout=0;                                     // Set the Timeout parameters
		timeouts.ReadTotalTimeoutConstant=MAXDWORD;                         // No TimeOut
		timeouts.ReadTotalTimeoutMultiplier=0;
		timeouts.WriteTotalTimeoutConstant=MAXDWORD;
		timeouts.WriteTotalTimeoutMultiplier=0;
		if(!SetCommTimeouts(hSerial, &timeouts)) {                          // Write the parameters
			return -6;    // Error while writting the parameters
		}
		return 1;                                                           // Opening successfull
#endif
#ifdef __GNUC__
		struct termios options;                                             // Structure with the device's options

		Log(LOG_INFO, "Serial: Opening device '%s' ...", device);

		// Open device
		fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd < 0) {
			Log(LOG_ERROR, "Serial: Could not open '%s'!", device);
			return -2;
		}
		fcntl(fd, F_SETFL, FNDELAY); // Open the device in nonblocking mode

		// Set parameters
		tcgetattr(fd, &options);                                            // Get the current options of the port
		bzero(&options, sizeof(options));                                   // Clear all the options
		speed_t         Speed;
		switch (bauds) {                                                    // Set the speed (Bauds)
		case 110  :
			Speed=B110;
			break;
		case 300  :
			Speed=B300;
			break;
		case 600  :
			Speed=B600;
			break;
		case 1200 :
			Speed=B1200;
			break;
		case 2400 :
			Speed=B2400;
			break;
		case 4800 :
			Speed=B4800;
			break;
		case 9600 :
			Speed=B9600;
			break;
		case 19200 :
			Speed=B19200;
			break;
		case 38400 :
			Speed=B38400;
			break;
		case 57600 :
			Speed=B57600;
			break;
		case 115200 :
			Speed=B115200;
			break;
		default :
			Log(LOG_ERROR, "Serial: Unknown baudrate, abborting.");
			return -4;
		}
		cfsetispeed(&options, Speed);                                       // Set the baud rate at 115200 bauds
		cfsetospeed(&options, Speed);
		options.c_cflag |= ( CLOCAL | CREAD |  CS8);                        // Configure the device : 8 bits, no parity, no control
		options.c_iflag |= ( IGNPAR | IGNBRK );
		options.c_cc[VTIME]=0;                                              // Timer unused
		options.c_cc[VMIN]=0;                                               // At least on character before satisfy reading
		tcsetattr(fd, TCSANOW, &options);                                   // Activate the settings
		return (1);                                                         // Success
#endif
	}


	void _close() {
#ifdef _WIN32
		if (hSerial) { CloseHandle(hSerial); hSerial = 0; }
#endif
#ifdef __GNUC__
		if (fd) { close(fd); fd = 0; }
#endif
	}


	char writeChar(const char Byte) {
#ifdef _WIN32
		DWORD dwBytesWritten;                                               // Number of bytes written
		if(!WriteFile(hSerial,&Byte,1,&dwBytesWritten,NULL)) {              // Write the char
			return -1;    // Error while writing
		}
		return 1;                                                           // Write operation successfull
#endif
#ifdef __GNUC__
		if (write(fd,&Byte,1)!=1) {                                         // Write the char
			return -1;    // Error while writting
		}
		return 1;                                                           // Write operation successfull
#endif
	}


	char writeString(const char *String) {
#ifdef _WIN32
		DWORD dwBytesWritten;                                               // Number of bytes written
		if(!WriteFile(hSerial,String,strlen(String),&dwBytesWritten,NULL)) { // Write the string
			return -1;    // Error while writing
		}
		return 1;                                                           // Write operation successfull
#endif
#ifdef __GNUC__
		int Lenght=strlen(String);                                          // Lenght of the string
		if (write(fd,String,Lenght)!=Lenght) {                              // Write the string
			return -1;    // error while writing
		}
		return 1;                                                           // Write operation successfull
#endif
	}


	char _write(const void *Buffer, const unsigned int NbBytes) {
#ifdef _WIN32
		DWORD dwBytesWritten;                                               // Number of byte written
		if(!WriteFile(hSerial, Buffer, NbBytes, &dwBytesWritten, NULL)) {   // Write data
			return -1;    // Error while writing
		}
		return 1;                                                           // Write operation successfull
#endif
#ifdef __GNUC__
		if (write (fd,Buffer,NbBytes)!=(ssize_t)NbBytes) {                            // Write data
			return -1;    // Error while writing
		}
		return 1;                                                           // Write operation successfull
#endif
	}


	char _readChar(char *pByte,unsigned int TimeOut_ms=0) {
#if defined (_WIN32) || defined(_WIN64)
		DWORD dwBytesRead = 0;
		timeouts.ReadTotalTimeoutConstant=TimeOut_ms;                       // Set the TimeOut
		if(!SetCommTimeouts(hSerial, &timeouts)) {                          // Write the parameters
			return -1;    // Error while writting the parameters
		}
		if(!ReadFile(hSerial,pByte, 1, &dwBytesRead, NULL)) {               // Read the byte
			return -2;    // Error while reading the byte
		}
		if (dwBytesRead==0) { return 0; }                                       // Return 1 if the timeout is reached
		return 1;                                                           // Success
#endif
#ifdef __GNUC__
		TimeOut         Timer;                                              // Timer used for timeout
		Timer.InitTimer();                                                  // Initialise the timer
		while (Timer.ElapsedTime_ms()<TimeOut_ms || TimeOut_ms==0) {        // While Timeout is not reached
			switch (read(fd,pByte,1)) {                                     // Try to read a byte on the device
			case 1  :
				return 1;                                             // Read successfull
			case -1 :
				return -2;                                            // Error while reading
			}
		}
		return 0;
#endif
	}


	int readStringNoTimeOut(char *String,char FinalChar,unsigned int MaxNbBytes) {
		unsigned int    NbBytes=0;                                          // Number of bytes read
		char            ret;                                                // Returned value from Read
		while (NbBytes<MaxNbBytes) {                                        // While the buffer is not full
			// Read a byte with the restant time
			ret = _readChar(&String[NbBytes]);
			if (ret==1) {                                                   // If a byte has been read
				if (String[NbBytes]==FinalChar) {                           // Check if it is the final char
					String  [++NbBytes]=0;                                  // Yes : add the end character 0
					return NbBytes;                                         // Return the number of bytes read
				}
				NbBytes++;                                                  // If not, just increase the number of bytes read
			}
			if (ret<0) { return ret; }                                          // Error while reading : return the error number
		}
		return -3;                                                          // Buffer is full : return -3
	}


	int readString(char *String,char FinalChar,unsigned int MaxNbBytes,unsigned int TimeOut_ms)	{
		if (TimeOut_ms==0) {
			return readStringNoTimeOut(String,FinalChar,MaxNbBytes);
		}

		unsigned int    NbBytes=0;                                          // Number of bytes read
		char            ret;                                                // Returned value from Read
		TimeOut         Timer;                                              // Timer used for timeout
		long int        TimeOutParam;
		Timer.InitTimer();                                                  // Initialize the timer

		while (NbBytes<MaxNbBytes) {                                        // While the buffer is not full
			// Read a byte with the restant time
			TimeOutParam=TimeOut_ms-Timer.ElapsedTime_ms();                 // Compute the TimeOut for the call of ReadChar
			if (TimeOutParam>0) {                                           // If the parameter is higher than zero
				ret = _readChar(&String[NbBytes],TimeOutParam);                // Wait for a byte on the serial link
				if (ret==1) {                                               // If a byte has been read

					if (String[NbBytes]==FinalChar) {                       // Check if it is the final char
						String  [++NbBytes]=0;                              // Yes : add the end character 0
						return NbBytes;                                     // Return the number of bytes read
					}
					NbBytes++;                                              // If not, just increase the number of bytes read
				}
				if (ret<0) { return ret; }                                      // Error while reading : return the error number
			}
			if (Timer.ElapsedTime_ms()>TimeOut_ms) {                        // Timeout is reached
				String[NbBytes]=0;                                          // Add the end caracter
				return 0;                                                   // Return 0
			}
		}
		return -3;                                                          // Buffer is full : return -3
	}


	int _read (void *Buffer,unsigned int MaxNbBytes,unsigned int TimeOut_ms) {
#if defined (_WIN32) || defined(_WIN64)
		DWORD dwBytesRead = 0;
		timeouts.ReadTotalTimeoutConstant=(DWORD)TimeOut_ms;                // Set the TimeOut
		if(!SetCommTimeouts(hSerial, &timeouts)) {                          // Write the parameters
			return -1;    // Error while writting the parameters
		}
		if(!ReadFile(hSerial,Buffer,(DWORD)MaxNbBytes,&dwBytesRead, NULL)) { // Read the bytes from the serial device
			return -2;    // Error while reading the byte
		}
		if (dwBytesRead!=(DWORD)MaxNbBytes) { return 0; }                       // Return 0 if the timeout is reached
		return 1;                                                           // Success
#endif
#ifdef __GNUC__
		TimeOut          Timer;                                             // Timer used for timeout
		Timer.InitTimer();                                                  // Initialise the timer
		unsigned int     NbByteRead=0;
		while (Timer.ElapsedTime_ms()<TimeOut_ms || TimeOut_ms==0) {        // While Timeout is not reached
			unsigned char* Ptr=(unsigned char*)Buffer+NbByteRead;           // Compute the position of the current byte
			int Ret=read(fd,(void*)Ptr,MaxNbBytes-NbByteRead);              // Try to read a byte on the device
			if (Ret==-1) { return -2; }                                         // Error while reading
			if (Ret>0) {                                                    // One or several byte(s) has been read on the device
				NbByteRead+=Ret;                                            // Increase the number of read bytes
				if (NbByteRead>=MaxNbBytes) {                               // Success : bytes has been read
					return 1;
				}
			}
		}
		return 0;                                                           // Timeout reached, return 0
#endif
	}


	void flushReceiver() {
#ifdef __GNUC__
		tcflush(fd,TCIFLUSH);
#endif
	}


	int _peek() {
		int Nbytes=0;
#ifdef __GNUC__
		ioctl(fd, FIONREAD, &Nbytes);
#endif
		return Nbytes;
	}


public:
#ifdef _WIN32
	HANDLE          hSerial;
	COMMTIMEOUTS    timeouts;
#endif
#ifdef __GNUC__
	int             fd;
#endif

};


#endif
