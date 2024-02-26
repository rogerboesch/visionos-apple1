
#include "ret_palette.h"

#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
#pragma mark - Atari color palette

ret_color ret_palette_atari[384];

void ret_palette_create_atari() {
	double brightness = 1.0;
	double warmth = 0.0;
	double PAL_PS = 21.5;
	double PAL_luma[]  = {0.08,0.15,0.26,0.40,0.52,0.65,0.76,0.84};

	byte palette_atari[384];
	int chroma, luma, i;

	double color[16][2] = {{0.0}};
	double pi = 4*atan(1);

	double A = 0.225;	// .225
	double delta = PAL_PS * (2*pi/360);

	for (i=2; i<14; i++) {
		color[i][0] = A * sin(delta*i - PAL_PS);
		color[i][1] = A * -sin(delta*i/2 - PAL_PS);

		if (i & 1) {
			color[i][1] = -color[i][1];
		}
	}

	for (chroma = 0; chroma < 16; chroma++) {
		double U = color[chroma][0];
		double V = color[chroma][1];
		double Y, R, G, B = 0.0;

		for (luma=0; luma<=7; luma++) {
			Y = PAL_luma[luma];

			Y = pow(Y, 2-brightness);

			R = Y + 1.403 * V;
			G = Y - 0.344 * U - 0.714 * V;
			B = Y + 1.770 * U;

			R *= 1.0 + warmth*0.2;
			G *= 1.0 + warmth*0.1;
			B *= 1.0 - warmth*0.1;

			if (R < 0) R = 0.0;
			if (G < 0) G = 0.0;
			if (B < 0) B = 0.0;

			if (R > 1) R = 1;
			if (G > 1) G = 1;
			if (B > 1) B = 1;

			palette_atari[chroma*3*8 + luma*3+0] = (byte)(R*255.0);
			palette_atari[chroma*3*8 + luma*3+1] = (byte)(G*255.0);
			palette_atari[chroma*3*8 + luma*3+2] = (byte)(B*255.0);
		}
	}
	
	palette_atari[0] = 0; palette_atari[1] = 0; palette_atari[2] = 0;
	
	// Now copy over to real palette
	memcpy(&ret_palette_atari[0], &palette_atari[0], 384);
}

// -----------------------------------------------------------------------------
#pragma mark - C64 color palette

ret_color ret_palette[] = {
	0x00, 0x00, 0x00, // Schwarz	0	$00	CTRL + 1 	144	Rechteck links unten	#000000
	0xff, 0xff, 0xff, // Weiß		1	$01	CTRL + 2 	5	inverses E				#ffffff
	0x88, 0x00, 0x00, // Rot		2	$02	CTRL + 3 	28	inverses Pfund			#880000
	0xaa, 0xff, 0xee, // Türkis		3	$03	CTRL + 4 	159	Dreieck unten links		#aaffee
	0xcc, 0x44, 0xcc, // Violett	4	$04	CTRL + 5 	156	inverses Halbschach		#cc44cc
	0x00, 0xcc, 0x55, // Grün		5	$05	CTRL + 6 	30	inverser Pfeil hoch		#00cc55
	0x00, 0x00, 0xaa, // Blau		6	$06	CTRL + 7 	31	inverser Linkspfeil		#0000aa
	0xee, 0xee, 0x77, // Gelb		7	$07	CTRL + 8 	158	inverses π				#eeee77
	0xdd, 0x88, 0x55, // Orange		8	$08	C= + 1 		129	inverses Pik			#dd8855
	0x66, 0x44, 0x00, // Braun		9	$09	C= + 2 		149	inv. 1/4-Kreis o.l.		#664400
	0xff, 0x77, 0x77, // Hellrot	10	$0a	C= + 3 		150	inverses Grafik-X		#ff7777
	0x33, 0x33, 0x33, // Grau 1		11	$0b	C= + 4 		151	inverser Kreisring		#333333
	0x77, 0x77, 0x77, // Grau 2		12	$0c	C= + 5 		152	inverses Kreuz			#777777
	0x00, 0xff, 0x00, // Hellgrün	13	$0d	C= + 6 		153	inv. Strich rechts		#aaff66
	0x00, 0x88, 0xff, // Hellblau	14	$0e	C= + 7 		154	inverses Karo			#0088ff
	0xbb, 0xbb, 0xbb  // Grau 3		15	$0f	C= + 8 		155	inverses Kreuz			#bbbbbb
};

// -----------------------------------------------------------------------------
#pragma mark - Palette access

float ret_brightness_table[] = {
	0.01f,
	0.1f,
	0.25f,
	0.4f,
	0.55f,
	0.7f,
	0.85f,
	1.0f,
	1.15f,
	1.3f,
	1.45f,
	1.6f,
	1.75f,
	1.9f,
	2.0f,
	2.1f
};

ret_color RETPaletteGetColor(byte index) {
	if (index >= RET_PALETTE_SIZE) {
		return RETPaletteGetColor(2);
	}

	ret_color color = ret_palette[index];
	return color;
}

ret_color RETPaletteGetColorWithBrightness(byte index, byte bindex) {
	if (index >= RET_PALETTE_SIZE) {
		return RETPaletteGetColor(2);
	}

	float brightness = ret_brightness_table[bindex];
	
	ret_color color = ret_palette[index];
	color.r = fminf(255, color.r*brightness);
	color.g = fminf(255, color.g*brightness);
	color.b = fminf(255, color.b*brightness);
	
	return color;

}
