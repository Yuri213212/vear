#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct vlstr{
	int length;
	int buflen;
	char *data;
};
