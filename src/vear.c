/***

vear v1.0
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
#ifdef LANG_EN
#include "lang_en/vear_ui.h"
#include "lang_en/vear_help.h"
#endif
#ifdef LANG_CH
#include "lang_ch/vear_ui.h"
#include "lang_ch/vear_help.h"
#endif
#ifdef LANG_JP
#include "lang_jp/vear_ui.h"
#include "lang_jp/vear_help.h"
#endif
#include "iniFormat.h"

#define tbuflen 1024
#define clwidth 587
#define clheight 375
#define SampleRate 44100
#define BufferLength 2940	//15fps

#define SYSMENU_Top	0x0010
enum menuEnum{
	Menu_Pause=0x8001,
	Menu_Reset,
	Menu_Color,
	Menu_Mode,
	Menu_Help,
};

#include "settings.h"

wchar_t wbuf[tbuflen];
int cxScreen,cyScreen,width,height,pause=0,color=1,mode=0,amp=0,transpose=0;
POINT apt[588];
HDC hdcMem;
HPEN hPenRed,hPenGreen,hPenCyan,hPenGray;
HBRUSH hBrushBg[NoteCount],hBrushBg0,hBrushRed,hBrushBlue;
RECT bgRect={0,0,clwidth,clheight};
short inBuffer[2][BufferLength];
WAVEHDR WaveHdr[2]={
	{(LPSTR)inBuffer[0],BufferLength*2,0,0,0,0,0,0},
	{(LPSTR)inBuffer[1],BufferLength*2,0,0,0,0,0,0}
};
WAVEFORMATEX waveformat={WAVE_FORMAT_PCM,1,SampleRate,SampleRate*2,2,16,0};

#include "graph.h"
#include "defproc.h"
#include "drawicon.h"

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

void displayBuffer(HWND hwnd,short *buffer){
	HBRUSH hBrush;
	float a,in;
	int i,temp;

	a=(1<<(amp>>1))*(amp&1?M_SQRT2:1.0);
	rlc_render(buffer,BufferLength,a);
	rlc_output(mode);
	if (pause) return;
	for (i=0;i<BufferLength;i+=5){
		in=buffer[i]/32768.0f*a;
		apt[i/5].y=93-(int)roundf(in*93.0f);
	}
	FillRect(hdcMem,&bgRect,hBrushBg[transpose]);
	SelectObject(hdcMem,hPenGreen);
	Polyline(hdcMem,apt,588);
	SelectObject(hdcMem,hBrushBlue);
	for (i=0;i<DisplayCount;++i){
		SelectObject(hdcMem,GetStockObject(NULL_PEN));
		temp=368.0f-180.0f*post[i];
		if (color){
			hBrush=CreateSolidBrush(getColor((post[i]*MinColor+3.0f-MinColor)/3.0f));
			SelectObject(hdcMem,hBrush);
			Rectangle(hdcMem,i*6,temp,i*6+6,370);
			DeleteObject(hBrush);
			SelectObject(hdcMem,GetStockObject(WHITE_PEN));
		}else{
			Rectangle(hdcMem,i*6,temp,i*6+6,370);
			SelectObject(hdcMem,hPenCyan);
		}
		temp=368.0f-180.0f*max[i];
		MoveToEx(hdcMem,i*6,temp,NULL);
		LineTo(hdcMem,i*6+5,temp);
	}
	InvalidateRect(hwnd,NULL,FALSE);
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
	static SCROLLINFO vsi={sizeof(SCROLLINFO),SIF_DISABLENOSCROLL|SIF_PAGE|SIF_POS|SIF_RANGE,0,16,1,16,0};
	static SCROLLINFO hsi={sizeof(SCROLLINFO),SIF_DISABLENOSCROLL|SIF_PAGE|SIF_POS|SIF_RANGE,0,NoteCount-1,1,0,0};
	static HWAVEIN hWaveIn=NULL;
	static int endflag=0;

	HDC hdc;
	HBITMAP hBitmap;
	HMENU hMenu;
	int i,j,temp;

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
		ReleaseDC(NULL,hdc);
		SelectObject(hdcMem,hBitmap);
		FillRect(hdcMem,&bgRect,GetStockObject(BLACK_BRUSH));
		SelectObject(hdcMem,hPenGray);
		for (i=0;i<GridLines;++i){
			MoveToEx(hdcMem,0,clheight*i/(GridLines*2),NULL);
			LineTo(hdcMem,clwidth,clheight*i/(GridLines*2));
		}
		for (i=1;i*GridDB<ThresDB;++i){
			MoveToEx(hdcMem,0,clheight/2+(clheight-clheight/2-6)*i*GridDB/ThresDB,NULL);
			LineTo(hdcMem,clwidth,clheight/2+(clheight-clheight/2-6)*i*GridDB/ThresDB);
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
		for (i=0;i<DisplayCount;++i){
			MoveToEx(hdcMem,i*6-1,clheight-6,NULL);
			LineTo(hdcMem,i*6-1,clheight);
		}
		hBrushBg0=CreatePatternBrush(hBitmap);
		SelectObject(hdcMem,GetStockObject(WHITE_BRUSH));
		for (j=0;j<NoteCount;++j){
			FillRect(hdcMem,&bgRect,hBrushBg0);
			for (i=0;i<DisplayCount;++i){
				switch ((i+NoteCount+StartNote-j)%NoteCount){
				case 0:
				case 2:
				case 4:
				case 5:
				case 7:
				case 9:
				case 11:
					SelectObject(hdcMem,hPenGray);
					MoveToEx(hdcMem,i*6+2,clheight/2+1,NULL);
					LineTo(hdcMem,i*6+2,clheight);
					SelectObject(hdcMem,GetStockObject(NULL_PEN));
					Rectangle(hdcMem,i*6,clheight-5,i*6+6,clheight+1);
					break;
				default:
					;
				}
				if (i==RefNote-StartNote){
					SelectObject(hdcMem,hBrushRed);
					Rectangle(hdcMem,i*6+1,clheight-4,i*6+5,clheight);
					SelectObject(hdcMem,GetStockObject(WHITE_BRUSH));
				}
			}
			hBrushBg[j]=CreatePatternBrush(hBitmap);
		}
		DeleteObject(hBitmap);
		SetScrollInfo(hwnd,SB_VERT,&vsi,TRUE);
		hsi.nPos=transpose;
		SetScrollInfo(hwnd,SB_HORZ,&hsi,TRUE);
		if (waveInOpen(&hWaveIn,WAVE_MAPPER,&waveformat,(DWORD_PTR)hwnd,0,CALLBACK_WINDOW)!=MMSYSERR_NOERROR){
			MessageBoxW(hwnd,szErr_AudioDevice,szTitle,MB_ICONERROR);
			DestroyWindow(hwnd);
			return 0;
		}
		waveInPrepareHeader(hWaveIn,&WaveHdr[0],sizeof(WAVEHDR));
		waveInPrepareHeader(hWaveIn,&WaveHdr[1],sizeof(WAVEHDR));
		PostMessage(hwnd,WM_SYSCOMMAND,SYSMENU_Top,0);
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
	case WM_SYSCOMMAND:
		if (wParam==SYSMENU_Top){
			hMenu=GetSystemMenu(hwnd,FALSE);
			temp=GetMenuState(hMenu,wParam,MF_BYCOMMAND)&MF_CHECKED;
			temp^=MF_CHECKED;
			SetWindowPos(hwnd,temp?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			CheckMenuItem(hMenu,wParam,MF_BYCOMMAND|temp);
			return 0;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)){
		case Menu_Pause:
			pause^=1;
			return 0;
		case Menu_Reset:
			pause=0;
			color=1;
			mode=0;
			amp=0;
			transpose=DefTranspose;
			SetScrollPos(hwnd,SB_VERT,16,TRUE);
			SetScrollPos(hwnd,SB_HORZ,transpose,TRUE);
			MoveWindow(hwnd,(cxScreen-width)/2,(cyScreen-height)/2,width,height,TRUE);
			return 0;
		case Menu_Color:
			color^=1;
			return 0;
		case Menu_Mode:
			mode^=1;
			return 0;
		case Menu_Help:
			MessageBoxW(NULL,szHelp,szTitle,MB_ICONINFORMATION);
			return 0;
		default:
			;
		}
		break;
	case WM_VSCROLL:
		switch(LOWORD(wParam)){
		case SB_TOP:
			amp=16;
			break;
		case SB_BOTTOM:
			amp=0;
			break;
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
		case SB_LEFT:
			transpose=0;
			break;
		case SB_RIGHT:
			transpose=NoteCount-1;
			break;
		case SB_LINELEFT:
		case SB_PAGELEFT:
			transpose-=1;
			break;
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			transpose+=1;
			break;
		case SB_THUMBTRACK:
			transpose=HIWORD(wParam);
			break;
		default:
			return 0;
		}
		transpose=(transpose+NoteCount)%NoteCount;
		SetScrollPos(hwnd,SB_HORZ,transpose,TRUE);
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
			temp=translateKey(HIWORD(lParam));
			if (temp<0) return 0;
			transpose=temp;
			SetScrollPos(hwnd,SB_HORZ,transpose,TRUE);
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
			displayBuffer(hwnd,(short *)((PWAVEHDR)lParam)->lpData);
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
		for (j=0;j<NoteCount;++j){
			DeleteObject(hBrushBg[j]);
		}
		DeleteObject(hBrushBg0);
		DeleteObject(hBrushRed);
		DeleteObject(hBrushBlue);
		DeleteDC(hdcMem);
		PostQuitMessage(0);
		return 0;
	}
	return myDefWindowProc(hwnd,message,wParam,lParam);
}

int main(){
	int argc;
	wchar_t **argv;
	HINSTANCE hInstance;
	int iCmdShow;
	STARTUPINFOW si;

	HICON hIcon;
	WNDCLASSW wndclass;
	FILE *fp;
	HMENU hMenu;
	HWND hwnd;
	MSG msg;
	int i;

	argv=CommandLineToArgvW(GetCommandLineW(),&argc);
	hInstance=GetModuleHandleW(NULL);
	GetStartupInfoW(&si);
	iCmdShow=si.dwFlags&STARTF_USESHOWWINDOW?si.wShowWindow:SW_SHOWNORMAL;

	hIcon=drawIcon();
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

	GetModuleFileNameW(NULL,wbuf,tbuflen);
	wbuf[tbuflen-1]=0;
	for (i=wcslen(wbuf)-1;wbuf[i]!='\\'&&i>=0;--i){
		wbuf[i]=0;
	}
	SetCurrentDirectoryW(wbuf);
	fp=_wfopen(szSettingFile,L"rb");
	if (fp){
		if (!readSettings(fp)){
			swprintf(wbuf,szWarn_SettingFile,szSettingFile);
			MessageBoxW(NULL,wbuf,argv[0],MB_ICONWARNING);
		}
		fclose(fp);
	}
	transpose=DefTranspose;
	rlc_init();
	for (i=0;i<588;++i){
		apt[i].x=i;
	}

	cxScreen=GetSystemMetrics(SM_CXSCREEN);
	cyScreen=GetSystemMetrics(SM_CYSCREEN);
	width=clwidth+GetSystemMetrics(SM_CXFIXEDFRAME)*2+GetSystemMetrics(SM_CXVSCROLL);
	height=clheight+GetSystemMetrics(SM_CYFIXEDFRAME)*2+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYHSCROLL)+GetSystemMetrics(SM_CYMENU);
	hMenu=CreateMenu();
	AppendMenuW(hMenu,MF_STRING,Menu_Pause,szMenu_Pause);
	AppendMenuW(hMenu,MF_STRING,Menu_Reset,szMenu_Reset);
	AppendMenuW(hMenu,MF_STRING,Menu_Color,szMenu_Color);
	AppendMenuW(hMenu,MF_STRING,Menu_Mode,szMenu_Mode);
	AppendMenuW(hMenu,MF_STRING,Menu_Help,szMenu_Help);
	hwnd=CreateWindowW(szAppName,szTitle,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VSCROLL|WS_HSCROLL,(cxScreen-width)/2,(cyScreen-height)/2,width,height,NULL,hMenu,hInstance,NULL);
	hMenu=GetSystemMenu(hwnd,FALSE);
	InsertMenuW(hMenu,0,MF_BYPOSITION|MF_STRING,SYSMENU_Top,szSysMenu_Top);
	ShowWindow(hwnd,iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DestroyIcon(hIcon);
	LocalFree(argv);
	return msg.wParam;
}
