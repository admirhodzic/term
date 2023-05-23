#include<bios.h>
#include<dos.h>
#include<alloc.h>
#include<stdio.h>
#include<stdlib.h>


int com_port[]={0x3f8,0x2f8,0x3e8,0x2e8};
int com_int[]={0xc,0xb,0xc,0xb};
unsigned char com_mask[]={0x10,0x8,0x10,0x8},com_mask2;

int com_port2,com_int2;

#define RBUF_SIZE 3000
#define SBUF_SIZE 1000
#define COMS 4
int cur_com;
/* mora staticki */
char mem_buf[COMS][RBUF_SIZE],mem_sbuf[COMS][SBUF_SIZE];

char *buf[COMS],*buf_end[COMS],*buf_start[COMS],*buf_get[COMS];
char *sbuf1[COMS],*sbuf2[COMS],*sbuf_start[COMS],*sbuf_end[COMS];

volatile int out_busy[]={0,0,0,0};

void interrupt (*old_driver)();
void interrupt (*old_handler[COMS])();

char tmp=0,tmp2;
int tmpi;

#define INTR(com,x,y) {_DX=com;_AH=x;_AL=y;geninterrupt(0x14);}

int comdrv_here()
{
        int myax;
        INTR(1,100,0);
        myax=_AX;
        if (myax==1234) return -1;
        else return 0;
}

void comdrv_remove()
{
        INTR(1,13,0);
}

void primi(void)
{
    _DX=com_port[cur_com];
    asm in al,dx ;
    tmp=_AL;

    *buf[cur_com]=tmp;
    buf[cur_com]++;
    if (buf[cur_com]==buf_end[cur_com]) buf[cur_com]=buf_start[cur_com];
}

void posalji(void)
{
     if (sbuf2[cur_com]==sbuf1[cur_com]) out_busy[cur_com]=0;
     else{
         out_busy[cur_com]=1;
         _AL=(char)*sbuf2[cur_com];
         _DX=com_port[cur_com];
         asm out dx,al
         sbuf2[cur_com]++;
         if (sbuf2[cur_com]==sbuf1[cur_com])
            sbuf1[cur_com]=sbuf2[cur_com]=sbuf_start[cur_com];
         }
}


void interrupt handler(void)
{

for(cur_com=0;cur_com<COMS;cur_com++){
       com_port2=com_port[cur_com];
    lloop:
       _DX=com_port2+2;
       asm{
         in al,dx
         test al,1
         jz int_req
         }

         goto kraj;
    int_req:;
       if (_AL!=2) goto recive;
       // posalji
       if (sbuf2[cur_com]==sbuf1[cur_com]) out_busy[cur_com]=0;
       else{
          out_busy[cur_com]=1;
          _AL=(char)*sbuf2[cur_com];
          _DX=com_port[cur_com];
          asm out dx,al
          sbuf2[cur_com]++;
          if (sbuf2[cur_com]==sbuf1[cur_com])
             sbuf1[cur_com]=sbuf2[cur_com]=sbuf_start[cur_com];
          }
       goto lloop;
    recive:
       if (_AL!=4) goto other;
        // primi
       _DX=com_port[cur_com];
       asm in al,dx ;
       tmp=_AL;

       *buf[cur_com]=tmp;
       buf[cur_com]++;
       if (buf[cur_com]==buf_end[cur_com]) buf[cur_com]=buf_start[cur_com];
       goto lloop;

    other:
       goto lloop;

kraj:;
    }
         // kraj interapta
asm{
         mov al,20h
         out 20h,al
        }
}

