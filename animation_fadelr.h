// uncomment one of the lines below to see the effects of
// changing the ease function on the movement animation
AnimEaseFunction moveEase_FadeLR =
      NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
//      NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
//      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;

/** Fade-Animation **/
uint16_t lastPixel = 0; // track the eye position
int8_t moveDir = 1; // track the direction of movement


/**************************** Fade LR-Animation ***************************/

void FadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}

void FadeAnimUpdate(const AnimationParam& param)
{
    if (param.state == AnimationState_Completed)
    {
        FadeAll(10);
        animations_2.RestartAnimation(param.index);
    }
}

void MoveAnimUpdate(const AnimationParam& param)
{
    // apply the movement animation curve
    float progress = moveEase_FadeLR(param.progress);

    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    if (moveDir > 0)
    {
        nextPixel = progress * PixelCount;
    }
    else
    {
        nextPixel = (1.0f - progress) * PixelCount;
    }

    // if progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and
    // the last
    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + moveDir; i != nextPixel; i += moveDir)
        {
            strip.SetPixelColor(i, CylonEyeColor);
            
            /*for( int y=i-12; y<=i+12; y++) {
                strip.SetPixelColor(y, CylonEyeColor);
            }*/
        }
    }

    int pcount = 12;
    int plen = 12; // = 0
    /*Serial.print("nextPixel = ");
    Serial.println(nextPixel);
    
    if (nextPixel < PixelCount-pcount) {
        Serial.println("1");
        plen = pcount - (PixelCount-nextPixel);
        Serial.print("plen = ");
        Serial.println(plen);
    }
    else if (nextPixel < pcount) {
        plen = nextPixel;
        Serial.println("2");
        Serial.print("plen = ");
        Serial.println(plen);
    }
    else {
        Serial.println("3");
        plen = pcount;
        Serial.print("plen = ");
        Serial.println(plen);
    }*/
    
    for (int y=nextPixel; y<=nextPixel+plen; y++) {
        strip.SetPixelColor(y, CylonEyeColor);
    }
    
    //strip.SetPixelColor(nextPixel, CylonEyeColor);

    lastPixel = nextPixel;

    if (param.state == AnimationState_Completed)
    {
        // reverse direction of movement
        moveDir *= -1;

        // done, time to restart this position tracking animation/timer
        animations_2.RestartAnimation(param.index);
    }
}

void Setup_FadeLR_Animation()
{
    // fade all pixels providing a tail that is longer the faster
    // the pixel moves.
    animations_2.StartAnimation(0, 10, FadeAnimUpdate);

    // take several seconds to move eye fron one side to the other
    animations_2.StartAnimation(1, 2000, MoveAnimUpdate);
}
