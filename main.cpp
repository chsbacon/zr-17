
//Standard defines
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

#define SPHERE_RADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f

// DEBUG shorthand functions (F is for floats, I is for ints)
#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f",str, vec[0], vec[1], vec[2]))

float myScore;
float enScore;
float pointVals[5]; // stores each point value for dropping off concentrations
int enDrillSquares[5][2]; // where the enemy has drilled since dropping off
int enDrillNumSinceDrop; // how many times the enemy has drilled since drop-off
int enDrillSquaresIdx; // where in enDrilSquares we will record the enemy's
    // next drill spot
int myDrillSquares[5][2]; // where we have drilled
bool infoFound; // have we gotten a 3, 6, or a 10?

#define TEN_SPAWN_WIDTH 12
#define TEN_SPAWN_HEIGHT 16
char possibleTenSquares[TEN_SPAWN_WIDTH][TEN_SPAWN_HEIGHT]; // stores whether
    // or not each square could be a ten
    // '*' : possible ten spot
    // 'x' : impossible ten spot

float vcoef; // A coefficient for our movement speed
float positionTarget[3]; // where we're going
float zeroVec[3]; // (0,0,0) -- just a convenience thing
#define SURFACE_Z 0.48f
int drillSquare[2]; // Will eventually store the optimal drilling square

void init() {
    infoFound = false;
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f, 0.1f, 2.8f);
    vcoef = 0.154; 
    
    myScore = 0.0f; // initialized because
    enScore = 0.0f; // we use these to calculate change in score
    
    // Initialize pointVals to contain the points gained from each concentration
    // Later, indices in pointVals will be used in lieu of the score
    // or concentration value by reason of convenience
    pointVals[0] = 2.5f; // 1
    pointVals[1] = 3.5f; // 3
    pointVals[2] = 5.0f; // 6
    pointVals[3] = 7.0f; // 10
    
    // Reset enemy-awareness variables
    enDrillNumSinceDrop = 0;
    enDrillSquaresIdx = 0;

    memset(possibleTenSquares, '*', TEN_SPAWN_HEIGHT * TEN_SPAWN_WIDTH);
        // at the start, all squares are possible tens
        // including the center lmao, but we ignore it
    memset(zeroVec, 0, 12);
}

