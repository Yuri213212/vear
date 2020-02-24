/***

vear v0.4 Beta
Copyright (C) 2016-2020 Yuri213212
Site:https://github.com/Yuri213212/vear
Email: yuri213212@vip.qq.com
License: CC BY-NC-SA 4.0
https://creativecommons.org/licenses/by-nc-sa/4.0/

***/

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <math.h>
#include "vear_help.h"
#include "vear_ui.h"

#define tbuflen 1024
#define clwidth 587
#define clheight 375
#define SampleRate 44100
#define BufferLength 2940	//15fps

struct rlcdt{
	int x;
	float a;
	float wd;
	float c1;
	float c2;
	float c3;
	float c4;
	float c5;
	float ileft;
	float vleft;
	float iright;
	float vright;
	float out;
	float pre;
	float max;
	int ttl;
};

WCHAR wbuf[tbuflen];
int cxScreen,cyScreen,width,height,pause=0,color=1,mode=0,amp=0,Transpose=0,defTp=0,TTLMAX=2,i,j,temp;
float b,in,DECAY=1.0/24.0,d=0.5,f=1.0,dv=0.5;
double BaseFreq=440.0,BaseNote=4.75,t1,t2,t3,p,w0,l;
struct rlcdt bar[100];
POINT apt[588];
HDC hdcMem;
HPEN hPenRed,hPenGreen,hPenCyan,hPenGray;
HBRUSH hBrushBg[12],hBrushBg0,hBrushRed,hBrushBlue;
RECT bgRect={0,0,clwidth,clheight};
short inBuffer[2][BufferLength];
WAVEHDR WaveHdr[2]={
	{(LPSTR)inBuffer[0],BufferLength*2,0,0,0,0,0,0},
	{(LPSTR)inBuffer[1],BufferLength*2,0,0,0,0,0,0}
};
WAVEFORMATEX waveformat={WAVE_FORMAT_PCM,1,SampleRate,SampleRate*2,2,16,0};

int translateKey(int x){
	x&=0x3fff;
	if (x>=16&&x<=27){
		return x-16;
	}
	if (x>=30&&x<=40){
		return x-30;
	}
	if (x==43||x==28){
		return 11;
	}
	if (x>=44&&x<=53){
		return x-44;
	}
	if (x==115){
		return 10;
	}
	return -1;
}

int getColor(float x){
	const float gamma=1.0/1.68,p1=1.0/6.0,p2=2.0/6.0,p3=3.0/6.0,p4=4.0/6.0,p5=5.0/6.0;

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
			R=x*3.0f;
		}else{
			R=(p2-x)*3.0f;
		}
	}else{
		R=0.0f;
	}
	r=(int)roundf(R*powf(x,gamma)*256.0f);
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
	g=(int)roundf(G*powf(x,gamma)*256.0f);
	if (g>255){
		g=255;
	}
	if (x>=p4){
		B=0.0f;
	}else if (x>=p3){
		B=(p4-x)*6.0f;
	}else if (x>=p2){
		B=1.0f;
	}else{
		B=x*3.0f;
	}
	b=(int)roundf(B*powf(x,gamma)*256.0f);
	if (b>255){
		b=255;
	}
	return RGB(r,g,b);
}

