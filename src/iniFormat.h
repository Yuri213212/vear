#ifndef INIFORMAT_H
#define INIFORMAT_H

#include <stdio.h>

typedef void* HINI;

//create empty ini object
//call ini_delete to free memory
HINI ini_new();

//destroy ini object
void ini_delete(HINI hini);

//read ini file to create ini object
//call ini_delete to free memory
HINI ini_read(FILE *fp);

//save ini object to ini file
void ini_write(FILE *fp,HINI hini);

//query value from ini object
//result is null when failed, or a pointer to copied value (auto decode, auto free)
//returned pointer will be void if ini_getValue or ini_delete was called
char *ini_getValue(HINI hini,char *section,char *key);

//change latest existing value or add new value to ini object
//value will be auto encoded and copied
void ini_setValue(HINI hini,char *section,char *key,char *value);

#endif
