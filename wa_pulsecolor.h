// NeoPixelFunFadeInOut
// This example will randomly pick a color and fade all pixels to that color, then
// it will fade them to black and restart over
// 
// This example demonstrates the use of a single animation channel to animate all
// the pixels at once.

NeoPixelAnimator animator_wait_pulse(PixelCount); // NeoPixel animation management object

uint16_t effectState = 0;  // general purpose variable used to store effect state


// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct MyAnimationState_pulse
{
    RgbColor StartingColor;
    RgbColor EndingColor;
};

// one entry per pixel to match the animation timing manager
MyAnimationState_pulse animationState_pulse[1];

// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
    //Serial.println("BlendAnimUpdate...");
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState_pulse[param.index].StartingColor,
        animationState_pulse[param.index].EndingColor,
        param.progress);

    // apply the color to the strip
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        strip.SetPixelColor(pixel, updatedColor);
    }
}

void PulseAnimUpdate(const AnimationParam& param)
{
    ///Serial.println("PulseAnimUpdate...");
    if (param.state == AnimationState_Completed)
    {
        if (effectState == 0)
        {
            //Serial.println("effectState = 0");
            // Fade upto a random color
            // we use HslColor object as it allows us to easily pick a hue
            // with the same saturation and luminance so the colors picked
            // will have similiar overall brightness
            //RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
            uint16_t time = 1000;
   
            animationState_pulse[0].StartingColor = strip.GetPixelColor(0);
            animationState_pulse[0].EndingColor = RgbColor(20,30,5);
    
            animator_wait_pulse.StartAnimation(0, time, BlendAnimUpdate);
        }
        else if (effectState == 1)
        {
            //Serial.println("effectState = 1");
            // fade to black
            uint16_t time = 1000;

            animationState_pulse[0].StartingColor = strip.GetPixelColor(0);
            animationState_pulse[0].EndingColor = CurrentRgbColor;
    
            animator_wait_pulse.StartAnimation(0, time, BlendAnimUpdate);
        }
    
        // toggle to the next effect state
        effectState = (effectState + 1) % 2;

        // done, time to restart this position tracking animation/timer
        animator_wait_pulse.RestartAnimation(param.index);
    }
}

void FadeInFadeOutRinseRepeat()
{ 
    animator_wait_pulse.StartAnimation(1, 1000, PulseAnimUpdate);
} 
