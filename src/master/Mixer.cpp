#include "mixer.h"

void MixerConfiguration::setName(const char* newName)
{
  if (newName == nullptr)
  {
    name[0] = '\0';
    return;
  }

  uint8_t index = 0;

  while ((index < maxNameLength) && (newName[index] != '\0'))
  {
    name[index] = newName[index];
    ++index;
  }

  name[index] = '\0';
}


Mixer::Mixer()
  : source(nullptr),
    servo(nullptr),
    enabled(false)
{
}


Mixer::Mixer(const ControlSource* source,
                   Servo*    servo,
             const int8_t    gainPositive,
             const int8_t    gainNegative,
             const uint8_t   deadBandPercent,
             const int8_t    offsetPercent)
  : Mixer()
{
  setGain(gainPositive, gainNegative);
  setDeadBand(deadBandPercent);
  setOffset(offsetPercent);

  if (!connectMixer(source, servo, MixerConfigMode::KEEP)) resetMixer();
}


Mixer::Mixer(const ControlSource* source,
             Servo* servo,
             const MixerConfiguration& configuration)
  : Mixer()
{
  setConfiguration(configuration);

  if (!connectMixer(source, servo, MixerConfigMode::KEEP)) resetMixer();
}


Mixer::Mixer(const Mixer& source, Servo* newServo, const MixerGainMode gainMode)
  : Mixer()
{
  configuration = source.configuration;

  if (gainMode == MixerGainMode::SWAP)
  {
    const int8_t gainPositive     = configuration.gainPositive;
    configuration.gainPositive    = configuration.gainNegative;
    configuration.gainNegative    = gainPositive;
  }

  if (!connectMixer(source.source, newServo, MixerConfigMode::KEEP))
  {
    resetMixer();
  }
  else if (!source.enabled)
  {
    deactivateMixer();
  }
}


bool Mixer::connectMixer(const ControlSource* source,
                         Servo* servo,
                         const MixerConfigMode configMode)
{
  if ((source == nullptr) || (servo == nullptr))   enabled = false;
  else
  {
    this->source = source;
    this->servo  = servo;
    controlLimit = source->getControlLimit();

    if (configMode == MixerConfigMode::RESET) resetConfiguration();
    else                                      updateScaledConfiguration();

    enabled = true;
  }

  return enabled;
}


void Mixer::resetConfiguration()
{
  configuration = MixerConfiguration{};
  updateScaledConfiguration();
}


void Mixer::resetMixer()
{
  source       = nullptr;
  servo        = nullptr;
  controlLimit = 0;
  enabled      = false;

  resetConfiguration();
}


void Mixer::deactivateMixer()
{
  enabled = false;
}


bool Mixer::activateMixer()
{
  if ((source == nullptr) || (servo == nullptr))  enabled = false;
  else
  {  
    controlLimit = source->getControlLimit();
    updateScaledConfiguration();
    enabled = true;
  }

  return enabled;
}


void Mixer::setGain(int8_t gainPositive, int8_t gainNegative)
{
  configuration.gainPositive = constrain(gainPositive, -100, 100);
  configuration.gainNegative = constrain(gainNegative, -100, 100);
}
  

void Mixer::setOffset(int8_t offsetPercent)
{
  configuration.offsetPercent = constrain(offsetPercent, -100, 100);
  updateScaledConfiguration();
}


void Mixer::setConfiguration(const MixerConfiguration& configuration)
{
  setName(configuration.name);
  setGain(configuration.gainPositive, configuration.gainNegative);
  this->configuration.offsetPercent = constrain(configuration.offsetPercent, -100, 100);
  this->configuration.deadBandLowerPercent = constrain(configuration.deadBandLowerPercent, -100, 0);
  this->configuration.deadBandUpperPercent = constrain(configuration.deadBandUpperPercent, 0, 100);

  updateScaledConfiguration();
}


const MixerConfiguration& Mixer::getConfiguration() const
{
  return configuration;
}

  
void Mixer::setDeadBandLimits(const int8_t lowerLimitPercent, const int8_t upperLimitPercent)
{
  configuration.deadBandLowerPercent = constrain(lowerLimitPercent, -100, 0);
  configuration.deadBandUpperPercent = constrain(upperLimitPercent, 0, 100);

  updateScaledConfiguration();
}
  

void Mixer::setDeadBand(const uint8_t deadBandPercent)
{
  const int8_t width = min(deadBandPercent, 100);
  configuration.deadBandLowerPercent = -(width / 2);
  configuration.deadBandUpperPercent = width + configuration.deadBandLowerPercent;

  updateScaledConfiguration();
}


void Mixer::updateScaledConfiguration()
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


void Mixer::setName(const char* name)
{
  configuration.setName(name);
}


const char* Mixer::getName() const
{
  return configuration.name;
}
