#include <Wire.h>
#include <SPI.h>
#include "Adafruit_PN532.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)
#define PN532_IRQ   (2)
#define PN532_RESET (3)  
#ifndef ROTL
#define ROTL(x,n) (((uintmax_t)(x) << (n)) | ((uintmax_t)(x) >> ((sizeof(x) * 8) - (n))))
#endif
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
/// shit sets
uint32_t cb[] = {
  0x6D835AFC, 0x7D15CD97, 0x0942B409, 0x32F9C923, 0xA811FB02, 0x64F121E8,
  0xD1CC8B4E, 0xE8873E6F, 0x61399BBB, 0xF1B91926, 0xAC661520, 0xA21A31C9,
  0xD424808D, 0xFE118E07, 0xD18E728D, 0xABAC9E17, 0x18066433, 0x00E18E79,
  0x65A77305, 0x5AE9E297, 0x11FC628C, 0x7BB3431F, 0x942A8308, 0xB2F8FD20,
  0x5728B869, 0x30726D5A
};

void transform(uint8_t* ru)
{
  //Transform
  uint8_t i;
  uint8_t p = 0;
  uint32_t v1 = (((uint32_t)ru[3] << 24) | ((uint32_t)ru[2] << 16) | ((uint32_t)ru[1] << 8) | (uint32_t)ru[0]) + cb[p++];
  uint32_t v2 = (((uint32_t)ru[7] << 24) | ((uint32_t)ru[6] << 16) | ((uint32_t)ru[5] << 8) | (uint32_t)ru[4]) + cb[p++];

  for (i = 0; i < 12; i += 2)
  {
    uint32_t t1 = ROTL(v1 ^ v2, v2 & 0x1F) + cb[p++];
    uint32_t t2 = ROTL(v2 ^ t1, t1 & 0x1F) + cb[p++];
    v1 = ROTL(t1 ^ t2, t2 & 0x1F) + cb[p++];
    v2 = ROTL(t2 ^ v1, v1 & 0x1F) + cb[p++];
  }

  //Re-use ru
  ru[0] = v1 & 0xFF;
  ru[1] = (v1 >> 8) & 0xFF;
  ru[2] = (v1 >> 16) & 0xFF;
  ru[3] = (v1 >> 24) & 0xFF;
  ru[4] = v2 & 0xFF;
  ru[5] = (v2 >> 8) & 0xFF;
  ru[6] = (v2 >> 16) & 0xFF;
  ru[7] = (v2 >> 24) & 0xFF;
}


uint32_t getkey(uint8_t* uid)
{
  int i;
  //Rotate
  uint8_t r = (uid[1] + uid[3] + uid[5]) & 7; //Rotation offset
  uint8_t ru[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Rotated UID
  for (i = 0; i < 7; i++)
    ru[(i + r) & 7] = uid[i];

  //Transform
  transform(ru);

  //Calc key
  uint32_t k = 0; //Key as int
  r = (ru[0] + ru[2] + ru[4] + ru[6]) & 3; //Offset
  for (i = 3; i >= 0; i--) 
    k = ru[i + r] + (k << 8);

  return k;
};
void setup(void) {
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);   
  Serial.begin(115200);
  Serial.println("Hello!");
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); 
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;                        
  uint8_t buf[16];
  digitalWrite(12, HIGH);
  delay(250);
  digitalWrite(12, LOW);
  digitalWrite(9, HIGH);
  delay(750);
  digitalWrite(9, LOW);
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    if (uidLength == 7)
    {
      uint8_t data[32];
      Serial.println("Smells right bitch");    
      uint32_t k = getkey(uid); //10 digit value
      uint8_t* ptr_2_k = (void*)&k;
      int khex = (k, HEX);
      Serial.print("KEY:"); Serial.println(k);
      Serial.print("KEH:"); Serial.println(khex);
      unsigned long thenumber = k;
      byte pwd[] = {0x00, 0x00, 0x00, 0x00};
      pwd[3]=thenumber&255;
      thenumber=thenumber>>8;
      pwd[2]=thenumber&255;
      thenumber=thenumber>>8;
      pwd[1]=thenumber&255;
      thenumber=thenumber>>8;
      pwd[0]=thenumber; 
       nfc.ntag2xx_Authenticate(pwd);
      for (uint8_t i = 0; i < 42; i++) 
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        Serial.print("PAGE ");
        if (i < 10)
        {
          Serial.print("0");
          Serial.print(i);
        }
        else
        {
          Serial.print(i);
        }
        Serial.print(": ");
        // Display the results, depending on 'success'
        if (success) 
        {
          digitalWrite(12, HIGH);
          // Dump the page data
          nfc.PrintHexChar(data, 4);
          digitalWrite(12, LOW);
        }
        else
        {
          Serial.println("Mom didnt tech me how to read!");
        }
      }
    }
  }
}
