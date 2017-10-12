
//double check minus signs

//Standard defines {
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
//}
// DEBUG shorthand functions (F is for floats, I is for ints)
#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f",str, vec[0], vec[1], vec[2]))
#define PRINT_VEC_I(str, vec) DEBUG(("%s %d %d %d",str, vec[0], vec[1], vec[2]))

float myScore;
float enScore;
float pointVals[5]; // stores each point value for dropping off concentrations
int enDrillSquares[3][2]; // where the enemy has drilled since dropping off
int enDrillNumSinceDrop; // how many times the enemy has drilled since drop-off
int myDrillSquares[3][2];
bool infoFound; // have we gotten a 3, 6, or a 10?

#define TEN_SPAWN_WIDTH 12
#define TEN_SPAWN_HEIGHT 16
char possibleTenSquares[TEN_SPAWN_WIDTH][TEN_SPAWN_HEIGHT];
    // '*' : possible ten spot
    // 'x' : impossible ten spot

float vcoef;
float positionTarget[3];
float zeroVec[3];

void init() {
    infoFound = false;
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f, 0.1f, 2.8f);
    vcoef = 0.154; // A coefficient for our movement speed
    
    myScore = 0.0f; // initialized because
    enScore = 0.0f; // we use these to calculate change in score
    
    // Initialize pointVals to contain the points gained from each concentration
    pointVals[0] = 0.0f; // *empty*
    pointVals[1] = 2.5f; // 1
    pointVals[2] = 3.5f; // 3
    pointVals[3] = 5.0f; // 6
    pointVals[4] = 7.0f; // 10
    
    // Reset enemy-awareness variables
    memset(enDrillSquares, 0, 24);
    enDrillNumSinceDrop = 0;

    memset(possibleTenSquares, '*', TEN_SPAWN_HEIGHT * TEN_SPAWN_WIDTH);
    
    memset(zeroVec, 0, 12);
}

