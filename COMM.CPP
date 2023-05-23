#include<dos.h>
#include<stdio.h>
#include<ctype.h>

#define TRUE -1
#define FALSE 0
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(b):(a))

#define INTR(com,x,y) {_DX=com;_AH=x;_AL=y;geninterrupt(0x14);}

class COMM{
    int port,buf_idx;
    char buf[20];
  public:
    int OpenComm(int );
    int OpenComm(char *initstr);
    void SetSpeed(long speed);
    void SetData(int);
    void SetParity(int p);
    void SetStop(int n);
    void CharOut(char c);
    int CharIn();
    int WaitChar();
    void UnGetChar(int c);
    long Read(char *buf,long len);
    long Write(char *buf,long len);
    void ClrInBuf();
    void ClrOutBuf();
    unsigned int cbInQue();
    unsigned int cbOutQue();
    void SetDTR(char state);
    void SetRTS(char state);
    int GetCTS();
    int GetCarrier();
    int GetLineReg();
    int GetModemReg();
    int DriverPresent();
    void DriverRemove();
};

int COMM::OpenComm(int com)
{
        char str[]="COMx:1200,8,N,1";
        str[3]='0'+com;
        return OpenComm(str);
}

int COMM::OpenComm(char *initstr)
{
    int cport,parity,stop,data,sp;
    long speed;

    if (sscanf(initstr,"COM%d:%ld,%d,%c,%d",&cport,&speed,&data,&parity,&stop)!=5)
        return FALSE;
    if (!DriverPresent()) return FALSE;
    port=MAX(1,MIN(4,cport));
    buf_idx=0;
    if (port!=cport) return FALSE;
    SetSpeed(speed);
    SetData(data);
    SetParity(parity);
    SetStop(stop);
    return TRUE;
}

void COMM::SetSpeed(long speed)
{
    int b;
    switch(speed){
        case 50: b=1;break;
        case 75: b=2;break;
        case 110: b=3;break;
        case 150: b=4;break;
        case 300: b=5;break;
        case 600: b=6;break;
        case 1200: b=7;break;
        case 2400: b=8;break;
        case 9600: b=9;break;
        case 19200: b=10;break;
        case 38400: b=11;break;
        case 115200: b=12;break;
        default:speed=2400;b=8;break;
    }

    INTR(port,0,b);
}

void COMM::SetData(int data)
{
    data=MAX(5,MIN(8,data));
    INTR(port,1,data);
}

void COMM::SetParity(int p)
{
    switch(toupper(p)){
        case 'N': p=0;break;
        case 'E': p=1;break;
        case 'O': p=2;break;
    }

    INTR(port,2,p);
}

void COMM::SetStop(int n)
{
    n=MAX(1,MIN(2,n));
    INTR(port,3,n);
}

void COMM::CharOut(char c)
{
    INTR(port,5,c);
}

int COMM::WaitChar()
{
    int c;
    while((c=(*this).CharIn())==-1);
    return c;
}

int COMM::CharIn()
{
    if (buf_idx) return buf[--buf_idx];
    INTR(port,6,0);
    return _AX;
}

void COMM::UnGetChar(int c)
{
    buf[buf_idx++]=c;
}

long COMM::Read(char *buf,long len)
{
    long l=len;
    while(l--)
        *buf++=CharIn();
    return len;
}

long COMM::Write(char *buf,long len)
{
    long l;
    for(l=0;l<len;l++,buf++)
        CharOut(*buf);
    return len;
}

void COMM::ClrInBuf()
{
    buf_idx=0;
    INTR(port,7,0);
}

void COMM::ClrOutBuf()
{
    INTR(port,8,0);
}

void COMM::SetDTR(char state)
{
        INTR(port,9,state);
}

void COMM::SetRTS(char state)
{
        INTR(port,10,state);
}

int COMM::GetCarrier()
{
        INTR(port,11,0);
        return _AX;
}

int COMM::GetCTS()
{
        INTR(port,12,0);
        return _AX;
}

int COMM::DriverPresent()
{
        int myax;
        INTR(1,100,0);
        myax=_AX;
        if (myax==1234) return TRUE;
        else return FALSE;
}

void COMM::DriverRemove()
{
        INTR(1,13,0);
}

unsigned int COMM::cbInQue()
{
        INTR(port,14,0);
        return _AX+buf_idx;
}

unsigned int COMM::cbOutQue()
{
        INTR(port,15,0);
        return _AX;
}

int COMM::GetLineReg()
{
        INTR(port,16,0);
        return _AX;
}

int COMM::GetModemReg()
{
        INTR(port,17,0);
        return _AX;
}

