#include <windows.h>
#include <windowsx.h>
#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>

#endif
#include <fstream>
#include <SDL.h>
#include <math.h>
#include <iostream>
#include <vector>

/**DEFINICJE STALYCH I ID**/
#define ID_Color 111
#define ID_Convert 555
/**************************/
using namespace std;
///DEFINICJE ZMIENNYCH STRUKTUR I UCHWYTOW///
typedef SDL_Colour Color;
SDL_Surface *screen; //struktura okna dla SDL
char const* tytul = "Okno obrazka";
int TrybKoloru; //wybraną opcje trybu koloru
int w=0, h=0; //wielkosc pliku graficznego

vector <Uint8> ByteRun;
vector <Uint8> RLEOut;

LPSTR Bufor,NowaNazwaPliku; //stringi przechowujące sciezke do pliku
DWORD dlugosc;

HWND hWnd;	//uchwyt okna glownego
HWND hCombo;  //uchwyt comboboxa
HWND hText,hText1; //uchwyty pól tekstowych

struct Header{
char Name [4]={'g','g','k','k'};
char TrybKonwersji[1];
char TrybKoloru[1];
char *width;
char *heigth;
};
Header GGKKHeader;
////////////////////////////////////////////////

int frame(); // okno SDL
void ConvertColor(Color bufor, int Tryb); // konwersja kolorów
void newfile(); //tworzenie nwego pliku
void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B); //setpixel...
void byterun(Color a[], int length,vector <Uint8> &ByteRun); //kompresja byterun
void RLE(Color a[],int length,vector <Uint8> &RLEOut); //rle
Color getPixel (int x, int y);  //pobierz pixel z ekranu...
Color RGBtoHSV(Color bufor);
Color RGBtoHSL(Color bufor);
Color RGBtoYUV(Color bufor);

void RLE(Color dane[],int length,vector <Uint8> &RLEOut)
{

int i=0;
long j=0,coun=0;
    while(i < length)
    {

        while ((i+j < length-1) && (dane[i+j].r == dane[i+j+1].r) && (dane[i+j].g == dane[i+j+1].g) && (dane[i+j].b == dane[i+j+1].b) && (j < 255))  {j++;}
        if(j>=2)
        {
            RLEOut.push_back(Uint8(j));
            RLEOut.push_back(dane[i+j].r);
            RLEOut.push_back(dane[i+j].g);
            RLEOut.push_back(dane[i+j].b);
            i += (j);
        }
        else
        {
            while((j==1) && (i+j < length-1) && (i+coun< length-1))
            {
                if((dane[i+coun].r == dane[i+coun+1].r) && (dane[i+coun].g == dane[i+coun+1].g) && (dane[i+coun].b == dane[i+coun+1].b)) j++;
                else{coun++;}
            }
            RLEOut.push_back(0);
            RLEOut.push_back(Uint8(coun));
            for(int k=0;k<coun;k++)
            {
            RLEOut.push_back(dane[i+k].r);
            RLEOut.push_back(dane[i+k].g);
            RLEOut.push_back(dane[i+k].b);
            }
            i+=(coun+1);
        }
    j=0;
    }
  cout<<"RLE: Skompresowano dane \n";
}

void byterun(Color dane[],int length,vector <Uint8> &ByteRun)
{
int i = 0;
    while (i < length)
    {
        if ((i < length-1) && (dane[i].r == dane[i+1].r) && (dane[i].g == dane[i+1].g) && (dane[i].b == dane[i+1].b))
        {
            int j = 0;
            while ((i+j < length-1) && (dane[i+j].r == dane[i+j+1].r) && (dane[i+j].g == dane[i+j+1].g) && (dane[i+j].b == dane[i+j+1].b) && (j < 127))  j++;
            ByteRun.push_back(Uint8(128-j));
            ByteRun.push_back(dane[i+j].r);
            ByteRun.push_back(dane[i+j].g);
            ByteRun.push_back(dane[i+j].b);
            i += (j+1);
        }
        else
        {
            int j=0;
            while ((i+j < length-1) && ((dane[i+j].r != dane[j+i+1].r) || (dane[i+j].g != dane[j+i+1].g) || (dane[i+j].b != dane[j+i+1].b)) && (j <128)) j++;
            if ((i+j == length-1) && (j < 128)) j++;
            ByteRun.push_back(Uint8(127+j));
            for (int k=0; k<j; k++) {
                ByteRun.push_back(dane[i+k].r);
                ByteRun.push_back(dane[i+k].g);
                ByteRun.push_back(dane[i+k].b);
            }
            i += j;
        }
    }
    cout<<"ByteRun: Skompresowano dane \n";
}

Color RGBtoYUV(Color bufor)
{
    double Y,U,V;
        Y=0.299*double(bufor.r)+0.587*double(bufor.g)+0.114*double(bufor.b);
        U=0.492*(double(bufor.b)-Y);
        V=0.877*(double(bufor.r)-Y);

        bufor.r=Uint8(round(Y));  //używam tej samej struktury która przechowywała wcześniej składowe kolorów RGB...
        bufor.g=Uint8(round(U));  //
        bufor.b=Uint8(round(V));  //
    return bufor;
}

