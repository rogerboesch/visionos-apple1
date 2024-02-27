#include "pia6820.h"
#include "screen.h"

static unsigned char _dspCr = 0, _dsp = 0, _kbdCr = 0, _kbd = 0;
static int kbdInterrups = 0, dspOutput = 0;

void pia6820_reset(void) {
	kbdInterrups = dspOutput = 0;
	_kbdCr = _dspCr = 0;
}

void pia6820_set_kdb_interrupts(int b) {
	kbdInterrups = b;
}

int pia6820_get_kbd_interrupts(void) {
	return kbdInterrups;
}

int pia6820_get_dsp_output(void) {
	return dspOutput;
}

void pia6820_write_dsp_cr(unsigned char dspCr) {
	if (!dspOutput && dspCr >= 0x80)
		dspOutput = 1;
	else
		_dspCr = dspCr;
}

void pia6820_write_dsp(unsigned char dsp) {
	if (dsp >= 0x80)
		dsp -= 0x80;

	outputDsp(dsp);
	_dsp = dsp;
}

void pia6820_write_kbd_cr(unsigned char kbdCr) {
	if (!kbdInterrups && kbdCr >= 0x80)
		kbdInterrups = 1;
	else
		_kbdCr = kbdCr;
}

void pia6820_write_kbd(unsigned char kbd) {
	_kbd = kbd;
}

unsigned char pia6820_read_dsp_cr(void) {
	return _dspCr;
}

unsigned char pia6820_read_dsp(void) {
	return _dsp;
}

unsigned char pia6820_read_kbd_cr(void) {
	if (kbdInterrups && _kbdCr >= 0x80) {
		_kbdCr = 0;

		return 0xA7;
	}

	return _kbdCr;
}

unsigned char pia6820_read_kbd(void) {
	return _kbd;
}
