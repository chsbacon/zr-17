//{"sha":"d2d1471f03b117a4e56fbe96cfe59ef086b18bd5"}


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

#define SPHERE_RADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f
#define XY_DRILL_PLANE_ANGLE 0.1963f

// DEBUG shorthand functions (F is for floats, I is for ints)
#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f",str, vec[0], vec[1], vec[2]))

float enScore;
int myDrillSquares[5][2]; // where we have drilled
bool infoFound; // have we gotten a 3, 6, or a 10?
bool tenFound;
int mySquare[3];

int sampNum;
bool pickUp[2];
bool isBlue;
float analyzer[3];


#define TEN_SPAWN_WIDTH 12
#define TEN_SPAWN_HEIGHT 16
char possibleTenSquares[TEN_SPAWN_WIDTH][TEN_SPAWN_HEIGHT]; // stores whether
    // or not each square could be a ten
    // '*' : possible ten spot
    // 'x' : impossible ten spot

float vcoef; // A coefficient for our movement speed
#define SURFACE_Z 0.48f
int drillSquare[2]; // Will eventually store the optimal drilling square
    // It's important that this is global because sometimes it doesn't
    // get updated
int stage;

int tenLoc[2];
float myState[12];
float enState[12];


void init(){
	    infoFound = false;
    tenFound = false;
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f, 0.1f, 2.8f);
    vcoef = 0.154f; 
    
    enScore = 0.0f; // initialized because
        // we use it to calculate change in score
    


    memset(possibleTenSquares, '*', TEN_SPAWN_HEIGHT * TEN_SPAWN_WIDTH);
        // at the start, all squares are possible tens
        // including the center lmao, but we ignore it
        
    stage = 0;
    sampNum = 0;//Number of samples we have
    
    api.getMyZRState(myState);
    isBlue = (myState[1] > 0);
    
    analyzer[0] = (isBlue) ? -0.30f : 0.30f;
    analyzer[1] = (isBlue) ? 0.48f : -0.48f;
    analyzer[2] = (isBlue) ? -0.16f : -0.16f;
}

