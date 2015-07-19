//
//  drawing.cpp
//  WalkingNet
//
//  Created by William Henning on 7/18/15.
//  Copyright (c) 2015 Test. All rights reserved.
//

#include "drawing.h"

void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY)
{
    for (int y=0; y<radius+.5; y++)
    {
        for (int x=0; x<sqrtf(radius*radius-y*y)+.5; x++)
        {
            SDL_RenderDrawPoint(renderer, centerX+x, centerY+y);
            SDL_RenderDrawPoint(renderer, centerX-x, centerY+y);
            SDL_RenderDrawPoint(renderer, centerX+x, centerY-y);
            SDL_RenderDrawPoint(renderer, centerX-x, centerY-y);
        }
    }
}

void SDL_DrawStripedCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY, float theta)
{
    SDL_DrawCircle(renderer, radius, centerX, centerY);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, centerX+radius*cosf(theta), centerY+radius*sinf(theta), centerX-radius*cosf(theta), centerY-radius*sinf(theta));
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}



void drawCurve(SDL_Renderer* renderer, trackObj track)
{
    float distBetweenPointsInPixels=DRAW_WIDTH/(track.heightList.size()-1);
    float drawBorderX=(SCREEN_WIDTH-DRAW_WIDTH)/2;
    float drawBorderY=(SCREEN_HEIGHT-DRAW_HEIGHT)/2;
    //Draw Dots
    for (int i=0; i<track.heightList.size(); i++)
    {
        float pixelX=distBetweenPointsInPixels*i+drawBorderX;
        float pixelY=(1-track.heightList[i])*DRAW_HEIGHT+drawBorderY;
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_DrawCircle(renderer, DOT_WIDTH/2, pixelX, pixelY);
    }
    //Draw Lines
    for (int i=1; i<track.heightList.size(); i++)
    {
        float thisPixelX=distBetweenPointsInPixels*i+drawBorderX;
        float thisPixelY=(1-track.heightList[i])*DRAW_HEIGHT+drawBorderY;
        
        float lastPixelX=distBetweenPointsInPixels*(i-1)+drawBorderX;
        float lastPixelY=(1-track.heightList[i-1])*DRAW_HEIGHT+drawBorderY;
        
        SDL_RenderDrawLine(renderer, thisPixelX, thisPixelY, lastPixelX, lastPixelY);
    }
}

void drawCurves(SDL_Renderer* renderer, std::vector<trackObj> track)
{
    for (int i=0; i<track.size(); i++) {
        drawCurve(renderer, track[i]);
    }
}

