#include "curveEngine.h"
#include "drawing.h"

const float G = 9.8;
const float FAIL_VALUE = INFINITY;

struct trackSimProgress
{
    int segment=0;
    bool running=true;
    float v=0;
    float p=0;
    float t=0;
};

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
int DRAW_WIDTH = 400;
int DRAW_HEIGHT = 400;
int BORDER = 100;
int DOT_WIDTH = 5;
int BALL_WIDTH = 7;


void resizeWindow(SDL_Window* window){
    SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void printTimes(std::vector<float> timeList)
{
    for (int i=0; i<timeList.size(); i++)
    {
        printf("Track %d: %.3f s\n", i, timeList[i]);
    }
}

//int main(int argc, const char * argv[]) {
//
//    SDL_Window* window=NULL;
//    SDL_Renderer* gRenderer=NULL;
//    SDL_Event e;
//    bool quit=false;
//
//    //Initialize SDL for video
//    if(SDL_Init(SDL_INIT_VIDEO)<0)
//    {
//        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
//    }
//    else
//    {
//        //Create window
//        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
//        if( window == NULL )
//        {
//            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
//        }
//        else
//        {
//            //Renderer is hardware accelerated and is frame rate capped
//            gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
//
//            //Update the surface
//            SDL_UpdateWindowSurface( window );
//        }
//    }
//
//    int width=80; //meters
//    int height=40; //meters
//    int pointsInTrack=20;
//    float initalVelocity=0; //m/s
//
//    std::vector<float> times;
//    std::vector<trackObj> tracks;
//    tracks.resize(11);
//
//    float objectMass=1;
//    float objectRadius=1;
//    float objectInertia=NULL; //If the moment inertia is NULL the program assumes the object is a cylinder and calculates the moment of inertia from its mass and radius
//
//
//
//    for (int t=0; t<11; t++) {
//        float deflection=(t-5)/5.0*.2;
//        genArcTrack(deflection, width, height, pointsInTrack, tracks[t]);
//    }
//    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, true, times, initalVelocity);
//    printTimes(times);
//
//
//    for (int t=0; t<11; t++)
//    {
//        float deflection=-.1+t/(10.0);
//        genSinTrack(deflection, width, height, pointsInTrack, tracks[t]);
//    }
//    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, true, times, initalVelocity);
//    printTimes(times);
//
//    for (int t=0; t<11; t++)
//    {
//        genRandTrack(width, height, pointsInTrack, tracks[t]);
//    }
//    runSim(window, gRenderer, tracks, objectRadius, objectMass, objectInertia, true, times, initalVelocity);
//    printTimes(times);
//
//    //Destroy window
//    SDL_DestroyWindow( window );
//    //Quit SDL subsystems
//    SDL_Quit();
//    return 0;
//}


void runSim(SDL_Window* window, SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes, float initialVelocity)
{
    float pixelPerUnit=DRAW_HEIGHT/trackList[0].height;
    SCREEN_HEIGHT=pixelPerUnit*trackList[0].height+2*BORDER;
    DRAW_HEIGHT=pixelPerUnit*trackList[0].height;
    SCREEN_WIDTH=pixelPerUnit*trackList[0].width+2*BORDER;
    DRAW_WIDTH=pixelPerUnit*trackList[0].width;
    resizeWindow(window);
    //If momentOfInertia is NULL generate it for a cylinder of given radius and mass
    if(!I)
    {
        I=m*r*r/2;
    }
    
    //Set trackTimes list length to that of trackList
    trackTimes.resize(trackList.size());
    for (int trackNum=0; trackNum<trackList.size(); trackNum++)
    {
        
        float vi=initialVelocity;
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
            float accelOnSeg;
            float time=solveTimeToRollThroughSegment(dX, dY, segLength, vi, m, r, I, G, accelOnSeg);
            if (time==INT_MIN) {
                printf("Imaginary solution for ball on track: %d, slope: %d\n", trackNum, point);
                totalTime=FAIL_VALUE;
                break;
            }
            
            totalTime+=time;
            vi+=time*accelOnSeg;
        }
        
        trackTimes[trackNum]=totalTime;
    }
    if(graphical)
    {
        SDL_Event e;
        int ballsRunning=(int)trackList.size();
        std::vector<trackSimProgress> progList;
        progList.resize(trackList.size());
        for (int i=0; i<trackList.size(); i++) {
            progList[i].v=initialVelocity;
        }
        float deltaT=1.0/60;
        bool running=true;
        
        while (ballsRunning>0&&running)
        {
            while( SDL_PollEvent(&e) != 0 ) {
                if( e.type == SDL_QUIT ) {
                    running = false;
                }
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            //SDL_Rect drawRect={(SCREEN_WIDTH-DRAW_WIDTH)/2, (SCREEN_HEIGHT-DRAW_HEIGHT)/2, DRAW_WIDTH, DRAW_HEIGHT};
            //SDL_RenderDrawRect(renderer, &drawRect);
            
            for (int trackNum=0; trackNum<trackList.size(); trackNum++)
            {
                
                //Update ball
                if (progList[trackNum].running==true)
                {
                    trackObj thisTrack=trackList[trackNum];
                    trackSimProgress& thisProg=progList[trackNum];
                    int point=thisProg.segment;
                    float tRemaining=deltaT;
                    float ballRealRadius=thisTrack.height/DRAW_HEIGHT*BALL_WIDTH;
                    float ballX=0;
                    float ballY=0;
                    float dY=0;
                    float dX=0;
                    float segLengthRemaining=0;
                    while (tRemaining>0)
                    {
                        //printf("Ran, %d\n", progList[trackNum].running);
                        float segLength=getTrackSegLength(thisTrack, point);
                        segLengthRemaining=segLength-thisProg.p;
                        if (segLengthRemaining<0) {
                            printf("Ran Save Feature\n");
                            thisProg.segment++;
                            thisProg.p=fabsf(segLengthRemaining);
                            segLength=getTrackSegLength(thisTrack, point);
                            segLengthRemaining=segLength-thisProg.p;
                        }
                        
                        
                        point=thisProg.segment;
                        float thisX=getXPosAtAnchor(thisTrack, point);
                        float nextX=getXPosAtAnchor(thisTrack, point+1);
                        
                        float thisY=getYPosAtAnchor(thisTrack, point);
                        float nextY=getYPosAtAnchor(thisTrack, point+1);
                        
                        //Distance from current ball position to end of segment
                        dY=(nextY-thisY)*segLengthRemaining/segLength;
                        dX=(nextX-thisX)*segLengthRemaining/segLength;
                        
                        //Finds time to end of segment
                        float accelOnSegment;
                        float time=solveTimeToRollThroughSegment(dX, dY, segLengthRemaining, thisProg.v, m, r, I, G, accelOnSegment);
                        if (time==INT_MIN) {
                            printf("Error imaginary solution for ball on track: %d, slope: %d\n", trackNum, point);
                            thisProg.running=false;
                            ballsRunning--;
                            break;
                        }
                        
                        //Ball crosses segment
                        if (time<tRemaining)
                        {
                            tRemaining-=time;
                            thisProg.segment++;
                            thisProg.p=0;
                            thisProg.v+=time*accelOnSegment;
                            thisProg.t+=segLengthRemaining/ballRealRadius;
                            //Ball finished track
                            if(thisProg.segment>thisTrack.heightList.size()-2)
                            {
                                //printf("Finished\n");
                                thisProg.running=false;
                                ballsRunning--;
                                break;
                            }
                        }
                        //Ball doesn't cross segment
                        else
                        {
                            float distMoved=thisProg.v*tRemaining-.5*accelOnSegment*tRemaining*tRemaining;
                            thisProg.p+=distMoved;
                            thisProg.t+=distMoved/ballRealRadius;
                            thisProg.v+=tRemaining*accelOnSegment;
                            tRemaining=0;
                        }
                        
                        thisX=getXPosAtAnchor(thisTrack, thisProg.segment);
                        thisY=getYPosAtAnchor(thisTrack, thisProg.segment);
                        
                        ballX=(thisProg.p*dX/segLengthRemaining+thisX)*DRAW_WIDTH/thisTrack.width+(SCREEN_WIDTH-DRAW_WIDTH)/2;
                        ballY=(thisTrack.height-thisProg.p*dY/segLengthRemaining-thisY)*DRAW_HEIGHT/thisTrack.height+(SCREEN_HEIGHT-DRAW_HEIGHT)/2;
                        
                        segLength=getTrackSegLength(thisTrack, point);
                        segLengthRemaining=segLength-thisProg.p;
                        
                        dY=(nextY-thisY)*segLengthRemaining/segLength;
                        dX=(nextX-thisX)*segLengthRemaining/segLength;
                    }
                    
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_DrawStripedCircle(renderer, BALL_WIDTH, ballX-dY/segLengthRemaining*BALL_WIDTH, ballY-dX/segLengthRemaining*BALL_WIDTH, thisProg.t);
                }
            }
            
            drawCurves(renderer, trackList);
            SDL_RenderPresent(renderer);
            
        }
    }
    
}

float solveTimeToRollThroughSegment(float dX, float dY, float segLength, float vi, float m, float r, float I, float G, float& accel)
{
    accel=-(dY/segLength)*m*G/(m+I/(r*r));
    float radicand=vi*vi+2*accel*segLength;
    if (radicand<0)
    {
        return INT_MIN;
    }
    float time1OnSegment=(vi+sqrtf(radicand))/-accel;
    float time2OnSegment=(vi-sqrtf(radicand))/-accel;
    float correctTime;
    if (time2OnSegment<0)
    {
        correctTime=time1OnSegment;
    }
    else
    {
        correctTime=time2OnSegment;
    }
    //printf("V: %f, R: %f, A: %f, L: %f, T1: %f, T2: %f\n", vi, radicand, accel, segLength, time1OnSegment, time2OnSegment);
    return correctTime;
}

float getXPosAtAnchor(trackObj& track, int segment){
    return (float)segment/(track.heightList.size()-1)*track.width;
}

float getYPosAtAnchor(trackObj& track, int segment){
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
    maxDeflection*=-1;
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

void genRandTrack(float width, float height, int res, trackObj& track)
{
    track.heightList.resize(res);
    track.width=width;
    track.height=height;
    
    track.heightList[0]=1.0;
    track.heightList[res-1]=0.0;
    for (int i=1; i<res-1; i++)
    {
        track.heightList[i]=(float)rand()/INT_MAX*.8+.1;
    }
}

void genEmptyTrack(float width, float height, int res, trackObj& track)
{
    
    track.heightList.resize(res);
    track.width=width;
    track.height=height;
    
    track.heightList[0]=1.0;
    track.heightList[res-1]=0.0;
}

void genCustTrack(float width, float height, trackObj& track, std::vector<float> heights)
{
    track.heightList.resize(heights.size()+2);
    track.width=width;
    track.height=height;
    
    track.heightList[0]=1.0;
    track.heightList[heights.size()]=0.0;
    for (int i=0; i<heights.size(); i++)
    {
        track.heightList[i+1]=heights[i];
    }
}

void genSinTrack(float deflection, float width, float height, int res,trackObj& track)
{
    genEmptyTrack(width, height, res, track);
    
    if (deflection>1||deflection<-.115)
    {
        printf("Warning, A ball can only roll down sin track with deflection approximately -.155<deflection<1\n");
    }
    deflection*=.73;
    for (int i=1; i<res-1; i++)
    {
        float val=i/(res-1.0);
        track.heightList[i]=-sin(val*2*M_PI)*deflection+1-val;
    }
}
