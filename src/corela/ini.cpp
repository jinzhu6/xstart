#include "corela.h"
#include "cont.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#define snprintf _snprintf
#define strnicmp _strnicmp
#else
#define strnicmp strncasecmp
#endif


/* ----------------------------------------------------------------------------------------
// _Ini_strnlen - Returns the length of the string "p" but no value higher than than "max".
//----------------------------------------------------------------------------------------- */
static INLINE size_t _Ini_strnlen(const char* p, size_t max) {
	size_t n = 0;
	while(p[n] != '\0') {
		n++;
		if(n >= max) {
			return max;
		}
	}
	return n > max ? max : n;
}

/* ----------------------------------------------------------------------------------------
// _IniStripString - Strips special characters from "szString" at the beginning and end.
//----------------------------------------------------------------------------------------- */
static char* _IniStripString(char* szString) {
	int n;
	char* szPtr;

	szPtr = szString;

	for(n = 0; ; n++) {
		switch(szString[n]) {
		case '\n':
		case '\f':
		case '\r':
		case '\t':
		case '\v':
		case '\a':
		case ' ':
			szPtr++;
			break;
		default:
			goto strip_end;
		}
	}

strip_end:

	for(n = (int)_Ini_strnlen(szPtr, MAX_INI_VALUE) - 1; n >= 0; n--) {
		switch(szPtr[n]) {
		case '\n':
		case '\f':
		case '\r':
		case '\t':
		case '\v':
		case '\a':
		case ' ':
			szPtr[n] = 0;
			break;
		default:
			goto stripped;
		}
	}

stripped:

	return szPtr;
}

/* ----------------------------------------------------------------------------------------
// _IniNew - Creates a new INI object
//----------------------------------------------------------------------------------------- */
static INI* _IniNew() {
	INI* ini = (INI*)malloc(sizeof(INI));
	ini->isDirty = false;
	ini->cSections = cnew();
	return ini;
}

/* ----------------------------------------------------------------------------------------
// _IniNewSection - Creates a new INI section
//----------------------------------------------------------------------------------------- */
static INISECTION* _IniNewSection(const char* szName, INI* ini) {
	INISECTION* iniSec = (INISECTION*)malloc(sizeof(INISECTION));
	iniSec->ini = ini;
	iniSec->cEntry = cnew();
	strncpy(iniSec->szName, szName, MAX_INI_KEY);
	return iniSec;
}

/* ----------------------------------------------------------------------------------------
// _IniNewEntry - Creates a new INI entry
//----------------------------------------------------------------------------------------- */
static INIENTRY* _IniNewEntry(const char* szKey, const char* szValue, INI* ini) {
	INIENTRY* ent;
	ent = (INIENTRY*)malloc(sizeof(INIENTRY));
	ent->ini = ini;
	strncpy(ent->szKey, szKey, MAX_INI_KEY);
	strncpy(ent->szValue, szValue, MAX_INI_VALUE);
	return ent;
}

