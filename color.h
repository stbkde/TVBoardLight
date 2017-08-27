/******************************* Color Fade In/Out ** ********************************/

void SetupAnimationSet()
{
    // setup some animations
    for (uint8_t pixel = 0; pixel < PixelCount; pixel++)
    {
        const uint8_t peak = 128;

        // pick a random duration of the animation for this pixel
        // since values are centiseconds, the range is 1 - 4 seconds
        uint16_t time = random(100, 400);

        // each animation starts with the color that was present
        RgbwColor originalColor = strip.GetPixelColor(pixel);
        // and ends with a random color
        // RgbwColor targetColor = RgbwColor(RgbColor(random(peak), random(peak), random(peak)) );
        RgbwColor targetColor = RgbwColor(150, random(peak), random(peak), random(peak));
        // with the random ease function
        AnimEaseFunction easing;

        switch (random(3))
        {
        case 0:
            easing = NeoEase::CubicIn;
            break;
        case 1:
            easing = NeoEase::CubicOut;
            break;
        case 2:
            easing = NeoEase::QuadraticInOut;
            break;
        }

#ifdef ARDUINO_ARCH_AVR
        // each animation starts with the color that was present
        animationState[pixel].StartingColor = originalColor;
        // and ends with a random color
        animationState[pixel].EndingColor = targetColor;
        // using the specific curve
        animationState[pixel].Easeing = easing;

        // now use the animation state we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations_PixelCount.StartAnimation(pixel, time, AnimUpdate);
#else
        // ESP
        // we must supply a function that will define the animation, in this example
        // we are using "lambda expression" to define the function inline, which gives
        // us an easy way to "capture" the originalColor and targetColor for the call back.
        //
        // this function will get called back when ever the animation needs to change
        // the state of the pixel, it will provide a animation progress value
        // from 0.0 (start of animation) to 1.0 (end of animation)
        //
        // we use this progress value to define how we want to animate in this case
        // we call RgbColor::LinearBlend which will return a color blended between
        // the values given, by the amount passed, hich is also a float value from 0.0-1.0.
        // then we set the color.
        //
        // There is no need for the MyAnimationState struct as the compiler takes care
        // of those details for us
        AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
        {
            // progress will start at 0.0 and end at 1.0
            // we convert to the curve we want
            float progress = easing(param.progress);

            // use the curve value to apply to the animation
            RgbwColor updatedColor = RgbwColor::LinearBlend(originalColor, targetColor, progress);
            strip.SetPixelColor(pixel, RgbColorMorning);
            //strip.SetPixelColor(pixel, updatedColor);
        };

        // now use the animation properties we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations_PixelCount.StartAnimation(pixel, time, animUpdate);
#endif
    }
}
/*
void FadeToColor(uint16_t time, RgbwColor targetColor) //since values are centiseconds, 1000 = 10 seconds
{
    LastBrightnessState = strip.GetBrightness();
  
    Serial.println("Fading to R:"+String(targetColor.R)+", G:"+String(targetColor.G)+", B:"+String(targetColor.B));

    if (targetColor != RgbColor(0) && LastRgbwColor == RgbColor(0)) { // on hard reset ?
        LastRgbColor = targetColor;
    }
    CurrentRgbwColor = targetColor;
    
    AnimEaseFunction easing = NeoEase::Linear;
  
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
      RgbColor originalColor = strip.GetPixelColor(pixel); // each animation starts with the color that was present
      AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
      {
          // progress will start at 0.0 and end at 1.0
          // we convert to the curve we want
          float progress = easing(param.progress);
  
          // use the curve value to apply to the animation
          RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
          strip.SetPixelColor(pixel, updatedColor);
      };
      
      // animations.StartAnimation(pixel, time, animUpdate);
      animations_PixelCount.StartAnimation(pixel, time/2 +(pixel*time)/PixelCount/2, animUpdate); // Do not update all pixels at once but the leftmost twice as fast
    }

    // if someone/what changed only the brightness, set now
    if (NextBrightnessState > 0) {
        strip.SetBrightness(NextBrightnessState);
        NextBrightnessState = 0;
    }
    
    CurrentBrightnessState = strip.GetBrightness();
}
*/
void FadeToRgbColor(uint16_t time, RgbColor targetColor) //since values are centiseconds, 1000 = 10 seconds
{
    LastBrightnessState = strip.GetBrightness();
  
    Serial.println("Fading to R:"+String(targetColor.R)+", G:"+String(targetColor.G)+", B:"+String(targetColor.B));

    if (targetColor != RgbColor(0) && LastRgbColor == RgbColor(0)) { // on hard reset ?
        LastRgbColor = targetColor;
    }
    CurrentRgbColor = targetColor;
    
    AnimEaseFunction easing = NeoEase::Linear;
  
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
      RgbColor originalColor = strip.GetPixelColor(pixel); // each animation starts with the color that was present
      AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
      {
          // progress will start at 0.0 and end at 1.0
          // we convert to the curve we want
          float progress = easing(param.progress);
  
          // use the curve value to apply to the animation
          RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
          strip.SetPixelColor(pixel, updatedColor);
      };
      
      // animations.StartAnimation(pixel, time, animUpdate);
      animations_PixelCount.StartAnimation(pixel, time/2 +(pixel*time)/PixelCount/2, animUpdate); // Do not update all pixels at once but the leftmost twice as fast
    }

    // if someone/what changed only the brightness, set now
    if (NextBrightnessState > 0) {
        strip.SetBrightness(NextBrightnessState);
        NextBrightnessState = 0;
    }
    
    CurrentBrightnessState = strip.GetBrightness();
}

