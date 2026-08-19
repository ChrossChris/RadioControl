void displayStatus()
{
  char text[40];
  if      (model == DPOWER_STREAMLINE) sprintf(text, "D-Power Streamline 270X");
  else if (model == MULTIPLEX_HERON)   sprintf(text, "Multiplex Heron");
  else if (model == GRAUPNER_AMIGO4)   sprintf(text, "Graupner Amigo IV");
  else if (model == GRAUPNER_EPS2000)  sprintf(text, "Graupner EPS2000");
  else                                 sprintf(text, "Modell nicht definiert");
  setText(text, 0);

  snprintf(text, sizeof(text), "LiPo %ds  %dmAh", accuCells, accuCapacity);
  setText(text, 1);
  snprintf(text, sizeof(text), "Zeit %02um%02us  Gas %u", (runningTimeSec / 60) % 100,
           runningTimeSec % 60, throttleTotal);
  setText(text, 2);
  snprintf(text, sizeof(text), "Verbrauch %dmAh", accuDischargeTotal_mAh);
  setText(text, 3);

  if (accuChargeLevel > 0) setLevelMeter("Akkuladung [%]", accuChargeLevel, 0, 100, 3);
  else                     setText("!! AKKU LEER !!", 9, ALIGN_CENTER);
}

void displayJoystickPosition()
{
  const char *labels[CHANNELS] = {"Hoehenruder", "Seitenruder", "Querruder", "Motor"};
  for (int i = 0; i < CHANNELS; i++)
  {
    const int value = (abs(joysticks[i]) > 5) ? joysticks[i] : 0;
    setLevelMeter(labels[i], value, joystickLimits[i][MIN], joystickLimits[i][MAX], i);
  }
}

void displayJoystickLimits()
{
  char text[40];
  snprintf(text, sizeof(text), "HR  hoch %4d%%  runter %4d%%", joystickLimits[0][MIN], joystickLimits[0][MAX]);
  setText(text, 1);
  snprintf(text, sizeof(text), "SR  links %4d%% rechts %4d%%", joystickLimits[1][MIN], joystickLimits[1][MAX]);
  setText(text, 3);
  snprintf(text, sizeof(text), "QR  runter%4d%%  hoch %4d%%", joystickLimits[2][MIN], joystickLimits[2][MAX]);
  setText(text, 5);
  snprintf(text, sizeof(text), "Motor maximal %4d%%", joystickLimits[3][MAX]);
  setText(text, 7);
}

void displayConfiguration()
{
  char modelText[40];
  char accuText[40];
  if      (model == MULTIPLEX_HERON)   sprintf(modelText, "Multiplex Heron");
  else if (model == DPOWER_STREAMLINE) sprintf(modelText, "D-Power Streamline");
  else if (model == GRAUPNER_AMIGO4)   sprintf(modelText, "Graupner Amigo IV");
  else if (model == GRAUPNER_EPS2000)  sprintf(modelText, "Graupner EPS2000");
  else                                 sprintf(modelText, "Modell nicht definiert");

  if      (accu == ACCU_4s_5500mAh) sprintf(accuText, "Akku: LiPo 4s 5500mAh");
  else if (accu == ACCU_4s_2400mAh) sprintf(accuText, "Akku: LiPo 4s 2400mAh");
  else if (accu == ACCU_3s_2200mAh) sprintf(accuText, "Akku: LiPo 3s 2200mAh");
  else if (accu == ACCU_3s_1800mAh) sprintf(accuText, "Akku: LiPo 3s 1800mAh");
  else                              sprintf(accuText, "Akku nicht definiert");

  setLevelMeter("Expo Seitenruder", expoSeite, 0, 100, 0);
  setLevelMeter("Motor-Hoehe-Mischer", motrHoeheMischer, 0, LIMIT_MH_MISCHER, 1);
  setText(modelText, 7);
  setText(accuText, 9);
}

void displayAddFunctionsNumeric()
{
  char text[40];
  snprintf(text, sizeof(text), "Expo HR %4d%%   Trimm HR %4d", expoHoehe, trimmHoehe); setText(text, 1);
  snprintf(text, sizeof(text), "Expo QR %4d%%   Trimm QR %4d", expoQuer, trimmQuer); setText(text, 3);
  snprintf(text, sizeof(text), "Expo SR %4d%%   Mix QS   %4d%%", expoSeite, querSeiteMischer); setText(text, 5);
  snprintf(text, sizeof(text), "Mix MH  %4d%%   Mix QF   %4d%%", motrHoeheMischer, querFlapsMischer); setText(text, 7);
}

void displayAddFunctionsGraphic()
{
  if (bitRead(remoteControlSetting, TRIMM_EXPO_HOEHE) == 0)
    setLevelMeter("Trimm Hoehenruder", trimmHoehe, -LIMIT_TRIMM, LIMIT_TRIMM, 0);
  else
    setLevelMeter("Expo Hoehenruder", expoHoehe, 0, 100, 0);

  if (bitRead(remoteControlSetting, TRIMM_EXPO_QUER) == 0)
    setLevelMeter("Trimm Querruder", trimmQuer, -LIMIT_TRIMM, LIMIT_TRIMM, 1);
  else
    setLevelMeter("Expo Querruder", expoQuer, 0, 100, 1);

  if (bitRead(remoteControlSetting, QUER_SEITE_FLAPS_MISCHER) == 0)
    setLevelMeter("Mischer Quer-Seite", querSeiteMischer, -LIMIT_QS_MISCHER, LIMIT_QS_MISCHER, 2);
  else
    setLevelMeter("Mischer Quer-Flaps", querFlapsMischer, -LIMIT_QF_MISCHER, LIMIT_QF_MISCHER, 2);

  setLevelMeter(bitRead(remoteControlSetting, BUTTERFLY) ? "Butterfly" : "Flaps",
                flaps, -LIMIT_FLAPS, LIMIT_FLAPS, 3);
}

void displayUnknownScreen()
{
  char text[32];
  snprintf(text, sizeof(text), "Unbekannter Screen: %u", statusScreen);
  setText(text, 4, ALIGN_CENTER);
}
