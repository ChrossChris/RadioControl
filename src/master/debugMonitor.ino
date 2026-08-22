void initDebugMonitor()
{
  Serial.begin(57600);
}

void updateDebugMonitor()
{
  char out[500];
  sprintf(out,"Thrust: [%4d|%6d|%6d|%5d]  Rudder: [%4d|%6d|%6d|%5d]  Elevator: [%4d|%6d|%6d|%5d]  Aileron: [%4d|%6d|%6d|%5d]  Expo: %4d [%d|%d]  DualRateLeft: %4d [%d]  DualRateRight: %4d [%d] \n",
    thrust.getValueRaw(),   thrust.getValueNormalized(),   thrust.getValue(),   thrust.getTrimmValueRaw(), 
    rudder.getValueRaw(),   rudder.getValueNormalized(),   rudder.getValue(),   rudder.getTrimmValueRaw(), 
    elevator.getValueRaw(), elevator.getValueNormalized(), elevator.getValue(), elevator.getTrimmValueRaw(), 
    aileron.getValueRaw(),  aileron.getValueNormalized(),  aileron.getValue(),  aileron.getTrimmValueRaw(),
    expo, switchExpo, switchXYCurve, dualRateLeft, switchDualRateLeft, dualRateRight, switchDualRateRight);
  Serial.print(out);
}
