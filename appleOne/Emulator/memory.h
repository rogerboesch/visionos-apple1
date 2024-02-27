#define FNAME_LEN_MAX   1024

void loadBasic(void);
int loadMonitor(void);
void resetMemory(void);
unsigned char memRead(unsigned short address);
void memWrite(unsigned short address, unsigned char value);
void dumpCore(void);
int loadCore(void);
int memMode(void);
void flipMode(void);
