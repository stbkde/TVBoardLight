
/******************************* Brightness Fade **********************************/

void SetupAnimationBrightFade()
{
    // setup some animations
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        const uint8_t peak = 128;

        // pick a random duration of the animation for this pixel
        // since values are centiseconds, the range is 1 - 4 seconds
        uint16_t time = random(100, 400);

        // each animation starts with the color that was present
        RgbColor originalColor = strip.GetPixelColor(pixel);
        // and ends with a random color
        RgbColor targetColor = RgbColor(random(peak), random(peak), random(peak));
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
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, targetColor, progress);
            strip.SetPixelColor(pixel, updatedColor);
        };

        // now use the animation properties we just calculated and start the animation
        // which will continue to run and call the update function until it completes
        animations_PixelCount.StartAnimation(pixel, time, animUpdate);
#endif
    }
}

uint16_t lastbright = 0;

void FadeToBrightness(uint16_t time, int targetBrightness) //since values are centiseconds, 1000 = 10 seconds
{
    if (targetBrightness < c_MinBrightness) {
        targetBrightness = c_MinBrightness;
    }
    uint16_t originalBrightness = strip.GetBrightness();

    AnimEaseFunction easing = NeoEase::Linear;
  
      AnimUpdateCallback animUpdate = [=](const AnimationParam& param)
      {
          // we convert to the curve we want
          // progress will start at 0.0 and end at 1.0
          float progress = easing(param.progress); 
          //Serial.println();
          //Serial.println("BrightFade: progress = "+String(progress));
          if (progress == 0.00) {
              progress = 0.01;
          }
          int updatedBrightness;
          if (originalBrightness < targetBrightness) {  // fading up
              //Serial.println("Fading up");
              updatedBrightness = (progress * (originalBrightness - targetBrightness));// + originalBrightness;
              updatedBrightness = ~updatedBrightness + 1; // +/-
              //Serial.println("BrightFade: Step: "+String(updatedBrightness)); 
              updatedBrightness += originalBrightness;
              //updatedBrightness *= -1;
          }
          else if (originalBrightness > targetBrightness) {  // fading down
              //Serial.println("Fading down");
              updatedBrightness = (progress * (targetBrightness - originalBrightness));// + originalBrightness;
              updatedBrightness = ~updatedBrightness + 1; // +/-
              //Serial.println("BrightFade: Step: "+String(updatedBrightness)); 
              updatedBrightness -= originalBrightness;
              updatedBrightness = ~updatedBrightness + 1;
              //updatedBrightness *= -1;
          }

          Serial.println("BrightFade: originalBrightness = "+String(originalBrightness));
          Serial.println("BrightFade: targetBrightness = "+String(targetBrightness));
          Serial.println("BrightFade: strip.GetBrightness() = "+String(strip.GetBrightness()));
          Serial.println("BrightFade: updatedBrightness = "+String(updatedBrightness));
          //Serial.println();
          
          if(strip.GetBrightness() != updatedBrightness) {
              //Serial.println("BrightFade: Setting brightness to "+String(updatedBrightness));
              strip.SetBrightness(updatedBrightness);
          }
          // use the curve value to apply to the animation
      };
      
      animations_PixelCount.StartAnimation(1, time, animUpdate);
      //animations_PixelCount.StartAnimation(pixel, time/2 +(pixel*time)/PixelCount/2, animUpdate); // Do not update all pixels at once but the leftmost twice as fast

}


