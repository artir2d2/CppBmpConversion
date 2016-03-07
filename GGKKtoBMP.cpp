#include <windows.h>  //biblioteka do okienka
#include <windowsx.h> //tu tez
#include <string>
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
#define ID_Color 111	    //id
#define ID_Convert 555	    //id Przycisku konwertuj potrzebny do identyfikcaji wiadomosci z przycisku "konwertuj"
/**************************/

using namespace std;

SDL_Surface *screen; //struktura okna dla SDL
char const* tytul = "Okno obrazka";
int TrybKoloru; //zmienna ta przekazywana do funkcji ConvertColor określa wybraną opcje trybu koloru
int w=0, h=0; //wielkosc pliku graficznego

LPSTR Bufor,NowaNazwaPliku; //stringi przechowujące sciezke do pliku
DWORD dlugosc;

HWND hWnd;	//uchwyt okna glownego
HWND hCombo;  //uchwyt comboboxa
HWND hText,hText1; //uchwyty pól tekstowych

struct Header{
char Name [4];
char TrybKonwersji[1];
char TrybKoloru[1];
char width[5];
char heigth[5];
};
Header GGKKHeader;

struct Color{
Uint8 r;
Uint8 g;
Uint8 b;
};

int frame(); //funkcja tworząca okno SDL
void ConvertColor(Color bufor, int TrybKoloru); //funkcja konwersji kolorów

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B); //setpixel...
void ladujBMP(char const* nazwa, int x, int y); //ładuj BMP...
Color getPixel (int x, int y);  //pobierz pixel z ekranu...
Color HSVtoRGB(Color bufor);
Color HSLtoRGB(Color bufor);
Color YUVtoRGB(Color bufor);
void openGGKK(LPSTR Bufor);
//void ByteRunDecomp(Color a[], vector <int> &CompressedData,int buffer);
void ByteRunDecomp(Color a[], int CompressedData[],long buffer);
void RLEDecomp(Color a[], int CompressedData[],long buffer);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<atoi(GGKKHeader.width)) && (y>=0) && (y<atoi(GGKKHeader.heigth)))
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
int frame() /*FUNKCJA INICJALIZUJACA W OKNIE OBRAZKA SDL*/
{
   if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }
    // make sure SDL cleans up before exit
    atexit(SDL_Quit);
    // create a new window
    screen = SDL_SetVideoMode(w ,h, 32,SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set video: %s\n", SDL_GetError());
        return 1;
    }
    SDL_WM_SetCaption( tytul , NULL );
}

Color HSLtoRGB(Color bufor)
{
    double R,G,B,C,X,m;
    double r1,g1,b1;
    double H = double(bufor.r)*1.41;

    C = double(1.- fabs(2*double(bufor.b) - 1.)*double(bufor.g));
    X = double(C * (1. - fabs(fmod((H/60),2.) - 1.)));
    m = double( double(bufor.b) - C/2.);

    if(H < 0) H=H+360.;
    if(H >= 0 && H < 60) {r1=C; g1=X; b1=0;};
    if(H >= 60 && H < 120) {r1=X; g1=C; b1=0;};
    if(H >= 120 && H < 180) {r1=0; g1=C; b1=X;};
    if(H >= 180 && H < 240) {r1=0; g1=X; b1=C;};
    if(H >= 240 && H < 300) {r1=X; g1=0; b1=C;};
    if(H >= 300 && H < 360) {r1=C; g1=0; b1=X;};

    R = r1 + m;
    G = g1 + m;
    B = b1 + m;

    bufor.r = Uint8(R);
    bufor.g = Uint8(G);
    bufor.b = Uint8(B);

    return bufor;

}

Color HSVtoRGB(Color bufor)
{
    double R,G,B,C,X,m;
    double r1,g1,b1;
    double H = double(bufor.r)*1.41;
    C = double(bufor.b) * double(bufor.g);
    X = double(C*(1.-fabs(fmod(double(double(bufor.r)/60),2.)-1)));
    m = double(double(bufor.b) - C);

    if(H < 0) H=H+360.;
    if(H >= 0 && H < 60) {r1=C; g1=X; b1=0;};
    if(H >= 60 && H < 120) {r1=X; g1=C; b1=0;};
    if(H >= 120 && H < 180) {r1=0; g1=C; b1=X;};
    if(H >= 180 && H < 240) {r1=0; g1=X; b1=C;};
    if(H >= 240 && H < 300) {r1=X; g1=0; b1=C;};
    if(H >= 300 && H < 360) {r1=C; g1=0; b1=X;};

    R = r1 + m;
    G = g1 + m;
    B = b1 + m;

    bufor.r = Uint8(R);
    bufor.g = Uint8(G);
    bufor.b = Uint8(B);

    return bufor;
}

Color YUVtoRGB(Color bufor)
{
    double R,G,B;
    R= double(bufor.r) + 1.14*double(bufor.b);
    G= double(bufor.r) - 0.395*double(bufor.g) - 0.581*double(bufor.b);
    B= double(bufor.r) + 2.033*double(bufor.g);

    bufor.r = Uint8(R);
    bufor.g = Uint8(G);
    bufor.b = Uint8(B);

    return bufor;
}

