#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../vlstr.h"
#include "../utf8Format.h"

struct iniKey{
	char *key;
	char *value;
	struct iniKey *prev;
	struct iniKey *next;
};

struct iniSection{
	char *section;
	struct iniKey kroot;
	struct iniSection *prev;
	struct iniSection *next;
};
