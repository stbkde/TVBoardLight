#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h> // instead of NeoPixelBus.h
#include <NeoPixelAnimator.h>

#define FASTLED_ALLOW_INTERRUPTS 0


//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);

// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
//
// NeoEsp8266Uart800KbpsMethod uses GPI02 instead

NeoPixelAnimator animations_PixelCount(PixelCount); // NeoPixel animation management object
NeoPixelAnimator animations_2(2); 

//NeoPixelAnimator animations(PixelCount, NEO_CENTISECONDS);
//String currentAnimation = "";

RgbColor PixelStripState[PixelCount]; // later

bool WaitingAnimationRunning = false;

/*
 * Save the complete state of an strip to restore
 * it after animation
 */
void SavePixelStripState()
{
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        PixelStripState[indexPixel] = strip.GetPixelColor(indexPixel);
    }
}

/*
 * For some animations for need "real" random
 */
void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    // Serial.println(seed);
    randomSeed(seed);
}