/* ----------------------------------------------------------------------------------------
// _IniParse_Entry - Parses a entry name from a file pointer
//----------------------------------------------------------------------------------------- */
static INIENTRY* _IniParse_Entry(FILE* hf, INISECTION* iniSec) {
	INIENTRY* iniEnt;
	char c;
	int n,t;
	char szBuf[2][MAX_INI_VALUE + 1];
	char* szKey, *szValue;
	int quotes = 0;

	t = 0;
	n = 0;

	while(fread(&c,1,1,hf)) {
		switch(c) {
		case ' ':
		case '\t':
		case '\v':
			if(n==0) {
				break;
			}
			if(n < MAX_INI_VALUE) {
				szBuf[t][n++] = c;
			}
			break;

		case '=':
			if(t == 0) {
				szBuf[t][n] = '\0';
				t = 1;
				n = 0;
				break;
			}
			if(n < MAX_INI_VALUE) {
				szBuf[t][n++] = c;
			}
			break;

		case '\n':
		case '\r':
			if(t == 1) {
				szBuf[t][n] = '\0';
				goto done;
			} else {
				break;
			}

		case '\\':
			char c2;
			fread(&c2,1,1,hf);
			if(c2 == '"') {
				if(n < MAX_INI_VALUE) {
					szBuf[t][n++] = '"';
				}
			} else if(c2 == 'n') {
				if(n < MAX_INI_VALUE) {
					szBuf[t][n++] = '\n';
				}
			} else if(c2 == 't') {
				if(n < MAX_INI_VALUE) {
					szBuf[t][n++] = '\t';
				}
			} else {
				if(n < MAX_INI_VALUE) {
					szBuf[t][n++] = '\\';
				}
				if(n < MAX_INI_VALUE) {
					szBuf[t][n++] = c2;
				}
			}
			break;

		case '"':
			if(t == 1 && n==0) {
				quotes = 1;

				while(fread(&c,1,1,hf)) {
					if(c == '\\') {
						char c2;
						fread(&c2,1,1,hf);
						if(c2 == '"') {
							if(n < MAX_INI_VALUE) {
								szBuf[t][n++] = '"';
							}
						} else if(c2 == 'n') {
							if(n < MAX_INI_VALUE) {
								szBuf[t][n++] = '\n';
							}
						} else if(c2 == 't') {
							if(n < MAX_INI_VALUE) {
								szBuf[t][n++] = '\t';
							}
						} else {
							if(n < MAX_INI_VALUE) {
								szBuf[t][n++] = '\\';
							}
							if(n < MAX_INI_VALUE) {
								szBuf[t][n++] = c2;
							}
						}
						continue;
					}

					if(c == '"') {
						break;
					}

					if(n < MAX_INI_VALUE) {
						szBuf[t][n++] = c;
					}
				}
				break;
			}

		default:
			if(n < MAX_INI_VALUE) {
				szBuf[t][n++] = c;
			}
		}
	}

	if(t == 0) {
		return 0;
	}
	szBuf[t][n] = '\0';

done:

	szKey   = _IniStripString(&szBuf[0][0]);
	if(!quotes) {
		szValue = _IniStripString(&szBuf[1][0]);
	} else {
		szValue = &szBuf[1][0];
	}

	iniEnt = _IniNewEntry(szKey, szValue, iniSec->ini);
	iniEnt->bInQuotes = quotes != 0;
	cpush(iniSec->cEntry, (void*)iniEnt);

	return iniEnt;
}

