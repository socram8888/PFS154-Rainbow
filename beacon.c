/*
 * LedStrip.c
 *
 * Created: 15/06/2018 18:14:15
 * Author : Marcos
 */ 

#include <pdk.h>
#include <stdint.h>

// [round((sin(x / 256 * pi / 2) ** (1/0.5)) * 255) for x in range(256)]
#define SIN_STEPS 256
static const uint8_t SIN_TBL[SIN_STEPS] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 52, 53, 54, 55, 57, 58, 59, 61, 62, 63, 65, 66, 67, 69, 70, 72, 73, 74, 76, 77, 79, 80, 82, 83, 85, 86, 88, 89, 90, 92, 93, 95, 97, 98, 100, 101, 103, 104, 106, 107, 109, 110, 112, 113, 115, 117, 118, 120, 121, 123, 124, 126, 127, 129, 131, 132, 134, 135, 137, 138, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166, 167, 169, 170, 172, 173, 175, 176, 178, 179, 181, 182, 183, 185, 186, 188, 189, 190, 192, 193, 194, 196, 197, 198, 200, 201, 202, 203, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 234, 235, 236, 237, 238, 238, 239, 240, 241, 241, 242, 243, 243, 244, 245, 245, 246, 246, 247, 248, 248, 249, 249, 250, 250, 250, 251, 251, 252, 252, 252, 253, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255};

uint8_t hue = 0;
uint8_t hueSector = 0;

void next_color() {
	uint8_t r, g, b;

	switch (hueSector) {
		case 0:
			r = 255;
			g = SIN_TBL[hue];
			b = 0;
			break;

		case 1:
			r = SIN_TBL[SIN_STEPS - hue - 1];
			g = 255;
			b = 0;
			break;

		case 2:
			r = 0;
			g = 255;
			b = SIN_TBL[hue];
			break;

		case 3:
			r = 0;
			g = SIN_TBL[SIN_STEPS - hue - 1];
			b = 255;
			break;

		case 4:
			r = SIN_TBL[hue];
			g = 0;
			b = 255;
			break;

		default:
			r = 255;
			g = 0;
			b = SIN_TBL[SIN_STEPS - hue - 1];
	}

	// Update duty cycles
	if (r != 0) {
		PWMG0DTL = r << 5;
		PWMG0DTH = r >> 3;

		if (!(PWMG0C & PWMG0C_ENABLE)) {
			PWMG0C = PWMG0C_ENABLE | PWMG0C_RESET_COUNTER | PWMG0C_OUT_PA0 | PWMG0C_CLK_IHRC;
		}
	} else {
		PWMG0C = 0;
	}

	if (g != 0) {
		PWMG1DTL = g << 5;
		PWMG1DTH = g >> 3;

		if (!(PWMG1C & PWMG1C_ENABLE)) {
			PWMG1C = PWMG1C_ENABLE | PWMG1C_RESET_COUNTER | PWMG1C_OUT_PA4 | PWMG1C_CLK_IHRC;
		}
	} else {
		PWMG1C = 0;
	}

	if (b != 0) {
		PWMG2DTL = b << 5;
		PWMG2DTH = b >> 3;

		if (!(PWMG2C & PWMG2C_ENABLE)) {
			PWMG2C = PWMG2C_ENABLE | PWMG2C_RESET_COUNTER | PWMG2C_OUT_PA3 | PWMG2C_CLK_IHRC;
		}
	} else {
		PWMG2C = 0;
	}

	hue++;
	if (hue == 0) {
		hueSector++;
		if (hueSector == 6) {
			hueSector = 0;
		}
	}
}

int main() {
	// Set clock and disable watchdog
	CLKMD = CLKMD_ENABLE_ILRC | CLKMD_ENABLE_IHRC | CLKMD_ILRC;

	// Set to output PA0, PA4 and PA3 for PWMG units
	PAC = (1 << 0) | (1 << 4) | (1 << 3);

	// Initialize with first color
	next_color();

	// Prescaler to 16, scaler to 31 to get around 120Hz refresh rate
	PWMG0S = PWMG0_PRESCALE_DIV16 | PWMG0_SCALE_DIV32;
	PWMG1S = PWMG1_PRESCALE_DIV16 | PWMG1_SCALE_DIV32;
	PWMG2S = PWMG2_PRESCALE_DIV16 | PWMG2_SCALE_DIV32;

	// Set upper bound to 255
	PWMG0CUBL = (uint8_t) (255 << 5);
	PWMG0CUBH = 255 >> 3;

	PWMG1CUBL = (uint8_t) (255 << 5);
	PWMG1CUBH = 255 >> 3;

	PWMG2CUBL = (uint8_t) (255 << 5);
	PWMG2CUBH = 255 >> 3;

	// Enable interrupts now
	INTEN = INTEN_TM2;
	__engint();

	// ILRC is about 54KHz. For an update rate of 40Hz, 54000 / 64 / 10 = 21.09
	TM2S = TM2S_PWM_RES_8BIT | TM2S_PRESCALE_DIV64 | TM2S_SCALE_NONE;
	TM2B = 21;
	TM2C = TM2C_CLK_ILRC | TM2C_OUT_DISABLE | TM2C_MODE_PERIOD;

	while (1) {
		__stopexe();
	}
}

void handler(void) __interrupt(0) {
	if (INTRQ & INTRQ_TM2) {
		INTRQ &= ~INTRQ_TM2;
		next_color();
	}
}
