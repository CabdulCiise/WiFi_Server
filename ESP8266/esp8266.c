/* Standard Includes */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* DriverLib Includes */
#include "driverlib.h"

/* Project Includes */
#include "ClockSystem/ClockSystem.h"
#include "BME280/bme280_support.h"
#include "esp8266_uart.h"
#include "esp8266.h"

#define HTML_SIZE 2048

char strHTML[HTML_SIZE];
uint8_t channel = 0;

void ESP8266_Init(void)
{
    // EN pin on ESP8266 pulled high
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);

    // RST pin on ESP8266 pulled low to reset, pull high with 10K
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN1);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN1);

    EUSCIA2_Init();

    ESP8266_Reset();

    // setup server
    ESP8266_SetupAP();
}

void ESP8266_SetupAP(void)
{
    // raise reset pin
    ESP8266_Start();

    // wait for startup sequence
    __delay_cycles(1000000);

    // Enable multiple connections
    ESP8266_SendCommand("AT+CIPMUX=1") ? printf("1\n") : printf("0\n");

    // Starting web server
    ESP8266_SendCommand("AT+CIPSERVER=1,80") ? printf("1\n") : printf("0\n");
}

void FindNetworkRequest(uint8_t* networkRequest)
{
    // look for network request string
    if(strstr(RX_Buffer, "+IPD") != NULL)
    {
        // wait for page request
        while(strstr(RX_Buffer, "HTTP") == NULL);

        // find requesting channel
        char* substr = strstr(RX_Buffer, "+IPD,");
        int channelIndex = substr - (char*)(&RX_Buffer) + 5;
        channel = RX_Buffer[channelIndex] - '0';

        // clear buffer
        RX_Count = 0;
        memset(RX_Buffer, '\0', RX_BUFFER_SIZE);

        *networkRequest = 1;
    }
}

void SendWebPage()
{
    char buffer[50];

    FormatHTML();

    sprintf(buffer, "AT+CIPSEND=%d,%d", channel, strlen(strHTML));
    ESP8266_SendCommand(buffer);
    __delay_cycles(1000);

    ESP8266_SendCommand(strHTML);
    __delay_cycles(100000);

    sprintf(buffer, "AT+CIPCLOSE=%d", channel);
    ESP8266_SendCommand(buffer);
    __delay_cycles(1000);

    // clear buffer
    RX_Count = 0;
    memset(RX_Buffer, '\0', RX_BUFFER_SIZE);
}

void FormatHTML(void)
{
    memset(strHTML, '\0', HTML_SIZE);

    strcat(strHTML, "HTTP/1.1 200 OK\r\n\Connection: close\r\n\r\n\
        <!DOCTYPE html>\
        <html>\
            <head>\
                <title>Environmental Data</title>\
                <style type=\"text/css\">\
                body {\
                    font-family: Arial, Helvetica, sans-serif;\
                    font-size: 2em;\
                    background: linear-gradient(-45deg, rgba(0,0,255,0.85), rgba(0,0,0,0.85));\
                    background-repeat: no-repeat;\
                    background-size: cover;\
                    background-attachment: fixed;\
                    color: white;\
                }\
                div {\
                  height: 500px;\
                  line-height: 500px;\
                  text-align: center;\
                }\
                span {\
                    display: inline-block;\
                    vertical-align: middle;\
                    line-height: normal;\
                }\
                fieldset {\
                    font-weight: bold;\
                    padding-top: 20px;\
                    padding-bottom: 20px;\
                    text-align: left;\
                    border: 0;\
                }\
                </style>\
            </head>\
            <body>\
                <h1>Environmental Data</h1>\
                <form action="" method=\"get\">\
                <div><span>\
                    <fieldset>\
                        Temperature: %0.1f&deg F<br>\
                        Humidity: %0.1f%%<br>\
                        Pressure: %0.1f inHg<br>\
                    </fieldset>\
                <span></div>\
                </form>\
            </body>\
        </html> \r\n\0");

    float temp = BME280_GetTemperature();
    float humidity = BME280_GetHumidity();
    float pressure = BME280_GetPressure();

    sprintf(strHTML, strHTML, temp, humidity, pressure);
}

uint8_t ESP8266_SendCommand(char* command)
{
    uint16_t success = 0, i = 0;

    // send command to ESP8266
    for(i = 0; i < strlen(command); i++)
    {
        MAP_UART_transmitData(EUSCI_A2_BASE, command[i]);
    }
    MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
    MAP_UART_transmitData(EUSCI_A2_BASE, '\n');

    // wait for response by looking for possible responses
    while(strstr(RX_Buffer, "OK") == NULL &&
          strstr(RX_Buffer, "ERROR") == NULL &&
          strstr(RX_Buffer, "FAIL") == NULL);

    // received success response
    if(strstr(RX_Buffer, "OK") != NULL)
    {
        success = 1;
    }

    // clear response buffer
    RX_Count = 0;
    memset(RX_Buffer, '\0', RX_BUFFER_SIZE);

    return success;
}
