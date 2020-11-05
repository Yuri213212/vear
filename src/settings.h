#define NoteCount	12

int StartNote=4,RefNote=48,DefTranspose=0,GridLines=16;
double BaseNote=4.75,BaseFreq=440.0;
float ThresDB=48.0,DecayDB=2.0,GridDB=3.0,Depress=0.5,Feedback=1.0,MinColor=2.0,Gamma=0.5;

int readnote(char *s){
	int result;

	switch (s[0]){
	case 'C':
		result=0;
		break;
	case 'D':
		result=2;
		break;
	case 'E':
		result=4;
		break;
	case 'F':
		result=5;
		break;
	case 'G':
		result=7;
		break;
	case 'A':
		result=9;
		break;
	case 'B':
		result=11;
		break;
	default:
		return -1;
	}
	switch (s[1]){
	case '-':
		break;
	case '#':
		result+=1;
		break;
	default:
		return -1;
	}
	if (s[2]>='0'&&s[2]<='9'){
		result+=(s[2]-'0')*NoteCount;
	}else{
		return -1;
	}
	return result;
}

int readSettings(FILE *fp){
	HINI hini;
	char *s;
	int temp;

	hini=ini_read(fp);
	if (!hini) return 0;
	s=ini_getValue(hini,"vear","BaseNote");
	if (s){
		temp=readnote(s);
		if (temp>=0){
			BaseNote=(double)temp/NoteCount;
		}
	}
	s=ini_getValue(hini,"vear","BaseFreq");
	if (s){
		sscanf(s,"%lf",&BaseFreq);
	}
	s=ini_getValue(hini,"vear","StartNote");
	if (s){
		temp=readnote(s);
		if (temp>=0){
			StartNote=temp;
		}
	}
	s=ini_getValue(hini,"vear","RefNote");
	if (s){
		temp=readnote(s);
		if (temp>=0){
			RefNote=temp;
		}
	}
	s=ini_getValue(hini,"vear","DefTranspose");
	if (s){
		sscanf(s,"%d",&DefTranspose);
		if (DefTranspose<0){
			DefTranspose=NoteCount-(-DefTranspose)%NoteCount;
		}else{
			DefTranspose%=NoteCount;
		}
	}
	s=ini_getValue(hini,"vear","GridLines");
	if (s){
		sscanf(s,"%d",&GridLines);
		if (GridLines<1){
			GridLines=1;
		}
	}
	s=ini_getValue(hini,"vear","ThresDB");
	if (s){
		sscanf(s,"%f",&ThresDB);
		if (ThresDB<0.0f){
			ThresDB=-ThresDB;
		}
	}
	s=ini_getValue(hini,"vear","DecayDB");
	if (s){
		sscanf(s,"%f",&DecayDB);
		if (DecayDB<0.0f){
			DecayDB=-DecayDB;
		}
	}
	s=ini_getValue(hini,"vear","GridDB");
	if (s){
		sscanf(s,"%f",&GridDB);
		if (GridDB<0.0f){
			GridDB=-GridDB;
		}
	}
	s=ini_getValue(hini,"vear","Depress");
	if (s){
		sscanf(s,"%f",&Depress);
		if (fabsf(Depress)>1.0f){
			Depress=1.0f/Depress;
		}
		if (Depress<0.0f){
			Depress=1.0f-Depress;
		}
	}
	s=ini_getValue(hini,"vear","Feedback");
	if (s){
		sscanf(s,"%f",&Feedback);
		if (fabsf(Feedback)>1.0f){
			Feedback=1.0f/Feedback;
		}
		if (Feedback<0.0f){
			Feedback=1.0f-Feedback;
		}
	}
	s=ini_getValue(hini,"vear","MinColor");
	if (s){
		sscanf(s,"%f",&MinColor);
		if (MinColor>3.0f){
			MinColor=3.0f;
		}
		if (MinColor<0.0f){
			MinColor=0.0f;
		}
	}
	s=ini_getValue(hini,"vear","Gamma");
	if (s){
		sscanf(s,"%f",&Gamma);
	}
	ini_delete(hini);
	return 1;
}