void loop() {
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	
	float myDeltaScore = game.getScore() - myScore;
	float enDeltaScore = game.getOtherScore() - enScore;
	myScore = game.getScore();
	enScore = game.getOtherScore();
    
    //if they are guarding drill other squares
    if (game.getNumSamplesHeld() == 3 || api.getTime() > 150) {
    // at some point, we should be more thoughtful about this logic
        DEBUG(("Heading back to base"));
        float dropOffAtt[3] = {0.0f, 0.0f, -1.0f};
        api.setAttitudeTarget(dropOffAtt);
        
        memcpy(positionTarget, myPos, 12);
        mathVecNormalize(positionTarget, 3);
        scale(positionTarget, 0.14f - SPHERE_RADIUS);
        
        if(game.atBaseStation()){
            float samples[3] = 
                {game.dropSample(0), game.dropSample(1), game.dropSample(2)}; 
            for (int i = 0; i < 3; i++) {
                int squares[1][2];
                memcpy(squares[0], myDrillSquares[i], 8);
                
                DEBUG(("Samp #%d %f %d @ (%d, %d)", i, samples[i], 
                    concentrationToPointValsIndex(samples[i]),
                    squares[0][0], squares[0][1] ));
                    
                int scores[1];
                scores[0] = concentrationToPointValsIndex(samples[i]);
                updateTenSquares(squares, scores, 1);
            }
        }
    }
    else { // Find a spot to drill
        float drillAtt[3] = {1.0f, 0.0f, 0.0f};
        api.setAttitudeTarget(drillAtt);
        
        int drillSquare[2]; // Will eventually store the optimal drilling square
        float minDist = 10; // Stores points for that square for comparisons

        for (int i=0; i<TEN_SPAWN_WIDTH; i++) { // iterate over possibleTenSquares
            for (int j=0; j<TEN_SPAWN_HEIGHT; j++) {
                if (!(i>3 and i<8 and j>5 and j<10)) { // exclude center
                    int zeroSquare[2] = {0,0};
                    int square[2];
                    tableLocToSquare(square, i, j);
                    if(!infoFound){
                        for (int samp = 0; samp < game.getNumSamplesHeld(); samp++) {
                            if (distSquared(square, myDrillSquares[samp]) <= 25 or distSquared(square, zeroSquare) <= 10) {
                                goto skip; // skip if it's close to samples we already have or close to the center
                            }
                        }
                    }
                    // go to the closest point that is a possible ten
                    if (possibleTenSquares[i][j] == '*') {
                        float testPos[3];
                        game.square2pos(square, testPos);
                        testPos[2] = 0.65f;
                        float distance = dist(myPos, testPos);
                        if (distance < minDist) {
                            memcpy(drillSquare, square, 8);
                            minDist = distance;
                        }
                    }
                    skip: continue;
                }
            }
        }
        drillAtSqr(drillSquare); // drill at the spot we picked
    }
    
    if (enDeltaScore == 1.0f || enDeltaScore == 2.0f || enDeltaScore == 3.0f){ // Possible score gains from drilling
        DEBUG(("Enemy just drilled")); // @TODO make this based on rotatation to account for 4th drill in a single spot
        game.pos2square(enPos, enDrillSquares[enDrillNumSinceDrop]);
        enDrillNumSinceDrop++;
        if (enDrillNumSinceDrop>2) enDrillNumSinceDrop = 0; // wrap, so that if they drill more than three before drop-off,
                                                            // we only catch the last three
        
    }
	
	if (enDeltaScore >= 2.5f && enDeltaScore != 3.0f) { // if they got points, but only drop-off points, not drilling points
	    DEBUG(("enemy dropped samples off for a total increase of: %f", enDeltaScore));
	    int enBatchPointVals[3] = {0.0, 0.0, 0.0}; // stores indices in pointVals array that correspond to sample concentration values
	    //memset(enBatchPointVals, 0, 12); // the compiler complained when this wasn't initialized
	    pointValues(enBatchPointVals, enDeltaScore);
	    PRINT_VEC_I("composed of: ", enBatchPointVals);
	    DEBUG(("Located at (%d, %d), (%d, %d), (%d, %d)", enDrillSquares[0][0], enDrillSquares[0][1], enDrillSquares[1][0], enDrillSquares[1][1],
	        enDrillSquares[2][0], enDrillSquares[2][1]));
	    updateTenSquares(enDrillSquares, enBatchPointVals, 3);
	    
	    // reset enemy-awareness variables because they're no longer relevant
	    memset(enDrillSquares, 0, 24);
	    enDrillNumSinceDrop = 0;
	}
	
	PRINT_VEC_F("positionTarget", positionTarget);
	// Movement code
	#define destination positionTarget
	float distance,flocal,fvector[3];
    #define ACCEL 0.0175f
    mathVecSubtract(fvector, destination, myPos, 3); // vector from us to the target
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
        api.setPositionTarget(destination);
    }
    
    if (!game.getDrillEnabled() and myRot[2] > 0.037f){
        api.setAttRateTarget(zeroVec);
    }
}

bool drillAtSqr(int* sqr){
    if (game.getDrillError()){
        game.stopDrill();
    }
    DEBUG(("Drilling at %d, %d", sqr[0], sqr[1]));
    game.square2pos(sqr, positionTarget);
    positionTarget[2] = 0.51;
    positionTarget[0] += 0.03;
    positionTarget[1] += 0.03;

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
            memcpy(myDrillSquares[game.getNumSamplesHeld()-1], sqr, 8);
            return true;
        }
    }
    return false;
}

// based on enemy's increase in points, goes through all possible sample concentration values, to determine which combination they got
void pointValues(int* result, float deltaScore){
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            for (int k=0; k<5; k++) {
                if (pointVals[i] + pointVals[j] + pointVals[k] == deltaScore) {    
                    result[0] = i;
                    result[1] = j;
                    result[2] = k;
                    return; // stop early to save time
                }
            }
        }
    }
}

/**
 * @param squares - array containing sets of squares positions
 * @param scores - array with point val ints corresponding in an unknown way to some squares
 * @param batchSize - how many score increases we are considering
 * Does not currently correspond to actual probability because that would inconvenient 
 * When we pick up a sample we would call this function with a batchSize of 1
 */
