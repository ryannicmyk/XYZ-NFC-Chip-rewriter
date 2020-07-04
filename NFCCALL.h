void basicrun() {
	uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t buf[16];
  digitalWrite(12, HIGH);
  delay(250);
  digitalWrite(12, LOW);
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
      uint8_t data[32];
      Serial.println("Smells right bitch");
      lcd.clear() ;
      lcd.print("CONNNECTED");
      uint32_t k = getkey(uid); //10 digit value
      uint8_t* ptr_2_k = (void*)&k;
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
      byte p14[] = {0x40, 0x0D, 0x03, 0x0A};
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
      lcd.print("40 0D 03 0A");
      lcd.setCursor(0,1);
      lcd.print("40 0D 03 0A");
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
};