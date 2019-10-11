/*
 * LedStrip.c
 *
 * Created: 15/06/2018 18:14:15
 * Author : Marcos
 */ 

#include <pdk.h>
#include <stdint.h>

#define SIN_STEPS 64
static const uint8_t SIN_TBL[SIN_STEPS] = {0, 0, 1, 1, 3, 4, 6, 8, 10, 13, 16, 19, 22, 26, 30, 34, 38, 43, 48, 53, 58, 64, 69, 75, 81, 87, 93, 99, 105, 112, 118, 124, 131, 137, 143, 150, 156, 162, 168, 174, 180, 186, 191, 197, 202, 207, 212, 217, 221, 225, 229, 233, 236, 239, 242, 245, 247, 249, 251, 252, 254, 254, 255, 255};

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
	PWMG0DTL = r << 5;
	PWMG0DTH = r >> 3;

	PWMG1DTL = g << 5;
	PWMG1DTH = g >> 3;

	PWMG2DTL = b << 5;
	PWMG2DTH = b >> 3;

	hue++;
	if (hue == SIN_STEPS) {
		hue = 0;
		hueSector++;
		if (hueSector == 6) {
			hueSector = 0;
		}
	}
}

int main() {
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

	// Enable PWM output on PAx pins with clock to IHRC
	PWMG0C = PWMG0C_ENABLE | PWMG0C_RESET_COUNTER | PWMG0C_OUT_PA0 | PWMG0C_CLK_IHRC;
	PWMG1C = PWMG1C_ENABLE | PWMG1C_RESET_COUNTER | PWMG1C_OUT_PA4 | PWMG1C_CLK_IHRC;
	PWMG2C = PWMG2C_ENABLE | PWMG2C_RESET_COUNTER | PWMG2C_OUT_PA3 | PWMG2C_CLK_IHRC;

	// Enable interrupts now
	INTEN = INTEN_TM2;
	__engint();

	// ILRC is about 54KHz. For an update rate of 10Hz, 54000 / 64 / 10 = 84.375
	TM2S = TM2S_PWM_RES_8BIT | TM2S_PRESCALE_DIV64 | TM2S_SCALE_NONE;
	TM2B = 84;
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
