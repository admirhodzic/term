#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<string.h>
#include<dos.h>
#include"comm.h"

#define XON 17
#define XOFF 19
#define FALSE 0
#define TRUE -1
#define MAX(x,y) ((x<y)?(y):(x))

void ProcessCh(int c);
int ProcessKey(int ch);
void Lex(int c);
void inschr();
void delchr();
void myinsline();
void mydelline();
void deltoeos();
void curdn(int n);
void curdn(int n);
void curup(int n);
void curleft(int n);
void curright(int n);
void attrset();
void LoadKey();
void deltoeos();
void chgscr(int s);
void InitScr();
void scroll_f();
void scroll_b();

char keys[255][20];

int _scr[5][2000],_scr_x[5]={1,1,1,1,1},_scr_y[5]={1,1,1,1,1};

int altset=0,
    seq=0,
    p[10],
    pn=1,
    a1=7,
    a2=0,
    save_x=1,
    save_y=1,
    printer=0;

FILE *prn;

int kraj=0,xoff=FALSE;

int cbreak()
{ return 1;}

main(int argc,char *argv[])
{
    COMM c;
    int ch=0,com,i;
    // char op[]="COMx:9600,8,N,1";

    if (argc<2) {puts("Usage: term <open_str> (eg.: COM1:9600,8,N,1)!");exit(1);}
    CommDrvInit();
    if (c.OpenComm(argv[1])==FALSE) {puts("error opening COM port");exit(1);}

    ctrlbrk(cbreak);

    LoadKey();
    clrscr();
    InitScr();
    chgscr(0);
    _setcursortype(_SOLIDCURSOR);
    attrset();

    do{
        if (kbhit()&& printer==0) {ch=getch();
             if (ch==0) {ch=getch();
                if (!ProcessKey(ch)) c.Write(keys[ch],strlen(keys[ch]));
                }
             else  {c.CharOut(ch);}}
        if ((ch=c.CharIn())!=-1) {ProcessCh(ch);}

        if (c.cbInQue()>1000) {xoff=TRUE;c.CharOut(XOFF);}
        if (xoff) if (c.cbInQue()<100) {xoff=FALSE;c.CharOut(XON);}

    }while(!kraj);
    CommDrvEnd();
}

int ProcessKey(int ch)
{
    switch(ch)
    {
        case 1: kraj=1;return 1;
    }
    return 0;
}

void ProcessCh(int c)
{
    int f,x;
    if (seq) {Lex(c);return;}
    if (printer &&(c!=27)) fputc(c,stdprn);
    else
      switch(c){
        case 27: seq=1;pn=1;for(f=0;f<10;f++) p[f]=0;break;
        case 7:  sound(1700);delay(250);nosound();break;
        case 8:  curleft(1);break;
        case 9:  gotoxy(((wherex()+8)/8)*8,wherey());break;
        case 10: if (wherey()<25) curdn(1);
                 else {x=wherex();gotoxy(1,1);mydelline();gotoxy(x,25);}
                 break;
        case 13: gotoxy(1,wherey());break;
        default: if (altset) c+=128;
                 if (printer) fputc(c,stdprn);
                 else {if (wherey()==25&&wherex()==80) {putch(c);clreol();}
                       else putch(c);}
                 break;
      }
}

void Lex(int c)
{
    if ((printer) && (seq==2) && (c!='.')) {
        seq=0;fputc(27,stdprn);fputc('[',stdprn);fputc(c,stdprn);return;}
    else if((printer) && (seq==1) && (c!='[')) {
        fputc(27,stdprn);fputc(c,stdprn);seq=0;return;}

    switch(seq)
    {
        case 1: switch(c)
                {
                    case '[':seq=2; return;
                    case '1':chgscr(0);seq=0;break;
                    case '2':chgscr(1);seq=0;break;
                    case '3':chgscr(2);seq=0;break;
                    case '4':chgscr(3);seq=0;break;
                    case '5':chgscr(4);seq=0;break;
                };break;
        case 2: switch(c)
                {
                    case '0':case '1':case '2':case '3':case '4':
                    case '5':case '6':case '7':case '8':case '9':
                      p[pn]=p[pn]*10+c-'0';break;
                    case ';': pn++;break;

                    case '!':printer=1;seq=0;break;
                    case '.':printer=0;seq=0;break;

                    case 'L':myinsline();seq=0;break;
                    case '@':inschr();seq=0;break;
                    case 'Z':gotoxy(((int)(wherex()/8))*8,wherey());seq=0;break;
                    case 'm':attrset();seq=0;break;
                    case 'B':curdn(p[1]);seq=0;break;
                    case 'H':gotoxy(MAX(p[2],1),MAX(p[1],1));seq=0;break;
                    case 'f':gotoxy(MAX(p[2],1),MAX(p[1],1));seq=0;break;
                    case 'D':curleft(p[1]);seq=0;break;
                    case 'C':curright(p[1]);seq=0;break;
                    case 'A':curup(p[1]);seq=0;break;
                    case 'P':delchr();seq=0;break;
                    case 'M':mydelline();seq=0;break;
                    case 'K':clreol();seq=0;break;
                    case 'T':scroll_f();seq=0;break;
                    case 'S':scroll_b();seq=0;break;
                    case 'J':if (p[1]==2) clrscr(); else deltoeos();
                             seq=0;break;
                    case 's':save_x=wherex();save_y=wherey();seq=0;break;
                    case 'u':gotoxy(save_x,save_y);seq=0;break;
                    default: seq=0;break;
                }
    }
}