int comdrv_init()
{
int f;

for(f=0;f<COMS;f++){
    buf[f]=mem_buf[f];
    buf_start[f]=buf[f];buf_get[f]=buf[f];buf_end[f]=buf[f]+RBUF_SIZE;
    sbuf1[f]=sbuf2[f]=sbuf_start[f]=mem_sbuf[f];
    sbuf_end[f]=sbuf_start[f]+SBUF_SIZE;
}

for(cur_com=0;cur_com<COMS;cur_com++){
      com_port2=com_port[cur_com];
      asm{
          mov dx,com_port2
          add dx,6
          in al,dx
          jmp a1}
      a1:asm{

          sub dx,5
          mov al,3          // 00000011b  IER
          out dx,al
          jmp a2}
      a2:asm{

          inc dx
          in al,dx
          jmp a3}
      a3:asm{

          sub dx,2
          in al,dx
          jmp a4}

       a4:asm{

          in al,21h
          push ax
          mov al,0xff
          out 0x21,al
          };

  old_handler[cur_com]=getvect(com_int[cur_com]);
  setvect(com_int[cur_com],handler);

  com_mask2=com_mask[cur_com];
        asm{
          mov dx,com_port2
          add dx,4     // * MCR
          mov al,9     // 00001001b
          out dx,al

          pop ax
          mov ah,0xff
          xor ah,com_mask2
          and al,ah
          out 0x21,al
         };
     }

return 0;
}

int comdrv_in()
{    if (buf_get[cur_com]!=buf[cur_com]){tmp=*buf_get[cur_com];buf_get[cur_com]++;
        if (buf_get[cur_com]==buf_end[cur_com])
           buf_get[cur_com]=buf_start[cur_com];
           return (unsigned char)tmp;}
    else return -1;
}

void storetosbuf(char c)
{
    while(sbuf1[cur_com]==sbuf_end[cur_com]) ;
    *sbuf1[cur_com]=c;
    sbuf1[cur_com]++;
}

int comdrv_out(char c)
{
   if (out_busy[cur_com]) storetosbuf(c);
   else{
       out_busy[cur_com]=1;
       _AL=c;
       _DX=com_port[cur_com];
       asm out dx,al
       }
   return 0;
}

int setcomdrv_speed(int b)
{
     switch(b){
         case 1 :mysetcom(9,0);break;      /* 50 bps */
         case 2 :mysetcom(6,0);break;      /*     75    */
         case 3 :mysetcom(4,0x17);break;   /*    110    */
         case 4 :mysetcom(3,0);break;      /*    150    */
         case 5 :mysetcom(1,0x80);break;   /*    300    */
         case 6 :mysetcom(0,0xc0);break;   /*    600    */
         case 7 :mysetcom(0,0x60);break;   /*   1200    */
         case 8 :mysetcom(0,0x30);break;   /*   2400    */
         case 9 :mysetcom(0,0xc);break;    /*   9600    */
         case 10:mysetcom(0,6);break;      /*  19200    */
         case 11:mysetcom(0,3);break;      /*  38400    */
         case 12:mysetcom(0,1);break;      /* 115200    */
         default:return 1;
         }
    return 0;
}

int mysetcom(unsigned char mybb1,unsigned char mybb2)
{
    com_port2=com_port[cur_com];
asm{
    cli
    mov dx,com_port2
    add dx,3
    in al,dx
    or al,10000000b
    out dx,al
    mov dx,com_port2
    mov al,mybb2
    out dx,al
    mov al,mybb1
    inc dx
    out dx,al
    mov dx,com_port2
    add dx,3
    in al,dx
    and al,01111111b
    out dx,al
    sti
   };
}

int setcomdrv_data(unsigned char d)
{
    d-=5;
    d&=3;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,3
        in al,dx
        and al,11111100b
        mov cl,d
        or al,cl
        out dx,al
    };
return 0;
}

int setcomdrv_parity(unsigned char p)
{
    switch(p){
        case 0:p=0;break;   // no
        case 1:p=0x18;break;// even
        case 2:p=0x8;break; // odd
        default:return 1;
    }
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,3
        in al,dx
        and al,11100111b
        mov cl,p
        or al,cl
        out dx,al
    }
    return 0;
}

int setcomdrv_stop(unsigned char s)
{
    s=((s==2)?4:0);
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,3
        in al,dx
        and al,11111011b
        mov cl,s
        or al,cl
        out dx,al
    }
    return 0;
}

