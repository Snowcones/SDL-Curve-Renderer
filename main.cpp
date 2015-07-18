#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <sys/time.h>
#include "drawing.h"

const float G = 9.8;

struct trackSimProgress
{
    int segment=0;
    bool running=true;
    float v=0;
    float p=0;
    float t=0;
};

void genArcTrack(float maxDeflection, float width, float height, int res, trackObj& track);
void runSim(SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes);

float getXPosAtAnchor(trackObj& track, int segment);
float getYPosAtAnchor(trackObj& track, int segment);
float getXPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart);
float getYPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart);
float getTrackSegLength(trackObj& track, int seg);

void printTimes(std::vector<float> timeList)
{
    for (int i=0; i<timeList.size(); i++)
    {
        printf("Track %d: %.3fs\n", i, timeList[i]);
    }
}

int main(int argc, const char * argv[]) {

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
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
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
    
    std::vector<trackObj> tracks;
    tracks.resize(11);
    for (int i=0; i<11; i++) {
        float defl=(i-5)/5.0*.2;
        genArcTrack(defl, 40, 40, 20, tracks[i]);
    }
    std::vector<float> times;
    runSim(gRenderer, tracks, 1, 1, 0, true, times);
    printTimes(times);
    
    int testTimer=0;
    
    while (!quit)
    {
        while( SDL_PollEvent(&e) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0);
        SDL_RenderClear( gRenderer );
        SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255);
        drawCurves(gRenderer, tracks);
        SDL_RenderPresent( gRenderer );
    }
    
    
    //Destroy window
    SDL_DestroyWindow( window );
    //Quit SDL subsystems
    SDL_Quit();
    return 0;
}


void runSim(SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes)
{
    //If momentOfInertia is NULL generate it for a cylinder of given radius and mass
    if(!I)
    {
        I=m*r*r/2;
    }
    
    //Make sure trackTimes list is as long as trackList
    trackTimes.resize(trackList.size());
    for (int trackNum=0; trackNum<trackList.size(); trackNum++)
    {
        
        float vi=0;
        float totalTime=0;
        trackObj thisTrack=trackList[trackNum];
        
        for (int point=0; point<thisTrack.heightList.size()-1; point++)
        {
            
            float thisX=(float)point/(thisTrack.heightList.size()-1)*thisTrack.width;
            float thisY=thisTrack.heightList[point]*thisTrack.height;
            
            float nextX=(float)(point+1)/(thisTrack.heightList.size()-1)*thisTrack.width;
            float nextY=thisTrack.heightList[point+1]*thisTrack.height;
            
            float dY=nextY-thisY;
            float dX=nextX-thisX;
            float segLength=sqrtf(dX*dX+dY*dY);
            float accelOnSegment=(dY/segLength)*m*G/(m+I/(r*r));
            
            float radicand=vi*vi-2*accelOnSegment*segLength;
            if (radicand<0)
            {
                printf("Error imaginary solution for ball on slope %d on track %d, Radicand: %f, Accel: %f\n", point, trackNum, radicand, accelOnSegment);
                break;
            }
            
            float time1OnSegment=(vi+sqrtf(radicand))/accelOnSegment;
            float time2OnSegment=(vi-sqrtf(radicand))/accelOnSegment;
            
            float correctTime;
            if (time2OnSegment<0)
            {
                correctTime=time1OnSegment;
            }
            else
            {
                correctTime=time2OnSegment;
            }
            
            totalTime+=correctTime;
            vi+=correctTime*(-1*accelOnSegment);
        }
        
        trackTimes[trackNum]=totalTime;
    }
    if(graphical)
    {
        SDL_Event e;
        int ballsRunning=(int)trackList.size();
        std::vector<trackSimProgress> progList;
        progList.resize(trackList.size());
        float deltaT=1.0/60;
        bool running=true;
        SDL_Texture *display = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    
        while (ballsRunning>0&&running)
        {
            SDL_SetRenderTarget(renderer, display);
            while( SDL_PollEvent(&e) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    running = false;
                }
            }
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect drawRect={(SCREEN_WIDTH-DRAW_WIDTH)/2, (SCREEN_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT};
            //SDL_RenderDrawRect(renderer, &drawRect);
            
            
            for (int trackNum=0; trackNum<trackList.size(); trackNum++)
            {
                if (progList[trackNum].running==true)
                {
                    trackObj thisTrack=trackList[trackNum];
                    trackSimProgress& thisProg=progList[trackNum];
                    int point=thisProg.segment;
                    //float vi=thisProg.v;
                    float tRemaining=deltaT;
                    float ballRealRadius=thisTrack.height/DRAW_HEIGHT*BALL_WIDTH;
                    float ballX=0;
                    float ballY=0;
                    float dY=0;
                    float dX=0;
                    float segLength=0;
                    while (tRemaining>0)
                    {

                        float thisX=getXPosAtAnchor(thisTrack, point);
                        float nextX=getXPosAtAnchor(thisTrack, point+1);
                        
                        float thisY=getYPosAtAnchor(thisTrack, point);
                        float nextY=getYPosAtAnchor(thisTrack, point+1);
                        
                        dY=nextY-thisY;
                        dX=nextX-thisX;
                    
                        segLength=getTrackSegLength(thisTrack, point);
                        float segLengthRemaining=segLength-thisProg.p;
                        
                        //float accelOnSegment;
                        //float time=solveTimeToRollThroughSegment(dX, dY, segLength, m, r, I, G, accelOnSegment);
                        
                        float accelOnSegment=(dY/segLength)*m*G/(m+I/(r*r));
                        float radicand=thisProg.v*thisProg.v-2*accelOnSegment*segLengthRemaining;
                        if (radicand<=0)
                        {
                            thisProg.running=false;
                            ballsRunning--;
                            printf("Error imaginary solution for ball on slope %d on track %d, Radicand: %f, Accel: %f\n", point, trackNum, radicand, accelOnSegment);
                            break;
                        }
                        float time1OnSegment=(thisProg.v+sqrtf(radicand))/accelOnSegment;
                        float time2OnSegment=(thisProg.v-sqrtf(radicand))/accelOnSegment;
                        float correctTime;
                        if (time2OnSegment<0) {
                            correctTime=time1OnSegment;
                        }
                        else {
                            correctTime=time2OnSegment;
                        }
                        if (correctTime<tRemaining)
                        {
                            //printf("correct: %f, remaining: %f\n", correctTime, tRemaining);
                            tRemaining-=correctTime;
                            thisProg.segment++;
                            thisProg.p=0;
                            thisProg.v+=correctTime*(-1*accelOnSegment);
                            thisProg.t+=segLengthRemaining/ballRealRadius;
                            if(thisProg.segment>thisTrack.heightList.size()-2)
                            {
                                thisProg.running=false;
                                ballsRunning--;
                            }
                            
                        }
                        else
                        {
                            float distMoved=thisProg.v*tRemaining-.5*accelOnSegment*tRemaining*tRemaining;
                            thisProg.p+=distMoved;
                            thisProg.t+=distMoved/ballRealRadius;
                            thisProg.v+=tRemaining*(-1*accelOnSegment);
                            tRemaining=0;
                        }
                        
                        thisX=(float)thisProg.segment/(thisTrack.heightList.size()-1)*thisTrack.width;
                        thisY=thisTrack.heightList[thisProg.segment]*thisTrack.height;
                        
                        ballX=(thisProg.p*dX/segLength+thisX)*DRAW_WIDTH/thisTrack.width+(SCREEN_WIDTH-DRAW_WIDTH)/2;
                        ballY=(thisTrack.height-thisProg.p*dY/segLength-thisY)*DRAW_HEIGHT/thisTrack.height+(SCREEN_HEIGHT-DRAW_WIDTH)/2;
                    }
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_DrawStripedCircle(renderer, BALL_WIDTH, ballX-dY/segLength*BALL_WIDTH, ballY-dX/segLength*BALL_WIDTH, thisProg.t);
                }
            }
            
            drawCurves(renderer, trackList);
            
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderCopy(renderer, display, NULL, NULL);
            SDL_RenderPresent(renderer);
            
        }
    }
    
}

