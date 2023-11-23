#define BACKGROUND_COLOR    RGBto565( 40, 40, 40)
#define BAR_COLOR           RGBto565(  0,  0,196) 
#define FRAME_COLOR         RGBto565(128,128,128)
#define FRAME_OFFSET        4
#define BORDER_OFFSET       5

void setLevelMeter(const char *text, int value, const int minValue, const int maxValue, const int line)
{
  const int fontWidth      = 60;
  // const int sizeLevelMeter = 100;
  // const int barWidth  = TFT_WIDTH * sizeLevelMeter/100 - fontWidth;
  const int x0 = BORDER_OFFSET; // (x0_in * TFT_WIDTH) / 100;
  const int dy = 20;
  const int barHeight = 30;
  const int barWidth  = TFT_WIDTH - 2*BORDER_OFFSET;
  int y0 = (line * (TFT_HEIGHT-BORDER_OFFSET)) / 4;
  
  
  int level;
  char out[8];
  
  font.setColor(WHITE,BLACK);
  font.setCharMinWd(13);
  font.setFont(&rre_term_10x16);
  font.printStr(0, y0, "                     ");
  font.printStr(0, y0, text);
    
  font.setFont(&rre_term_10x16);
  sprintf(out,"%4d",value);
  font.printStr(ALIGN_RIGHT,y0+5,out);

  y0 += dy;
  tft.fillRect(x0, y0, barWidth, barHeight, BACKGROUND_COLOR);

  value = constrain(value, minValue, maxValue);
  
  // Level diagram: |  <---|      |
  if ((minValue < 0) && (maxValue > 0))
  {
    if (value < 0)
    {
      level = map(value, minValue, 0, 0, barWidth/2);
      tft.fillRect(x0 + level, y0+FRAME_OFFSET, barWidth/2 - level, barHeight-2*FRAME_OFFSET, BAR_COLOR);
    }
    else
    {
      level = map(value, 0, maxValue, 0, barWidth/2);
      tft.fillRect(x0 + barWidth/2, y0+FRAME_OFFSET, level, barHeight-2*FRAME_OFFSET, BAR_COLOR);
    }
    tft.drawFastVLine(x0 +   barWidth/2, y0,   barHeight,    FRAME_COLOR);
    
    tft.drawFastVLine(x0 + 1*barWidth/6, y0+5, barHeight-10, FRAME_COLOR);
    tft.drawFastVLine(x0 + 2*barWidth/6, y0+5, barHeight-10, FRAME_COLOR);
    tft.drawFastVLine(x0 + 4*barWidth/6, y0+5, barHeight-10, FRAME_COLOR);
    tft.drawFastVLine(x0 + 5*barWidth/6, y0+5, barHeight-10, FRAME_COLOR);

    
  }

  // Bar diagram: |          oooo|
  else if (minValue < 0)
  {
    level = map(value, minValue, maxValue, 0, barWidth);
    tft.fillRect(x0+level, y0+FRAME_OFFSET, barWidth-level, barHeight-2*FRAME_OFFSET, BAR_COLOR);
    for (int i = 0; i < 5; i++) tft.drawFastVLine(x0 + i*barWidth/5, y0, barHeight, FRAME_COLOR);

  }

  // Bar diagram: |ooooooooo    |
  else 
  {
    level = map(value, minValue, maxValue, 0, barWidth);
    tft.fillRect(x0, y0+FRAME_OFFSET, level,  barHeight-2*FRAME_OFFSET, BAR_COLOR);
    for (int i = 0; i < 5; i++) tft.drawFastVLine(x0 + i*barWidth/5, y0, barHeight, FRAME_COLOR);
  }
  


}

  
//  // Initial einmal alles löschen (ging auch durch Auffüllen der übrigen Zeichen, aber
//  // Code-Effizienz plus Sonderfall ohne Ausschlag gegenüber Verständlichkeit hat verloren ;-)
//
//  for (int idx = DISPLAY_WIDTH - sizeLevelMeter; idx < DISPLAY_WIDTH; idx++)  str[idx] = ' ';
//
//  // Sinnvoller Wert übergeben, für die die Anzeige generiert werden kann?  
//  if      ((sizeLevelMeter < 1) || (sizeLevelMeter > 20))  sprintf(str,"ERR: levelMeter OoS ");
//  else if (minValue >  0 )                                 sprintf(str,"ERR: minValue > 0   ");
//  else if (maxValue <= 0 )                                 sprintf(str,"ERR: maxValue < 0   "); 
//
//  // Eigentliche Ausgabe 
//  else if (minValue < 0)
//  {
//    if ( (sizeLevelMeter % 2) == 0)  sizeLevelMeter--;
//    int i0  = DISPLAY_WIDTH - sizeLevelMeter / 2 - 1;
//    str[i0] = '|';
//    if (value < 0) 
//    {
//      int digits = map(abs(value), 0, abs(minValue), 0, sizeLevelMeter/2 + 1);
//      if (digits > sizeLevelMeter/2)            digits      = sizeLevelMeter/2;
//      for (int idx = 1; idx < digits; idx++)    str[i0-idx] = '-';
//      str[i0 - digits] = '<';
//    }
//    else if (value > 0) 
//    {
//      int digits = map(value, 0, maxValue, 0, sizeLevelMeter/2 + 1);
//      if (digits > sizeLevelMeter/2)            digits      = sizeLevelMeter/2;
//      for (int idx = 1; idx < digits; idx++)    str[i0+idx] = '-';
//      str[i0 + digits] = '>';
//    }
//  }
//
//  // Balkenanzeige von null bis zum Maximalwert
//  else
//  {
//    int i0     = DISPLAY_WIDTH - sizeLevelMeter;
//    int digits = map(value, minValue, maxValue, 0, sizeLevelMeter*2+1);
//    digits     = constrain(digits, 0, sizeLevelMeter*2);
//    for (int idx = 0;        idx < digits/2; idx++)        str[i0+idx]        = 0xDB;
//    if ((digits % 2) == 1)                                 str[i0 + digits/2] = '>';
//  }
//}