void ConvertColor(Color bufor,int TrybKoloru) //zmienna tryb przekazywana jest z Combo Boxa i przyjmuje wartości 0-3
{
        switch (TrybKoloru)
    {
    case 3:

        break;
    case 0:  //konwersja z HSV
        HSVtoRGB( bufor);
        break;
    case 1:  //konwersja z HSL
        HSLtoRGB( bufor);
        break;
    case 2: //konwersja z YUV
        YUVtoRGB( bufor);
        break;
    }
}

void openGGKK(LPSTR Bufor)
{
    FILE* plik;
    plik = fopen(Bufor, "rb");
    if (plik==NULL) {fputs ("File error",stderr); exit (1);}
    long lSize;
    int charBufor[1];
    fseek (plik , 0 , SEEK_END);
    lSize = ftell(plik);
    rewind (plik);

    int *CompressedData= new int[lSize-14];
    //Odczyt nagłówka
    for(int i=0;i<4;i++) {charBufor[0]=fgetc(plik);  GGKKHeader.Name[i]=charBufor[0];}
    if(GGKKHeader.Name[0]!='g' || GGKKHeader.Name[1]!='g' ||  GGKKHeader.Name[2]!='k' ||   GGKKHeader.Name[3]!='k'){cout<<"Wybrany plik nie jest plikiem GGKK!";};

    GGKKHeader.TrybKonwersji[0]=char(fgetc(plik));
    GGKKHeader.TrybKoloru[0]=char(fgetc(plik));
    for(int i=0;i<4; i++) {charBufor[0]=fgetc(plik); GGKKHeader.width[i]=charBufor[0];}
    for(int i=0;i<4; i++) {charBufor[0]=fgetc(plik); GGKKHeader.heigth[i]=charBufor[0];}
    //koniec odczytu nagłówka
    cout<<"odczytano dane z naglowka \n";
    w=atoi(GGKKHeader.width);
    h=atoi(GGKKHeader.heigth);

     frame(); //wywołanie funkcji okna sdl

    long tablica=w*h; //wielkosc pliku wyjsciowego w piks
    Color *Out= new Color [tablica]; //tablica danych wynikowych

    int iter=0;
    //pobieranie zawartości pliku do wektora
    while (iter<lSize-14)
    {
    charBufor[0]=fgetc(plik);
    CompressedData[iter]=charBufor[0];
    iter++;
    }
cout<<"zapisano dane do wektora \n";

if(GGKKHeader.TrybKonwersji[0]=='0') {cout<<"Byterun"; ByteRunDecomp(Out,CompressedData,lSize-14);}
else RLEDecomp(Out,CompressedData,lSize-14);

cout<<"zdekompresowano dane \n"<<tablica<<endl;

for(int i=0;i<tablica;i++) ConvertColor(Out[i],GGKKHeader.TrybKoloru[0]);
cout<<"przekonwertowano kolory \n";

    long liczba=0;

    int x=0,y=0;
    for(y=0;y<h;y++)
    {
        for(x=0;x<w;x++)
        {
            setPixel(x,y,Out[liczba].r,Out[liczba].g,Out[liczba].b);
            liczba=liczba+1;
        }
    }
    cout<<"wyswietlono piksele \n";
char rozszerz[6]=".bmp";
strcat(NowaNazwaPliku, rozszerz);
SDL_SaveBMP( screen, NowaNazwaPliku );
free (charBufor);
fclose(plik);
delete(CompressedData);
 SDL_Flip(screen);
}

void RLEDecomp(Color a[], int CompressedData[],long buffer)
{
long i=0, g=0;
int temp = 0;
while (i < buffer)
{
if (CompressedData[i]== 0)
{
temp=CompressedData[i+1];
for(int j=0; j<temp;j++)
{
a[g].r=Uint8(CompressedData[i+2+3*j]);
a[g].g=Uint8(CompressedData[i+3+3*j]);
a[g].b=Uint8(CompressedData[i+4+3*j]);
g++;
i+=3*temp+2;
}
}
else
{
temp=CompressedData[i];
for(int j=0; j<temp; j++)
{
a[g].r=Uint8(CompressedData[i+1]);
a[g].g=Uint8(CompressedData[i+2]);
a[g].b=Uint8(CompressedData[i+3]);
g++;
}
i+=4;
}
}
}

void ByteRunDecomp(Color a[], int CompressedData[],long buffer)
{
long i = 0,g=0,l=0;
while (i < buffer )
{
    if (CompressedData[i]-128 <0)
    {
        for (int j=0; j<-(CompressedData[i]-129); j++)
        {
            a[g].r=Uint8(CompressedData[i+1]);
            a[g].g=Uint8(CompressedData[i+2]);
            a[g].b=Uint8(CompressedData[i+3]);
            g++;
        }
        i += 4;

    }
    else
    {
        for (int j=0; j<(CompressedData[i]-127); j++)
        {
            a[g].r=Uint8(CompressedData[i+1+j*3]);
            a[g].g=Uint8(CompressedData[i+2+j*3]);
            a[g].b=Uint8(CompressedData[i+3+j*3]);
            g++;
        }
        i += 3*(CompressedData[i]-127)+1;
    }
l++;
}
}



LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND g_hPrzycisk;
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
		"Program1",
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

                openGGKK(Bufor);
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

