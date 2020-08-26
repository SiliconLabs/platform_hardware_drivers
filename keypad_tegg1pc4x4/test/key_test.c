#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_timer.h"
#include "retargetserial.h"
#include "key_scan.h"

volatile uint16_t timer_5ms = 0;

void TIMER2_IRQHandler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER2);
  TIMER_IntClear(TIMER2, flags);
  timer_5ms++;
}

void TIMER2_init(void)
{
  uint32_t timerFreq = 0;

  CMU_ClockEnable(cmuClock_TIMER2, true);

  // Initialize the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = timerPrescale2;
  timerInit.enable = false;

  // Configure but do not start the timer
  TIMER_Init(TIMER2, &timerInit);

  timerFreq = CMU_ClockFreqGet(cmuClock_TIMER2) / (timerInit.prescale + 1);
  TIMER_TopSet(TIMER2, (timerFreq / 200));//1000 for 1ms, 200 for 5ms

  // Start the timer
  //TIMER_Enable(TIMER2, true);

  // Enable TIMER2 compare event interrupts to update the duty cycle
  TIMER_IntEnable(TIMER2, TIMER_IF_OF);
  NVIC_EnableIRQ(TIMER2_IRQn);
}


uint8_t key_test = 0xFF;
uint8_t key_repeat = 0xFF;
/**
 * @brief key press wakeup callback
 *
 * @param interrupt pin
 *
 * @return none
 *
 */
void app_key_wakeup(uint8_t pin)
{
  //Test code, key jitter, avoid multiple set the timer.
  if(key_test == pin)
	  return;
  key_test = pin;

  //printf("key wakeup %d\r\n", pin);
  //Start the key timer
  TIMER_Enable(TIMER2, true);
}

/**
 * @brief key detect callback
 *
 * @param key value, KEY_NONE means key release.
 *
 * @return none
 *
 */
void app_key_detect(key_code_t key)
{
  if(key == KEY_NONE){
	printf("key release\r\n");
	key_test =0xFF;
	//Key have release, stop the key timer
	TIMER_Enable(TIMER2, false);
  }else{
	key_repeat = key;
	printf("key detect %d\r\n", key);
  }
}

int main(void)
{
  /* Chip errata */
  CHIP_Init();

  TIMER2_init();

  RETARGET_SerialInit();
  GPIO_PinModeSet(gpioPortB, 4, gpioModePushPull, 1);
  printf("test key\r\n");

  key_init(app_key_detect, app_key_wakeup);

  /* Infinite loop */
  while (1) {
	 if (timer_5ms >= 2){
		 timer_5ms -= 2; //make a 10ms time slice.
		 //GPIO_PinOutToggle(gpioPortD,3);
		 //printf("key scan\r\n");
		 key_scan();
	 }
  }
}
