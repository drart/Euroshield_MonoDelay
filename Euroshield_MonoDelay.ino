#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            InputI2S;       //xy=241,415
AudioAnalyzePeak         peak;          //xy=435,576
AudioMixer4              InputMixer;    //xy=742,537
AudioEffectDelay         Delay;         //xy=756,734
AudioFilterStateVariable filter;        //xy=904,638
AudioMixer4              OutputMixer;   //xy=1148,539
AudioOutputI2S           OutputI2S;      //xy=1334,460
AudioConnection          patchCord1(InputI2S, 0, OutputMixer, 0);
AudioConnection          patchCord2(InputI2S, 0, InputMixer, 0);
AudioConnection          patchCord3(InputI2S, 1, peak, 0);
AudioConnection          patchCord4(InputMixer, Delay);
AudioConnection          patchCord5(Delay, 0, filter, 0);
AudioConnection          patchCord6(filter, 0, OutputMixer, 1);
AudioConnection          patchCord7(filter, 0, InputMixer, 1);
AudioConnection          patchCord8(OutputMixer, 0, OutputI2S, 0);
AudioConnection          patchCord9(OutputMixer, 0, OutputI2S, 1);
AudioControlSGTL5000     codec;          //xy=371,690
AudioControlSGTL5000     sgtl5000_1;     //xy=501,434
// GUItool: end automatically generated code


//parameters, change as desired
float wetFactor = 0.5f; //starting wet/dry mix, range: 0.0 -> 1.0
float cvControllableDelayTime = 50.f; //max amt of delay time controllable via CV, range: 0.0 -> 400.0
float feedbackMinimum = 0.5f; //minimum feedback lvl, when knob is fully ccw, range: 0.0 -> 1.0
int lowpassFilterCutoff = 12000; //range: 0 -> 20000
float lowpassFilterResonance = 0.7f; //range: 0.0 -> 5.0, resonance amplifies cutoff frequency when value is past 0.707

//do not change
int upperPotInput = 20;
int lowerPotInput = 21;
#define buttonInput 2
#define analogReadMax 1024
int maxDelayTime = 400;
float knobControllableDelayTime = maxDelayTime - cvControllableDelayTime;
Bounce Button = Bounce();
float prevPeakValue = 0.0f;
void setup()
{
  pinMode(buttonInput, INPUT_PULLUP);
  Button.attach(buttonInput);
  Button.interval(5);
  AudioMemory(200);
  // Enable the audio shield, select input, and enable output
  codec.enable();
  codec.inputSelect(AUDIO_INPUT_LINEIN);
  codec.volume(0.82);
  codec.adcHighPassFilterDisable();
  codec.lineInLevel(0,0);
  codec.unmuteHeadphone();

  InputMixer.gain(0, 0.5); //dry input into delay

  //we only use a single tap, and use feedback to have multiple echoes
  Delay.disable(1); Delay.disable(2); Delay.disable(3); Delay.disable(4); Delay.disable(5); Delay.disable(6); Delay.disable(7);
  
  //Delay output is passed thru a low pass filter, set the frequency and resonance of the filter here
  filter.frequency(lowpassFilterCutoff);
  filter.resonance(lowpassFilterResonance);
}

void loop()
{
  if (peak.available())
  {
     prevPeakValue = peak.read() * cvControllableDelayTime;
  }
  
  int upperPotValue = analogRead(upperPotInput);
  int lowerPotValue = analogRead(lowerPotInput);
  if (Button.rose())
  {
    wetFactor += 0.1f;
    if (wetFactor > 1.0f) wetFactor = 0.0f;
  }
  
  OutputMixer.gain(0, 1.0f - wetFactor);
  OutputMixer.gain(1, wetFactor);
  
  Delay.delay(0, int(prevPeakValue + knobControllableDelayTime * (float(upperPotValue) / (float)analogReadMax)));
  float feedbackFactor = (float(lowerPotValue) / (float)analogReadMax) * (1.0f - feedbackMinimum); 
  InputMixer.gain(1, feedbackFactor + feedbackMinimum);
  
}
