/* DriverLib Includes */
#include "driverlib.h"

/* Project Includes */
#include "BME280/bme280_support.h"
#include "ClockSystem/ClockSystem.h"
#include "UART_Terminal/UART_Terminal.h"
#include "ESP8266/esp8266.h"

/* Function Prototypes */
void Init_System(void);
void LED_Init(void);

void main(void)
{
    uint8_t networkRequest = 0;

    Init_System();

    /* Cycle through looking for network and page requests
     * once found send webpage and repeat */
    while(1)
    {
        /* Look for network request */
        if(!networkRequest)
            FindNetworkRequest(&networkRequest);

        /* Send Web page and clear receive buffer */
        else
        {
            SendWebPage();
            networkRequest = 0;
        }
        MAP_PCM_gotoLPM0();
    }
}

/* Do all required startup initializations */
void Init_System(void)
{
    MAP_WDT_A_holdTimer();          // halting the Watchdog
    LED_Init();                     // drive LEDs low

    Setup_Clocks();                 // setting MCLK and SMCLK

    BME280_Init();                  // setup BME sensor

    Termianl_Init();                // UART setup for terminal
    ESP8266_Init();                 // setup for ESP8266 module and internet access

    MAP_PCM_gotoLPM0();             // got to sleep
}

/* Turn all MSP432 LEDs off */
void LED_Init(void)
{
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
}
