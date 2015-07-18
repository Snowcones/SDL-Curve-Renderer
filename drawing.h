//
//  drawing.h
//  WalkingNet
//
//  Created by William Henning on 7/18/15.
//  Copyright (c) 2015 Test. All rights reserved.
//

#ifndef __WalkingNet__drawing__
#define __WalkingNet__drawing__

#include <stdio.h>
#include <SDL2/SDL.h>
#include <vector>

struct trackObj
{
    //Height List is normalized
    std::vector<float> heightList;
    float height;
    float width;
};

const int SCREEN_WIDTH = 640;
const int DRAW_WIDTH = 400;
const int SCREEN_HEIGHT = 480;
const int DRAW_HEIGHT = 400;
const int DOT_WIDTH = 5;
const int BALL_WIDTH = 7;

void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY);
void drawCurve(SDL_Renderer* renderer, trackObj track);
void drawCurves(SDL_Renderer* renderer, std::vector<trackObj> track);
void SDL_DrawStripedCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY, float theta);

#endif /* defined(__WalkingNet__drawing__) */
