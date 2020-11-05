HICON drawIcon(){
	const POINT aptLt[12]={
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
	const POINT aptDk[11]={
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
	const BYTE ANDmaskIcon[]={
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
	const BITMAP bitmapmask={0,16,16,2,1,1,(void *)ANDmaskIcon};
	static ICONINFO iconinfo={TRUE};

	HDC hdc,hdcMem;
	HPEN hPenRed,hPenGreen;
	HBRUSH hBrushLtPink,hBrushDkPink;
	HICON hIcon;

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
	return hIcon;
}
