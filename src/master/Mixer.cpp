#include "Mixer.h"



Mixer::Mixer(const MixerType            type,
             const RcBaseInput&         source,
             Servo&                     servo)
  : type(type),
    source(&source), 
    servo(&servo),
    controlLimit(source.getControlLimit())
{
  const MixerConfiguration defaultConfiguration;
  setConfiguration(defaultConfiguration);
  enabled = true;
}


Mixer::Mixer(const MixerType            type,
             const RcBaseInput&         source,
             Servo&                     servo,
             const MixerConfiguration&  configuration)
  : type(type),
    source(&source), 
    servo(&servo), 
    controlLimit(source.getControlLimit())
{
  setConfiguration(configuration);
  enabled = true;
}


void  Mixer::deactivateMixer()                            { enabled = false;           }
void  Mixer::activateMixer()                              { enabled = true;            }
MixerType Mixer::getType() const                          { return type;               }
const MixerConfiguration& Mixer::getConfiguration() const { return configuration;      }


void Mixer::setConfiguration(const MixerConfiguration& configuration)
{
  setGain(configuration.gainPositive, configuration.gainNegative);
  this->configuration.offsetPercent = constrain(configuration.offsetPercent, -100, 100);
  this->configuration.deadBandLowerPercent = constrain(configuration.deadBandLowerPercent, -100, 0);
  this->configuration.deadBandUpperPercent = constrain(configuration.deadBandUpperPercent, 0, 100);

  updateConfiguration();
}


void Mixer::setGain(int8_t gainPositive, int8_t gainNegative)
{
  configuration.gainPositive = constrain(gainPositive, -100, 100);
  configuration.gainNegative = constrain(gainNegative, -100, 100);
}
  

void Mixer::setOffset(int8_t offsetPercent)
{
  configuration.offsetPercent = constrain(offsetPercent, -100, 100);
  updateConfiguration();
}

 
void Mixer::setDeadBandLimits(const int8_t lowerLimitPercent, const int8_t upperLimitPercent)
{
  configuration.deadBandLowerPercent = constrain(lowerLimitPercent, -100, 0);
  configuration.deadBandUpperPercent = constrain(upperLimitPercent, 0, 100);

  updateConfiguration();
}
  

void Mixer::setDeadBand(const uint8_t deadBandPercent)
{
  const int8_t width = min(deadBandPercent, 100);
  configuration.deadBandLowerPercent = -(width / 2);
  configuration.deadBandUpperPercent = width + configuration.deadBandLowerPercent;

  updateConfiguration();
}


void Mixer::updateConfiguration()
{
  int32_t percent = configuration.offsetPercent;

  if (percent >= 0) offset = (percent * controlLimit + 50) / 100;
  else              offset = (percent * controlLimit - 50) / 100;

  percent = configuration.deadBandUpperPercent;
  deadBandUpperLimit = (percent * controlLimit + 50) / 100;

  percent = configuration.deadBandLowerPercent;
  deadBandLowerLimit = (percent * controlLimit - 50) / 100;
}


void Mixer::runMixer()
{
  if (!enabled) return;

  int32_t controlValue = source->getValue();

  if (   (controlValue >= deadBandLowerLimit) 
      && (controlValue <= deadBandUpperLimit) )  controlValue = 0;
  else if (controlValue > 0)  controlValue = (controlValue * configuration.gainPositive) / 100;
  else                        controlValue = (controlValue * configuration.gainNegative) / 100;

  
  if (offset != 0)
  {
    if      (controlValue > 0) controlValue = map(controlValue, 0,  controlLimit,  offset, controlLimit);
    else if (controlValue < 0) controlValue = map(controlValue, -controlLimit, 0, -controlLimit, offset);
  }
  
  controlValue = constrain(controlValue, -controlLimit, controlLimit);

  servo->addActuation(controlValue);
}












