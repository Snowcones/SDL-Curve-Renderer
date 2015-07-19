#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include "drawing.h"
#include <iostream>
#include <sys/time.h>

void genArcTrack(float maxDeflection, float width, float height, int res, trackObj& track);
void genRandTrack(float width, float height, int res, trackObj& track);
void genEmptyTrack(float width, float height, int res, trackObj& track);
void genSinTrack(float deflection, float width, float height, int res,trackObj& track);

void runSim(SDL_Window* windown, SDL_Renderer* renderer, std::vector<trackObj>& trackList, float r, float m, float I, bool graphical, std::vector<float>& trackTimes, float initialVelocity);
float solveTimeToRollThroughSegment(float dX, float dY, float vi, float segLength, float m, float r, float I, float G, float& accel);

float getXPosAtAnchor(trackObj& track, int segment);
float getYPosAtAnchor(trackObj& track, int segment);
float getXPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart);
float getYPosAlongTrack(trackObj& track, int segment, float distanceFromSegStart);
float getTrackSegLength(trackObj& track, int seg);

void resizeWindow(SDL_Window* window);
void printTimes(std::vector<float> timeList);