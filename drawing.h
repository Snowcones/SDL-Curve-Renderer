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

extern int SCREEN_WIDTH;
extern int DRAW_WIDTH;
extern int SCREEN_HEIGHT;
extern int DRAW_HEIGHT;
extern int DOT_WIDTH;
extern int BALL_WIDTH;

void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY);
void drawCurve(SDL_Renderer* renderer, trackObj track);
void drawCurves(SDL_Renderer* renderer, std::vector<trackObj> track);
void SDL_DrawStripedCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY, float theta);

#endif /* defined(__WalkingNet__drawing__) */