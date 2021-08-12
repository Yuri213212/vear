#define UNICODE
#define _UNICODE

#include <windows.h>
#include <CommCtrl.h>
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

HINSTANCE hInstance;
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