void updateTenSquares(int (*squares)[2], int *scores, int batchSize) {
    // iterate through every cell in the possibleTenSquares array
    for (int i=0; i<TEN_SPAWN_WIDTH; i++) { // column
        for (int j=0; j<TEN_SPAWN_HEIGHT; j++) { // row
            if (possibleTenSquares[i][j] != 'x' // only check things we haven't ruled out yet
                && !(i>3 and i<8 and j>5 and j<10) ) { // exclude center
                
                possibleTenSquares[i][j] = 'x';
                for (int scoreIdx = 0; scoreIdx < batchSize; scoreIdx++) {
                    if (scores[scoreIdx] <= 1) {
                        possibleTenSquares[i][j] = '*'; // as long as any of the scores hit the target,
                        //we can assume that any locations not in their immedidate
                        // vicinity should default to "impossible for the 10"
                        break;
                    }
                }
                for (int itIdx = 0; itIdx < (batchSize == 3 ? 6 : 1); itIdx++) { // go through each permutation of score and location
                // if batchSize is 1, only do this once
                    if (batchSize > 1) { // permute the set, if batchSize is > 1
                        int score1 = scores[1];
                        int swapIdx = (itIdx%2) * 2; // alternates the first and third based on itIdx
                        scores[1] = scores[swapIdx];
                        scores[swapIdx] = score1;
                        
                        // for this iteration we've decided scores, in its current order, now maps directly to squares, i.e. scores[0] -> squares[0]
                    }
                    for (int idx=0; idx<batchSize; idx++) { // go through each sample in this permutation of the batch
                        if (scores[idx] == 0) {
                            continue;
                        }

                        // Normalize to the range of the table
                        int squareNorm[2] = {squares[idx][0] + (squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
                            squares[idx][1] + (squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
                        int mirrorSquare[2] = {-squares[idx][0] + (-squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_WIDTH/2,
                            -squares[idx][1] + (-squares[idx][0]>0 ? -1 : 0) + TEN_SPAWN_HEIGHT/2};
                        // the ternary is to account for there being no zero column or row
                        int testTen[2] = {i, j};
                        int dist1 = distSquared(testTen, squareNorm); // find the distance between the assumed bullseye
                        int dist2 = distSquared(testTen, mirrorSquare);
                        
                        // next we determine if i,j being the bullseye is consistent with this square/score pairing
                        bool possible = ((dist1 == 0 || dist2 == 0) && scores[idx] == 4) // a 10
                                || ((dist1==1 || dist1==2 || dist2==1 || dist2==2) && scores[idx] == 3) // a 6
                                || ((dist1==4 || dist1==5 || dist2==4 || dist2==5) && scores[idx] == 2); // a 3
                                // normal square (1 point) corresponds to 1 and thus will always be false
                        if(possible){
                            infoFound = true;    
                        }
                                
                        if (dist1 <= 5 || dist2 <= 5) {  // only update cells within the radius of the target
                            possibleTenSquares[i][j] = possible ? '*' : 'x';
                        }
                    }
                }
            }
        }
    }
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

void tableLocToSquare(int* result, int i, int j) {
    result[0] = i - TEN_SPAWN_WIDTH/2;
    result[1] = j - TEN_SPAWN_HEIGHT/2;
}

int concentrationToPointValsIndex(float concentration) {
    for (int i = 0; i < 5; i++) {
        if (pointVals[i] == 5 * concentration + 2) {
            return i;
        }
    }
}
//Vector math functions {
float dist(float* vec1, float* vec2) {
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}
int distSquared(int* vec1, int* vec2) {
    return mathSquare(vec1[0]-vec2[0]) + mathSquare(vec1[1]-vec2[1]);
}
float angle(float* vec1, float* vec2){
    return acosf(mathVecInner(vec1,vec2,3)/(mathVecMagnitude(vec1,3)*mathVecMagnitude(vec2,3)));
}
void scale(float* vec, float scale){
    for (int i=0; i<3; i++)
        vec[i] *= scale;
}
//}