#include "curveEngine.h"

int main(int argc, const char * argv[])
{
    
    SDL_Window* window=NULL;
    SDL_Renderer* gRenderer=NULL;
    
    //Initialize SDL for video
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "Brachistochrone AI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
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
    
    int width=80; //meters
    int height=40; //meters
    int pointsInTrack=20;
    float initalVelocity=0; //m/s
    bool isGraphical=false; //Decides if simulation will run with animation for run instantly without
    
    std::vector<float> times;
    std::vector<trackObj> tracks;
    tracks.resize(11);
    
    float objectMass=1;
    float objectRadius=1;
    float objectInertia=NULL; //If the moment inertia is NULL the program assumes the object is a cylinder and calculates the moment of inertia from its mass and radius
    
    //First Sim
    for (int t=0; t<11; t++) {
        float deflection=(t-5)/5.0*.2;
        genArcTrack(deflection, width, height, pointsInTrack, tracks[t]);
    }
    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, isGraphical, times, initalVelocity);
    printTimes(times);
    
    //Second Sim
    for (int t=0; t<11; t++)
    {
        float deflection=-.1+t/(10.0);
        genSinTrack(deflection, width, height, pointsInTrack, tracks[t]);
    }
    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, isGraphical, times, initalVelocity);
    printTimes(times);
    
    //Third Sim
    for (int t=0; t<11; t++)
    {
        genRandTrack(width, height, pointsInTrack, tracks[t]);
    }
    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, isGraphical, times, initalVelocity);
    printTimes(times);
    
    //Destroy window
    SDL_DestroyWindow( window );
    //Quit SDL subsystems
    SDL_Quit();
    return 0;
}