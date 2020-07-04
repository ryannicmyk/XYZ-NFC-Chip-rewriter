#include <Wire.h>
#include <LiquidCrystal.h>
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
LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);
char vernum[5] = "V2.1";
char datecur[6] = "8/02";
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

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

String menuItems[] = {"FLASH 200M", "FLASH 340M", "NEW 340M", "READ TAG", "INFO", "VERSION"};
int readKey;
int savedDistance = 0;
int menuPage = 0;
int maxMenuPages = 2;
int cursorPosition = 0;
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B00000, //  *
  B00100, //   *
  B00010, //    *
  B00101, //     *
  B00010, //    *
  B00100, //   *
  B00000, //  *
  B00000  //
};

#include <Wire.h>
#include <LiquidCrystal.h>

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Booting");
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
  Serial.begin(115200);
  lcd.clear();
  lcd.print("Booting.");
  Serial.println("Hello!");
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    lcd.clear() ;
    lcd.print("BRD MISSNG");
    while (1);
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  lcd.clear() ;
  lcd.print("FND BRD");
  delay(200);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  lcd.clear() ;
  lcd.print("CLEAR");
  delay(200);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card ...");
  lcd.clear() ;
  lcd.print("WAITING");
}



void loop() {
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
}


void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }


  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}

void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = digitalRead(6);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: 
        break;
      case 1:  
        button = 0;
        switch (cursorPosition) { 
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

int evaluateButton(int x) {
  int result = 0;
  if (analogRead(0) < 50) {
    result = 1; // right
  } else if (analogRead(1) < 480) {
    result = 2; // up
  } else if (analogRead(1) > 492) {
    result = 3; // down
  } else if (analogRead(0) < 790) {
    result = 4; // left
  }
  return result;
}

void drawInstructions() {
  lcd.setCursor(0, 1); 
  lcd.print("Use ");
  lcd.write(byte(1)); 
  lcd.print("/");
  lcd.write(byte(2)); 
  lcd.print(" buttons");
}

void menuItem1() { 
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t buf[16];
  uint8_t data[32];
  int activeButton = 0;
  lcd.clear();
  lcd.print("     READY!");
  lcd.setCursor(0, 2);
  lcd.print("BRING NEAR SPOOL");
  lcd.setCursor(0, 1);
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    if (uidLength == 7)
    {
      Serial.println("Smells right bitch");
      lcd.clear() ;
      lcd.print("CONNNECTED");
      uint32_t k = getkey(uid); //10 digit value
      int khex = (k, HEX);
      Serial.print("KEY:"); Serial.println(k, HEX);
      Serial.print("KEH:"); Serial.println(khex);
      unsigned long thenumber = k;
      byte pwd[] = {0x00, 0x00, 0x00, 0x00};
      pwd[3] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[2] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[1] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[0] = thenumber;
      lcd.clear();
      lcd.print("3");
      delay(1000);
      lcd.clear();
      lcd.print("2");
      delay(1000);
      lcd.clear();
      lcd.print("1");
      delay(1000);
      nfc.ntag2xx_Authenticate(pwd);
      for (uint8_t i = 0; i < 42; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        delay(50);
        lcd.clear();
        lcd.print("READING ");
        lcd.write(byte(1));
        Serial.print("PAGE ");
        if (i < 10)
        {
          Serial.print("0");
          Serial.print(i, HEX);
        }
        else
        {
          Serial.print(i, HEX);
        }
        Serial.print(": ");
        // Display the results, depending on 'success'
        if (success)
        {
          digitalWrite(12, HIGH);
          // Dump the page data
          nfc.PrintHexChar(data, 4);
          delay(50);
          lcd.clear() ;
          lcd.print("READING ");
          lcd.write(byte(2));
        }
        else
        {
          Serial.println("Mom didnt tech me how to read!");
          lcd.clear() ;
          lcd.print("ERROR");
        }
      }
      delay(2500);
      lcd.clear() ;
      lcd.print("REPROGRAMMING");
      Serial.print("Writing");
      byte p14[] = {0x40, 0x0D, 0x03, 0x00};
      byte p15[] = {0x08, 0x1F, 0x31, 0x54};
      byte p16[] = {0x50, 0xB1, 0xE0, 0xCE};
      byte p17[] = {0x52, 0xE7, 0x4F, 0x76};
      nfc.ntag2xx_WritePage(20, p14);
      lcd.clear();
      lcd.setCursor(0,0);
      Serial.print("Reading");
      lcd.print("READING");
      nfc.ntag2xx_ReadPage(14, buf);
      lcd.clear();
      lcd.print("READ DONE");
      nfc.PrintHexChar(buf, 4);
      lcd.clear();
      lcd.print("WRITING");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("40 0D 03 00");
      lcd.setCursor(0,1);
      lcd.print("40 0D 03 00");
      delay(4000);
      Serial.print("Done!");
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("DONE!");
    }
    else
    {
      Serial.println("WRONT TAG");
    }

  }
}