int clr_buf()  { buf[cur_com]=buf_get[cur_com]=buf_start[cur_com]; }
int clr_sbuf() { sbuf1[cur_com]=sbuf2[cur_com]=sbuf_start[cur_com]; }

int drvset_dtr(unsigned char state)
{
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,4  /* MCR */
        in al,dx
        and al,11111110b
        mov ah,state
        and ah,1
        or al,ah
        out dx,al
        and al,1
        xor ah,ah
    }
}

int drvset_rts(unsigned char state)
{
    state=state ? 2 : 0;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,4  /* MCR */
        in al,dx
        and al,11111101b
        mov ah,state
        or al,ah
        out dx,al
        and al,1
        xor ah,ah
    }
}

char drvget_linestatusreg()
{
    char state;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,5
        in al,dx
        mov state,al
    }
    return state;
}

char drvget_modemstatusreg()
{
    char state;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,6
        in al,dx
        mov state,al
    }
    return state;
}

char drvget_carrier()
{
    char state=0;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,6  /* MSR */
        in al,dx
        and al,10000000b
        mov state,al
    }
    state=state ? -1 : 0;
    return state;
}

char drvget_cts()
{
    char state=0;
    com_port2=com_port[cur_com];
    asm{
        mov dx,com_port2
        add dx,4  /* MCR */
        in al,dx
        and al,00010000b
        mov state,al
    }
    state=state ? -1 : 0;
    return state;
}

void interrupt far driver(unsigned _bp,unsigned _di1,unsigned _si1,
                          unsigned _ds1,unsigned _es1,unsigned _dx,
                          unsigned _cx,unsigned _bx,unsigned _ax,
                          unsigned ip,unsigned cs1,unsigned flags)
{

    cur_com=_DX;
    tmp=_AH;
    tmp2=_AL;
    cur_com--;
    /* OK znaci trazeni com */
    switch(tmp){
        case 0: _ax=setcomdrv_speed(tmp2);break; /* set speed */
        case 1: _ax=setcomdrv_data(tmp2);break;  /* set data  */
        case 2: _ax=setcomdrv_parity(tmp2);break; /* set parity*/
        case 3: _ax=setcomdrv_stop(tmp2);break; /* set stop  */
        case 5: _ax=comdrv_out(tmp2);break; /* send char  */
        case 6: _ax=comdrv_in();break; /* get char (return -1 if no char in buffer) */
        case 7: clr_buf();break; /* clear in buffer */
        case 8: clr_sbuf();break; /* clear out buffer */
        case 9: _ax=drvset_dtr(tmp2);break;
        case 10: _ax=drvset_rts(tmp2);break;
        case 11: _ax=drvget_carrier();break;
        case 12: _ax=drvget_cts();break;
        case 13: {int f;
             for(f=0;f<COMS;f++) setvect(com_int[f],old_handler[f]);
             setvect(0x14,old_driver);
             exit(0);break;}
        case 14: if (buf[cur_com]>=buf_get[cur_com])
                    _ax=FP_OFF(buf[cur_com])-FP_OFF(buf_get[cur_com]);
                 else{
                    _ax=FP_OFF(buf[cur_com])-FP_OFF(buf_start[cur_com]);
                    _ax+=FP_OFF(buf_end[cur_com])-FP_OFF(buf_get[cur_com]);}
                 break;  // in quewe
        case 15: _ax=FP_OFF(sbuf1[cur_com])-FP_OFF(sbuf2[cur_com]);break;  // out quewe
        case 16: _ax=drvget_linestatusreg();break;
        case 17: _ax=drvget_modemstatusreg();break;
        case 100: _ax=1234;break;
        default: _ax=-1;
    }
    kraj:;
}


CommDrvInit()
{
    comdrv_init();
    old_driver=getvect(0x14);
    setvect(0x14,driver);
    return 0;
}

CommDrvEnd()
{
    int f;
    for(f=COMS-1;f>=0;f--) setvect(com_int[f],old_handler[f]);
    setvect(0x14,old_driver);
    return 0;
}

