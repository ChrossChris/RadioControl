void initDebugMonitor()
{
  Serial.begin(57600);
}

void updateDebugMonitor()
{
  char out[700];
  sprintf(out,"S %d | J1[%4d|%4d] J2[%4d|%4d] J3[%4d|%4d] J4[%4d|%4d]  ||  PPM[%4d %4d %4d %4d | %4d %4d %4d %4d]  ||  PM[%4d] P_CL[%4d] P_CR[%4d]  ||  PL1[%4d] PL2[%4d] PR1[%4d] PR2c[%4d]  ||  SW_CL[%1d] SW_CR[%1d] SW1[%1d] SW2[%1d] SW3[%2d] SW4[%2d]  ||  TB1[%2d] TB2[%2d]  ||  SWL1[%1d] SWL2[%1d] || SWR1[%1d] SWR2[%1d] SWRrot[%1d] BT [%1d]\n",
    screen,
    thrust.value, thrust.trimm, rudder.value, rudder.trimm, elevator.value, elevator.trimm, aileron.value, aileron.trimm,
    ppm[0],          ppm[1],          ppm[2],             ppm[3],          ppm[4],          ppm[5],           ppm[6],          ppm[7],
    potiMain,        potiCenterLeft,  potiCenterRight,    potiLeft1,       potiLeft2,       potiRight1,       potiRight2Cont,
    switchCenter[0], switchCenter[1], switchCenter[2],    switchCenter[3], switchCenter[4], switchCenter[5],
    toggleButton[0], toggleButton[1], switchLeft[0],      switchLeft[1],
    switchRight[0],  switchRight[1],  switchRightRotary,  buttonRight);
  Serial.print(out);
}
