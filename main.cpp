#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <sys/time.h>

const int SCREEN_WIDTH = 640;
const int DRAW_WIDTH = 400;
const int SCREEN_HEIGHT = 480;
const int DRAW_HEIGHT = 400;
const int DOT_WIDTH = 10;
const int BALL_WIDTH = 7;
const float G = 9.8;

struct trackObj
{
    //Height List is normalized
    std::vector<float> heightList;
    float height;
    float width;
};

struct trackSimProgress
{
    int segment=0;
    bool running=true;
    float v=0;
    float p=0;
    float t=0;
};

void SDL_DrawCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY);
void drawCurve(SDL_Renderer* renderer, trackObj track);
void drawCurves(SDL_Renderer* renderer, std::vector<trackObj> track);
void genArcTrack(float maxDeflection, float width, float height, int res, trackObj& track);
void runSim(SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes);
void SDL_DrawStripedCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY, float theta);


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
    
    //Test data
//    trackObj testTrack;
//    genArcTrack(.3, 1, 1, 20, testTrack);
//    //1-sqrtf(2)/2
    
    std::vector<trackObj> tracks;
    tracks.resize(11);
    for (int i=0; i<11; i++) {
        float defl=(i-5)/5.0*.3;
        genArcTrack(defl, 40, 40, 20, tracks[i]);
    }
    std::vector<float> times;
    times.resize(11);
    runSim(gRenderer, tracks, 1, 1, 0, true, times);
    for (int i=0; i<11; i++) {
        printf("Time %d: %f\n", i, times[i]);
    }
    
    int testTimer=0;
    
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
        
        testTimer++;
        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0);
        SDL_RenderClear( gRenderer );
        
        SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255);
        
        //Draw data
        drawCurves(gRenderer, tracks);
        
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

void SDL_DrawCircle2(SDL_Renderer* renderer, float radius, float centerX, float centerY)
{
    int pixelArea=radius*radius;
    int pixelCount=0;
    SDL_Point* pointData=(SDL_Point*)malloc(sizeof(SDL_Point)*pixelArea);
    for (int y=0; y<radius+.5; y++)
    {
        for (int x=0; x<sqrtf(radius-y*y)+.5; x++)
        {
            pointData[pixelCount]={(int)centerX+x, (int)centerY+y};
            pointData[pixelCount+1]={(int)centerX-x, (int)centerY+y};
            pointData[pixelCount+2]={(int)centerX+x, (int)centerY-y};
            pointData[pixelCount+3]={(int)centerX-x, (int)centerY-y};
            pixelCount+=4;
        }
    }
    SDL_RenderDrawPoints(renderer, pointData, pixelCount);
}

void SDL_DrawCircle3(SDL_Renderer* renderer, float radius, float centerX, float centerY)
{
    int pixelArea=radius*radius;
    int pixelCount=0;
    SDL_Point* pointData=(SDL_Point*)malloc(sizeof(SDL_Point)*pixelArea);
    for (int y=0; y<radius+.5; y++)
    {
        int x=sqrtf(radius*radius-y*y);
        SDL_RenderDrawLine(renderer, -x+centerX, y+centerY, x+centerX, y+centerY);
        SDL_RenderDrawLine(renderer, -x+centerX, -y+centerY, x+centerX, -y+centerY);
    }
    SDL_RenderDrawPoints(renderer, pointData, pixelCount);
}

void SDL_DrawStripedCircle(SDL_Renderer* renderer, float radius, float centerX, float centerY, float theta)
{
    SDL_DrawCircle3(renderer, radius, centerX, centerY);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, centerX+radius*cosf(theta), centerY+radius*sinf(theta), centerX-radius*cosf(theta), centerY-radius*sinf(theta));
    //SDL_RenderDrawLine(renderer, centerX+radius*sinf(theta), centerY-radius*cosf(theta), centerX-radius*sinf(theta), centerY+radius*cosf(theta));
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

void runSim(SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes)
{
    //If momentOfInertia is NULL generate it for a cylinder of given radius and mass
    if(!I)
    {
        I=m*r*r/2;
    }
    
    //Don't draw sim in realtime instead calculate it instantly
//    if (!graphical)
//    {
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
                    //printf("Error imaginary solution for ball on slope %d on track %d, Radicand: %f, Accel: %f\n", point, trackNum, radicand, accelOnSegment);
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
                
                //Test
//                if (trackNum==10) {
//                    //printf("time on segment %d: %f, Vi: %f\n", point, correctTime, vi);
//                    printf("Radicand: %f, Accel: %f, dY/L: %f\n", radicand, accelOnSegment, dY/segLength);
//                    //printf("time1: %f, time2: %f\n", time1OnSegment, time2OnSegment);
//                }
                
                totalTime+=correctTime;
                vi+=correctTime*(-1*accelOnSegment);
            }
            
            trackTimes[trackNum]=totalTime;
        }
//    }
//    else
//    {
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
                        float thisX=(float)point/(thisTrack.heightList.size()-1)*thisTrack.width;
                        float thisY=thisTrack.heightList[point]*thisTrack.height;
                        float nextX=(float)(point+1)/(thisTrack.heightList.size()-1)*thisTrack.width;
                        float nextY=thisTrack.heightList[point+1]*thisTrack.height;
                        dY=nextY-thisY;
                        dX=nextX-thisX;
                        segLength=sqrtf(dX*dX+dY*dY);
                        float segLengthRemaining=segLength-thisProg.p;
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
    //}
    
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
        
        //printf("YPos: %f, XPos: %f, Radicand: %f, Center: %f, Radius: %f\n", yPos, xPos, radius*radius-powf(centerXY-xPos, 2), centerXY, radius);
        track.heightList[i]=yPos;
    }
}












