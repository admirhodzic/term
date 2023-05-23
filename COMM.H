extern "C"  CommDrvInit();
extern "C"  CommDrvEnd();

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
