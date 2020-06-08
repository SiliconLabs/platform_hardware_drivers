#include "em_device.h"
#include "em_chip.h"
#include "TFA9896.h"
#include "wav_files.h"
#include "udelay.h"
#include "em_gpio.h"


int main(void)
{
  /* Chip errata */
  CHIP_Init();
  TFA9896_init();
  UDELAY_Calibrate();
  GPIO_PinModeSet(gpioPortC, 8, gpioModeInput, 0);
  GPIO_PinModeSet(gpioPortC, 9, gpioModeInput, 0);
  /* Infinite loop */
  while (1) {
	TFA9896_start(WAV_A_Desc);
	while (GPIO_PinInGet(gpioPortC, 8));
	TFA9896_stop();
	while (GPIO_PinInGet(gpioPortC, 9));
	TFA9896_start(WAV_E_Desc);
	while (GPIO_PinInGet(gpioPortC, 8));
	TFA9896_stop();
	while (GPIO_PinInGet(gpioPortC, 9));
	TFA9896_start(WAV_Cs_Desc);
	while (GPIO_PinInGet(gpioPortC, 8));
	TFA9896_stop();
	while (GPIO_PinInGet(gpioPortC, 9));
  }
}
