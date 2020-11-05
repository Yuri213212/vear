#define DisplayCount	98
#define Density		1
#define BarCount	(DisplayCount*Density+2)

struct rlcdt{
	float c1;
	float c2;
	float c3;
	float c4;
	float c5;
	float i;
	float v;
	float out;
};

struct rlcdt bar[BarCount];
float pre[DisplayCount][2],post[DisplayCount],max[DisplayCount];
int rlcframe=0;

void rlc_init(){
	double t,t1,t2,t3,s,c1,c2;
	int i;

	t=pow(2.0,1.0/(NoteCount*Density));
	t1=t-1.0;
	t2=sqrt(t);
	t3=sqrt(6.0*t-t*t-1.0);
	for (i=0;i<BarCount;++i){
		s=2.0*M_PI*BaseFreq*pow(2.0,(double)(i+StartNote*Density-1)/(NoteCount*Density)-BaseNote)/SampleRate;
		c1=exp(-0.5*s*t1/t2)*cos(0.5*s*t3/t2);
		c2=exp(-0.5*s*t1/t2)*sin(0.5*s*t3/t2);
		bar[i].c1=c1;
		bar[i].c2=c2*2.0*t1/t3;
		bar[i].c3=(c1-c2*t3/t1)*0.5;
		bar[i].c4=c1+c2*t1/t3;
		bar[i].c5=t1/t2/s;
		bar[i].i=0.0f;
		bar[i].v=0.0f;
		bar[i].out=0.0f;
		pre[i][0]=0.0f;
		pre[i][1]=0.0f;
		post[i]=0.0f;
		max[i]=0.0f;
	}
}

void rlc_render(short *buffer,int length,float amp){
	float in,out,i0,v0,b;
	int i,j;

	for (i=0;i<BarCount;++i){
		bar[i].out=0.0f;
	}
	for (j=0;j<length;++j){
		in=buffer[j]/32768.0f*amp;
		for (i=0;i<BarCount;++i){
			i0=bar[i].i;
			v0=bar[i].v;
			b=in-v0-i0*0.5f;
			bar[i].i=i0*bar[i].c1+b*bar[i].c2;
			bar[i].v=in-i0*bar[i].c3-b*bar[i].c4;
			out=(bar[i].v-v0)*bar[i].c5;
			bar[i].out+=out*out;
		}
	}
	for (i=0;i<BarCount;++i){
		bar[i].out=bar[i].out*2.0f/length;
	}
}

void rlc_output(int mode){
	float out;
	int i;

	rlcframe^=1;
	for (i=0;i<DisplayCount;++i){
		pre[i][rlcframe]=bar[i*Density+1].out-(bar[i*Density].out+bar[i*Density+2].out)*Depress;
		post[i]=(pre[i][rlcframe]+pre[i][rlcframe^1]*Feedback)/(Feedback+1.0f);
		switch (mode){
		case 0:
			out=post[i];
			break;
		case 1:
			out=bar[i*Density+1].out;
			break;
		default:
			out=0.0f;
		}
		if (out>0.0f){
			out=log2f(out)*3.0f/ThresDB+1.0f;
		}else{
			out=-1.0f;
		}
		if (out<0.0f){
			out=0.0f;
		}
		post[i]=out;
		max[i]-=DecayDB/ThresDB;
		if (max[i]<=out){
			max[i]=out;
		}
	}
}

int getColor(float x){
	const float p1=1.0/6.0,p2=2.0/6.0,p3=3.0/6.0,p4=4.0/6.0,p5=5.0/6.0;

	float R,G,B;
	int r,g,b;

	if (x>1.0f){
		x=1.0f;
	}
	if (x<0.0f){
		x=0.0f;
	}
	if (x>=p5){
		R=1.0f;
	}else if (x>=p4){
		R=(x-p4)*6.0f;
	}else if (x<=p2){
		if (x<=p1){
			R=1.0f;
		}else{
			R=(p2-x)*6.0f;
		}
	}else{
		R=0.0f;
	}
	r=(int)roundf(R*powf(x,Gamma)*256.0f);
	if (r>255){
		r=255;
	}
	if (x>=p5){
		G=(1.0f-x)*6.0f;
	}else if (x>=p3){
		G=1.0f;
	}else if (x>=p2){
		G=(x-p2)*6.0f;
	}else{
		G=0.0f;
	}
	g=(int)roundf(G*powf(x,Gamma)*256.0f);
	if (g>255){
		g=255;
	}
	if (x>=p4){
		B=0.0f;
	}else if (x>=p3){
		B=(p4-x)*6.0f;
	}else if (x>=p1){
		B=1.0f;
	}else{
		B=x*6.0f;
	}
	b=(int)roundf(B*powf(x,Gamma)*256.0f);
	if (b>255){
		b=255;
	}
	return RGB(r,g,b);
}