Color RGBtoHSL(Color bufor)
{
    double cmax,cmin,delta,H,S,L,r1,g1,b1;

        r1=double(double(bufor.r)/255.);
        g1=double(double(bufor.g)/255.);
        b1=double(double(bufor.b)/255.);
        cmax=max(max(r1,g1),b1);
        cmin=min(min(r1,g1),b1);
        delta=double(cmax-cmin);

        if( r1 == cmax )
		H=double(double(g1-b1)/delta);
        else if(g1==cmax) H=double(double(b1-r1)/delta)+2.;
        else H =double(double(r1-g1)/delta)+4.;
        H=H*60.;
        if(H<0)H=H+360;
        H=round(double(H*0.7111)); //H

        L=double(double((cmax)+double(cmin))/2.); //L
        if(cmax-cmin==0.) S=0;
        else S=double(delta/(1.-fabs(2.*L-1.))); //S

        bufor.r=Uint8(H);  //używam tej samej struktury która przechowywała wcześniej składowe kolorów RGB...
        bufor.g=Uint8(round(S*100));  //
        bufor.b=Uint8(round(L*100));  //
    return bufor;
}

Color RGBtoHSV(Color bufor)
{
    double cmax,cmin,delta,S,H,V,r1,g1,b1;
        r1=double(double(bufor.r)/255.);
        g1=double(double(bufor.g)/255.);
        b1=double(double(bufor.b)/255.);

        cmax=max(max(r1,g1),b1);
        cmin=min(min(r1,g1),b1);
        delta=double(cmax-cmin);
        if( r1 == cmax )
		H=double(double(g1-b1)/delta);
        else if(g1==cmax) H=double(double(b1-r1)/delta)+2.;
        else H =double(double(r1-g1)/delta)+4.;
        H=H*60.;
        if(H<0)H=H+360.;
        H=double(H*0.7111); //H
        if(cmax==0) S=0;
        else S=double(delta/cmax)*100.; //S
        V=cmax*100.; //V
        bufor.r=Uint8(H);  //używam tej samej struktury która przechowywała wcześniej składowe kolorów RGB...
        bufor.g=Uint8(S);  //
        bufor.b=Uint8(V);  //
    return bufor;
}

void ConvertColor(Color bufor,int Tryb) //zmienna tryb przekazywana jest z Combo Boxa i przyjmuje wartości 0-3
{
    switch (Tryb)
    {

    case 0:  //konwersja na HSV
        RGBtoHSV(bufor);
        GGKKHeader.TrybKoloru[1]='0';
        break;
    case 1:  //konwersja na HSL
        RGBtoHSL(bufor);
        GGKKHeader.TrybKoloru[1]='1';
        break;
    case 2: //konwersja na YUV
        RGBtoYUV(bufor);
        GGKKHeader.TrybKoloru[1]='2';
        break;
    default:  GGKKHeader.TrybKoloru[1]='3';
    }
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<w) && (y>=0) && (y<h))
  {
    /* Zamieniamy poszczególne skladowe koloru na format koloru pixela */
    Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);
    /* Pobieramy informacji ile bajtów zajmuje jeden pixel */
    int bpp = screen->format->BytesPerPixel;
    /* Obliczamy adres pixela */
    Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;
    /* Ustawiamy wartosc pixela, w zaleznci od formatu powierzchni*/
    switch(bpp)
    {
        case 1: //8-bit
            *p = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32 *)p = pixel;
            break;
    }
  }
}

void newfile(int w, int h)
{
/*****************************TWORZENIE PLIKU**************************************/
    char rozszerz[6]=".ggkk";
    strcat(NowaNazwaPliku, rozszerz);
    FILE* plik;
    plik = fopen(NowaNazwaPliku, "wb");
/***********************************TWORZYENIE TABLICY PRZECHOWUJACEJ SUROWE DANE********************************/

    Color *RawData;  //wskaznik do zmiennej reprezentujacej surowe dane odczytane z obrazka
    Color  bufor;   //przechowuje pojedynczy pixel do obrobki
    RawData=new Color  [w*h];
    int p=0;
    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++)
        {
            bufor=getPixel(x,y);
            ConvertColor(bufor,TrybKoloru);
            RawData[p++]=bufor;
        }
/*******************************KOMPRESJA DANYCH Z TABLICY ********************************/
GGKKHeader.width=new char[4];
GGKKHeader.heigth=new char[4];
GGKKHeader.width=itoa(w,GGKKHeader.width,10);
GGKKHeader.heigth=itoa(h,GGKKHeader.heigth,10);

byterun(RawData,w*h,ByteRun);  //wywołanie funkci kompresujacych
RLE(RawData,w*h,RLEOut);

