// Macro definitions and pointers
#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myQuatAtt (&myState[6])
#define myRot (&myState[10])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])
#define MAXDRILLS 3
//int maxDrills;
// stores the target drilling location
int siteCoords[3];

// flag for if we are retargeting
bool newLoc;

// flag for if we are moving back to drop
bool dropping;

// flag for if we are drilling
bool drilling;

// flag for if we are guarding
bool guarding;


// which corner of the square we are drilling (0-3)
int corner;

// the enemy's score
float enScore;

void init(){
  // they start with 0 points
  enScore = 0;

  // we begin the game looking for a square
  newLoc = true;
	
	// movement parameters
	#define SPEEDCONST 0.35f
  #define DERIVCONST 2.35f
  api.setPosGains(SPEEDCONST, 0, DERIVCONST);

  // we start the game not drilling
  drilling = false;
  // we start the game not guarding
  guarding = false;
	
}

void loop() {
    //maxDrills = 2*(game.getNumSamplesHeld()>=4||game.getFuelRemaining()<0.09f||api.getTime()>165) + 3*(game.getNumSamplesHeld()<4);
    // variable used in multiple contexts to save space
    float flocal;
    
    // stores the position we are going to
    float positionTarget[3];
    
    // contains the position of the origin
    float zeroVec[3];
    memset(zeroVec, 0.0f, 12); // sets first 3 floats to 0
    
    // our state
    float myState[13]; // we use quaternions for our own attitude
    
    
    // arrays used in multiple contexts to save space
    float usefulVec[3];
    int usefulIntVec[2];
    
    // the square we are currently on
    int mySquare[3];
    
    // their state
    float enState[12];
    

    // contains the change in the enemy's score since the last second
    float enDeltaScore = game.getOtherScore() - enScore;
    // updates the enemy's score
    enScore = enScore + enDeltaScore;
    
    // if a sample is ready to pick up, pick it up
    if (game.checkSample()) {
        // if we want to drill more than 5, we have to drop the last one
        game.dropSample(4);
        game.pickupSample();
    }
    
    // if we are at the base, drop off our samples
    if (game.atBaseStation()) {
        for (int i=0; i<5; i++) {
            game.dropSample(i);
        }
        //after dropping, find a new square
        newLoc = true;
    }
        
    int samplesHeld = game.getNumSamplesHeld();

    // don't drop off if we have no samples
    if (samplesHeld<2) {
        dropping = false;
    }
    
    // update state arrays
    api.getMySphState(myState);
    api.getOtherZRState(enState);
    if (dist(myPos, enPos)<0.08 && myPos[2] > 0.24) {
        newLoc=true;
    }
    //convert quaternion into attitude vector
    float myAtt[3];
    zeroVec[0] -= 1; // we avoid a new vector to save space
    api.quat2AttVec(zeroVec, myQuatAtt, myAtt);
    zeroVec[0] += 1; // zeroVec should be restored to its original value
    
    
    game.pos2square(myPos,mySquare);
    
    // @ USEFUL VEC IS NOW THE 3D POSITION OF OUR SQUARE @
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    if (geyserOnMe
    or game.getDrills(mySquare) > MAXDRILLS - 1) {
        DEBUG(("Broke"));
        if (samplesHeld > 3) {
            dropping = true;
        }
        newLoc = true;
        drilling = false;
    }
    // must be larger than all distances we check
    float minDist = 100;

                  
    // selects the best square to drill
    if (newLoc and !game.checkSample() and not drilling) {
        for (int i = -6; i<6; i++){ // checks all clumps of 4 squares (blocks)
        // and sees which is both closest to us and in the center.
            for (int j = -8; j<8; j++){
                if (i*j !=0 and (i < -4 or i > 3 or j < -4 or j > 3)) {
                    usefulIntVec[0] = i;
                    usefulIntVec[1] = j;
                    if (game.getTerrainHeight(usefulIntVec) < 0.44f && game.getDrills(usefulIntVec) < 1) {
                        float sqrPos [3];
                        game.square2pos(usefulIntVec, sqrPos);
                        sqrPos[2] = game.getTerrainHeight(usefulIntVec);
                        if (dist(usefulVec, sqrPos) < minDist && dist(enPos, sqrPos) > 0.25f) {
                            minDist = dist(usefulVec, sqrPos);
                            memcpy(siteCoords, usefulIntVec, 8);
                        }
                    }
                }
            }
        }
        // once we have selected a new drill square, we set this flag so that
        // we don't immediately pick a different one
        newLoc = false;
    }
    
    // if they found the 10
    if (enDeltaScore == 3.5f) {
        game.pos2square(enPos, siteCoords);
        siteCoords[0] *= -1;
        siteCoords[1] *= -1;
        newLoc = false;
    }
    
    // stores whether we are at the square we are targetubg
    bool onSite = (mySquare[0] == siteCoords[0] and mySquare[1] == siteCoords[1]);
    
    // drilling translational movement
    if ((not dropping) and (not guarding)) {
        // set positionTarget to the drill square
        game.square2pos(siteCoords, positionTarget);
        // adjust positionTarget to the corner of a square
        // avoid crashing into terrain
        positionTarget[2] = game.getTerrainHeight(siteCoords) - 0.13f;
        for (int i = 0; i < 2; i++) {
            positionTarget[i] -=0.024f;
        }
        DEBUG(("target square: (%i, %i)", siteCoords[0],siteCoords[1]));
        DEBUG(("current square: (%i %i)", mySquare[0],mySquare[1]));
        
        
        // @ USEFUL VEC IS NOW OUR ATTITUDE TARGET @
        // check drilling conditions
        if ((mathVecMagnitude(myVel, 3) < 0.01f
        and (mathVecMagnitude(myRot, 3) < 0.035f or drilling)
        and (onSite))
        and not game.getDrillError()
        and (myPos[2] - positionTarget[2] < 0.02f 
        and myPos[2] - positionTarget[2] > -0.02f)) {
            usefulVec[0] = -myAtt[1];
            usefulVec[1] = myAtt[0];
            usefulVec[2] = myAtt[2] * -5.0f;
            if (!game.getDrillEnabled()) {
                game.startDrill();
            }
            
            zeroVec[2] = 0.04f;
            drilling = true;
            
        }
        // if we are not aligned to start drilling
        else {
            usefulVec[0] = myAtt[0];
            usefulVec[1] = myAtt[1];
            usefulVec[2] = 0.0f;
            
            DEBUG(("Slowing"));
            drilling = false;
            zeroVec[2] = -0.04f;
        }
        api.setAttitudeTarget(usefulVec);
       
    }
    // if we are not drilling, then drop off samples
    else {
        memcpy(positionTarget, myPos, 12);
            // if we have more points than them and
            // we're closer to the origin than them
            guarding = (game.getScore() > enScore and game.getScore() > 38.0f 
                        and game.getFuelRemaining()>0.06f && (mathVecMagnitude(enPos, 3) > mathVecMagnitude(myPos, 3) + 0.1f
                             or guarding));
            if (guarding) {
                memcpy(positionTarget, enPos, 12);
            }
            // scale either enPos or myPos depending if we are guarding or not
            #define BASE_STATION_RADIUS 0.24f
            // depending on whether or not we are guarding, there are two
            // different target distances from the origin
            scale(positionTarget, (((BASE_STATION_RADIUS-0.01f) - (0.11f * guarding))
            / mathVecMagnitude(positionTarget, 3)));
        // rotate to satisfy drop off requirement
        zeroVec[2] -= 1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2] = 0.0f; //Slow down        
    }
    
    PRINTVEC("myQuatAtt", myQuatAtt);
    // invert rotation
    // now rotates fundamental basis rotation vector (k-hat) to our basis
    myQuatAtt[3] *= -1;
    api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    
    if (drilling) {
        api.setAttRateTarget(usefulVec);
        //Cornering now is always towards center if we have samples when we start, as we will likely have to upload next
        positionTarget[0] += (0.01f)*(((positionTarget[0]<0)-(positionTarget[0]>0))&&samplesHeld>3);
        positionTarget[1] += (0.01f)*(((positionTarget[1]<0)-(positionTarget[1]>0))&&samplesHeld>3);
    }
    
    // if our drill breaks or we get a geyser, stop the current drill
    
    // @ FLOCAL IS NOW REMAINING FUEL @
    flocal = game.getFuelRemaining();
    // if we have samples and either time or fuel is running out
    if (!drilling //not in the middle of drilling (possibly the 3rd drill which gives 3 pts)
    and (api.getTime()>157 // time is greater than 157 
    or (flocal < 0.16f and flocal >  0.07f))) {
        // drop off what we have
        dropping=true;
    }

    // prioritize dropping over drilling
    if (dropping) {
        drilling = false;
    }
    
    // if we have low fuel and we are moving towards the terrain
    if (flocal < 0.03f and myVel[2] > 0.0f) {
        // move up to avoid terrain crash penalties
        memcpy(positionTarget, myPos, 12);
        positionTarget[2] -= 1.0f;
    }
    
    // turn off the drill if we aren't in drill mode
    if (not drilling) {
        game.stopDrill();
    }
    bool scDown = 0;
    if (samplesHeld%2==0 && samplesHeld>0 && drilling) {
        newLoc = true;
        scDown++;
        DEBUG(("SCALE DOWN MVMENT"));
    }

    // create aliases for variables with potentially confusing names
	#define destination positionTarget
    #define fvector usefulVec
    
    #define ACCEL .014f
    
    // store the vector from us to the destination in fvector
    mathVecSubtract(fvector, destination, myPos, 3);
    
    // @ FLOCAL IS NOW A FACTOR RELATED TO PROXIMITY TO OUR DESTINATION @
    
    flocal = 0.065f / (0.0325f + mathVecMagnitude(fvector, 3));
    scale(myVel, flocal);
    mathVecSubtract(fvector, fvector, myVel, 3);
    scale(fvector, 0.22f);
    while (mathVecMagnitude(fvector,3)>0.045f){
        scale(fvector,.99f);
    }
    //if on 3rd drill
    for (int i = 0; i < 3; i++) {
        fvector[i]*=(0.25f*scDown+!scDown);
    }
    // if we're on a geyser
    if (geyserOnMe) {
        scale(fvector, 15 / mathVecMagnitude(fvector, 3));
    }
    //geyserCollisionAvoidanceCode
    float checker[2];
    memcpy(checker, fvector, 8);
    mathVecNormalize(checker, 2);
    for (int i = 0; i < 2; i++) {
        checker[i]*=0.12;
        checker[i]+=myPos[i];
    }
    int checkSqrs[2];
    game.pos2square(checker, checkSqrs);
    if (game.isGeyserHere(checkSqrs)) {
        fvector[0]*=-1;
        fvector[1]*=2;
    }
    api.setVelocityTarget(fvector);
}

/**
 * @param vec1 - a 3D position vector
 * @param vec2 - another 3D position vector
 * @return the euclidean distance between vec1 and vec2
 */
float dist(float* vec1, float* vec2) {
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}

/**
 * @param coorVec - a square
 * Changes zeros to ones to reflect that squares cannot
 * have a row or column equal to zero
 */
void fixEdge(int coorVec[2]) {
    if (coorVec[1] == 0) {
        coorVec[1] = 1;
    }
    if (coorVec[0] == 0) {
        coorVec[0] = 1;
    }
}

/**
 * @param vec - a length three float array
 * @param scale - a float coefficient
 * multiplies each element in vec by scale
 */
void scale (float* vec, float scale) {
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}