//Fix bug of DefWindowProc
LRESULT myDefWindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
	switch (message){
	case WM_NCRBUTTONDOWN:
		return 0;
	case WM_NCRBUTTONUP:
		PostMessage(hwnd,WM_RBUTTONUP,0,lParam);
		DefWindowProc(hwnd,WM_NCRBUTTONDOWN,wParam,lParam);
		break;
	case WM_NCLBUTTONDOWN:
		switch (wParam){
		case HTZOOM:
		case HTREDUCE:
		case HTCLOSE:
		case HTHELP:
			return 0;
		}
		break;
	case WM_NCLBUTTONUP:
		switch (wParam){
		case HTZOOM:
		case HTREDUCE:
		case HTCLOSE:
		case HTHELP:
			PostMessage(hwnd,WM_LBUTTONUP,0,lParam);
			DefWindowProc(hwnd,WM_NCLBUTTONDOWN,wParam,lParam);
		}
		break;
	case WM_SYSCOMMAND:
		if ((wParam&0xFFF0)==SC_MOVE&&(wParam&0x0F)){
			return DefWindowProc(hwnd,message,SC_SIZE|9,lParam);
		}
		break;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}
