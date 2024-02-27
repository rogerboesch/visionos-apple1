/*
 * Memory map
 *
 * Apple I 8K mode & emulator 64K mode, common usage
 * -----------------------------------------------------------------------
 * $0000            Ststem & User Space
 * $0200 - $027F    Input Buffer used by monitor
 * $0FFF
 * -----------------------------------------------------------------------
 * $D010	        KBD	Keyboard input register. b7 is always 1 by hardware.
 * 		            Read KBD will automatcically clear KBDCR's b7.
 * $D011	KBDCR	When key is pressed, b7 is set by hardware.
 * $D012	DSP	    Bit b6..b0 is output character for the terminal.
 *	                Writing to DSP will set b7 by hardware.
 *                  The termianl clear b7 after the character is accepted.
 * $D013	DSPCR	Unused.
 * -----------------------------------------------------------------------
 * $E000            Apple I Integer BASIC
 * $E2B3            Re-entry address
 * $EFFF
 * -----------------------------------------------------------------------
 * $FF00            Monitor
 * $FFEF	        Echo
 * $FFFF
 * ----------------------------------------------------------------------- 
 */

/* Apple I 8K mode memory map
 * --------------------------------- 
 * Start Type
 * addr
 * --------------------------------- 
 * $0000 4KB RAM
 * $1000 unused
 * $D010 Display and Keyboard I/O
 * $D014 unused
 * $E000 4KB RAM
 * $F000 unused
 * $FF00 256B ROM^ (Woz Monitor)
 * ---------------------------------
 * ^ ROM can be written by Load core  
 */

/* Emulator 32K mode memory map
 * ---------------------------------
 * Start Type
 * addr
 * --------------------------------- 
 * $0000 32K RAM
 * $8000 unused
 * $D010 Display and Keyboard I/O
 * $D014 unused
 * $E000 8K ROM^ 
 * ---------------------------------
 * ^ ROM can be written by Load core  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pia6820.h"
#include "memory.h"
#include "screen.h"
#include "statusbar.h"

#define MEMMAX 0xFFFF
#define FNAME_LEN_MAX 1024

static unsigned char mem[65536];
static int mode = 8; /* 8 = Apple I 8K mode, 32 = napple1 32K mode */

char* platform_file_path(char *name, char *extension);

void flipMode(void)
{
	if (mode == 8)
		mode = 32;
	else 
		mode = 8;

	/* update message buffer */
	statusbar_print("");
}

int memMode(void) 
{
	return mode;
}

void loadBasic(void) {
	FILE *fd = fopen(platform_file_path("basic", "rom"), "rb");
	char input[MSG_LEN_MAX +1];
	
	if (!fd) {
		statusbar_input("Failed to open 'basic.rom' - press key to continue", input);
		return;
	}

    size_t s = fread(&mem[0xE000], 1, 4096, fd);

    if (s) {
        statusbar_input("BASIC loaded - press key to continue", input);
    }
    else {
        statusbar_input("Load BASIC failed - press key to continue", input);
	}

	fclose(fd);
	return;
}

int loadMonitor(void)
{
    FILE *fd = fopen(platform_file_path("monitor", "rom"), "rb");

	if (fd) {
		fread(&mem[0xFF00], 1, 256, fd);
		fclose(fd);
	}
	else{
		return 0;
	}

	return 1;
}

void resetMemory(void)
{
	if (memMode() > 8)
		memset(mem, 0, 0xE000); /* rom is starting from 0xE000 */
	else
		memset(mem, 0, 0x10000 - 256); /* rom is within tail 256b */
}

unsigned char memRead(unsigned short address)
{
	if (address == 0xD013)
		return readDspCr();
	if (address == 0xD012)
		return readDsp();
	if (address == 0xD011)
		return readKbdCr();
	if (address == 0xD010)
		return readKbd();

	return mem[address];
}

void memWrite(unsigned short address, unsigned char value)
{
	if (address < 0x1000)	
		mem[address] = value;
	else if (address < 0x8000 && (memMode() > 8) )
		mem[address] = value;
	else if (address == 0xD013)
		writeDspCr(value);
	else if (address == 0xD012)
		writeDsp(value);
	else if (address == 0xD011)
		writeKbdCr(value);
	else if (address == 0xD010)
		writeKbd(value);
	else if (address >= 0xE000 && address < 0xF000 && memMode() == 8)
		mem[address] = value;
	else
		;

	return;
}

void dumpCore(void)
{
	int i;
	FILE *fd;
	char corename[5 + MSG_LEN_MAX +1]; /* 'core/' + input string */

	sprintf(corename, "%s", "dump");

	fd = fopen(corename, "w");
	for (i = 0; i <= MEMMAX; i++)
		fputc(mem[i], fd);
	fclose(fd);
    
	printf("Core dumped: %s", corename);
}

int loadCore(void) {
	size_t s = 0;
	unsigned char buf[65536];
	int i;

    FILE *fd = fopen(platform_file_path("example", "core"), "rb");
	if (fd) {
		s = fread(&buf[0], 1, MEMMAX+1, fd);
		fclose(fd);
	}
    
	if (!s) {
		printf("ERROR: Failed to open core file");
		return 0;
	}

	/* 0xF000 is unused area of 8K mode or
	 * ROM area of 32K mode. So,  if 0xF000 has a value,
	 * The mode should better change to 32K mode.
	 */
	if ((buf[0xF000] != 0) && (memMode() == 8)) {
		flipMode();
	}

	if (memMode() == 8) {
		for (i = 0;      i <= 0x0FFF; i++) mem[i] = buf[i];
		for (i = 0xE000; i <= 0xEFFF; i++) mem[i] = buf[i];
		for (i = 0xFF00; i <= 0xFFFF; i++) mem[i] = buf[i];
	} else {
		for (i = 0;      i <= 0x7FFF; i++) mem[i] = buf[i];
		for (i = 0xE000; i <= 0xFFFF; i++) mem[i] = buf[i];
	}
	return 1;
}
