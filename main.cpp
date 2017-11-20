
// #define dev
//Standard defines
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])

#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])

#define mathSquare(num) num*num

int mySquare[3];

#define SPHERE_RADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f

// DEBUG shorthand functions (F is for floats, I is for ints)
#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f",str, vec[0], vec[1], vec[2]))

float enScore;
bool infoFound; // have we gotten a 3, 6, or a 10?

#define TEN_SPAWN_WIDTH 12
#define TEN_SPAWN_HEIGHT 16
char possibleTenSquares[TEN_SPAWN_WIDTH][TEN_SPAWN_HEIGHT]; // stores whether
    // or not each square could be a ten
    // '*' : possible ten spot
    // 'x' : impossible ten spot

float vcoef; // A coefficient for our movement speed
int drillSquare[2]; // Will eventually store the optimal drilling square
    // It's important that this is global because sometimes it doesn't
    // get updated
void init() {
    drillSquare[0] = 1;
    drillSquare[1] = 1;
    infoFound = false;
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f, 0.1f, 2.8f);
    vcoef = 0.154f; 
    
    enScore = 0.0f; // initialized because
        // we use it to calculate change in score
    


    memset(possibleTenSquares, '*', TEN_SPAWN_HEIGHT * TEN_SPAWN_WIDTH);
        // at the start, all squares are possible tens
        // including the center lmao, but we ignore it
}