void DisplayBuffer(HWND hwnd,short *buffer){
	const float AMPREG=32768.0*M_SQRT1_2,OUTREG=2.0/BufferLength,LOGREG=M_LN10/M_LN2/16.0,THRES=1.0/180.0;

	static float left,out;
	static HBRUSH hBrush;

	if (pause) return;
	for (j=0;j<BufferLength;++j){
		in=((int)buffer[j]<<(amp>>1))/(amp&1?AMPREG:32768.0f);
		if (!(j%5)){
			apt[j/5].y=93-(int)roundf(in*93.0f);
		}
		for (i=0;i<100;++i){
			bar[i].ileft=bar[i].iright;
			bar[i].vleft=bar[i].vright;
			b=(in-bar[i].vleft-bar[i].ileft*0.5f)*p;
			bar[i].iright=bar[i].ileft*bar[i].c1+b*bar[i].c2;
			bar[i].vright=in-bar[i].iright-bar[i].ileft*bar[i].c3-b*bar[i].c4;
			out=(bar[i].vright-bar[i].vleft)*bar[i].c5;
			bar[i].out+=out*out;
		}
	}
	FillRect(hdcMem,&bgRect,hBrushBg[Transpose]);
	SelectObject(hdcMem,hPenGreen);
	Polyline(hdcMem,apt,588);
	SelectObject(hdcMem,hBrushBlue);
	left=bar[0].out;
	bar[0].out=0.0f;
	for (i=1;i<99;++i){
		SelectObject(hdcMem,GetStockObject(NULL_PEN));
		switch (mode){
		case 0:
			left=bar[i].out-(bar[i+1].out+left)*d;
			out=(left+bar[i].pre*f)*dv;
			bar[i].pre=left;
			break;
		case 1:
			bar[i].pre=bar[i].out-(bar[i+1].out+left)*d;
			out=bar[i].out;
			break;
		}
		out=out>0.0f?log10f(out*OUTREG)*LOGREG+1.0f:0.0f;
		if (bar[i].ttl>0){
			--bar[i].ttl;
		}else{
			bar[i].max-=DECAY;
		}
		if (bar[i].max-THRES<=out){
			bar[i].ttl=TTLMAX;
		}
		if (out<0.0f){
			out=0.0f;
		}
		if (bar[i].max<out){
			bar[i].max=out;
		}
		temp=368.0f-180.0f*out;
		if (color){
			hBrush=CreateSolidBrush(getColor((out*2.0f+1.0f)/3.0f));
			SelectObject(hdcMem,hBrush);
			Rectangle(hdcMem,bar[i].x+1,temp,bar[i].x+7,370);
			DeleteObject(hBrush);
			SelectObject(hdcMem,GetStockObject(WHITE_PEN));
		}else{
			Rectangle(hdcMem,bar[i].x+1,temp,bar[i].x+7,370);
			SelectObject(hdcMem,hPenCyan);
		}
		temp=368.0f-180.0f*bar[i].max;
		MoveToEx(hdcMem,bar[i].x+1,temp,NULL);
		LineTo(hdcMem,bar[i].x+6,temp);
		left=bar[i].out;
		bar[i].out=0.0f;
	}
	bar[99].out=0.0f;
	InvalidateRect(hwnd,NULL,FALSE);
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
	static SCROLLINFO vsi={sizeof(SCROLLINFO),SIF_DISABLENOSCROLL|SIF_PAGE|SIF_POS|SIF_RANGE,0,16,1,16,0};
	static SCROLLINFO hsi={sizeof(SCROLLINFO),SIF_DISABLENOSCROLL|SIF_PAGE|SIF_POS|SIF_RANGE,0,11,1,0,0};
	static int endflag=0;
	static HBITMAP hBitmap;
	static HWAVEIN hWaveIn=0;

	HDC hdc;

	switch (message){
	case WM_CREATE:
		hPenRed=CreatePen(PS_SOLID,1,RGB(255,0,0));
		hPenGreen=CreatePen(PS_SOLID,1,RGB(0,255,0));
		hPenCyan=CreatePen(PS_SOLID,1,RGB(48,192,255));
		hPenGray=CreatePen(PS_SOLID,1,RGB(32,32,32));
		hBrushRed=CreateSolidBrush(RGB(255,0,0));
		hBrushBlue=CreateSolidBrush(RGB(0,102,204));
		hdc=GetDC(NULL);
		hdcMem=CreateCompatibleDC(hdc);
		hBitmap=CreateCompatibleBitmap(hdc,clwidth,clheight);
		ReleaseDC(hwnd,hdc);
		SelectObject(hdcMem,hBitmap);
		FillRect(hdcMem,&bgRect,GetStockObject(BLACK_BRUSH));
		SelectObject(hdcMem,hPenGray);
		for (i=0;i<16;++i){
			MoveToEx(hdcMem,0,clheight*i/32,NULL);
			LineTo(hdcMem,clwidth,clheight*i/32);
		}
		for (i=1;i<16;++i){
			MoveToEx(hdcMem,0,clheight/2+(clheight-clheight/2-6)*i/16,NULL);
			LineTo(hdcMem,clwidth,clheight/2+(clheight-clheight/2-6)*i/16);
		}
		SelectObject(hdcMem,GetStockObject(WHITE_PEN));
		MoveToEx(hdcMem,0,clheight/2,NULL);
		LineTo(hdcMem,clwidth,clheight/2);
		SelectObject(hdcMem,hPenRed);
		MoveToEx(hdcMem,0,clheight/4,NULL);
		LineTo(hdcMem,clwidth,clheight/4);
		SelectObject(hdcMem,hPenGray);
		MoveToEx(hdcMem,0,clheight-6,NULL);
		LineTo(hdcMem,clwidth,clheight-6);
		for (i=1;i<99;++i){
			MoveToEx(hdcMem,bar[i].x,clheight-6,NULL);
			LineTo(hdcMem,bar[i].x,clheight);
		}
		hBrushBg0=CreatePatternBrush(hBitmap);
		SelectObject(hdcMem,GetStockObject(WHITE_BRUSH));
		for (j=0;j<12;++j){
			FillRect(hdcMem,&bgRect,hBrushBg0);
			for (i=1;i<99;++i){
				switch ((i+15-j)%12){
				case 0:
				case 2:
				case 4:
				case 5:
				case 7:
				case 9:
				case 11:
					SelectObject(hdcMem,hPenGray);
					MoveToEx(hdcMem,bar[i].x+3,clheight/2+1,NULL);
					LineTo(hdcMem,bar[i].x+3,clheight);
					SelectObject(hdcMem,GetStockObject(NULL_PEN));
					Rectangle(hdcMem,bar[i].x+1,clheight-5,bar[i].x+7,clheight+1);
					break;
				default:
					;
				}
				if (i==45){
					SelectObject(hdcMem,hBrushRed);
					Rectangle(hdcMem,bar[i].x+2,clheight-4,bar[i].x+6,clheight);
					SelectObject(hdcMem,GetStockObject(WHITE_BRUSH));
				}
			}
			hBrushBg[j]=CreatePatternBrush(hBitmap);
		}
		DeleteObject(hBitmap);
		SetScrollInfo(hwnd,SB_VERT,&vsi,TRUE);
		hsi.nPos=Transpose;
		SetScrollInfo(hwnd,SB_HORZ,&hsi,TRUE);
		if (waveInOpen(&hWaveIn,WAVE_MAPPER,&waveformat,(DWORD_PTR)hwnd,0,CALLBACK_WINDOW)!=MMSYSERR_NOERROR){
			MessageBoxW(hwnd,szErr_AudioDevice,szTitle,MB_ICONERROR);
			DestroyWindow(hwnd);
			return 0;
		}
		waveInPrepareHeader(hWaveIn,&WaveHdr[0],sizeof(WAVEHDR));
		waveInPrepareHeader(hWaveIn,&WaveHdr[1],sizeof(WAVEHDR));
		SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
		return 0;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		hdc=GetDC(hwnd);
		BitBlt(hdc,0,0,clwidth,clheight,hdcMem,0,0,SRCCOPY);
		ReleaseDC(hwnd,hdc);
		ValidateRect(hwnd,NULL);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)){
		case 40001:
			pause^=1;
			return 0;
		case 40002:
			amp=mode=pause=0;
			color=1;
			SetScrollPos(hwnd,SB_VERT,16,TRUE);
			SetScrollPos(hwnd,SB_HORZ,Transpose=defTp,TRUE);
			MoveWindow(hwnd,(cxScreen-width)/2,(cyScreen-height)/2,width,height,TRUE);
			return 0;
		case 40003:
			color^=1;
			return 0;
		case 40004:
			mode^=1;
			return 0;
		case 40005:
			MessageBoxW(NULL,szHelp,szTitle,MB_ICONINFORMATION);
			return 0;
		default:
			;
		}
		break;
	case WM_VSCROLL:
		switch(LOWORD(wParam)){
		case SB_LINEUP:
		case SB_PAGEUP:
			++amp;
			break;
		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			--amp;
			break;
		case SB_THUMBTRACK:
			amp=16-HIWORD(wParam);
			break;
		default:
			return 0;
		}
		if (amp>16){
			amp=16;
		}else if (amp<0){
			amp=0;
		}
		SetScrollPos(hwnd,SB_VERT,16-amp,TRUE);
		return 0;
	case WM_HSCROLL:
		switch(LOWORD(wParam)){
		case SB_LINELEFT:
		case SB_PAGELEFT:
			Transpose-=1;
			break;
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			Transpose+=1;
			break;
		case SB_THUMBTRACK:
			Transpose=HIWORD(wParam);
			break;
		default:
			return 0;
		}
		Transpose=(Transpose+12)%12;
		SetScrollPos(hwnd,SB_HORZ,Transpose,TRUE);
		return 0;
	case WM_KEYDOWN:
		switch (wParam){
		case VK_LEFT:
			SendMessage(hwnd,WM_HSCROLL,SB_LINELEFT,0);
			return 0;
		case VK_RIGHT:
			SendMessage(hwnd,WM_HSCROLL,SB_LINERIGHT,0);
			return 0;
		case VK_UP:
			SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0);
			return 0;
		case VK_DOWN:
			SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0);
			return 0;
		}
		if (HIWORD(lParam)>>9) return 0;
		switch (wParam){
		case VK_ESCAPE:
			SendMessage(hwnd,WM_COMMAND,40002,0);
			return 0;
		case VK_SPACE:
			SendMessage(hwnd,WM_COMMAND,40001,0);
			return 0;
		case VK_TAB:
			SendMessage(hwnd,WM_COMMAND,40003,0);
			return 0;
		case VK_F1:
			SendMessage(hwnd,WM_COMMAND,40004,0);
			return 0;
		case '1':
		case '2':
			mode=wParam-'1';
			return 0;
		default:
			if ((temp=translateKey(HIWORD(lParam)))<0) return 0;
			Transpose=temp;
			SetScrollPos(hwnd,SB_HORZ,Transpose,TRUE);
			return 0;
		}
	case MM_WIM_OPEN:
		waveInAddBuffer(hWaveIn,&WaveHdr[0],sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn,&WaveHdr[1],sizeof(WAVEHDR));
		waveInStart(hWaveIn);
		return 0;
	case MM_WIM_DATA:
		if (endflag){
			waveInClose(hWaveIn);
		}else{
			DisplayBuffer(hwnd,(short *)((PWAVEHDR)lParam)->lpData);
			waveInAddBuffer(hWaveIn,(PWAVEHDR)lParam,sizeof(WAVEHDR));
		}
		return 0;
	case MM_WIM_CLOSE:
		waveInUnprepareHeader(hWaveIn,&WaveHdr[0],sizeof(WAVEHDR));
		waveInUnprepareHeader(hWaveIn,&WaveHdr[1],sizeof(WAVEHDR));
		DestroyWindow(hwnd);
		return 0;
	case WM_CLOSE:
		endflag=1;
		waveInStop(hWaveIn);
		waveInReset(hWaveIn);
		return 0;
	case WM_DESTROY:
		SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		DeleteObject(hPenRed);
		DeleteObject(hPenGreen);
		DeleteObject(hPenCyan);
		DeleteObject(hPenGray);
		for (j=0;j<12;++j){
			DeleteObject(hBrushBg[j]);
		}
		DeleteObject(hBrushBg0);
		DeleteObject(hBrushRed);
		DeleteObject(hBrushBlue);
		DeleteDC(hdcMem);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

