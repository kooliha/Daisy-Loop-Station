#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
Switch       Ch1Button;

#define kBuffSize 48000 * 5 // 5 seconds at 48kHz

// Loopers and the buffers they'll use
Looper              looper_l;
Looper              looper_r;
float DSY_SDRAM_BSS buffer_l[kBuffSize];
float DSY_SDRAM_BSS buffer_r[kBuffSize];



void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{

    Ch1Button.Debounce();


    //if you press the button, toggle the record state
    if(Ch1Button.RisingEdge())
    {
        looper_l.TrigRecord();
        looper_r.TrigRecord();
    }

    // if you hold the button longer than 1000 ms (1 sec), clear the loop
    if(Ch1Button.TimeHeldMs() >= 1000.f)
    {
        looper_l.Clear();
        looper_r.Clear();
    }

    // Set the led to 5V if the looper is recording
   // patch.WriteCvOut(2, 5.f * looper_l.Recording());

    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        // store the inputs * the input gain factor
        float in_l = IN_L[i];
        float in_r = IN_R[i];

        // store signal = loop signal * loop gain + in * in_gain
        float sig_l = looper_l.Process(in_l);
        float sig_r = looper_r.Process(in_r);

        // send that signal to the outputs
        OUT_L[i] = sig_l;
        OUT_R[i] = sig_r;
    }
}

int main(void)
{
    // Initialize the hardware
    hw.Configure();
    hw.Init();

    // Init the loopers
    looper_l.Init(buffer_l, kBuffSize);
    looper_r.Init(buffer_r, kBuffSize);

    // Init the button
    Ch1Button.Init(hw.GetPin(28), 1000);

    // Start the audio callback
    hw.StartAudio(AudioCallback);

    // loop forever
    while(1) {}
}

