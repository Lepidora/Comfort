#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "SampleBuffer.h"

const int kNumPrograms = 1;

enum EParams
{
    kPathOnePan = 0,
    kPathOneGain = 1,
    kPathOneDelay = 2,
    kPathTwoPan = 3,
    kPathTwoGain = 4,
    kPathTwoDelay = 5,
    kDryWet = 6,
    kNumParams
};

using namespace iplug;
using namespace igraphics;

class Comfort final : public Plugin
{
private:
    SampleBuffer<double> pathOneSamples;
    SampleBuffer<double> pathTwoSamples;
public:
  Comfort(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    double ApplyGain(double sample, double gainDb);
#endif
};
