#include "iniFormat.h"

char *str_copy(char *si){
	char *so;

	so=(char *)malloc(strlen(si)+1);
	strcpy(so,si);
	return so;
}

char *str_decode(char *si){
	int i,j=0,state=1;
	char *so;

	if (si[0]!='\"') return str_copy(si);
	so=(char *)malloc(strlen(si)+1);
	for (i=1;si[i];++i){
		switch (state){
		case 0://out
			if (si[i]=='\"'){
				state=1;
				break;
			}
			so[j++]=si[i];
			break;
		case 1://in
			if (si[i]=='\"'){
				state=2;
				break;
			}
			so[j++]=si[i];
			break;
		case 2://exiting
			if (si[i]=='\"'){
				state=1;
			}else{
				state=0;
			}
			so[j++]=si[i];
			break;
		default:
			;
		}
	}
	so[j]=0;
	return so;
}

char *str_encode(char *si){
	int i,j=1,quote=0;
	char *so;

	so=(char *)malloc((strlen(si)<<1)+3);
	so[0]='\"';
	for (i=0;si[i];++i){
		so[j++]=si[i];
		if (si[i]=='\"'){
			so[j++]=si[i];
			quote=1;
		}else if (si[i]==';'||si[i]=='\n'){
			quote=1;
		}
	}
	so[j++]='\"';
	so[j]=0;
	if (quote||isspace(so[1])||isspace(so[j-2])) return so;
	free(so);
	return str_copy(si);
}

void iniKey_new_r(struct iniKey *kroot,char *key,char *value){
	struct iniKey *kp;

	kp=(struct iniKey *)malloc(sizeof(struct iniKey));
	kp->key=str_copy(key);
	kp->value=str_copy(value);
	kp->prev=kroot->prev;
	kp->next=kroot;
	kroot->prev->next=kp;
	kroot->prev=kp;
}

void iniKey_new_w(struct iniKey *kroot,char *key,char *value){
	struct iniKey *kp;

	kp=(struct iniKey *)malloc(sizeof(struct iniKey));
	kp->key=str_copy(key);
	kp->value=str_encode(value);
	kp->prev=kroot->prev;
	kp->next=kroot;
	kroot->prev->next=kp;
	kroot->prev=kp;
}

void iniKey_delete(struct iniKey *kp){
	free(kp->key);
	free(kp->value);
	kp->prev->next=kp->next;
	kp->next->prev=kp->prev;
	free(kp);
}

struct iniKey *iniKey_search(struct iniKey *kroot,char *key){
	struct iniKey *kp;

	for (kp=kroot->prev;kp!=kroot;kp=kp->prev){//later data will override previous data, so search backward
		if (!strcmp(kp->key,key)){
			return kp;
		}
	}
	return NULL;
}

struct iniSection *iniSection_new(struct iniSection *sroot,char *section){
	struct iniSection *sp;

	sp=(struct iniSection *)malloc(sizeof(struct iniSection));
	sp->section=str_copy(section);
	sp->kroot.key=NULL;
	sp->kroot.value=NULL;
	sp->kroot.prev=&sp->kroot;
	sp->kroot.next=&sp->kroot;
	sp->prev=sroot->prev;
	sp->next=sroot;
	sroot->prev->next=sp;
	sroot->prev=sp;
	return sp;
}

void iniSection_delete(struct iniSection *sp){
	for (;sp->kroot.next!=&sp->kroot;){
		iniKey_delete(sp->kroot.next);
	}
	free(sp->section);
	sp->prev->next=sp->next;
	sp->next->prev=sp->prev;
	free(sp);
}

struct iniSection *iniSection_search(struct iniSection *sroot,char *section){
	struct iniSection *sp;

	for (sp=sroot->prev;sp!=sroot;sp=sp->prev){//later data will override previous data, so search backward
		if (!strcmp(sp->section,section)){
			return sp;
		}
	}
	return NULL;
}

struct iniKey *ini_searchKey(struct iniSection *sroot,char *section,char *key){
	struct iniSection *sp;
	struct iniKey *kp;

	sp=iniSection_search(sroot,section);
	if (!sp) return NULL;
	kp=iniKey_search(&sp->kroot,key);
	if (!kp) return NULL;
	return kp;
}

struct iniSection *ini_new(){
	struct iniSection *sroot;
	sroot=(struct iniSection *)malloc(sizeof(struct iniSection));
	sroot->section=NULL;
	sroot->kroot.key=NULL;
	sroot->kroot.value=NULL;
	sroot->kroot.prev=NULL;
	sroot->kroot.next=NULL;
	sroot->prev=sroot;
	sroot->next=sroot;
	return sroot;
}