void menuItem2() { 
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 2");
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t buf[16];
  uint8_t data[32];
  int activeButton = 0;
  lcd.clear();
  lcd.print("     READY!");
  lcd.setCursor(0, 2);
  lcd.print("BRING NEAR TAG");
  lcd.setCursor(0, 1);
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    if (uidLength == 7)
    {
      uint8_t cfg_page_base = 0x29;
      Serial.println("Smells right bitch");
      lcd.clear() ;
      lcd.print("CONNNECTED");
      uint32_t k = getkey(uid); //10 digit value
      int khex = (k, HEX);
      Serial.print("KEY:"); Serial.println(k, HEX);
      Serial.print("KEH:"); Serial.println(khex);
      unsigned long thenumber = k;
      byte pwd[] = {0x00, 0x00, 0x00, 0x00};
      pwd[3] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[2] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[1] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[0] = thenumber;
      lcd.clear();
      lcd.print("3");
      delay(1000);
      lcd.clear();
      lcd.print("2");
      delay(1000);
      lcd.clear();
      lcd.print("1");
      lcd.print("REPROGRAMMING");
      nfc.ntag2xx_Authenticate(pwd);
      /// a
      /// a 
      /// 20300500
      byte p14[] = {0x20, 0x30, 0x05, 0x00};
      byte p15[] = {0x08, 0x1F, 0x31, 0x54};
      byte p16[] = {0x50, 0xB1, 0xE0, 0xCE};
      byte p17[] = {0x52, 0xE7, 0x4F, 0x76};
      nfc.ntag2xx_WritePage(20, p14);
      //FIANL TAG LOCK
    }
    else
    {
      Serial.println("WRONT TAG");
    }

  }
}
void menuItem3() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 2");
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t buf[16];
  uint8_t data[32];
  int activeButton = 0;
  lcd.clear();
  lcd.print("     READY!");
  lcd.setCursor(0, 2);
  lcd.print("BRING NEAR NEW");
  lcd.setCursor(0, 1);
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    if (uidLength == 7)
    {
      Serial.println("Smells right bitch");
      lcd.clear() ;
      lcd.print("CONNNECTED");
      uint32_t k = getkey(uid); //10 digit value
      int khex = (k, HEX);
      Serial.print("KEY:"); Serial.println(k, HEX);
      Serial.print("KEH:"); Serial.println(khex);
      unsigned long thenumber = k;
      uint8_t password[] = {0x00, 0x00, 0x00, 0x00};
      byte pwd[] = {0x00, 0x00, 0x00, 0x00};
      pwd[3] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[2] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[1] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[0] = thenumber;
      password[3] = thenumber & 255;
      thenumber = thenumber >> 8;
      password[2] = thenumber & 255;
      thenumber = thenumber >> 8;
      password[1] = thenumber & 255;
      thenumber = thenumber >> 8;
      password[0] = thenumber;
      lcd.clear();
      lcd.print("3");
      delay(1000);
      lcd.clear();
      lcd.print("2");
      delay(1000);
      lcd.clear();
      lcd.print("1");
      lcd.print("REPROGRAMMING");
      /// a
      /// a 
      /// 20300500
      uint8_t buf33[4];
      nfc.mifareultralight_ReadPage(3, buf33);
      int capacity = buf33[2] * 8;
      Serial.print(F("Tag capacity "));
      Serial.print(capacity);
      Serial.println(F(" bytes"));

    uint8_t cfg_page_base = 0x29;   // NTAG213
    if (capacity == 0x3E) {
        cfg_page_base = 0x83;       // NTAG215
    } else if (capacity == 0x6D) {
        cfg_page_base = 0xE3;       // NTAG216
    }

    // PWD page, set new password
    nfc.mifareultralight_WritePage(cfg_page_base + 2, password);
    buf33[0] = (1 << 7) | 0x0;
    nfc.mifareultralight_WritePage(cfg_page_base + 1, buf33);
    uint8_t auth0 = 0x07;
    buf33[0] = 0;
    buf33[1] = 0;
    buf33[2] = 0;
    buf33[3] = auth0;
    nfc.mifareultralight_WritePage(cfg_page_base, buf33);
    nfc.ntag2xx_Authenticate(pwd);

      byte p8[] = {0x5A, 0x51, 0x42, 0xFE};
       byte p9[] = {0xE2, 0x38, 0x34, 0x36};
      byte p10[] = {0x20, 0x30, 0x05, 0x00};
      byte p11[] = {0x8B, 0xEF, 0x02, 0x00};
      byte p12[] = {0xC4, 0x30, 0x16, 0x3D};
      byte p13[] = {0x58, 0x48, 0x50, 0x4D};
      byte p14[] = {0x30, 0x30, 0x35, 0x30};
      byte p17[] = {0x80, 0x00, 0x00, 0x00};
      byte p20[] = {0x20, 0x30, 0x05, 0x00};
      byte p21[] = {0x54, 0x05, 0x32, 0x54};
      byte p22[] = {0xB4, 0xBA, 0xE3, 0xCE}; 
      byte p23[] = {0x2E, 0xE9, 0x4A, 0x76};
      ///pages
      nfc.ntag2xx_WritePage(8, p8);
      nfc.ntag2xx_WritePage(9, p9);
      nfc.ntag2xx_WritePage(10, p10);
      nfc.ntag2xx_WritePage(11, p11); // DS
      nfc.ntag2xx_WritePage(12, p12); /// default spool
      nfc.ntag2xx_WritePage(13, p13);
      nfc.ntag2xx_WritePage(14, p14);
      nfc.ntag2xx_WritePage(17, p17);
      nfc.ntag2xx_WritePage(20, p20); /// CHSP
      nfc.ntag2xx_WritePage(21, p21);
      nfc.ntag2xx_WritePage(22, p22);
      nfc.ntag2xx_WritePage(23, p23);
      //FIANL TAG LOCK
    }
    else
    {
      Serial.println("WRONT TAG");
    }

  }
  
}
void menuItem4() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t buf[16];
  uint8_t data[32];
  int activeButton = 0;
  lcd.clear();
  lcd.print("     READY!");
  lcd.setCursor(0, 2);
  lcd.print("BRING NEAR SPOOL");
  lcd.setCursor(0, 1);
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    if (uidLength == 7)
    {
      Serial.println("Smells right bitch");
      lcd.clear() ;
      lcd.print("CONNNECTED");
      uint32_t k = getkey(uid); //10 digit value
      int khex = (k, HEX);
      Serial.print("KEY:"); Serial.println(k, HEX);
      Serial.print("KEH:"); Serial.println(khex);
      unsigned long thenumber = k;
      byte pwd[] = {0x00, 0x00, 0x00, 0x00};
      pwd[3] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[2] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[1] = thenumber & 255;
      thenumber = thenumber >> 8;
      pwd[0] = thenumber;
      lcd.clear();
      lcd.print("3");
      delay(1000);
      lcd.clear();
      lcd.print("2");
      delay(1000);
      lcd.clear();
      lcd.print("1");
      delay(1000);
      nfc.ntag2xx_Authenticate(pwd);
      for (uint8_t i = 0; i < 42; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        delay(50);
        lcd.clear();
        lcd.print("READING ");
        lcd.write(byte(1));
        Serial.print("uint8_t password");
        if (i < 10)
        {
          Serial.print("0");
          Serial.print(i, HEX);
        }
        else
        {
          Serial.print(i, HEX);
        }
        Serial.print(": ");
        // Display the results, depending on 'success'
        if (success)
        {
          digitalWrite(12, HIGH);
          // Dump the page data
          nfc.PrintHexChar(data, 4);
          delay(50);
          lcd.clear() ;
          lcd.print("READING ");
          lcd.write(byte(2));
        }
        else
        {
          Serial.println("Mom didnt tech me how to read!");
          lcd.clear() ;
          lcd.print("ERROR");
        }
      }
      delay(2500);
      lcd.clear() ;
    }
    else
    {
      Serial.println("WRONT TAG");
    }

  }
}

void menuItem5() { 
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("By paws#3621");
  lcd.setCursor(1, 1);
  lcd.print("16 Hrs & 9 RDB");
  delay(3000);
  lcd.clear();
  
}

void menuItem6() { // Function executes when you select the 4th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print(vernum);
  lcd.setCursor(3, 1);
  lcd.print("BLTON ");
  lcd.print(datecur);
  delay(1500);
  lcd.clear();

}


void menuItem7() { // Function executes when you select the 6th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 6");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem8() { // Function executes when you select the 7th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 7");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem9() { // Function executes when you select the 8th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 8");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem10() { // Function executes when you select the 9th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 9");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
