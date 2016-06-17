#ifndef _FC7081A6_68B8_4ec2_8DDC_88A60BFEA460_
#define _FC7081A6_68B8_4ec2_8DDC_88A60BFEA460_

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

//#define INLINE inline

#if 0
INLINE size_t strlen(const char * String)
{
	const char *eos = String;

	while(*eos++);

	return((int)(eos - String - 1));
}

INLINE char* strcpy(char * dst, const char * src)
{
	char * cp = dst;

	while(*cp++ = *src++);

	return( dst );
}

INLINE char* strcat(char * dst, const char * src)
{
	char * cp = dst;

	while( *cp )
		cp++;

	while(*cp++ = *src++);

	return( dst );
}
#endif


class String
{
public:

	// std ctor
	String()
	{
		m_Str = (char*)malloc(1);
		m_Str[0] = 0;
		m_Max = 0;
	}

	// ctor (String)
	String(const String &String)
	{
		m_Max = String.len();
		m_Str = (char*)malloc(m_Max + 1);

		char* pstr = String();
		for(int n=0; n <= m_Max; n++)
			m_Str[n] = pstr[n];
	}

	// ctor (char*)
	String(const char *s)
	{
		if(s)
		{
			m_Max = strlen(s);
			m_Str = (char*)malloc(m_Max + 1);
			for(int n=0; n <= m_Max; n++)
				m_Str[n] = s[n];
		}
	}

	// ctor (char)
	String(char c)
	{
		m_Str = (char*)malloc(2);
		m_Max = 1;
		m_Str[0] = c;
		m_Str[1] = 0;
	}

	// ctor(int)
	String(int num)
	{
		m_Str = (char*)malloc(33);
		m_Max = 32;
		_itoa(num, m_Str, 10);
	}

	// ctor(long)
	String(long num)
	{
		m_Str = (char*)malloc(33);
		m_Max = 32;
		_ltoa(num, m_Str, 10);
	}

	// ctor(unsigned long)
	String(unsigned long num)
	{
		m_Str = (char*)malloc(33);
		m_Max = 32;
		_ultoa(num, m_Str, 10);
	}

	// ctor(double)
	String(double num)
	{
		m_Str = (char*)malloc(65);
		m_Max = 64;
		_gcvt(num, 16, m_Str);
	}

	// ctor(double)
	String(double num, int prec)
	{
		char format[16];
		sprintf(format, "%s.%df", "%", prec);

		m_Str = (char*)malloc(65);
		m_Max = 64;
		_snprintf(m_Str, 64, format, num);
		
		/*m_Str = (char*)malloc(65);
		m_Max = 64;
		_gcvt(num, prec+1, m_Str);
		if(m_Str[strlen(m_Str)-1] == '.') m_Str[strlen(m_Str)-1] = 0;*/
	}

	// dtor
	virtual ~String()
	{
		free(m_Str);
	}

	// len
	INLINE int len() const
	{
		return strlen(m_Str);
	}

	// max
	#undef max
	INLINE int max()
	{
		return m_Max;
	}

	// set max
	INLINE void max(int max)
	{
		m_Str = (char*)realloc((void*)m_Str, max);
		m_Max = max;
	}

	// ()
	INLINE char* operator()(int n = 0) const
	{
		return &m_Str[n];
	}

	// char*
	INLINE operator char*() const
	{
		return &m_Str[0];
	}

	// + (String)
	INLINE String operator+(const String &add)
	{
		String tmp(*this);
		tmp << add;
		return tmp;
	}

	// =
	INLINE String& operator=(const String &c)
	{
		m_Max = c.len();
		m_Str = (char*)realloc(m_Str, m_Max + 1);
		char* pc = c();
		for(int n=0; n <= m_Max; n++)
			m_Str[n] = pc[n];
		return *this;
	}

	// <<
	INLINE String& operator<<(const String &c)
	{
		m_Max += c.len();
		m_Str = (char*)realloc(m_Str, m_Max + 1);
		strcat(m_Str, c());

		return *this;
	}

	// <<
	INLINE String& operator<<(const char *s)
	{
		m_Max += strlen(s);
		m_Str = (char*)realloc(m_Str, m_Max + 1);
		strcat(m_Str, s);

		return *this;
	}

	// []
	INLINE char& operator[](int n)
	{
		return m_Str[n];
	}

	// !=
	INLINE int operator!=(const char* c)
	{
		if(strcmp(m_Str, c) == 0)
			return 0;
		return 1;
	}

	// ==
	INLINE int operator==(const char* c)
	{
		if(strcmp(m_Str, c) == 0)
			return 1;
		return 0;
	}

	// ==
	INLINE int operator==(const String& s)
	{
		if(strcmp(m_Str, s.c_str()) == 0)
			return 1;
		return 0;
	}

	// c_str
	INLINE char* c_str() const
	{
		return m_Str;
	}

private:
	char	*m_Str;
	int		 m_Max;
};


#endif
