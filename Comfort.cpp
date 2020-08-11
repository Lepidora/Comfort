#include "Comfort.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

Comfort::Comfort(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms)),pathOneSamples(GetSampleRate()),pathTwoSamples(GetSampleRate())
{
    
    
    
    GetParam(kPathOnePan)->InitDouble("Path One Pan", 0., -64., 64.0, 0.01, "");
    GetParam(kPathOneGain)->InitDouble("Path One Gain", 0., -64., 16., 0.01, "dB");
    GetParam(kPathOneDelay)->InitDouble("Path One Delay", 0., 0., 1000., 1., "ms");
    
    GetParam(kPathTwoPan)->InitDouble("Path Two Pan", 0., -64., 64.0, 0.01, "");
    GetParam(kPathTwoGain)->InitDouble("Path Two Gain", 0., -64., 16., 0.01, "dB");
    GetParam(kPathTwoDelay)->InitDouble("Path Two Delay", 0., 0., 1000., 1., "ms");
    
    GetParam(kDryWet)->InitDouble("Dry/Wet", 100., 0., 100., 0.01, "%");
    

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-200).GetVShifted(-100), kPathOnePan));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-100).GetVShifted(-100), kPathOneGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(0).GetVShifted(-100), kPathOneDelay));
      
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-200).GetVShifted(20), kPathTwoPan));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-100).GetVShifted(20), kPathTwoGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(0).GetVShifted(20), kPathTwoDelay));
      
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(200).GetVShifted(-100), kDryWet));
      
  };
#endif
}

#if IPLUG_DSP
void Comfort::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    //const double gain = GetParam(kPathOnePan)->Value() / 100.;
    
    const double onePan = (GetParam(kPathOnePan)->Value() / 128.) + 0.5;
    const double oneGain = GetParam(kPathOneGain)->Value();
    const double oneDelay = GetParam(kPathOneDelay)->Value();
    
    const double twoPan = (GetParam(kPathTwoPan)->Value() / 128.) + 0.5;
    const double twoGain = GetParam(kPathTwoGain)->Value();
    const double twoDelay = GetParam(kPathTwoDelay)->Value();
    
    const double dryWet = GetParam(kDryWet)->Value() / 100.;
    
    const double sampleRate = GetSampleRate();
    
    const unsigned int oneSampleDelay = (unsigned int)((sampleRate / 1000.0) * oneDelay);
    const unsigned int twoSampleDelay = (unsigned int)((sampleRate / 1000.0) * twoDelay);
    
    printf("one: %g two: %g\n", onePan, twoPan);
    
    const int nChans = NOutChansConnected();
  
    for (int s = 0; s < nFrames; s++) {
        
        double sampleTotal = 0.;
        
        for (int c = 0; c < nChans; c++) {
            sampleTotal += inputs[c][s];
        }
        
        double sampleAvg = sampleTotal / (double)nChans;
        
        double oneOldSample = sampleAvg;//ApplyGain(sampleAvg, oneGain);
        double twoOldSample = sampleAvg;//ApplyGain(sampleAvg, twoGain);
        
        pathOneSamples.push(oneOldSample);
        pathTwoSamples.push(twoOldSample);
        
        double oneNewSample = 0.;
        double twoNewSample = 0.;
        
        if (oneSampleDelay == 0) {
            oneNewSample = sampleAvg;
        } else {
            while (pathOneSamples.size() > 0 && pathOneSamples.size() > oneSampleDelay) {
                oneNewSample = pathOneSamples.pop();
            }
        }
        
        if (twoSampleDelay == 0) {
            twoNewSample = sampleAvg;
        } else {
            while (pathTwoSamples.size() > 0 && pathTwoSamples.size() > twoSampleDelay) {
                twoNewSample = pathTwoSamples.pop();
            }
        }
        
        for (int c = 0; c < nChans; c++) {
            
            if (c == 0) {
                
                double oneLeftAmplitude = cos(onePan * 0.5 * M_PI);
                double twoLeftAmplitude = cos(twoPan * 0.5 * M_PI);
                
                //printf("Left: One: %g Two: %g\n", oneLeftAmplitude, twoLeftAmplitude);
                
                double oneLeftSample = ApplyGain(oneNewSample, oneGain) * oneLeftAmplitude * dryWet;
                double twoLeftSample = ApplyGain(twoNewSample, twoGain) * twoLeftAmplitude * dryWet;
                
                double dryOriginal = inputs[c][s] * (1. - dryWet);
                
                outputs[c][s] = oneLeftSample + twoLeftSample + dryOriginal;
            }
            
            if (c == 1) {
                
                double oneRightAmplitude = sin(onePan * 0.5 * M_PI);
                double twoRightAmplitude = sin(twoPan * 0.5 * M_PI);
                
                //printf("Right: One: %g Two: %g\n", oneRightAmplitude, twoRightAmplitude);
                
                double oneRightSample = ApplyGain(oneNewSample, oneGain) * oneRightAmplitude * dryWet;
                double twoRightSample = ApplyGain(twoNewSample, twoGain) * twoRightAmplitude * dryWet;
                
                double dryOriginal = inputs[c][s] * (1. - dryWet);
                
                outputs[c][s] = oneRightSample + twoRightSample + dryOriginal;
            }
        }
    }
}

double Comfort::ApplyGain(double sample, double gainDb) {
    
    double type = 20.0;
    
    double index = gainDb / type;
    
    double power = pow(10.0, index);
    
    return sample * power;
}

#endif
