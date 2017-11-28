//{"sha":"d2d1471f03b117a4e56fbe96cfe59ef086b18bd5"}

//Known issues:
//Will break if searching across the line
//Garbage values for sumI and sumJ

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

bool infoFound; // have we gotten a 3, 6, or a 10?
bool tenFound;
int mySquare[3];
int targetSquare[3];
float targetPos[3];
int startSquare[3];

int sampNum;
bool pickUp[2];
bool isBlue;
float analyzer[3];
bool startedOn;
bool searching;
int iOff;
int jOff;
int theTen[2];
int sumI;
int sumJ;

char tenSquares[5][5]; // stores whether
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
int count;


void init(){
    searching = false;
    startedOn = false;
	infoFound = false;
    tenFound = false;
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f, 0.1f, 2.8f);
    vcoef = 0.154f; 
    stage = 0;
    sampNum = 0;//Number of samples we have
    int sumI = 0;
    int sumJ = 0;
    
    api.getMyZRState(myState);
    isBlue = (myState[1] > 0);
    
    analyzer[0] = (isBlue) ? -0.30f : 0.30f;
    analyzer[1] = (isBlue) ? 0.48f : -0.48f;
    analyzer[2] = (isBlue) ? -0.16f : -0.16f;
    
    // tenSquares starts out all possible
    for(int i = 0; i < 5; i ++){
        for(int j = 0; j < 5; j ++){
            tenSquares[i][j] = 'O';
        }
    }
}

void loop(){
    float zeroVec[3] = {0.0, 0.0, 0.0}; // just a convenience thing
    float positionTarget[3]; // where we're going
    // Update state
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	int myFuel = game.getFuelRemaining()*100;
	int sampNum = game.getNumSamplesHeld();
	game.pos2square(myPos,mySquare);

	bool geyserOnMe;
    geyserOnMe=game.isGeyserHere(mySquare);
	
	game.pos2square(myPos, mySquare);
    float drillSquarePos[3];
    game.square2pos(drillSquare,drillSquarePos);
    
    game.getAnalyzer(pickUp);
    
    if(stage == 0){
        if(tenFound){
            DEBUG(("Ten found at %d, %d", theTen[0], theTen[1]));
        }
        // first, get the analyzer
        else if(game.hasAnalyzer() != isBlue + 1){
            memcpy(positionTarget, analyzer, 12);
            targetPos[0] = positionTarget[0];
            targetPos[1] = -positionTarget[1];
            targetPos[2] = positionTarget[2];
        }
        // then, search in a line along the y-axis
        else if(!infoFound){
            memcpy(positionTarget, targetPos, 12);
            searching = true;
            DEBUG(("Concentration, %f", game.analyzeTerrain()));
            if(game.analyzeTerrain() > .1f){
                DEBUG(("We got something"));
                game.pos2square(myPos, startSquare);
                infoFound = true;
                if(dist(myPos, analyzer) < .06){ // if we start on the info, keep note of that
                    startedOn = true;
                }
            }
        }
        else{
            count = 0; // the number of squares that could be 10s
            sumI = 0; // the sum of x indices
            sumJ = 0; // the sum of the y indices
            iOff = startSquare[0] - mySquare[0]; // the relative x offset (from the center of tenSquares)
            jOff = startSquare[1] - mySquare[1]; // the relative y offset (from the center of tenSquares)
            for(int i = -2; i < 3; i ++){
                for(int j = -2; j < 3; j ++){ // iterate through every possible 10 value
                    if(!startedOn){ // if we didn't start on the interesting square we can act like there's a 1 right before
                    // if the distance between the offset square and the cell I am looking at is close or far enough, disregard them
                        if((i)*(i) + (j+(-1 + isBlue * 2))*(j+(-1 + isBlue * 2)) <= 5){  
                            tenSquares[i][j] = 'X'; // X means not a 10 there
                        }
                    }
                    if(game.analyzeTerrain() == .1){
                        // this is inline distSquared
                        if((i*iOff)*(i*iOff) + (j*jOff)*(j*jOff) <= 5){ // different possible range for a one
                            tenSquares[i][j] = 'X';
                        }
                    }
                    if(game.analyzeTerrain() == .3){
                        // this is inline distSquared
                        if((i*iOff)*(i*iOff) + (j*jOff)*(j*jOff) < 4 or (i*iOff)*(i*iOff) + (j*jOff)*(j*jOff) > 5){ // different possible range for a three
                            tenSquares[i][j] = 'X';
                        }
                    }
                    if(game.analyzeTerrain() == .6){
                        if((i*iOff)*(i*iOff) + (j*jOff)*(j*jOff) > 2){ // different possible range for a six
                            tenSquares[i][j] = 'X';
                        }
                    }
                    if(game.analyzeTerrain() == 1.0f){ // if it's a 10 we know where it is
                        searching = false;
                        tenFound = true;
                        theTen[0] = mySquare[0];
                        theTen[1] = mySquare[1];
                    }
                    if(tenSquares[i][j] == 'O'){
                        //if it's a possible ten, increase the count by one and add the indexes to the total
                        count ++;
                        sumI += i;
                        sumJ += j;
                        theTen[0] = startSquare[0] + i;
                        theTen[1] = startSquare[1] + j;
                    }
                }
            }
            //calculates the centroid, or the average square index
            targetSquare[0] = sumI / count + startSquare[0];
            targetSquare[1] = sumJ / count + startSquare[1];
            targetSquare[2] = 0; //current altitude
            DEBUG(("Target square %d + " " + %d",  count, sumJ));
            if(count == 1){ // if there's only one possiblity, our work is done
                searching = false;
                tenFound = true;
                //findTen is finished. theTen now contains the location of the ten
            }
            game.square2pos(targetSquare, positionTarget);
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
     searching? scale(fvector,.045f): scale(fvector, .24);
     if (geyserOnMe){
         flocal=mathVecMagnitude(fvector,3)/.2f;
         fvector[0]/=flocal;
         fvector[1]/=flocal;
         fvector[2]=0.01f;
     }
     api.setVelocityTarget(fvector);
}

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