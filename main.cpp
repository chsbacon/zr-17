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
int siteCoords[10];

// flag for if we are retargeting
bool newLoc;

// flag for if we are moving back to drop
bool dropping;

// flag for if we are drilling
bool drilling;

// total number of samples drilled
int samples;

// which corner of the square we are drilling (0-3)
int corner;

// the enemy's score
float enScore;

bool justHitGeyser;

int crrSquare[10];

bool just10[2];
float befScore;

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
  
  justHitGeyser = false;
  
  just10[0] = false;
  just10[1] = false;
  befScore = 0;
  
}

void loop() {
    // variable used in multiple contexts to save space
    float flocal;
    
    // stores the position we are going to
    float positionTarget[10];
    
    // contains the position of the origin
    float zeroVec[10];
    memset(zeroVec, 0.0f, 12); // sets first 3 floats to 0
    
    // our state
    float myState[13]; // we use quaternions for our own attitude
    
    
    // arrays used in multiple contexts to save space
    float usefulVec[10];
    int usefulIntVec[10];
    
    // the square we are currently on
    int mySquare[10];
    
    // their state
    float enState[17];
    

    // contains the change in the enemy's score since the last second
    float enDeltaScore = game.getOtherScore() - enScore;
    // updates the enemy's score
    enScore = enScore + enDeltaScore;
    
    // if a sample is ready to pick up, pick it up
    if (game.checkSample()) {
        // if we want to drill more than 5, we have to drop the last one
        game.dropSample(game.getNumSamplesHeld()>=5?0:4);
        samples += (bool)(game.pickupSample());
    }
    
    // if we are at the base, drop off our samples
    if (game.atBaseStation() bitand game.getNumSamplesHeld() != 0) {
        for (int i=0; i<5; i++) {
            game.dropSample(i);
        }
        //after dropping, find a new square
        newLoc = true;
        dropping = false;
        just10[1] = false;
    }
    
    // update state arrays
    api.getMySphState(myState);
    api.getOtherZRState(enState);
    
    DEBUG(("BAAA %d %d", just10[0], just10[1]));
    
    //convert quaternion into attitude vector
    float myAtt[10];
    zeroVec[0]--; // we avoid a new vector to save space
    api.quat2AttVec(zeroVec, myQuatAtt, myAtt);
    zeroVec[0]++; // zeroVec should be restored to its original value
    
    
    game.pos2square(myPos,mySquare);
    
    // @ USEFUL VEC IS NOW THE 3D POSITION OF OUR SQUARE @
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    
    if(drilling bitand game.isGeyserHere(mySquare))
        justHitGeyser = true;
        
    corner = myPos[1]<0?1:-1;
    int corner2 = myPos[0]<0?1:-1;
    
    // must be larger than all distances we check
    float minDist = 100.0;
    
    if (newLoc bitand !game.checkSample() bitand not drilling bitand !just10[0]) {
         for (int i = -6; i <= 6; i++) {
             for (int j = -8; j <= 8; j++) {
                
                /*bool cond2 = i*corner2 <= mySquare[0]*corner2 &&
                j*corner <= mySquare[1]*corner;*/
                
                //DEBUG(("%d %d %d %d %d %d", i, j, i*corner2 < mySquare[0]*corner2, j*corner < mySquare[1]*corner, corner, mySquare[1]));
                
                if(justHitGeyser bitand
                i*corner2 <= mySquare[0]*corner2 bitand
                j*corner <= mySquare[1]*corner)
                    continue;
                
                 if (i*j !=0 bitand (i < -2 bitor i > 2 bitor j < -2 bitor j > 2)) {
                     
                    usefulIntVec[0] = i;
                    usefulIntVec[1] = j;
                    game.square2pos(usefulIntVec, usefulVec);
                    usefulVec[2] = game.getTerrainHeight(usefulIntVec);
                    game.pos2square(myPos, crrSquare);
                    
                    int magicX = (usefulIntVec[0]-crrSquare[0]);
                    int magicY = (usefulIntVec[1]-crrSquare[1]);
                    bool cont = false;
                    
                    for(int i2 = crrSquare[0] + magicX; i2 < usefulIntVec[0]; i += magicX) {
                        for(int j2 = crrSquare[1] + magicY; j2 < usefulIntVec[1]; j += magicY) {
                            if(i2*j2 == 0)
                                continue;
                            usefulIntVec[0] = i2;
                            usefulIntVec[1] = j2;
                            if(game.isGeyserHere(usefulIntVec))
                                cont = true;
                        }
                    }
                     
                    float di = dist(usefulVec, myPos);
                     
                    if(usefulVec[2] != 0.40f || dist(usefulVec, enState) < 0.35f || game.getDrills(usefulIntVec) != 0 || cont)
                        continue;
                        
                    if(minDist > di) {
                        
                        //DEBUG(("BAAA"));
                        
                        memcpy(siteCoords, usefulIntVec, 8);
                        minDist = di;
                        
                    }
                     
                 }
             }
         }
         newLoc = false;
         justHitGeyser = false;
    }
    
    enPos[0] *= -1;
    enPos[1] *= -1;
    float DI = dist(enPos, myPos);
    
    // if they found the 10
    DEBUG(("TT %f %f", game.getFuelRemaining() - DI/2.55f, api.getTime() + DI*4));
    game.pos2square(enPos, usefulIntVec);
    if (enDeltaScore == 3.5f bitand (game.getFuelRemaining() - DI/2.55f > 0.27 bitand api.getTime() + DI*4 <= 120.0f) bitand game.getDrills(usefulIntVec) == 0) {
        
        // drill at the other 10
        game.pos2square(enPos, siteCoords);
        drilling = false;
        just10[0] = true;
        
    }
    
    // stores whether we are at the square we are targetubg
    bool onSite = (mySquare[0] == siteCoords[0] bitand mySquare[1] == siteCoords[1]);
    
    // drilling translational movement
    if ((not dropping)) {
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
            // scale either enPos or myPos depending if we are guarding or not
            #define BASE_STATION_RADIUS 0.24f
            // depending on whether or not we are guarding, there are two
            // different target distances from the origin
            scale(positionTarget, ((BASE_STATION_RADIUS-0.01f)) / mathVecMagnitude(positionTarget, 3));
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
    
    if (drilling)
        api.setAttRateTarget(usefulVec);
    
    // if our drill breaks or we get a geyser, stop the current drill
    if ((game.getDrillError() 
    or geyserOnMe
    or game.getDrills(mySquare) > MAXDRILLS - 1) and drilling) {
        //DEBUG(("Broke"));
        if (game.getNumSamplesHeld() > 3 || (just10[1] and game.getNumSamplesHeld() > 1)) { // !!!
            dropping = true;
        }
        newLoc = true;
        drilling = false;
    }
    
    if(game.getScore() - befScore == 3.5f) {
        just10[0] = false;
        just10[1] = true;
    }
    
    befScore = game.getScore();
    
    // @ FLOCAL IS NOW REMAINING FUEL @
    flocal = game.getFuelRemaining();
    // if we have samples and either time or fuel is running out
    if (game.getNumSamplesHeld() > 1 
    and ((!(int)((api.getTime() - 161) / 4)) // time is within 4 sec of 161 
    or (flocal < 0.16f and flocal >  0.9f)
    or !drilling and api.getTime() > 170)) {
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
    scale(fvector, 0.25f - (0.09f * flocal));
    
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