if(ByteRun.size()<RLEOut.size()){
        GGKKHeader.TrybKonwersji[1]='1';
        for(int i=0; i<4;i++) putc (GGKKHeader.Name[i] , plik);
        putc (GGKKHeader.TrybKonwersji[1] , plik);
        putc (GGKKHeader.TrybKoloru[1] , plik);
        for(int i=0; i<4;i++)  putc (GGKKHeader.width[i] , plik);
        for(int i=0; i<4;i++)  putc (GGKKHeader.heigth[i] , plik);
        for(int i=0;i<ByteRun.size();i++) putc (ByteRun[i] , plik);
}
else
{
        GGKKHeader.TrybKonwersji[1]='0';
        for(int i=0; i<4;i++) putc (GGKKHeader.Name[i] , plik);
        putc (GGKKHeader.TrybKonwersji[1] , plik);
        putc (GGKKHeader.TrybKoloru[1] , plik);
        for(int i=0; i<4;i++)  putc (GGKKHeader.width[i] , plik);
        for(int i=0; i<4;i++)  putc (GGKKHeader.heigth[i] , plik);
        for(int i=0;i<ByteRun.size();i++) putc (ByteRun[i] , plik);
}
delete(RawData); //usuwam tablicę dynamiczna
RLEOut.clear();
ByteRun.clear();
fclose (plik);
SDL_Flip(screen);
}

Color getPixel (int x, int y) {
    Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<w) && (y>=0) && (y<h)) {
        //determine position
        char* pPosition=(char*)screen->pixels ;
        //offset by y
        pPosition+=(screen->pitch*y) ;
        //offset by x
        pPosition+=(screen->format->BytesPerPixel*x);
        //copy pixel data
        memcpy(&col, pPosition, screen->format->BytesPerPixel);
        //convert color
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}

int frame() /*FUNKCJA INICJALIZUJACA W OKNIE OBRAZKA SDL*/
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }
    /* LADUJ BMP */
        SDL_Surface* bmp;
        SDL_Rect dstrect; //do wczytywania bmp
        bmp = SDL_LoadBMP(Bufor); //wczytywanie bmp z nazwy
        dstrect.x = 0;
        dstrect.y = 0;
        w=bmp->w;  //ustalamy wielkosc wczytanego obrazka
        h=bmp->h;
    /*************/
    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    screen = SDL_SetVideoMode(w, h, 32,SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_BlitSurface(bmp, 0, screen, &dstrect); //wyswietl bmp
    if ( !screen )
    {
        printf("Unable to set video: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Flip(screen);
    SDL_FreeSurface(bmp); //usuwanie bmp
    SDL_WM_SetCaption( tytul , NULL );
}

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND g_hPrzycisk;
// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wc;		//struktura majaca informacje o klasie okna

	ZeroMemory(&wc, sizeof(WNDCLASSEX)); //zerowanie prestrzeni pamieci dla struktury klasy okna (wc- window class)
	//wypelnianie strukturki
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "WindowClass";
	//rejestrowanie klasy
	RegisterClassEx(&wc);

	RECT wr = { 0, 0, w, h };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	//tworzenie okna ************
	hWnd = CreateWindowEx(NULL,
		"WindowClass",
		"Konwerter BMP do GGKK",
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		500,
		150,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow); 	//pokaz okno

	MSG msg;  //ta struktura przechowuje info o wydarzeniach z nia zwiazanych
	/*************************************TU WSZYSTKIE KONTROLKI *************************************/
    hCombo = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
		CBS_DROPDOWN, 5, 50, 70, 200, hWnd, (HMENU)ID_Color, hInstance, NULL);

	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) "HSV");
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) "HSL");
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) "YUV");

	//Nazwa nowego pliku
    hText1 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
    WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 100, 50, 380, 25, hWnd, NULL, hInstance, NULL);
	SetWindowText(hText1, "Nazwa nowego pliku");

    //sciezka dostepu do pliku
    hText = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
    WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 5, 5, 475, 40, hWnd, NULL, hInstance, NULL);
	SetWindowText(hText, "Wpisz tu sciezke do pliku");

	g_hPrzycisk = CreateWindowEx(0, "BUTTON", "Konwertuj", WS_CHILD | WS_VISIBLE,
		250, 80, 180, 30, hWnd, (HMENU)ID_Convert, hInstance, NULL);

	/*************************************TU KONIEC KONTROLEK*************************************/

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
            break;
		}
	}

//GlobalFree(Bufor);
	return msg.wParam;
}
/*************************************************************************************************************************************/

/*OBSLUGA KOMUNIKATOW*/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
/**************************/
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
/**************************/
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
/**************************/
        case WM_COMMAND:
            switch (wParam)
            {
            /**************************/
            case ID_Convert:
                TrybKoloru = ComboBox_GetCurSel( hCombo ); //pobieram tryb koloru, jsli pusty: brak konwersji koloru
                dlugosc = GetWindowTextLength(hText);
                Bufor = (LPSTR)GlobalAlloc(GPTR, dlugosc + 1);
                GetWindowText(hText, Bufor, dlugosc + 1); // pobralem sciezke dostepu do zmiennej bufor
                dlugosc = GetWindowTextLength(hText1);
                NowaNazwaPliku = (LPSTR)GlobalAlloc(GPTR, dlugosc + 1);
                GetWindowText(hText1, NowaNazwaPliku, dlugosc + 1); //tu sciezka dla nowego pliku
                frame();
                newfile(w,h);
                break;
            /**************************/
            }
         break;
/**************************/
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
