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

// total number of samples drilled
int samples;

// which corner of the square we are drilling (0-3)
int corner;

// the enemy's score
float enScore;

// right after they found the 10, before going towards it
bool justFoundThe10;

bool justHitGeyser;

int red;

void init(){
    
  // they start with 0 points
  enScore = 0;

  // we begin the game looking for a square
  newLoc = true;
	
	// movement parameters
	#define SPEEDCONST 0.35f
  #define DERIVCONST 2.35f
  api.setPosGains(SPEEDCONST, 0, DERIVCONST);

  // we begin the game not dropping samples
  dropping = false;

  // we start with no samples
  samples = 0;

  // we start the game not drilling
  drilling = false;
  // we start the game not guarding
  guarding = false;
  
  justFoundThe10 = false;
  
  red = 2;
  
  justHitGeyser = false;
	
}

void loop() {
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
        samples += (bool)(game.pickupSample());
    }
    
    // if we are at the base, drop off our samples
    if (game.atBaseStation() && game.getNumSamplesHeld() != 0) {
        for (int i=0; i<5; i++) {
            game.dropSample(i);
        }
        //after dropping, find a new square
        newLoc = !justFoundThe10;
        dropping = false;
    }
    
    // update state arrays
    api.getMySphState(myState);
    api.getOtherZRState(enState);
    
    //if(red == 2)
    red = (myState[1]>0?1:-1);
    
    //convert quaternion into attitude vector
    float myAtt[3];
    zeroVec[0] -= 1; // we avoid a new vector to save space
    api.quat2AttVec(zeroVec, myQuatAtt, myAtt);
    zeroVec[0] += 1; // zeroVec should be restored to its original value
    
    
    game.pos2square(myPos,mySquare);
    
    // @ USEFUL VEC IS NOW THE 3D POSITION OF OUR SQUARE @
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    
    if(drilling and game.isGeyserHere(mySquare))
        justHitGeyser = true;
        
    corner = myPos[1]<0?1:-1;
    int corner2 = myPos[0]<0?1:-1;
    
    // must be larger than all distances we check
    float minDist = 100.0;
    // contains a position above us, so that we favor high drill spots
    float modPos[3];
    memcpy(modPos, myPos, 8);
    modPos[2] = -10.0f;
    
    if (newLoc and !game.checkSample() and not drilling) {
        DEBUG(("TESTTTTTT %d", justHitGeyser));
         for (int i = -6; i <= 6; i++) {
             for (int j = -8; j <= 8; j++) {
                
                if(justHitGeyser &&
                i*corner2*-1 + 1 >= mySquare[0]*corner2*-1 &&
                j*corner*-1 + 1 >= mySquare[1]*corner*-1)
                    continue;
                
                 if (i*j !=0 and (i < -2 or i > 2 or j < -2 or j > 2)) {
                     
                    usefulIntVec[0] = i;
                    usefulIntVec[1] = j;
                    game.square2pos(usefulIntVec, usefulVec);
                    usefulVec[2] = game.getTerrainHeight(usefulIntVec);
                     
                    float di = dist(usefulVec, myPos);
                     
                    if(usefulVec[2] != 0.40f || dist(usefulVec, enState) < 0.35f || game.getDrills(usefulIntVec) != 0)
                        continue;
                        
                    if(minDist > di) {
                        
                        //DEBUG(("BAAA"));
                        
                        memcpy(siteCoords, usefulIntVec, 8);
                        minDist = di;
                        corner = 0;
                        
                    }
                     
                 }
             }
         }
         newLoc = false;
         justHitGeyser = false;
    }
    
    // if they found the 10
    if (enDeltaScore == 3.5f) {
        
        // drill at the other 10
        game.pos2square(enPos, siteCoords);
        siteCoords[0] *= -1;
        siteCoords[1] *= -1;
        justFoundThe10 = true;
        
    }
    
    // stores whether we are at the square we are targetubg
    bool onSite = (mySquare[0] == siteCoords[0] and mySquare[1] == siteCoords[1]);
    
    // drilling translational movement
    if ((not dropping) and (not guarding)) {
        // set positionTarget to the drill square
        game.square2pos(siteCoords, positionTarget);
        
        // adjust positionTarget to the corner of a square
        positionTarget[0] += corner2 * 0.029f;
        positionTarget[1] += corner * 0.029f;
        
        positionTarget[2] = game.getTerrainHeight(siteCoords) - 0.13f;
        
        
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
            
            //DEBUG(("Slowing"));
            drilling = false;
            zeroVec[2] = -0.04f;
        }
        api.setAttitudeTarget(usefulVec);
        justFoundThe10 = false;
       
    }
    // if we are not drilling, then drop off samples
    else {
        memcpy(positionTarget, myPos, 12);
        
        // if we are down in the terrain
        if (myPos[2] > 0.29f) {
            // move straight up
            positionTarget[2] = 0.05f;
        }
        else {
            // if we have more points than them and
            // we're closer to the origin than them
            guarding = (game.getScore() > enScore and game.getScore() > 38.0f 
                        and (mathVecMagnitude(enPos, 3) > mathVecMagnitude(myPos, 3) + 0.1f
                             or guarding));
            if (guarding) {
                memcpy(positionTarget, enPos, 12);
            }
            // scale either enPos or myPos depending if we are guarding or not
            #define BASE_STATION_RADIUS 0.24f
            // depending on whether or not we are guarding, there are two
            // different target distances from the origin
            scale(positionTarget, ((BASE_STATION_RADIUS-0.01f) - (0.16f * guarding))
                  / mathVecMagnitude(positionTarget, 3));
        }
        // rotate to satisfy drop off requirement
        zeroVec[2] -= 1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2] = 0.0f; //Slow down        
    }
    
    //PRINTVEC("myQuatAtt", myQuatAtt);
    
    // invert rotation
    // now rotates fundamental basis rotation vector (k-hat) to our basis
    myQuatAtt[3] *= -1;
    api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    
    if (drilling) {
        api.setAttRateTarget(usefulVec);
    }
    
    // if our drill breaks or we get a geyser, stop the current drill
    if ((game.getDrillError() 
    or geyserOnMe
    or game.getDrills(mySquare) > MAXDRILLS - 1) and drilling) {
        //DEBUG(("Broke"));
        if (game.getNumSamplesHeld() > 3) {
            dropping = true;
        }
        newLoc = true;
        drilling = false;
    }
    // @ FLOCAL IS NOW REMAINING FUEL @
    flocal = game.getFuelRemaining();
    // if we have samples and either time or fuel is running out
    if (game.getNumSamplesHeld() > 1 
    and ((!(int)((api.getTime() - 161) / 4)) // time is within 4 sec of 161 
    or (flocal < 0.16f and flocal >  0.9f))) {
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
    
    //don't drop off if we have no samples
    if (!game.getNumSamplesHeld()) {
        dropping = false;
    }
    
    // turn off the drill if we aren't in drill mode
    if (not drilling) {
        game.stopDrill();
    }

    // create aliases for variables with potentially confusing names
	#define destination positionTarget
    #define fvector usefulVec
    
    #define ACCEL .014f
    
    // store the vector from us to the destination in fvector
    mathVecSubtract(fvector, destination, myPos, 3);
    
    // @ FLOCAL IS NOW A FACTOR RELATED TO PROXIMITY TO OUR DESTINATION @
    
    flocal = 0.05f / (0.05f + mathVecMagnitude(fvector, 3));
    scale(myVel, 0.2f + flocal);
    mathVecSubtract(fvector, fvector, myVel, 3);
    scale(fvector, 0.27f - (0.09f * flocal));
    
    // if we're on a geyser
    if (geyserOnMe) {
        // don't bother moving vertically
        fvector[2] = 0.0f;
        scale(fvector, 5 / mathVecMagnitude(fvector, 3));
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