void loop() {
    float zeroVec[3] = {0.0, 0.0, 0.0}; // just a convenience thing
    float positionTarget[3]; // where we're going
    // Update state
    float myState[12];
    float enState[12];
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	int myFuel = game.getFuelRemaining()*100;
	enScore = game.getOtherScore();
	int sampNum = game.getNumSamplesHeld();
	game.pos2square(myPos,mySquare);
	bool geyserOnMe;
    geyserOnMe=game.isGeyserHere(mySquare);
    
    float drillSquarePos[3];
    game.square2pos(drillSquare,drillSquarePos);
    
    //checking fuel to see if we need to go back to base
    float magnitude = sqrtf(mathSquare(myPos[0]) + mathSquare(myPos[1]) + mathSquare(-0.1f-myPos[2])); //finding magnitude
    DEBUG(("Magnitude is %f", magnitude));
    float fuel2base = magnitude*(22*(mathSquare(.6*magnitude - .9)));//multiplying magnitude by fuel2square algorithm
    DEBUG(("The fuel to get back to the base is %f",fuel2base));


    
    int tenLoc[2];
    tenLoc[0] = 7; //just for testing
    tenLoc[1] = 7;
    
    float squarePos[3];
    float shortestDist = 10000;
    float secondShortestDist = 100000;
    float tenPos[3];
    game.square2pos(tenLoc, tenPos);
    int bestSix[2][2];
    bestSix[0][0] = 0;
    bestSix[0][1] = 0;
    bestSix[1][0] = 0;
    bestSix[1][1] = 0;
    tenPos[2] = game.getTerrainHeight(tenLoc);
    for(int i = -1; i<2; i++) {
        for(int j = -1; j<2;j++) {
            int square[2];
            square[0] = i+tenLoc[0];
            square[1] = j+tenLoc[1];
            if(square[0] == 0){
                square[0] += i;
            }
            if(square[1] == 0){
                square[1] += i;
            }
            //goes through each square around the 10 and finds the tallest ont (not complete)
            game.square2pos(square, squarePos);
            squarePos[2] = game.getTerrainHeight(square);
            if(game.getDrills(square) < 2 and shortestDist > dist(squarePos, tenPos) + mathVecMagnitude(squarePos, 3) and dist(squarePos, tenPos) > 0.03f){
                bestSix[0][0] = square[0];
                bestSix[0][1] = square[1];
                shortestDist = dist(squarePos, tenPos) + mathVecMagnitude(squarePos, 3);
                
            }
            else if(game.getDrills(square) < 2 and secondShortestDist > dist(squarePos, tenPos) + mathVecMagnitude(squarePos, 3) and dist(squarePos, tenPos) > 0.03f){
                bestSix[1][0] = square[0];
                bestSix[1][1] = square[1];
                secondShortestDist = dist(squarePos, tenPos) + mathVecMagnitude(squarePos, 3);
                
            }
        }
    }
    DEBUG(("best square %d %d", bestSix[0][0], bestSix[0][1]));
    DEBUG(("second best square %d %d", bestSix[1][0], bestSix[1][1]));
    
    // Find a spot to drill
     // drill at the spot we picked
     
    float xyLookAxis[3];
    xyLookAxis[0] = myAtt[0];
    xyLookAxis[1] = myAtt[1];
    xyLookAxis[2] = 0;
    DEBUG(("Drilling at %d, %d", drillSquare[0], drillSquare[1]));
    api.setAttitudeTarget(xyLookAxis);
    
    #define XY_DRILL_PLANE_ANGLE 0.1963f
        
    game.square2pos(drillSquare, positionTarget);
    //sets positionTarget to the square to be drilled
    PRINT_VEC_F("position target", positionTarget);
    float xyPos[3];
    float xyPositionTarget[3];
    memcpy(xyPositionTarget, positionTarget, 12);
    memcpy(xyPos, myPos, 12);
    xyPos[2] = 0;
    xyPositionTarget[2] = 0;
    
    //creates two vectors based off the positionTarget and myPos that are in the same xy plane 
    switch (game.getNumSamplesHeld()) {
        case 0:
        case 1:
            drillSquare[0] = bestSix[1][0];
            drillSquare[1] = bestSix[1][1];
            break;
        case 2:
            drillSquare[0] = bestSix[0][0];
            drillSquare[1] = bestSix[0][1];
            break;
        case 3:
        case 4:
            drillSquare[0] = tenLoc[0];
            drillSquare[1] = tenLoc[1];
            break;
    }
    

    if(dist(xyPos, xyPositionTarget) < .06) {
        //checks to see if the sphere is above the square
        positionTarget[2] = game.getTerrainHeight(drillSquare) - 0.13f;
        //sets the drill height
        DEBUG(("%f terrain height", game.getTerrainHeight(drillSquare)));
        if (dist(myPos, positionTarget) < 0.03f and mathVecMagnitude(myVel, 3) < 0.02f
        and mathVecMagnitude(myRot, 3) < 0.04f and !game.getDrillEnabled() and angle(myAtt, xyLookAxis, 3) <= XY_DRILL_PLANE_ANGLE){
            DEBUG(("Starting Drill"));
            game.startDrill();
        }
        else if (game.getDrillEnabled()) {
            DEBUG(("Drilling"));
            float drillVec[3];
            drillVec[0] = 0;
            drillVec[1] = 0;
            drillVec[2] = 0.5f;
            api.setAttRateTarget(drillVec);
            if (game.checkSample()){
                game.pickupSample();
                //picks up sample and changes the square (temporary)
            }
        }
    }
    else{
        positionTarget[2] = 0.26f;
        //if the sphere is under a certain height, move to that height before traveling in the x or y direction
        if(myPos[2] >= .30f) {
            positionTarget[0] = myPos[0];
            positionTarget[1] = myPos[1];
        }
    }

    
    
    
    if ((sampNum == 5) or (myFuel<=fuel2base)) {
        if(myPos[2] < 0.285f){
            DEBUG(("Heading back to base"));
            float dropOffAtt[3];
            dropOffAtt[0] = 0.0f;
            dropOffAtt[1] = 0.0f;
            dropOffAtt[2] = -1.0f;
            api.setAttitudeTarget(dropOffAtt); // Must be pointing in a certain
                // direction in order to drop off
            
            // Go the closest point that is a specifed dist from the origin
            memcpy(positionTarget, myPos, 12);
            mathVecNormalize(positionTarget, 3);
            #define DROP_OFF_RADIUS_TARGET 0.23f
            scale(positionTarget, DROP_OFF_RADIUS_TARGET);
            
            if(game.atBaseStation()) {
                float samples[5];
                    // store the concentrations from each sample
                for (int i = 0; i < sampNum; i++) { // for each sample
            
                    samples[i] = game.dropSample(i);
                    
                }
            }
        }
        else{
            memcpy(positionTarget, myPos, 12);
            positionTarget[2] = 0.28f;
            
        }
    }
	
	if(dist(myPos, positionTarget) > 0.02f or angle(myAtt, xyLookAxis, 3) >= XY_DRILL_PLANE_ANGLE or game.getDrillError() or geyserOnMe) {
	    game.stopDrill();
	    //this stops the drill only if the sphere is moved or we have a full inventory, 
	    //this also prevents geysers from breaking drilling because our position would be far from position target 
	}
	PRINT_VEC_F("positionTarget", positionTarget);
	
	if(geyserOnMe){
	    memcpy(positionTarget, zeroVec, 12);
	}
	
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    float distance,flocal,fvector[3];
    #define ACCEL .014f
    //mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    distance = mathVecNormalize(fvector, 3);
    mathVecSubtract(fvector, destination, myPos, 3);
    
    scale(myVel,.28f);
    mathVecSubtract(fvector,fvector,myVel,3);
    scale(fvector,.24f);
    if (geyserOnMe){
        flocal=mathVecMagnitude(fvector,3)/.2f;
        fvector[0]/=flocal;
        fvector[1]/=flocal;
        fvector[2]=0.01f;
    }
    api.setVelocityTarget(fvector);
}


//Vector math functions
float dist(float* vec1, float* vec2) {
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}
int distSquared(int* vec1, int* vec2) {
    return mathSquare(vec1[0]-vec2[0]) + mathSquare(vec1[1]-vec2[1]);
}
float angle(float* vec1, float* vec2, int len){
    return acosf(mathVecInner(vec1, vec2, len)
        / (mathVecMagnitude(vec1, len) * mathVecMagnitude(vec2, len)));
}
void scale(float* vec, float scale){
    for (int i=0; i<3; i++)
        vec[i] *= scale;
}