void loop(){
    float zeroVec[3] = {0.0, 0.0, 0.0}; // just a convenience thing
    float positionTarget[3]; // where we're going
    // Update state
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
    
    game.getAnalyzer(pickUp);

    
    //checking fuel to see if we need to go back to base
    float magnitude = sqrtf(mathSquare(myPos[0]) + mathSquare(myPos[1]) + mathSquare(-0.1f-myPos[2])); //finding magnitude
    DEBUG(("Magnitude is %f", magnitude));
    float fuel2base = magnitude*(22*(mathSquare(.6*magnitude - .9)));//multiplying magnitude by fuel2square algorithm
    DEBUG(("The fuel to get back to the base is %f",fuel2base));
    
    if(stage == 0){
        if(tenFound or true){
            DEBUG(("WE FOUND 10 at (%d, %d)", myDrillSquares[sampNum - 1][0], myDrillSquares[sampNum - 1][1]));
            stage++;
            tenLoc[0] = 5;//myDrillSquares[sampNum - 1][0];
            tenLoc[1] = 3;//myDrillSquares[sampNum - 1][1];
            
        }
        else if(!pickUp[isBlue] and game.hasAnalyzer() != isBlue + 1){
            //api.setPositionTarget(ANALYZER_POSITION);
            memcpy(positionTarget, analyzer, 12);
            //api.setPositionTarget(positionTarget);
        }
        else { // Find a spot to drill
            
            
            #define LARGE_NUMBER 10.0f
                // while we're hunting, any possible square will do
            #define PROXIMITY_REQUIREMENT 0.453f
                // if infoFound, it must be within 1 ten radius
                // square width * dist from one end of concentration to the other
                // 0.08 * sqrt(4^2,4^2)
            // If no possible squares are closer than minDist, then we will
            // default to last loop's drill spot
            // (this will only be the case when we found the ten)
            float minDist = infoFound ? PROXIMITY_REQUIREMENT : LARGE_NUMBER;
            
            DEBUG(("sampNum: %d", sampNum));
            if (infoFound) {
                //NARROW IT DOWN
                DEBUG(("INFO FOUND"));
            }
            else{
                for (int c=0; c<TEN_SPAWN_WIDTH; c++) { // iterate over possibleTenSquares
                    for (int r=0; r<TEN_SPAWN_HEIGHT; r++) {
                        if (possibleTenSquares[c][r] == '*') { // exclude center
                            #define zeroSquare (int*)zeroVec
                            // Store (col,row) as a square
                            int square[2];
                            square[0] = c + (c>=TEN_SPAWN_WIDTH/2 ? 1 : 0) - TEN_SPAWN_WIDTH/2;
                            square[1] = r + (r>=TEN_SPAWN_HEIGHT/2 ? 1 : 0) - TEN_SPAWN_HEIGHT/2;
                            // We are reversing this operation from updateTenSquares:
                            // {i + (i>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
                            // j + (j>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
                            
                            float testPos[3];
                            game.square2pos(square, testPos);
                            testPos[2] = SURFACE_Z;
                            float distance = dist(myPos, testPos);
                            
                            // Go over samples we already have to make sure
                            // this spot isn't too close to them
                            for (int samp=0; samp<sampNum; samp++) {
                                int mirrorSquare[2] = {-myDrillSquares[samp][0],
                                    -myDrillSquares[samp][1]};
                                    // reflect across the origin
                                #define HUNTING_DRILL_SPACING 22
                                    // when we're hunting, we want widely spaced
                                    // drill spots, so info doesn't overlap
                                #define EXCLUDE_RADIUS 0
                                    // once we find info, don't drill in the same spot
                                    // unless no other spots are possible
                                #define CENTER_INFO_RADIUS 10
                                    // tens can't spawn in the center, so we want our
                                    // info to not include any of that area
                                #define CENTER_KEEPAWAY_RADIUS 4
                                    // even if the ten is close to the center,
                                    // we still don't want to drill the center itself
                                
                                if (distSquared(square, myDrillSquares[samp])
                                <= (infoFound ? EXCLUDE_RADIUS : HUNTING_DRILL_SPACING)
                                    // if close to a previous drill spot 
                                or distSquared(square, mirrorSquare) <= HUNTING_DRILL_SPACING
                                    // or that drill spot reflected across the origin
                                or distSquared(square, zeroSquare)
                                <= (infoFound ? CENTER_KEEPAWAY_RADIUS : CENTER_INFO_RADIUS)) {
                                    // or too close to the center
                                    goto skip;
                                        // ignore square as a potential drill spot
                                }
                            }
                            // go to the closest point that is a possible ten
                            if (distance < minDist) { // if this one is closer
                                // than the closest so far
                                // Make it the new closest
                                memcpy(drillSquare, square, 8);
                                minDist = distance;
                            }
                            skip: continue;
                        }
                    }
                }
            }
            //Check at the spot we picked
            DEBUG(("Checking at %d, %d", drillSquare[0], drillSquare[1]));
            game.square2pos(drillSquare, positionTarget);
            positionTarget[2] = -0.16f;
            
            if (dist(myPos, positionTarget) < 0.03f and mathVecMagnitude(myVel, 3) < 0.01f
            and mathVecMagnitude(myRot, 3) < 0.04f and !game.getDrillEnabled()){
                DEBUG(("CHECKING"));
                
                memcpy(myDrillSquares[sampNum], drillSquare, 8);
                DEBUG(("myDrillSquares (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)",myDrillSquares[0][0],myDrillSquares[0][1], myDrillSquares[1][0],myDrillSquares[1][1],myDrillSquares[2][0],myDrillSquares[2][1]));
                float analysis = game.analyzeTerrain();
                DEBUG(("Samp #%d %f score: %f @ (%d, %d)", 
                    sampNum, 
                    analysis,
                    (5 * analysis + 2),
                    drillSquare[0], drillSquare[1] ));
                #ifdef dev
                updateTenSquares(&(drillSquare), 5 * analysis + 2, 1);
                #else
                updateTenSquares(&(drillSquare), 5 * analysis + 2, 1);
                #endif
                sampNum++;//We have to manually increment # of samples because we don't actually have any
                
                infoFound = (2.5f < (5 * analysis + 2)) and ((5 * analysis + 2) < 7.0f);
            }
        }
    }
    if(stage == 1){
        int secondTen[2];
        secondTen[0] = -1*tenLoc[0];
        secondTen[1] = -1*tenLoc[1];
        
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
        if(game.getDrills(tenLoc) < 3){
                drillSquare[0] = tenLoc[0];
                drillSquare[1] = tenLoc[1];
        }
        else if(game.getDrills(tenLoc)<3){
                drillSquare[0] = secondTen[0];
                drillSquare[1] = secondTen[1];
        }
        else{
            drillSquare[0] = secondTen[0];
            drillSquare[1] = secondTen[1] + 1;
        }
        
    
        if(dist(xyPos, xyPositionTarget) < .06) {
            //checks to see if the sphere is above the square
            positionTarget[2] = game.getTerrainHeight(drillSquare) - 0.13f;
            //sets the drill height
            //fDEBUG(("%f terrain height", game.getTerrainHeight(drillSquare)));
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
            positionTarget[2] = 0.285f;
            //if the sphere is under a certain height, move to that height before traveling in the x or y direction
            if(myPos[2] >= 0.285f) {
                positionTarget[0] = myPos[0];
                positionTarget[1] = myPos[1];
            }
        }
    
        
        
        
        if ((sampNum == 5) or (myFuel<=fuel2base) or myFuel < 5) {
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


/**
 * @param squares - array containing sets of squares positions
 * @param scores - array with point val ints corresponding to squares
 * @param batchSize - how many score increases we are considering
 * Goes over every position in possibleTenSquares and updates whether that
 * spot could still be a ten based on new information
 */
void updateTenSquares(int (*squares)[2], float deltaScore, int batchSize) {
    float sum;
    float scores[5];
    float pointVals[5]; // stores each point value for dropping off concentrations
    pointVals[0] = 2.5f; // 1
    pointVals[1] = 3.5f; // 3
    pointVals[2] = 5.0f; // 6
    pointVals[3] = 7.0f; // 10
    
    // Initialize pointVals to contain the points gained from each concentration
    // Later, indices in pointVals will be used in lieu of the score
    // or concentration value by reason of convenience
    
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            for (int k=0; k<4; k++) {
                for (int l=0; l<4; l++) {
                    for (int m=0; m<4; m++) {
                        // 4 possibilities: 1, 3, 6, or 10
                        sum = 0.0f;
                        switch (batchSize) {
                            // Notice the lack of break statements here.
                            // case 3 will also include cases 2 and 1
                            case 5:
                                scores[4] = m;
                                sum += pointVals[m];
                            case 4:
                                scores[3] = l;
                                sum += pointVals[l];
                            case 3:
                                scores[2] = k;
                                sum += pointVals[k];
                            case 2:
                                scores[1] = j;
                                sum += pointVals[j];
                            case 1:
                                scores[0] = i;
                                sum += pointVals[i];
                        }
                        if (sum == deltaScore) {
                            // iterate through every cell in the possibleTenSquares array
                            for (int c=0; c<TEN_SPAWN_WIDTH; c++) { // column
                                for (int r=0; r<TEN_SPAWN_HEIGHT; r++) { // row
                                    if (possibleTenSquares[c][r] != 'x' // only check things we haven't ruled out yet
                                        and !(c>3 and c<8 and r>5 and r<10) ) { // exclude center
                                        
                                        possibleTenSquares[c][r] = 'x';
                                        for (int scoreIdx = 0; scoreIdx < batchSize; scoreIdx++) {
                                            if (scores[scoreIdx] == 0) {
                                                possibleTenSquares[c][r] = '*'; // as long as any of the scores hit the target,
                                                //we can assume that any locations not in their immedidate
                                                // vicinity should default to "impossible for the 10"
                                                break;
                                            }
                                        }
                                        int scoreOrder[5]; // initialize order as 0, 1, 2, ... batchSize-1
                                        for (int i = 0; i < batchSize; i++) {
                                            scoreOrder[i] = i;
                                        }
                                        int perm[5]; // current permutation of score and location
                                        do {
                                            // go through each permutation of score and location
                                            for (int i = 0; i<batchSize; i++) {
                                                perm[i] = scores[scoreOrder[i]];
                                            }
                                            for (int idx=0; idx<batchSize; idx++) { // go through each sample in this permutation of the batch
                                                // Normalize to the range of the table
                                                int squareNorm[2] = {squares[idx][0] + (squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
                                                    squares[idx][1] + (squares[idx][1]>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
                                                int mirrorSquare[2] = {-squares[idx][0] + (-squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
                                                    -squares[idx][1] + (-squares[idx][1]>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
                                                // the ternary is to account for there being no zero column or row
                                                int testTen[2] = {c, r};
                                                int dist1 = distSquared(testTen, squareNorm); // find the distance between the assumed bullseye
                                                int dist2 = distSquared(testTen, mirrorSquare);
                                                
                                                // next we determine if i,j being the bullseye is consistent with this square/score pairing
                                                bool possible = ((dist1 == 0 or dist2 == 0) and perm[idx] == 3) // a 10
                                                    or ((dist1==1 or dist1==2 or dist2==1 or dist2==2) and perm[idx] == 2) // a 6
                                                    or ((dist1==4 or dist1==5 or dist2==4 or dist2==5) and perm[idx] == 1); // a 3
                                                    // normal square (1 point) corresponds to 1 and thus will always be false
                                                
                                                tenFound = (deltaScore == pointVals[3]);// Points in a ten
                                                
                                                if (dist1 <= 5 or dist2 <= 5) {  // only update cells within the radius of the target
                                                    possibleTenSquares[c][r] = possible ? '*' : 'x';
                                                }
                                            }
                                        } while (nextPermutation(scoreOrder, batchSize));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @return {bool} are there any more permutations? 
 */
bool nextPermutation(int* a, int n) {
    // Find the largest index k such that a[k] < a[k + 1]. If no such index exists, the permutation is the last permutation.
    for (int k = n-1; k >= 0; k--) {
        if (a[k] < a[k + 1]) {
            // Find the largest index l greater than k such that a[k] < a[l].
            for (int l = n-1; l > k; l--) {
                if (a[k] < a[l]) {
                    // Swap the value of a[k] with that of a[l].
                    int temp1 = a[l];
                    a[l] = a[k];
                    a[k] = temp1;
                    // Reverse the sequence from a[k + 1] up to and including the final element a[n].
                    for (int s = 0; s < (n - (k+1))/2; s++) {
                        int temp2 = a[s+k+1];
                        a[s+k+1] = a[n-1-s];
                        a[n-1-s] = temp2;
                    }
                    return true;
                }
            }
        }
    }
    return false;
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