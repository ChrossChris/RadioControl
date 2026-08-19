#define BACKGROUND_COLOR RGBto565(40, 40, 40)
#define BAR_COLOR        RGBto565(0, 0, 196)
#define FRAME_COLOR      RGBto565(128, 128, 128)
#define FRAME_OFFSET     4
#define BORDER_OFFSET    5

void setLevelMeter(const char *text, int value, const int minValue, const int maxValue, const int line)
{
  const int x0 = BORDER_OFFSET;
  const int barHeight = 30;
  const int barWidth = TFT_WIDTH - 2 * BORDER_OFFSET;
  int y0 = (line * (TFT_HEIGHT - BORDER_OFFSET)) / 4;
  char out[26] = "                         ";

  font.setColor(WHITE, BLACK);
  font.setCharMinWd(13);
  font.setFont(&rre_term_10x16);
  font.printStr(0, y0, out);
  font.printStr(0, y0, const_cast<char *>(text));
  snprintf(out, sizeof(out), "%6d", value);
  font.printStr(ALIGN_RIGHT, y0 + 5, out);

  y0 += 20;
  tft.fillRect(x0, y0, barWidth, barHeight, BACKGROUND_COLOR);
  value = constrain(value, minValue, maxValue);

  if ((minValue < 0) && (maxValue > 0))
  {
    if (value < 0)
    {
      const int level = map(value, minValue, 0, 0, barWidth / 2);
      tft.fillRect(x0 + level, y0 + FRAME_OFFSET, barWidth / 2 - level,
                   barHeight - 2 * FRAME_OFFSET, BAR_COLOR);
    }
    else
    {
      const int level = map(value, 0, maxValue, 0, barWidth / 2);
      tft.fillRect(x0 + barWidth / 2, y0 + FRAME_OFFSET, level,
                   barHeight - 2 * FRAME_OFFSET, BAR_COLOR);
    }
    tft.drawFastVLine(x0 + barWidth / 2, y0, barHeight, FRAME_COLOR);
    for (int i = 1; i < 6; i++)
      if (i != 3) tft.drawFastVLine(x0 + i * barWidth / 6, y0 + 5, barHeight - 10, FRAME_COLOR);
  }
  else
  {
    const int level = map(value, minValue, maxValue, 0, barWidth);
    tft.fillRect(x0, y0 + FRAME_OFFSET, level, barHeight - 2 * FRAME_OFFSET, BAR_COLOR);
    for (int i = 0; i <= 5; i++)
      tft.drawFastVLine(x0 + i * (barWidth - 1) / 5, y0, barHeight, FRAME_COLOR);
  }
}

void setText(const char *text, const int line)
{
  setText(text, line, 0);
}

void setText(const char *text, const int line, const int alignment)
{
  const int y0 = line * 23;
  font.setColor(WHITE, BLACK);
  font.setCharMinWd(10);
  font.setFont(&rre_term_10x16);
  tft.fillRect(0, y0, TFT_WIDTH, 20, BLACK);
  font.printStr(alignment, y0, const_cast<char *>(text));
}