void ini_delete(struct iniSection *sroot){
	if (!sroot) return;
	if (sroot->section){
		free(sroot->section);
	}
	for (;sroot->next!=sroot;){
		iniSection_delete(sroot->next);
	}
	free(sroot);
}

struct iniSection *ini_read(FILE *fp){
	int c,state=0,endf=0;
	HVLSTR sstr,kstr,vstr;
	struct iniSection *sroot,*sp=NULL;

	if (!fp) return NULL;
	sstr=vlstr_new();
	kstr=vlstr_new();
	vstr=vlstr_new();
	sroot=ini_new();
	utf8_checkBOM(fp);
	for (c=fgetc(fp);;c=fgetc(fp)){
		switch (state){
		case 0://start
			if (c<0||isspace(c)) break;
			if (c==';'){
				state=1;
				break;
			}
			if (c=='['){
				vlstr_clear(sstr);
				state=2;
				break;
			}
			if (c=='='||c=='\"'||sp==NULL){
				state=-1;
				break;
			}
			vlstr_addc(kstr,c);
			state=3;
			break;
		case 1://comment
			if (c=='\n'){
				state=0;
			}
			break;
		case 2://section
			if (c<0||c==';'||c=='\n'||c=='['){
				state=-1;
				break;
			}
			if (c==']'){
				sp=iniSection_new(sroot,vlstr_getData(sstr));
				state=0;
				break;
			}
			vlstr_addc(sstr,c);
			break;
		case 3://key
			if (c<0||c==';'||c=='\n'||c=='['||c==']'||c=='\"'){
				state=-1;
				break;
			}
			if (c=='='){
				state=4;
				break;
			}
			vlstr_addc(kstr,c);
			break;
		case 4://value start
			if (c<0||c==';'||c=='\n'){
				vlstr_trimEnd(kstr);
				iniKey_new_r(&sp->kroot,vlstr_getData(kstr),"");
				vlstr_clear(kstr);
				if (c==';'){
					state=1;
				}else{
					state=0;
				}
				break;
			}
			if (isspace(c)) break;
			vlstr_addc(vstr,c);
			if (c=='\"'){
				state=6;
			}else{
				state=5;
			}
			break;
		case 5://value
			if (c<0||c==';'||c=='\n'){
				vlstr_trimEnd(kstr);
				vlstr_trimEnd(vstr);
				iniKey_new_r(&sp->kroot,vlstr_getData(kstr),vlstr_getData(vstr));
				vlstr_clear(kstr);
				vlstr_clear(vstr);
				if (c==';'){
					state=1;
				}else{
					state=0;
				}
				break;
			}
			vlstr_addc(vstr,c);
			if (c=='\"'){
				state=6;
			}
			break;
		case 6://quote
			vlstr_addc(vstr,c);
			if (c=='\"'){
				state=5;
			}
			break;
		default://error
			vlstr_delete(sstr);
			vlstr_delete(kstr);
			vlstr_delete(vstr);
			ini_delete(sroot);
			return NULL;
		}
		if (c<0){
			if (endf) break;
			ungetc('\n',fp);
			endf=1;
		}
	}
	vlstr_delete(sstr);
	vlstr_delete(kstr);
	vlstr_delete(vstr);
	if (!state) return sroot;
	ini_delete(sroot);
	return NULL;
}

void ini_write(FILE *fp,struct iniSection *sroot){
	struct iniSection *sp;
	struct iniKey *kp;

	if (!fp||!sroot) return;
	for (sp=sroot->next;sp!=sroot;sp=sp->next){
		fprintf(fp,"[%s]\r\n",sp->section);
		for (kp=sp->kroot.next;kp!=&sp->kroot;kp=kp->next){
			fprintf(fp,"%s=%s\r\n",kp->key,kp->value);
		}
		fprintf(fp,"\r\n");
	}
}

char *ini_getValue(struct iniSection *sroot,char *section,char *key){
	struct iniKey *kp;

	if (!sroot||!section||!key) return NULL;
	if (sroot->section){
		free(sroot->section);
		sroot->section=NULL;
	}
	kp=ini_searchKey(sroot,section,key);
	if (kp){
		sroot->section=str_decode(kp->value);
	}
	return sroot->section;
}

void ini_setValue(struct iniSection *sroot,char *section,char *key,char *value){
	struct iniSection *sp;
	struct iniKey *kp;

	if (!sroot||!section||!key||!value) return;
	sp=iniSection_search(sroot,section);
	if (!sp){
		sp=iniSection_new(sroot,section);
		iniKey_new_w(&sp->kroot,key,value);
		return;
	}
	kp=iniKey_search(&sp->kroot,key);
	if (!kp){
		iniKey_new_w(&sp->kroot,key,value);
		return;
	}
	free(kp->value);
	kp->value=str_encode(value);
}