/* ----------------------------------------------------------------------------------------
// _IniParse_Section - Parses a section name from a file pointer
//----------------------------------------------------------------------------------------- */
static INISECTION* _IniParse_Section(FILE* hf, INI* ini) {
	char c;
	int n;
	char szBuf[MAX_INI_KEY + 1];
	INISECTION* iniSec;

	n = 0;
	while(fread(&c,1,1,hf)) {
		switch(c) {
		case ']':
			szBuf[n] = '\0';
			iniSec = _IniNewSection(szBuf, ini);
			cpush(ini->cSections, (void*)iniSec);
			return iniSec;

		case '\n':
		case '\r':
		case '\f':
		case '\t':
		case '\v':
		case '\a':
			c = ' ';

		default:
			if(n < MAX_INI_KEY) {
				szBuf[n++] = c;
			}
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------
// _IniParse - Parses a INI file
//----------------------------------------------------------------------------------------- */
static void _IniParse(FILE* hf, INI* ini) {
	INISECTION* iniSec = 0;
	char c;

	while(fread(&c,1,1,hf)) {
		switch(c) {
		case ' ':
		case '\n':
		case '\r':
		case '\f':
		case '\t':
		case '\v':
		case '\a':
			break;

		case ';':
		case '\'':
		case '/':
		case '#':
			while(fread(&c,1,1,hf)) {
				if(c=='\n' || c=='\r') {
					break;
				}
			}

		case '[':
			iniSec = _IniParse_Section(hf, ini);
			break;

		default:
			if(iniSec) {
				fseek(hf, -1, SEEK_CUR);
				_IniParse_Entry(hf, iniSec);
			}
			break;
		}
	}
}

/* ----------------------------------------------------------------------------------------
// _IniDestroy - Destroys an INI object
//----------------------------------------------------------------------------------------- */
void _IniDestroy(INI* ini) {
	INISECTION* iniSec;

	while(ccount(ini->cSections)) {
		iniSec = (INISECTION*)cfirst(ini->cSections)->pData;

		while(ccount(iniSec->cEntry)) {
			free(cpop(iniSec->cEntry));
		}

		cdel(iniSec->cEntry);

		free(cpop_front(ini->cSections));
	}
	cdel(ini->cSections);
	free(ini);
}

/* ----------------------------------------------------------------------------------------
// INIOpen - Opens or creates a INI object
//----------------------------------------------------------------------------------------- */
INI* INIOpen(const char* szFile, coBool bCreate) {
	INI* ini;
	FILE* hf;

	if(!szFile) {
		return 0;
	}

	ini = _IniNew();

	hf = fopen(szFile, "rb");
	if(!hf) {
		if(!bCreate) {
			_IniDestroy(ini);
			return 0;
		}
		ini->isDirty = true;
	}

	if(hf) {
		_IniParse(hf, ini);
		fclose(hf);
	}

	strncpy(ini->szFile, szFile, MAX_INI_FILE);

	return ini;
}

/* ----------------------------------------------------------------------------------------
// _IniSave - Saves an INI object to a file
//----------------------------------------------------------------------------------------- */
int _IniSave(INI* ini, const char* szFile) {
	FILE* hf;
	CONT_ITEM* ciSec, *ciEnt;
	INISECTION* iniSec;
	INIENTRY* iniEnt;
	char szFormat[64];
	char szLine[MAX_INI_KEY+MAX_INI_VALUE+64];

	if(!ini) {
		return 0;
	}

	hf = fopen(szFile, "wb");
	if(!hf) {
		return 0;
	}

	ciSec = cfirst(ini->cSections);
	while(ciSec) {
		iniSec = (INISECTION*)cidata(ciSec);

		sprintf(szFormat, "[%%.%ds]\n", MAX_INI_KEY);
		sprintf(szLine, szFormat, iniSec->szName);
		fwrite(szLine, strlen(szLine),1,hf);
//		fprintf(hf, szFormat, iniSec->szName);

		ciEnt = cfirst(iniSec->cEntry);
		while(ciEnt) {
			iniEnt = (INIENTRY*)cidata(ciEnt);
			if(iniEnt->bInQuotes) {
				sprintf(szFormat, "%%.%ds=\"%%.%ds\"\n", MAX_INI_KEY, MAX_INI_VALUE);
			} else {
				sprintf(szFormat, "%%.%ds=%%.%ds\n", MAX_INI_KEY, MAX_INI_VALUE);
			}
			sprintf(szLine, szFormat, iniEnt->szKey, iniEnt->szValue);
			fwrite(szLine, strlen(szLine),1,hf);
//			fprintf(hf, szFormat, iniEnt->szKey, iniEnt->szValue);

			ciEnt = cinext(ciEnt);
		}

		strcpy(szLine, "\n");
		fwrite(szLine,strlen(szLine),1,hf);
//		fprintf(hf, "\n");

		ciSec = cinext(ciSec);
	}
	fclose(hf);

	return 1;
}

/* ----------------------------------------------------------------------------------------
// IniClose - Closes and saves a INI file
//----------------------------------------------------------------------------------------- */
void INIClose(INI* ini, int bSave) {
	if(!ini) {
		return;
	}
	if(bSave && ini->isDirty) {
		_IniSave(ini, ini->szFile);
	}
	_IniDestroy(ini);
}


/* ----------------------------------------------------------------------------------------
// INIEnumSections
//----------------------------------------------------------------------------------------- */
INISECTION* INIEnumSections(INI* ini, coDword dwNum) {
	coDword n=0;
	CONT_ITEM* ci = cfirst(ini->cSections);
	while(ci) {
		if(n==dwNum) {
			return (INISECTION*)cidata(ci);
		}
		n++;
		ci=cinext(ci);
	}
	return 0;
}

/* ----------------------------------------------------------------------------------------
// INIEnumEntries
//----------------------------------------------------------------------------------------- */
INIENTRY* INIEnumEntries(INISECTION* sec, coDword dwNum) {
	coDword n=0;
	CONT_ITEM* ci = cfirst(sec->cEntry);
	while(ci) {
		if(n==dwNum) {
			return (INIENTRY*)cidata(ci);
		}
		n++;
		ci=cinext(ci);
	}
	return 0;
}


/* ----------------------------------------------------------------------------------------
// INIGetSection - Gets or creates an INI section
//----------------------------------------------------------------------------------------- */
INISECTION* INIGetSection(INI* ini, const char* szSec, coBool bCreate) {
	INISECTION* iniSec;

	CONT_ITEM* ci = cfirst(ini->cSections);
	while(ci) {
		iniSec = (INISECTION*)cidata(ci);
		if(strnicmp(iniSec->szName, szSec, MAX_INI_KEY) == 0) {
			return iniSec;
		}
		ci = cinext(ci);
	}

	if(bCreate) {
		iniSec = _IniNewSection(szSec, ini);
		cpush(ini->cSections, (void*)iniSec);
		ini->isDirty = true;
		return iniSec;
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------
// INIGetEntry - Gets an entry in a section
//----------------------------------------------------------------------------------------- */
INIENTRY* INIGetEntry(INISECTION* iniSec, const char* szKey, coBool bCreate, const char* szCreationValue) {
	INIENTRY* iniEnt = 0;

	CONT_ITEM* ci = cfirst(iniSec->cEntry);
	while(ci) {
		iniEnt = (INIENTRY*)cidata(ci);
		if(strnicmp(iniEnt->szKey, szKey, MAX_INI_KEY) == 0) {
			return iniEnt;
		}
		ci = cinext(ci);
	}

	if(bCreate) {
		if(szCreationValue) {
			iniEnt = _IniNewEntry(szKey, szCreationValue, iniSec->ini);
		} else {
			iniEnt = _IniNewEntry(szKey, "", iniSec->ini);
		}
		cpush(iniSec->cEntry, (void*)iniEnt);
		iniSec->ini->isDirty = true;
		return iniEnt;
	}

	return 0;
}



#if 0
/* ----------------------------------------------------------------------------------------
// IniGetEntryAsFloat
//----------------------------------------------------------------------------------------- */
float INIGetEntryAsFloat(INISECTION* iniSec, const char* szKey, int bCreate) {
	INIENTRY* iniEnt = INIGetEntry(iniSec, szKey, bCreate, "0.0");
	return INIGetEntryFloat(iniEnt);
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryAsInt
//----------------------------------------------------------------------------------------- */
int INIGetEntryAsInt(INISECTION* iniSec, const char* szKey, int bCreate) {
	INIENTRY* iniEnt = INIGetEntry(iniSec, szKey, bCreate, "0");
	return INIGetEntryNum(iniEnt);
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryAsDword
//----------------------------------------------------------------------------------------- */
waDword INIGetEntryAsDword(INISECTION* iniSec, const char* szKey, int bCreate) {
	INIENTRY* iniEnt = INIGetEntry(iniSec, szKey, bCreate, "0");
	return strtoul(iniEnt->szValue, 0, 0);
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryAsString
//----------------------------------------------------------------------------------------- */
char* INIGetEntryAsString(char* szOut, int nMax, INISECTION* iniSec, const char* szKey, int bCreate) {
	INIENTRY* iniEnt = INIGetEntry(iniSec, szKey, bCreate, "");
	strncpy(szOut, iniEnt->szValue, nMax);
	return iniEnt->szValue;
}
#endif



/* ----------------------------------------------------------------------------------------
// INIGetEntryStr
//----------------------------------------------------------------------------------------- */
char* INIGetEntryStr(INIENTRY* iniEnt, char* str, int nMaxLen) {
	strncpy(str, iniEnt->szValue, nMaxLen);
	str[nMaxLen-1] = '\0';
	return str;
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryNum
//----------------------------------------------------------------------------------------- */
long INIGetEntryNum(INIENTRY* iniEnt) {
	long l=0;
	sscanf(iniEnt->szValue , "%d", &l);
	return l;
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryHex
//----------------------------------------------------------------------------------------- */
coDword INIGetEntryHex(INIENTRY* iniEnt) {
	coDword h=0;
	if(!sscanf(iniEnt->szValue, "%X", &h)) {
		sscanf(iniEnt->szValue, "%x", &h);
	}
	return h;
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryFloat
//----------------------------------------------------------------------------------------- */
float INIGetEntryFloat(INIENTRY* iniEnt) {
	float f=0;
	sscanf(iniEnt->szValue , "%f", &f);
	return f;
}

/* ----------------------------------------------------------------------------------------
// INIGetEntryVec3
//----------------------------------------------------------------------------------------- */
/*VEC3* INIGetEntryVec3(INIENTRY* iniEnt, VEC3* v)
{
    sscanf(iniEnt->szValue, " ( %f , %f , %f ) ", &v->x, &v->y, &v->z);
    return v;
}*/

/* ----------------------------------------------------------------------------------------
// INIGetEntryColor
//----------------------------------------------------------------------------------------- */
/*WACOLORF* INIGetEntryColor(INIENTRY* iniEnt, WACOLORF* c)
{
    sscanf(iniEnt->szValue, " ( %f , %f , %f , %f ) ", &c->r, &c->g, &c->b, &c->a);
    return c;
}*/

/* ----------------------------------------------------------------------------------------
// INIGetEntryMatrix
//----------------------------------------------------------------------------------------- */
/*MATRIX44* INIGetEntryMatrix(INIENTRY* iniEnt, MATRIX44* m)
{
    sscanf(iniEnt->szValue, " ( %f , %f , %f , %f ) , ( %f , %f , %f , %f ) , ( %f , %f , %f , %f ) , ( %f , %f , %f , %f ) ",
           &m->_11, &m->_12, &m->_13, &m->_14,
           &m->_21, &m->_22, &m->_23, &m->_24,
           &m->_31, &m->_32, &m->_33, &m->_34,
           &m->_41, &m->_42, &m->_43, &m->_44);
    return m;
}*/


/* ----------------------------------------------------------------------------------------
// INISetEntry - Set entry value
//----------------------------------------------------------------------------------------- */
void INISetEntryStr(INIENTRY* iniEnt, const char* str) {
	strncpy(iniEnt->szValue, str, MAX_INI_VALUE);
	iniEnt->bInQuotes = coTrue;
	iniEnt->ini->isDirty = true;
}


/* ----------------------------------------------------------------------------------------
// IniSetEntryLong - Set entry value
//----------------------------------------------------------------------------------------- */
void INISetEntryNum(INIENTRY* iniEnt, int n) {
	char szBuf[16];
	snprintf(szBuf, 16, "%d", n);
	strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
	iniEnt->bInQuotes = coFalse;
	iniEnt->ini->isDirty = true;
}

/* ----------------------------------------------------------------------------------------
// IniSetEntryHex - Set entry value
//----------------------------------------------------------------------------------------- */
void INISetEntryHex(INIENTRY* iniEnt, coDword h) {
	char szBuf[16];
	snprintf(szBuf, 16, "%x", h);
	strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
	iniEnt->bInQuotes = coFalse;
	iniEnt->ini->isDirty = true;
}

/* ----------------------------------------------------------------------------------------
// INISetEntryFloat - Set entry value
//----------------------------------------------------------------------------------------- */
void INISetEntryFloat(INIENTRY* iniEnt, float f) {
	char szBuf[64];
	snprintf(szBuf, 64, "%f", f);
	strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
	iniEnt->bInQuotes = coFalse;
	iniEnt->ini->isDirty = true;
}

/* ----------------------------------------------------------------------------------------
// INISetEntryVec3 - Set entry value
//----------------------------------------------------------------------------------------- */
/*void INISetEntryVec3(INIENTRY* iniEnt, VEC3* v)
{
    char szBuf[64];
    snprintf(szBuf, 64, "(%f, %f, %f)", v->x, v->y, v->z);
    strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
    iniEnt->bInQuotes = waFalse;
}*/

/* ----------------------------------------------------------------------------------------
// INISetEntryColor - Set entry value
//----------------------------------------------------------------------------------------- */
/*void INISetEntryColor(INIENTRY* iniEnt, WACOLORF* c)
{
    char szBuf[96];
    snprintf(szBuf, 96, "(%f, %f, %f, %f)", c->r, c->g, c->b, c->a);
    strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
    iniEnt->bInQuotes = waFalse;
}*/

/* ----------------------------------------------------------------------------------------
// INISetEntryMatrix - Set entry value
//----------------------------------------------------------------------------------------- */
/*void INISetEntryMatrix(INIENTRY* iniEnt, MATRIX44* m)
{
    char szBuf[96*4];
    snprintf(szBuf, 96*4, "\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f)",
           m->_11, m->_12, m->_13, m->_14,
           m->_21, m->_22, m->_23, m->_24,
           m->_31, m->_32, m->_33, m->_34,
           m->_41, m->_42, m->_43, m->_44);
    strncpy(iniEnt->szValue, szBuf, MAX_INI_VALUE);
    iniEnt->bInQuotes = waFalse;
}*/
