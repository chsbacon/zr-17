//{"sha":"d8e0a43485410962f0709c68d98868511c08dbe7"}

//Standard defines{
float myState[12];
float enState[12];

#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])

#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])

#define SPHERERADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f
//}

//Variable declarations{
//float analyzer[2][6];
bool samplesHeld[3];
float zeroVec[3];
float drillVec[3];
int nextMineSqr[2];
bool isBlue;
float positionTarget[3];
float sampVals[3];
float searchVals[3];
bool pregame;
int pregameDrillSqrs[4][2];
float tempVec[3];
float tempVar;
float vcoef;
int stage;
bool drillUp;
bool keepSearching;
float drillSide;
float spinVec[3];
int difDrill[2];
float endDrillVec[3];

void init(){
    spinVec[0] = 0.0f;
    spinVec[1] = 0.0f;
    spinVec[2] = 0.4f;
    
    vcoef=.154;//A coefficient for our movement speed
    // zeroVec[0]=zeroVec[1]=zeroVec[2]=0;
	memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
	#define SPEEDCONST 0.45f
    #define DERIVCONST 2.8f
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f,0.1f,2.8f);
    
	difDrill[0] = 4;
	difDrill[1] = 4;
	
}
void loop(){
    api.getMyZRState(myState);
    api.getOtherZRState(enState);//Makes sure our data on where they are is up to date
    if(moveDrill(difDrill)) {
        difDrill[0]--;
        difDrill[1]--;
    }
    moveDrill(difDrill);
	//This function is called once per second.  Use it to control the satellite.
}

bool moveDrill(int drillSquare[2]) {
    float drillLoc[3];
    float drillStartVec[3];
    float angleToComplete;
    //float angleToComplete;
    game.square2pos(drillSquare, drillLoc);
    drillLoc[2] = .52;
    memcpy(tempVec, myAtt, 12);
    tempVec[2] = 0;
        
    if((game.getDrills(drillLoc) < 3 && !game.checkSample() == true)^(game.checkSample() == true)) {
        api.setPositionTarget(drillLoc);
        api.setAttRateTarget(zeroVec);
        if(dist(myPos, drillLoc) < .01) {
            api.setVelocityTarget(zeroVec);
            api.setAttitudeTarget(tempVec);  
            if(mathVecMagnitude(myVel, 3) < .01 && angle(tempVec, myAtt) < .19634f) {
                //checks to make sure the sphere is stopped and looking within the right constraints 
                if(game.getDrillEnabled() == false && dist(myRot, zeroVec) < .01f) {
                    //if the drill is off and our sphere is not moving, start drilling
                    game.startDrill();
                    /*memcpy(drillStartVec, myAtt, 12);
                    drillStartVec[2] = 0;
                    mathVecCross(endDrillVec, spinVec, drillStartVec);*/
                    DEBUG(("Drilling..."));
                }
                if(game.getDrillEnabled() == true) {
                    // if the drill is on, spin!!!
                    //angleToComplete = ((2*3.1415) - angle(myAtt, endDrillVec));
                    spinVec[0] = 0;
                    //spinVec[2] = 2*sin((1/3)*angleToComplete);
                    spinVec[1] = 0;
                    spinVec[2] = .4;
                    api.setAttRateTarget(spinVec);
                    //DEBUG(("%f", (180/3.1415)*angleToComplete));
                    DEBUG(("Spinning..."));
                }
                if(game.isGeyserHere(drillSquare)) {
                    //as soon as you find a geyser, turn off the drill
                    game.stopDrill();
                    return false;
                }
                if(game.checkSample() == true) {
                    // see a sample, pick it up
                    game.pickupSample();
                    DEBUG(("Picking up..."));
                    if(game.getNumSamplesHeld() == 3) {
                        //at 3 samples, stop drilling, this is subject to change
                        game.stopDrill();
                        DEBUG(("Stopping Drill..."));
                        return true;
                    }
                }
            }
        }
    }
    else {
        return false; 
    }
}

float dist(float* vec1, float* vec2){
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}
float angle(float* vec1, float* vec2){
    return acosf(mathVecInner(vec1,vec2,3)/(mathVecMagnitude(vec1,3)*mathVecMagnitude(vec2,3)));
}
void scale(float* vec, float scale){
    for (int i=0; i<3; i++)
        vec[i] *= scale;
}
    