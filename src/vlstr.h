#ifndef VLSTR_H
#define VLSTR_H

typedef void* HVLSTR;

//create empty string object
//call vlstr_delete to free memory
HVLSTR vlstr_new();

//destroy string object
void vlstr_delete(HVLSTR hvlstr);

//clone string object
//call vlstr_delete to free memory
HVLSTR vlstr_clone(HVLSTR hvlstr);

//clear data of string object
void vlstr_clear(HVLSTR hvlstr);

//add char to end
void vlstr_addc(HVLSTR hvlstr,char c);

//add string to end
void vlstr_adds(HVLSTR hvlstr,char *s);

//add data of string object to end
void vlstr_addstr(HVLSTR hvlstr,HVLSTR hvlstr2);

//concatenate 2 string object and create new string object
//call vlstr_delete to free memory
HVLSTR vlstr_concat(HVLSTR hvlstr1,HVLSTR hvlstr2);

//remove blank characters from end
void vlstr_trimend(HVLSTR hvlstr);

//compare to another string object
int vlstr_compare(HVLSTR hvlstr,HVLSTR hvlstr2);

//get length of string object
int vlstr_length(HVLSTR hvlstr);

//get the pointer to internal data
//do NOT modify the data, the structure may get corrupted
char *vlstr_getdata(HVLSTR hvlstr);

//get a copy of current data
//call free to free memory
char *vlstr_copydata(HVLSTR hvlstr);

#endif
