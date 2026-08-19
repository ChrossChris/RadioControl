#ifndef TONE_SEQUENCE_H
#define TONE_SEQUENCE_H

struct ToneSequence
{
  ToneSequence(int frequencyValue, int durationValue, int pauseValue)
    : frequency(frequencyValue), duration(durationValue), pause(pauseValue)
  {
  }

  int frequency;
  int duration;
  int pause;
};

typedef ToneSequence TS;

#endif