void loop() {
    // Update state
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	float enDeltaScore = game.getOtherScore() - enScore;
	myScore = game.getScore();
	enScore = game.getOtherScore();
	int sampNum = game.getNumSamplesHeld();
	
    float drillSquarePos[3];
    game.square2pos(drillSquare,drillSquarePos);
    if (sampNum == 5 
    or (sampNum >= 2 and angle(myPos, drillSquarePos, 2) > 2.8f)) {
        DEBUG(("Heading back to base"));
        float dropOffAtt[3] = {0.0f, 0.0f, -1.0f};
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
                // Format the data for updateTenSquares
                samples[i] = game.dropSample(i);
                int squares[1][2];
                memcpy(squares[0], myDrillSquares[i], 8);
                
                DEBUG(("Samp #%d %f score: %f @ (%d, %d)", i+1, samples[i],
                    (5 * samples[i] + 2),
                    squares[0][0], squares[0][1] ));
                    // 5 * samples[i] + 2
                updateTenSquares(squares, 5 * samples[i] + 2, 1);
            }
        }
    }
    else { // Find a spot to drill
        float drillAtt[3] = {1.0f, 0.0f, 0.0f};
        api.setAttitudeTarget(drillAtt); // direction requirement for drilling
        
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
        DEBUG(("myDrillSquares"));
        DEBUG(("(%d %d) (%d %d) (%d %d)",myDrillSquares[0][0],myDrillSquares[0][1],
        myDrillSquares[1][0],myDrillSquares[1][1],myDrillSquares[2][0],myDrillSquares[2][1]));
        DEBUG(("sampNum: %d", sampNum));
        for (int c=0; c<TEN_SPAWN_WIDTH; c++) { // iterate over possibleTenSquares
            for (int r=0; r<TEN_SPAWN_HEIGHT; r++) {
                if (possibleTenSquares[c][r]) { // exclude center
                    int zeroSquare[2] = {0,0};
                    
                    // Store (col,row) as a square
                    int square[2];
                    tableLocToSquare(square, c, r);
                    
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
                                // ignore `square` as a potential drill spot
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
        // drill at the spot we picked
        if (game.getDrillError()){
            game.stopDrill();
        }
        DEBUG(("Drilling at %d, %d", drillSquare[0], drillSquare[1]));
        game.square2pos(drillSquare, positionTarget);
        positionTarget[2] = 0.35;
    
        if (dist(myPos, positionTarget) < 0.03f and mathVecMagnitude(myVel, 3) < 0.01f
        and mathVecMagnitude(myRot, 3) < 0.04f and !game.getDrillEnabled()){
            DEBUG(("Starting Drill"));
            game.startDrill();
        }
        else if (game.getDrillEnabled()) {
            DEBUG(("Drilling"));
            float drillVec[3] = { myAtt[1], -myAtt[0], 0};
            api.setAttitudeTarget(drillVec);
            if (game.checkSample()){
                game.pickupSample();
                game.stopDrill();
                memcpy(myDrillSquares[game.getNumSamplesHeld()-1], drillSquare, 8);
            }
        }
    }
    
    if (enDeltaScore == 1.0f or enDeltaScore == 2.0f or enDeltaScore == 3.0f){
        // Possible score gains from drilling
        DEBUG(("Enemy just drilled"));
        if (enDrillSquaresIdx>4) enDrillSquaresIdx = 0;
            // wrap, so that if they drill more than three before drop-off,
            // we only catch the last three
        game.pos2square(enPos, enDrillSquares[enDrillSquaresIdx]);
        enDrillNumSinceDrop++;
        enDrillSquaresIdx++;
    }
	
	if (enDeltaScore >= 2.5f and enDeltaScore != 3.0f) { // if they got points, but only drop-off points, not drilling points
	    int numSamples = enDrillNumSinceDrop>5 ? 5 : enDrillNumSinceDrop;
	        // stores which indices in the pointVals array
	        // that correspond to sample concentration values
	    DEBUG(("enemy dropped %d samples off for a total increase of: %f", numSamples, enDeltaScore));
	    updateTenSquares(enDrillSquares, enDeltaScore, numSamples);
	    
	    // reset stale enemy-awareness variables
	    enDrillNumSinceDrop = 0;
	    enDrillSquaresIdx = 0;
	}
	
	PRINT_VEC_F("positionTarget", positionTarget);
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
    if (!game.getDrillEnabled() and myRot[2] > 0.037f){
        api.setAttRateTarget(zeroVec);
    }
    if (api.getTime()%15 == 0) {
        DEBUG(("possibleTenSquares: "));
        for (int i=0; i<TEN_SPAWN_HEIGHT; i++) { // print out all of possibleTenSquares
            DEBUG(("%c %c %c %c | %c %c %c %c | %c %c %c %c", possibleTenSquares[0][i],
            possibleTenSquares[1][i], possibleTenSquares[2][i],
            possibleTenSquares[3][i], i>5 and i<10 ? '-' : possibleTenSquares[4][i],
            i>5 and i<10 ? '-' : possibleTenSquares[5][i], i>5 and i<10 ? '-' : possibleTenSquares[6][i],
            i>5 and i<10 ? '-' : possibleTenSquares[7][i], possibleTenSquares[8][i],
            possibleTenSquares[9][i], possibleTenSquares[10][i],
            possibleTenSquares[11][i] ));
        }
    }
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
                                                if (possible){
                                                    infoFound = true;
                                                }
                                                        
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

void tableLocToSquare(int* result, int i, int j) {
    result[0] = i + (i>=TEN_SPAWN_WIDTH/2 ? 1 : 0) - TEN_SPAWN_WIDTH/2;
    result[1] = j + (j>=TEN_SPAWN_HEIGHT/2 ? 1 : 0) - TEN_SPAWN_HEIGHT/2;
    // We are reversing this operation from updateTenSquares:
    // {i + (i>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
    // j + (j>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
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