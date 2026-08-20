void initDebugMonitor()
{
  Serial.begin(57600);
}

void updateDebugMonitor()
{
  char out[128];
  sprintf(out,"Thrust: [%4d|%6d|%5d]  Rudder: [%4d|%6d|%5d]  Elevator: [%4d|%6d|%5d]  Aileron: [%4d|%6d|%5d]\n",
    thrust.getRawValue(),   thrust.getValue(),   thrust.getRawTrimm(), 
    rudder.getRawValue(),   rudder.getValue(),   rudder.getRawTrimm(), 
    elevator.getRawValue(), elevator.getValue(), elevator.getRawTrimm(), 
    aileron.getRawValue(),  aileron.getValue(),  aileron.getRawTrimm());
  Serial.print(out);
}