int main(){
	static int argc;
	static WCHAR **argv;
	static HINSTANCE hInstance;
	static int iCmdShow;
	static STARTUPINFOW si;
	static POINT aptLt[12]={
		{0,4},
		{2,2},
		{6,2},
		{8,4},
		{8,7},
		{7,8},
		{7,9},
		{3,13},
		{2,13},
		{1,14},
		{0,14},
		{0,4}
	};
	static POINT aptDk[11]={
		{1,6},
		{3,4},
		{5,4},
		{6,5},
		{6,7},
		{5,8},
		{5,9},
		{3,11},
		{2,11},
		{1,12},
		{1,6}
	};
	static BYTE ANDmaskIcon[]={
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00,
		0x00,0x00
	};
	static BITMAP bitmapmask={0,16,16,2,1,1,ANDmaskIcon};
	static ICONINFO iconinfo={TRUE,0,0,0,0};

	HDC hdc,hdcMem;
	HPEN hPenRed,hPenGreen;
	HBRUSH hBrushLtPink,hBrushDkPink;
	HICON hIcon;
	WNDCLASSW wndclass;
	FILE *fp;
	HMENU hMenu;
	HWND hwnd;
	MSG msg;

	argv=CommandLineToArgvW(GetCommandLineW(),&argc);
	hInstance=GetModuleHandleW(NULL);
	GetStartupInfoW(&si);
	iCmdShow=si.dwFlags&STARTF_USESHOWWINDOW?si.wShowWindow:SW_SHOWNORMAL;
	GetModuleFileNameW(NULL,wbuf,tbuflen);
	wbuf[tbuflen-1]=0;
	for (i=wcslen(wbuf)-1;wbuf[i]!='\\'&&i>=0;--i){
		wbuf[i]=0;
	}
	SetCurrentDirectoryW(wbuf);
	hdc=GetDC(NULL);
	hdcMem=CreateCompatibleDC(hdc);
	iconinfo.hbmMask=CreateBitmapIndirect(&bitmapmask);
	iconinfo.hbmColor=CreateCompatibleBitmap(hdc,16,16);
	ReleaseDC(NULL,hdc);
	SelectObject(hdcMem,iconinfo.hbmColor);
	hPenRed=CreatePen(PS_SOLID,1,RGB(255,0,0));
	hPenGreen=CreatePen(PS_SOLID,1,RGB(0,255,0));
	hBrushLtPink=CreateSolidBrush(RGB(255,128,192));
	hBrushDkPink=CreateSolidBrush(RGB(192,64,128));
	SelectObject(hdcMem,GetStockObject(NULL_PEN));
	SelectObject(hdcMem,GetStockObject(BLACK_BRUSH));
	Rectangle(hdcMem,0,0,17,17);
	SelectObject(hdcMem,hBrushLtPink);
	Polygon(hdcMem,aptLt,12);
	SelectObject(hdcMem,hBrushDkPink);
	Polygon(hdcMem,aptDk,11);
	SelectObject(hdcMem,hPenRed);
	MoveToEx(hdcMem,7,8,NULL);
	LineTo(hdcMem,15,8);
	SelectObject(hdcMem,hPenGreen);
	LineTo(hdcMem,13,8);
	LineTo(hdcMem,11,13);
	LineTo(hdcMem,11,3);
	LineTo(hdcMem,9,8);
	LineTo(hdcMem,6,8);
	DeleteObject(hPenRed);
	DeleteObject(hPenGreen);
	DeleteObject(hBrushLtPink);
	DeleteObject(hBrushDkPink);
	hIcon=CreateIconIndirect(&iconinfo);
	DeleteDC(hdcMem);
	DeleteObject(iconinfo.hbmMask);
	DeleteObject(iconinfo.hbmColor);
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc=WndProc;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=hInstance;
	wndclass.hIcon=hIcon;
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground=NULL;
	wndclass.lpszMenuName=szAppName;
	wndclass.lpszClassName=szAppName;
	if (!RegisterClassW(&wndclass)){
		MessageBoxW(NULL,szErr_RegisterClass,argv[0],MB_ICONERROR);
		DestroyIcon(hIcon);
		return 0;
	}
	if ((fp=_wfopen(szSettingFile,L"rb"))){
		fscanf(fp,"%x",&temp);
		BaseNote=(temp>>4)+(temp&15)/12.0;
		fscanf(fp,"%lf",&BaseFreq);
		fscanf(fp,"%d",&defTp);
		Transpose=defTp;
		fscanf(fp,"%d",&TTLMAX);
		fscanf(fp,"%d",&temp);
		DECAY=1.0/temp;
		fscanf(fp,"%f",&d);
		if (abs(d)>1.0f){
			d=1.0f/d;
		}
		if (d<0.0f){
			d=1.0f-d;
		}
		fscanf(fp,"%f",&f);
		if (abs(f)>1.0f){
			f=1.0f/f;
		}
		if (f<0.0f){
			f=1.0f-f;
		}
		dv=1.0f/(f+1.0f);
		fclose(fp);
	}
	t1=pow(2.0,1.0/12.0)-1.0;
	t2=pow(2.0,1/24.0);
	t3=sqrt(6.0*pow(2.0,1.0/12.0)-pow(2.0,1.0/6.0)-1.0);
	p=2.0*t1/t3;
	for (i=0;i<100;++i){
		bar[i].x=i*6-7;
		w0=2.0*M_PI*BaseFreq*pow(2.0,(i+3)/12.0-BaseNote);
		l=t2/t1/w0;
		bar[i].a=w0*t1/t2/2.0;
		bar[i].wd=w0*t3/t2/2.0;
		bar[i].c1=exp(-bar[i].a/SampleRate)*cos(bar[i].wd/SampleRate);
		bar[i].c2=exp(-bar[i].a/SampleRate)*sin(bar[i].wd/SampleRate);
		bar[i].c3=(-bar[i].a*bar[i].c1-bar[i].wd*bar[i].c2)*l;
		bar[i].c4=(-bar[i].a*bar[i].c2+bar[i].wd*bar[i].c1)*l;
		bar[i].c5=SampleRate*t1/t2/w0;
		bar[i].max=bar[i].pre=bar[i].out=bar[i].vright=bar[i].iright=bar[i].vleft=bar[i].ileft=0.0f;
		bar[i].ttl=0;
	}
	for (i=0;i<588;++i){
		apt[i].x=i;
	}
	cxScreen=GetSystemMetrics(SM_CXSCREEN);
	cyScreen=GetSystemMetrics(SM_CYSCREEN);
	width=clwidth+GetSystemMetrics(SM_CXFIXEDFRAME)*2+GetSystemMetrics(SM_CXVSCROLL);
	height=clheight+GetSystemMetrics(SM_CYFIXEDFRAME)*2+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYHSCROLL)+GetSystemMetrics(SM_CYMENU);
	hMenu=CreateMenu();
	AppendMenuW(hMenu,MF_STRING,40001,szMenu_Pause);
	AppendMenuW(hMenu,MF_STRING,40002,szMenu_Reset);
	AppendMenuW(hMenu,MF_STRING,40003,szMenu_Color);
	AppendMenuW(hMenu,MF_STRING,40004,szMenu_Mode);
	AppendMenuW(hMenu,MF_STRING,40005,szMenu_Help);
	hwnd=CreateWindowW(szAppName,szTitle,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VSCROLL|WS_HSCROLL,(cxScreen-width)/2,(cyScreen-height)/2,width,height,NULL,hMenu,hInstance,NULL);
	ShowWindow(hwnd,iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	LocalFree(argv);
	return msg.wParam;
}
