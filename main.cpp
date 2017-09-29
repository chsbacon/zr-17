
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

#define SPHERERADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f
//}
// DEBUG shorthand functions (F is for floats, I is for ints)
#define PRINT_VEC_F(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1],pr vec[2]))
#define PRINT_VEC_I(str, vec) DEBUG(("%s %d %d %d", str, vec[0], vec[1], vec[2]))

float myScore;
float enScore;
float pointVals[5]; // stores each point value for dropping off the various concentrations
int enDrillSquares[3][2]; // where the enemy has drilled since dropping off
int enDrillNumSinceDrop; // how many times the enemy has drilled since dropping off

bool possibleTenSquares[2][4][6]; // odds that a square is a 10, refers to -X+Y and +X+Y ten zones smushed together

bool samplesHeld[3];

void init(){
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f,0.1f,2.8f);
    
    myScore = 0.0f;
    enScore = 0.0f; // this is important because we use these to calculate change in score
    
    // Initialize pointVals to contain the points gained from each concentration
    pointVals[0] = 0.0f; // *empty*
    pointVals[1] = 2.5f; // 1
    pointVals[2] = 3.5f; // 3
    pointVals[3] = 5.0f; // 6
    pointVals[4] = 7.0f; // 10
    
    // Reset enemy-awareness variables
    memset(enDrillSquares, 0, 24);
    enDrillNumSinceDrop = 0;

    memset(possibleTenSquares, 00000001, 48);
}

void loop(){
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	game.getSamplesHeld(samplesHeld);
	
	float myDeltaScore = game.getScore() - myScore;
	float enDeltaScore = game.getOtherScore() - enScore;
	myScore = game.getScore();
	enScore = game.getOtherScore();

    if (enDeltaScore == 1.0f || enDeltaScore == 2.0f || enDeltaScore == 3.0f){ //Possible score gains from drilling
        DEBUG(("Enemy just drilled")); // @TODO make this based on rotatation to account for 4th drill in a single spot
        game.pos2square(enPos, enDrillSquares[enDrillNumSinceDrop]);
        enDrillNumSinceDrop++;
        if (enDrillNumSinceDrop>2) enDrillNumSinceDrop = 0; // wrap, so that if they drill more than three before drop-off, we only catch the last three
        
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
	    DEBUG(("possibleTenSquares: "));
        for (int i=0; i<6; i++) { // print out all of possibleTenSquares
            DEBUG(("%d %d %d %d | %d %d %d %d", possibleTenSquares[0][0][i], possibleTenSquares[0][1][i],
            possibleTenSquares[0][2][i], possibleTenSquares[0][3][i], possibleTenSquares[1][0][i], possibleTenSquares[1][1][i],
            possibleTenSquares[1][2][i], possibleTenSquares[1][3][i] ));
        }
	    // reset enemy-awareness variables because they're no longer relevant
	    memset(enDrillSquares, 0, 24);
	    enDrillNumSinceDrop = 0;
	}
	
}

// based on enemy's increase in points, goes through all possible sample concentration values, to determine which combination they got
void pointValues(int* result, float deltaScore){
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            for (int k=0; k<5; k++) {
                if (pointVals[i] + pointVals[j] + pointVals[k] == deltaScore) { // if the shoe fits...    
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
   // Declare and initialize a bunch of arrays that correspond to relative positions of parts of the high concentration zone
   
    for (int i=0; i<2; i++) {
        for (int j=0; j<4; j++) {
            for (int k=0; k<6; k++) {
                if (possibleTenSquares[i][j][k]) {
                    if (scores[0] > 1 and scores[1] > 1 and scores[2] > 1 ) {
                        possibleTenSquares[i][j][k] = false; // as long as any of the scores hit the target, we can assume that any locations not in their immedidate
                            // vicinity should default to impossible for the 10
                    }
                    else {
                        DEBUG(("This batch won't be super helpful"));
                    }
                    for (int itIdx = 0; itIdx<6; itIdx++) { // go through each permutation of score and location
                        // @TODO exclude configurations that split between sides
                        
                        int score1 = scores[1];
                        int swapIdx = (itIdx%2) * 2; // alternates the first and third based on itIdx
                        scores[1] = scores[swapIdx];
                        scores[swapIdx] = score1;
                        
                        // for this iteration we've decided scores, in its current order, now maps directly to squares, i.e. scores[0] -> squares[0]
                        
                        for (int idx=0; idx<3; idx++) { // go through each sample in this permutation of the batch
                            if (scores[idx] == 0) {
                                continue;
                            }
                            // (col, row) because ZR is dumb
                            // [0] = left or right rectangle [1] and [2] are column and row normalized to the range [0,3][0,5]
                            int squareNorm[3] = {-1, (squares[idx][1]<0 ? -squares[idx][0] : squares[idx][0]),
                                (squares[idx][1]<0 ? -squares[idx][1] : squares[idx][1]) - 3 };
                            squareNorm[0] = squareNorm[1]>0; // is this location on the left or the right?
                            //either add 6 or subtract 3 to normalize to 0
                            squareNorm[1] += squareNorm[0] ? -3 : 6;
                            
                            if (squareNorm[0] != i) { // if not both on left or both on right then skip
                                continue;
                            }
                            
                            int testTen[2] = {j, k};
                            int dist = distSquared(testTen, &(squareNorm[1])); // find the distance between the assumed bullseye (i,j,k will iterate through all possible
                            // bullseyes) and the assumed sample location
                            
                            // next we determine if i,j,k being the bullseye is consistent with this square/score pairing
                            bool possible = (dist == 0 && scores[idx] == 4) // bullseye(10 points) corresponds to 4
                                    || ((dist==1 || dist==2) && scores[idx] == 3) // six ring (6 points) corresponds to 3
                                    || ((dist==4 || dist==5) && scores[idx] == 2); // three ring (3 points) corresponds to 2
                                    // normal square (1 point) corresponds to 1 and thus will always be false
                                    
                            // DEBUG(("testTen: %d %d  squareNorm: %d %d %d and.. it is a %s", testTen[0], testTen[1], squareNorm[0],
                            //     squareNorm[1], squareNorm[2], dist<=5&&possible?"go!":":("));
                            if (dist <= 5 and possible) {  // only update cells within the radius of the target
                                possibleTenSquares[i][j][k] = true; // we only set cells to true with this statement
                                
                                // This next bit is totally optional; it's just to prove that tableLocToSquare works
                                int pos[2] = {-1, -1}; // initialize array to store a test position
                                tableLocToSquare(pos, i, j, k, false); // convert to [col, row]-style square
                                DEBUG(("the 10 could be at %d %d", pos[0], pos[1]));
                            }
                        }
                    }
                }
            }
        }
    }
}

void tableLocToSquare(int* result, int i, int j, int k, bool top) {
    result[0] = j + (i ? 3 : -6);
    result[1] = k + 3;
    if (top) {
        result[0] = -result[0];
        result[1] = -result[1];
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