float getXPosAtAnchor(trackObj& track, int segment)
{
    return (float)segment/(track.heightList.size()-1)*track.width;
}

float getYPosAtAnchor(trackObj& track, int segment)
{
    return track.heightList[segment]*track.height;
}

float getXPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart)
{
    float xPos;
    float thisX=getXPosAtAnchor(track, segment);
    
    if (distanceFromSegStart==0) //Position is at left height anchor (thisX)
    {
        xPos=thisX;
    }
    else //Position is between anchors solve for it (thisX<pos<nextX)
    {
        float nextX=getXPosAtAnchor(track, segment+1);
        float dX=nextX-thisX;
        float segLength=getTrackSegLength(track, segment);
        xPos=distanceFromSegStart*dX/segLength+thisX;
    }
    
    return xPos;
}

float getYPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart)
{
    float yPos;
    float thisY=track.heightList[segment]*track.height;

    if (distanceFromSegStart==0) //Position is at left height anchor (thisX)
    {
        yPos=thisY;
    }
    else //Position is between anchors solve for it (thisX<pos<nextX)
    {
        float nextY=track.heightList[segment+1]*track.height;
        float dY=nextY-thisY;
        float segLength=getTrackSegLength(track, segment);
        yPos=distanceFromSegStart*dY/segLength+thisY;
    }

    return yPos;
}

float getTrackSegLength(trackObj& track, int seg)
{
    float thisX=getXPosAtAnchor(track, seg);
    float nextX=getXPosAtAnchor(track, seg+1);
    float thisY=getYPosAtAnchor(track, seg);
    float nextY=getYPosAtAnchor(track, seg+1);
    float dX=nextX-thisX;
    float dY=nextY-thisY;
    return sqrtf(dX*dX+dY*dY);
}

void genArcTrack(float maxDeflection, float width, float height, int res, trackObj& track)
{
    if (maxDeflection==0)
    {
        maxDeflection=.001;
    }
    if (width!=height)
    {
        printf("Error arctrack must have width=height\n");
        return;
    }
    track.heightList.resize(res);
    track.width=width;
    track.height=height;
    
    track.heightList[0]=1.0;
    track.heightList[res-1]=0.0;
    for (int i=1; i<res-1; i++)
    {
        float l=sqrtf(2);
        float radius=(l*l/4+maxDeflection*maxDeflection)/(2*maxDeflection);
        float centerXY=.5+(radius-maxDeflection)*(sqrtf(2)/2);
        float xPos=(float)i/(res-1);
        float yPos;
        
        if (maxDeflection>0) {
            yPos=centerXY-sqrtf(radius*radius-powf(centerXY-xPos, 2));
        }
        else {
            yPos=centerXY+sqrtf(radius*radius-powf(centerXY-xPos, 2));
        }
        
        track.heightList[i]=yPos;
    }
}
