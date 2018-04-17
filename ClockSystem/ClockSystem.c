/* Driver Library */
#include "driverlib.h"

/* Project Includes */
#include "ClockSystem.h"

/* Setup clock sources: MCLK => 48 MHz, SMCLK => 12 MHz*/
void Setup_Clocks(void)
{
    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting the external clock frequency */
    MAP_CS_setExternalClockSourceFrequency(32000, 48000000);

    /* Starting HFXT in non-bypass mode without a timeout.
     * Change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    MAP_CS_startHFXT(false);

    /* Initializing MCLK (48MHz) to HFXT */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Initializing SMCLK (12MHz) to HFXT */
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);

    uint32_t MCLK = MAP_CS_getMCLK();       // used for Systic Timer
    uint32_t SMCLK = MAP_CS_getSMCLK();     // used for BME280 I2C communication
}
