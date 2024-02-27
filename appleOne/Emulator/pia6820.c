#include "pia6820.h"
#include "screen.h"

static unsigned char _dspCr = 0, _dsp = 0, _kbdCr = 0, _kbd = 0;
static int kbdInterrups = 0, dspOutput = 0;

void resetPia6820(void)
{
	kbdInterrups = dspOutput = 0;
	_kbdCr = _dspCr = 0;
}

void setKdbInterrups(int b)
{
	kbdInterrups = b;
}

int getKbdInterrups(void)
{
	return kbdInterrups;
}

int getDspOutput(void)
{
	return dspOutput;
}

void writeDspCr(unsigned char dspCr)
{
	if (!dspOutput && dspCr >= 0x80)
		dspOutput = 1;
	else
		_dspCr = dspCr;
}

void writeDsp(unsigned char dsp)
{
	if (dsp >= 0x80)
		dsp -= 0x80;

	outputDsp(dsp);
	_dsp = dsp;
}

void writeKbdCr(unsigned char kbdCr)
{
	if (!kbdInterrups && kbdCr >= 0x80)
		kbdInterrups = 1;
	else
		_kbdCr = kbdCr;
}

void writeKbd(unsigned char kbd)
{
	_kbd = kbd;
}

unsigned char readDspCr(void)
{
	return _dspCr;
}

unsigned char readDsp(void)
{
	return _dsp;
}

unsigned char readKbdCr(void)
{
	if (kbdInterrups && _kbdCr >= 0x80)
	{
		_kbdCr = 0;

		return 0xA7;
	}

	return _kbdCr;
}

unsigned char readKbd(void)
{
	return _kbd;
}
