#define FNAME_LEN_MAX   1024

void memory_load_basic(void);
int memory_load_wozmon(void);
void memory_reset(void);
unsigned char memory_read(unsigned short address);
void memory_write(unsigned short address, unsigned char value);
void memory_dump_core(void);
int memory_load_example_core(void);
int memory_get_mode(void);
void memory_flip_mode(void);