void FadeToHexColor(uint16_t time, String hexstring) // hexstring is a string like #ff0000 for red; since values are centiseconds, 1000 = 10 seconds
{
    long number = (long) strtol( &hexstring[1], NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;
    RgbColor targetColor = RgbColor(r, g, b);

    FadeToRgbColor(time, targetColor);
} 

void FadeToRgbwColor(uint16_t time, RgbwColor targetColor) //since values are centiseconds, 1000 = 10 seconds
{
    LastBrightnessState = strip.GetBrightness();
  
    Serial.println("Fading to RGBW: "+targetColor.toString(coldiv));

    if (targetColor != RgbwColor(0) && LastRgbwColor == RgbwColor(0)) { // on hard reset ?
        LastRgbwColor = targetColor;
    }
    CurrentRgbwColor = targetColor;
    
    AnimEaseFunction easing = NeoEase::Linear;
  
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
      RgbwColor originalColor = strip.GetPixelColor(pixel); // each animation starts with the color that was present
      AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
      {
          // progress will start at 0.0 and end at 1.0
          // we convert to the curve we want
          float progress = easing(param.progress);
  
          // use the curve value to apply to the animation
          RgbwColor updatedColor = RgbwColor::LinearBlend(originalColor, targetColor, progress);
          //strip.SetPixelColor(pixel, updatedColor);
      };
      
      // animations.StartAnimation(pixel, time, animUpdate);
      animations_PixelCount.StartAnimation(pixel, time/2 +(pixel*time)/PixelCount/2, animUpdate); // Do not update all pixels at once but the leftmost twice as fast
    }

    // if someone/what changed only the brightness, set now
    if (NextBrightnessState > 0) {
        strip.SetBrightness(NextBrightnessState);
        NextBrightnessState = 0;
    }
    
    CurrentBrightnessState = strip.GetBrightness();
}


#ifdef ARDUINO_ARCH_AVR
// for AVR, you need to manage the state due to lack of STL/compiler support
// for Esp8266 you can define the function using a lambda and state is created for you
// see below for an example
struct MyRgbAnimationState
{
    RgbColor StartingColor;  // the color the animation starts at
    RgbColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

struct MyRgbwAnimationState
{
    RgbwColor StartingColor;  // the color the animation starts at
    RgbwColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

MyAnimationState animationState[PixelCount];
// one entry per pixel to match the animation timing manager


void AnimUpdate(const AnimationParam& param)
{
    // first apply an easing (curve) to the animation
    // this simulates acceleration to the effect
    float progress = animationState[param.index].Easeing(param.progress);

    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbwColor updatedColor = RgbwColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        progress);
    // apply the color to the strip
    strip.SetPixelColor(param.index, updatedColor);
}
#endif
