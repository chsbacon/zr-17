

#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f",str, vec[0], vec[1], vec[2]))
#define PRINT_VEC_I(str, vec) DEBUG(("%s %d %d %d",str, vec[0], vec[1], vec[2]))
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
    spinVec[2] = 0.5f;
    
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
    if(moveDrill(difDrill) && game.getNumSamplesHeld() == 2) {
        difDrill[0]--;
        difDrill[1]--;
    }
    moveDrill(difDrill);
    
    //PRINT_VEC_F("positionTarget", positionTarget);
	// Movement code
	float distance,flocal,fvector[3];
    #define ACCEL 0.0175f
    mathVecSubtract(fvector, positionTarget, myPos, 3); // vector from us to the target
    distance = mathVecNormalize(fvector, 3); // distance to target
    if (distance > 0.05f) { // If not close, pick a velocity
        flocal = vcoef;
        if (flocal*flocal/ACCEL>distance-.02f){//Cap on how fast we go
            flocal = sqrtf(distance*ACCEL)-.02f;
            //DEBUG(("Slower"));
        }
        scale(fvector, flocal);
        api.setVelocityTarget(fvector);
    }
    else {// if we are very close
        api.setPositionTarget(positionTarget);
    }
  
	//This function is called once per second.  Use it to control the satellite.
}

bool moveDrill(int drillSquare[2]) {
    float drillLoc[3];
    
    #define SATELLITE_DRILL_LOOK_ANGLE .19634f
    #define VELOCITY_STOPPED 0.01f
    #define POSITION_CLOSE 0.01f
    #define ROTATION_STOPPED 0.01f
    #define MAX_DRILLS_ON_SQUARE 4
    #define FLOOR_DRILL_HEIGHT .34f
    #define SIZE_FLOAT 4
    #define MAX_SAMPS 5f
    
    game.square2pos(drillSquare, drillLoc);
    drillLoc[2] = FLOOR_DRILL_HEIGHT;
    memcpy(tempVec, myAtt, 12);
    tempVec[2] = 0;
    
    if(game.checkSample()) {
        // see a sample, pick it up
        game.pickupSample();
        DEBUG(("Picking up..."));
        return true;
    }
    if(game.getDrills(drillLoc) < MAX_DRILLS_ON_SQUARE && game.getNumSamplesHeld() <= MAX_SAMPS) {
        memcpy(positionTarget, drillLoc, 3*SIZE_FLOAT);
        api.setAttitudeTarget(tempVec); 
        //sets moving algorithm
        if(dist(myPos, drillLoc) < POSITION_CLOSE) {
            api.setVelocityTarget(zeroVec);
            api.setAttRateTarget(zeroVec); 
            //stop all satellite movement
            if(mathVecMagnitude(myVel, 3) < VELOCITY_STOPPED and angle(tempVec, myAtt) < SATELLITE_DRILL_LOOK_ANGLE) {
                //checks to make sure the sphere is stopped and looking within the right constraints 
                if(!game.getDrillEnabled() and dist(myRot, zeroVec) < ROTATION_STOPPED) {
                    //if the drill is off and our sphere is not moving, start drilling
                    game.startDrill();
                    DEBUG(("Drilling..."));
                }
                if(game.getDrillEnabled()) {
                    // if the drill is on, spin!!!
                    api.setAttRateTarget(spinVec);
                    DEBUG(("Spinning..."));
                }
                if(game.isGeyserHere(drillSquare)) {
                    //as soon as you find a geyser, turn off the drill
                    game.stopDrill();
                    return false;
                }
            }
        }
        else if(game.getDrillEnabled()) {
            game.stopDrill();
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
    