void inschr()
{
     movetext(wherex(),wherey(),79,wherey(),wherex()+1,wherey());
}

void myinsline()
{
    int x,y;
    x=wherex();y=wherey();
    insline();
    gotoxy(1,y);clreol();
    gotoxy(x,y);
}

void mydelline()
{
    int x,y;
    x=wherex();y=wherey();
    delline();
    gotoxy(1,25);clreol();
    gotoxy(x,y);
}

void delchr()
{
     movetext(wherex()+1,wherey(),80,wherey(),wherex(),wherey());
}

void deltoeos()
{
    int f,x,y;
    x=wherex();y=wherey();
    for(f=wherex();f<=80;f++) putch(' ');
    gotoxy(1,y+1);for(f=1;f<25-y;f++) mydelline();
    gotoxy(x,y);
}

void curdn(int n)
{
    n=MAX(1,n);
    gotoxy(wherex(),wherey()+n);
}

void curup(int n)
{
    n=MAX(1,n);
    gotoxy(wherex(),wherey()-n);
}

void curleft(int n)
{
    n=MAX(1,n);
    gotoxy(wherex()-n,wherey());
}

void curright(int n)
{
    n=MAX(1,n);
    gotoxy(wherex()+n,wherey());
}

void attrset()
{
    int f;
    for(f=1;f<=pn;f++)
    {
        switch(p[f])
        {
            case -1: a1=7;break;
            case 12: altset=1;break;
            case 5 : a1=9;break;
            case 1 : a1=1;break;
            case 7 : a1=0;a2=7;break;
            case 4 : a1=1;break;
            case 10: altset=0;break;
            case 0 : a1=7;a2=0;break;
            default: if (p[f]>29 && p[f]<40) a1=p[f]-30;
                     if (p[f]>39 && p[f]<50) a2=p[f]-40;
        }
    }
    textattr(a1);textbackground(a2);
}

void LoadKey()
{
    FILE *fp;
    int i,f;
    char str[50],cmnt[50];

    fp=fopen("keys.def","rt");
    while(!feof(fp)){
        if (fscanf(fp," %d %s %s",&i,str,cmnt)!=3) puts("error in keys.def!");
        else {for(f=0;f<strlen(str);f++)
                if (str[f]=='^') {str[f]=str[f+1]-64;strcpy(str+f+1,str+f+2);}
            strcpy(keys[i],str);}
    }
    fclose(fp);
}

void InitScr()
{
    int f,t;
    for(t=0;t<5;t++){
     for(f=0;f<2000;f++) _scr[t][f]=0x007;
     _scr_x[t]=1;_scr_y[t]=1;
    }

}

void chgscr(int s)
{
    static cs=0;

    gettext(1,1,80,25,_scr[cs]);
    _scr_x[cs]=wherex();_scr_y[cs]=wherey();
    cs=s;
    puttext(1,1,80,25,_scr[s]);
    gotoxy(_scr_x[s],_scr_y[s]);
}

void scroll_f()
{
int x,y;
x=wherex(); y=wherey();
gotoxy(1,1);
myinsline();
clreol();
gotoxy(x,y);
}

void scroll_b()
{
int x,y;
x=wherex(); y=wherey();
gotoxy(1,1);
mydelline();
gotoxy(1,25);
clreol();
gotoxy(x,y);
}

