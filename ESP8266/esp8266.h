#ifndef ESP8266_ESP8266_H_
#define ESP8266_ESP8266_H_

void ESP8266_Init(void);
uint8_t ESP8266_SendCommand(char* command);

void ESP8266_SetupAP(void);
void FindNetworkRequest(uint8_t* networkRequest);
void SendWebPage();
void FormatHTML(void);

#endif /* ESP8266_ESP8266_H_ */
