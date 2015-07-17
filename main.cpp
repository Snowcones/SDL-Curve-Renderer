#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>

const int SCREEN_WIDTH = 640;
const int DRAW_WIDTH = 600;
const int SCREEN_HEIGHT = 480;
const int DRAW_HEIGHT = 400;
const int DOT_WIDTH = 20;
const int LINE_WIDTH = 5;
void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY);
void drawCurve(SDL_Renderer* renderer, std::vector<float> points);

int main(int argc, const char * argv[]) {
    //The window we render to
    SDL_Window* window=NULL;
    SDL_Renderer* gRenderer=NULL;
    SDL_Event e;
    bool quit=false;
    
    //Initialize SDL for video
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Renderer is hardware accelerated and is frame rate capped
            gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
            
            //Update the surface
            SDL_UpdateWindowSurface( window );
        }
    }
    
    while (!quit)
    {
        //Handle events on queue
        while( SDL_PollEvent(&e) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }
        
        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0);
        SDL_RenderClear( gRenderer );
        
        std::vector<float> points;
        points.resize(21);
        for (int i=0; i<points.size(); i++)
        {
            float x=i/20.0;
            points[i]=sinf(2*M_PI*x)/2+.5;
        }
        
        drawCurve(gRenderer, points);
        
        //Update screen
        SDL_RenderPresent( gRenderer );
    }
    
    //Destroy window
    SDL_DestroyWindow( window );
    
    //Quit SDL subsystems
    SDL_Quit();
    
    return 0;
}

void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY)
{
    for (int y=0; y<radius+.5; y++)
    {
        for (int x=0; x<sqrtf(radius-y*y)+.5; x++)
        {
            SDL_RenderDrawPoint(renderer, centerX+x, centerY+y);
            SDL_RenderDrawPoint(renderer, centerX-x, centerY+y);
            SDL_RenderDrawPoint(renderer, centerX+x, centerY-y);
            SDL_RenderDrawPoint(renderer, centerX-x, centerY-y);
        }
    }
}

void drawCurve(SDL_Renderer* renderer, std::vector<float> points)
{
    float distBetweenPointsInPixels=DRAW_WIDTH/points.size();
    float drawBorderX=(SCREEN_WIDTH-DRAW_WIDTH)/2;
    float drawBorderY=(SCREEN_HEIGHT-DRAW_HEIGHT)/2;
    //Draw Dots
    for (int i=0; i<points.size(); i++)
    {
        float pixelX=distBetweenPointsInPixels*i+drawBorderX;
        float pixelY=(1-points[i])*DRAW_HEIGHT+drawBorderY;
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_DrawCircle(renderer, DOT_WIDTH/2, pixelX, pixelY);
    }
    //Draw Lines
    for (int i=1; i<points.size(); i++)
    {
        float thisPixelX=distBetweenPointsInPixels*i+drawBorderX;
        float thisPixelY=(1-points[i])*DRAW_HEIGHT+drawBorderY;
        
        float lastPixelX=distBetweenPointsInPixels*(i-1)+drawBorderX;
        float lastPixelY=(1-points[i-1])*DRAW_HEIGHT+drawBorderY;
        
        SDL_RenderDrawLine(renderer, thisPixelX, thisPixelY, lastPixelX, lastPixelY);
    }
}










