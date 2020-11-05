#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct vlstr{
	int length;
	int buflen;
	char *data;
};

struct vlstr *vlstr_new(){
	struct vlstr *str;

	str=(struct vlstr *)malloc(sizeof(struct vlstr));
	str->length=0;
	str->buflen=sizeof(void *);
	str->data=(char *)malloc(str->buflen);
	str->data[0]=0;
	return str;
}

void vlstr_delete(struct vlstr *str){
	if (!str) return;
	free(str->data);
	free(str);
}

struct vlstr *vlstr_clone(struct vlstr *stri){
	struct vlstr *str;

	if (!stri) return NULL;
	str=(struct vlstr *)malloc(sizeof(struct vlstr));
	str->length=stri->length;
	str->buflen=stri->buflen;
	str->data=(char *)malloc(str->buflen);
	strcpy(str->data,stri->data);
	return str;
}

void vlstr_clear(struct vlstr *str){
	if (!str) return;
	str->length=0;
	str->data[0]=0;
}

void vlstr_addc(struct vlstr *str,char c){
	char *s;

	if (!str) return;
	if (str->length+1>=str->buflen){
		str->buflen<<=1;
		s=(char *)malloc(str->buflen);
		strcpy(s,str->data);
		free(str->data);
		str->data=s;
	}
	str->data[str->length]=c;
	++str->length;
	str->data[str->length]=0;
}

void vlstr_adds(struct vlstr *str,char *si){
	int length;
	char *s;

	if (!str||!si) return;
	length=strlen(si)+1;
	if (str->length+length>=str->buflen){
		for (str->buflen<<=1;str->length+length>=str->buflen;str->buflen<<=1);
		s=(char *)malloc(str->buflen);
		strcpy(s,str->data);
		free(str->data);
		str->data=s;
	}
	strcpy(&str->data[str->length],si);
	str->length+=length;
}

void vlstr_addstr(struct vlstr *str,struct vlstr *str2){
	if (!str||!str2) return;
	vlstr_adds(str,str2->data);
}

struct vlstr *vlstr_concat(struct vlstr *str1,struct vlstr *str2){
	struct vlstr *str;

	if (!str1||!str2) return NULL;
	str=vlstr_new();
	vlstr_adds(str,str1->data);
	vlstr_adds(str,str2->data);
	return str;
}

void vlstr_trimend(struct vlstr *str){
	if (!str) return;
	for (--str->length;str->length>=0&&isspace((int)str->data[str->length]);--str->length){
		str->data[str->length]=0;
	}
	if (str->data[str->length]){
		++str->length;
	}
}

int vlstr_compare(struct vlstr *str,struct vlstr *str2){
	return strcmp(str->data,str2->data);
}

int vlstr_length(struct vlstr *str){
	if (!str) return -1;
	return str->length;
}

char *vlstr_getdata(struct vlstr *str){
	if (!str) return NULL;
	return str->data;
}

char *vlstr_copydata(struct vlstr *str){
	char *s;

	if (!str) return NULL;
	s=(char *)malloc(strlen(str->data)+1);
	strcpy(s,str->data);
	return s;
}
