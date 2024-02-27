#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "screen.h"
#include "terminal.h"

static unsigned char screenTbl[40 * 24];
static int indexX, indexY;
static long long interval_start; /* interval start time in u sec */

int nrow, ncol;

char getch_screen(void)
{
	return (char)terminal_getch();
}

void init_screen(void)
{
	/* Determine main screen window size.
	 * To reserve bottom 1 line for the message buffer, 
	 * -1 from $LINES 
	 */
    nrow = 24;
    ncol = 40;
	
	/* Create 'screen' window */
	//screen = newwin(nrow, ncol, 0, 0);

	/* Set screen window color as Green On Black */
    terminal_init();
}

void updateScreen(void)
{
	int i, j;
	unsigned char c;

    terminal_clear();

	for (j = 0; j < nrow; ++j)
	{
		for (i = 0; i < ncol; ++i)
		{
		  	terminal_setcursor(j, i);
			c = screenTbl[j * ncol + i];
			if (c < '!') 
				c = ' ';
			terminal_printchar(c);
		}
	}

    terminal_setcursor(indexY, indexX); /* put cursor */
    terminal_refresh();
}

void resetScreen(void)
{
	indexX = indexY = 0;
	memset(screenTbl, 0, nrow * ncol);
	updateScreen();
}

static void synchronizeOutput(void)
{
	int processed; /* processed real time in u sec */
	int delay; /* delay u sec to be added to real time */
	struct timeval t;
      
	gettimeofday(&t, NULL);
	processed = (int)(t.tv_usec + t.tv_sec * 1000000 
			  - interval_start);
	if (processed < 0)
		processed = 0;

	/* Video output refreshes screen by 60 Hz. 
	 * In real time, it takes 1 sec / 60 hz. 
	 * So, 1000000 usec / 60 hz. 
	 */  
	delay = 1000000 / 60 - processed;
	if (delay < 0)
		delay = 0;
	usleep((unsigned int)delay); 

	gettimeofday(&t, NULL);
	interval_start = (long long)(t.tv_usec + t.tv_sec * 1000000);
}

static void newLine(void)
{
	int i;

	for (i = 0; i < (nrow-1) ; ++i)
		memcpy(&screenTbl[i * ncol], 
		       &screenTbl[(i + 1) * ncol], 
		       ncol);

	memset(&screenTbl[ncol * (nrow-1)], 0, ncol);
}

void outputDsp(unsigned char dsp)
{
	switch (dsp)
	{
	case 0x5F:
		if (indexX == 0)
		{
			indexY--;
			indexX = ncol-1;
		}
		else
			indexX--;

		screenTbl[indexY * ncol + indexX] = 0;
		break;
	case 0x0A:
	case 0x0D:
		indexX = 0;
		indexY++;
		break;
	case 0x00:
	case 0x7F:
		break;
	default:
		screenTbl[indexY * ncol + indexX] = dsp;
		indexX++;
		break;
	}

	if (indexX == ncol)
	{
		indexX = 0;
		indexY++;
	}
	if (indexY == nrow)
	{
		newLine();
		indexY--;
	}

	updateScreen();

	synchronizeOutput();
}

void select_screen(void)
{
	//touchwin(screen);
	//wrefresh(screen);
}
