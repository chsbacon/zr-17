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

// total number of samples drilled
int samples;

// which corner of the square we are drilling (0-3)
int corner;

// the enemy's score
float enScore;

void init(){
    // they start with 0 points
    enScore=0;
    
    // we begin the game looking for a square
    newLoc=true;
	
	// movement parameters
	#define SPEEDCONST .35f
    #define DERIVCONST 2.35f
    api.setPosGains(SPEEDCONST,0,DERIVCONST);

    // we begin the game not dropping samples
    dropping=false;
    
    // we start with no samples
    samples=0;
    
    // we start the game not drilling
	drilling=false;
}

void loop() {
    // variable used in multiple contexts to save space
    float flocal;
    
    // stores the position we are going to
    float positionTarget[3];
    
    // contains the position of the origin
    float zeroVec[3];
    memset(zeroVec, 0.0f, 12);// Sets all places in an array to 0
    
    // update our state
    float myState[13]; // we use quaternions for our own attitude
    api.getMySphState(myState);
    
    // update their state
    float enState[12];
    api.getOtherZRState(enState);
    
    // arrays used in multiple contexts to save space
    float usefulVec[3];
    int usefulIntVec[2];
    
    // the square we are currently on
    int mySquare[3];
    game.pos2square(myPos,mySquare);
    
    // contains the change in the enemy's score since the last second
    float enDeltaScore = game.getOtherScore() - enScore;
    // updates the enemy's score
    enScore = enScore + enDeltaScore;
    
    // if a sample is ready to pick up, pick it up
    if (game.checkSample()) {
        // if we want to drill more than 5, we have to drop the last one
        game.dropSample(4);
        
        game.pickupSample();
        samples += 1;
    }
    
    // if we are at the base, drop off our samples
    if (game.atBaseStation()){
        for (int i=0; i<5; i++){
            game.dropSample(i);
        }
        //after dropping, find a new square
        newLoc = true;
        dropping = false;
    }
    //convert quaternion into attitude vector
    float myAtt[3];
    zeroVec[0] -= 1; // we avoid a new vector to save space
    api.quat2AttVec(zeroVec, myQuatAtt, myAtt);
    zeroVec[0] += 1; // zeroVec should be restored to its original value
    
    // @ USEFUL VEC IS NOW THE 3D POSITION OF OUR SQUARE @
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    
    // must be larger than all distances we check
    float maxDist=100;
    
    // contains a position above us, so that we favor high drill spots
    float modPos[3];
    memcpy(modPos, myPos, 8);
    modPos[2] = 0.2f;
    
    // selects the best square to drill
    if (newLoc and !game.checkSample() and not drilling){
        for (int i= -6; i<6; i++){ // This checks all of the sets of 4 squares
        // and sees which is both closest to us and in the center.
            for (int j = -8; j<8; j++){ // excludes the center
                if (i*j != 0 and (i < -3 or i > 2 or j < -3 or j > 2)){
                    int heights[4]; // contains the number of squares at each level
                    memset(heights, 0, 16);
                    // Decides which corner to drill
                    for (int a=0; a<4; a++){ // cycles through 4 corners
                        usefulIntVec[0] = i + a%2; // goes 0 1 0 1
                        usefulIntVec[1] = j + a/2; // goes 0 0 1 1
                        fixEdge(usefulIntVec);
                        
                        //tallies up how many squares within each height exist in a block 
                        #define POINTS_PER_HEIGHT 12.5f
                        #define FLAT_DEFICIT -5.0f
                        int index = (game.getTerrainHeight(usefulIntVec) * POINTS_PER_HEIGHT) + FLAT_DEFICIT;
                        if (game.getDrills(usefulIntVec) == 0 
                        or (mySquare[0] - i == (mySquare[0] - i) % 2 
                        and (mySquare[1] - j) == (mySquare[1] - j) % 2)){
                            heights[index] += 1;
                        }
                    }
                    float goodHeight=.4f;
                    for (int other=1;other<4;other++){
                        if (heights[other]==3){
                            heights[0]=3;
                            goodHeight=other*.08f+.4f;// silent fail specific to goodheight
                        }
                    }
                    // DEBUG(("%i %i %i %i", heights[0],heights[1],heights[2],heights[3]));
                    if (heights[0]>1){
                        // DEBUG(("GROUP"));
                        for (int a=0;a<4;a++){
                            usefulIntVec[0]=i+a%2;usefulIntVec[1]=j+a/2;
                            fixEdge(usefulIntVec);
                            game.square2pos(usefulIntVec,usefulVec);
                            usefulVec[2]=game.getTerrainHeight(usefulIntVec);
                            float score=dist(usefulVec,modPos);
                            if (usefulVec[2]==goodHeight
                            and score<maxDist 
                            and game.getDrills(usefulIntVec)<1 
                            // and i*i+j*j>8 and i*i<16 and j*j<16 
                            and dist(enPos,usefulVec)>.35f){
                                memcpy(siteCoords,usefulIntVec,8);
                                
                                corner=a;
                                
                                //  EBUG(("Changed %f", score));
                                maxDist = score;
                            }
                        }
                    }
                    
                    
                }
            }
        }
        newLoc=false;
    }
    if (enDeltaScore==3.5f){
        game.pos2square(enPos,siteCoords);
        siteCoords[0]*=-1;
        siteCoords[1]*=-1;
    }
    
    bool onSite=(mySquare[0]==siteCoords[0] and mySquare[1]==siteCoords[1]);
    
    // drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    if ((not dropping)){
        // adjust positiontarget to the corner of a square
        game.square2pos(siteCoords,positionTarget);
        positionTarget[0]+=((corner%2)*-2+1)*0.027f;
        positionTarget[1]+=((corner/2)*-2+1)*0.027f;
        
        
        // positionTarget[2]=myPos[2];// vertical movement to avoid terrain
        if (!onSite and (game.getTerrainHeight(mySquare)>game.getTerrainHeight(siteCoords) or dist(myPos,positionTarget)>.05f)){
            positionTarget[2]=.27f;
            DEBUG(("O"));
            if (myPos[2]>.29f){
                DEBUG(("U"));
                memcpy(positionTarget,myPos,8);
            }
        }
        else{
            DEBUG(("D"));
            positionTarget[2]=game.getTerrainHeight(siteCoords)-.13f;
        }
        DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
        DEBUG(("%i %i", mySquare[0],mySquare[1]));
        
        
        // if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
        and (onSite))
        and not game.getDrillError()
        and (myPos[2]-positionTarget[2]<.02f and myPos[2]-positionTarget[2]>-0.02f)){
            usefulVec[0]=-myAtt[1];usefulVec[1]=myAtt[0];usefulVec[2]=myAtt[2]*-5;
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            zeroVec[2]=.04f;
            drilling=true;
            
        }
        else{
            usefulVec[0]=myAtt[0];usefulVec[1]=myAtt[1];usefulVec[2]=0;
            
            //  memcpy(usefulVec,myRot,12);
            //  scale(usefulVec,-0.6f);
            //  api.setAttRateTarget(usefulVec);
            DEBUG(("Slowing"));
            drilling=false;
            zeroVec[2]=-.04f;
        }
        api.setAttitudeTarget(usefulVec);
       
    }
    // otherwise, drop off our samples
    else{
        memcpy(positionTarget,myPos,12);
        if (myPos[2]>.29f){
            positionTarget[2]=.05f;
        }
        else{
            // maybe take out the mathvecMagnitude expression for codesize
            scale(positionTarget,.23f/mathVecMagnitude(positionTarget,3));// go to a position that is .09 in the same direction at the enemy. In other words, between them and the origin.
        }
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]=.05f;// Slow down
    }
    
    PRINTVEC("myQuatAtt",myQuatAtt);
    myQuatAtt[3]*=-1;// inverts rotation - now rotates fundamental basis rotation vector (k-hat) to our basis
    api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    if (drilling){
        api.setAttRateTarget(usefulVec);
    }
    // if our drill breaks or we get a geyser, stop the current drill
    flocal=game.getFuelRemaining();
    if (game.getDrillError() 
    or geyserOnMe 
    or game.getDrills(mySquare)>MAXDRILLS-1){
        DEBUG(("Broke"));
        if (game.getNumSamplesHeld()>3){
            dropping=true;
        }
        newLoc=true;
        drilling=false;
    }
    if (game.getNumSamplesHeld()>1 and ((!(int)((api.getTime()-161)/4)) or (flocal<.16f and flocal> .12f))){// at the end of the game, drop off what we have
        dropping=true;
        
    }
    if (mathVecMagnitude(enPos,3)<.18f){
        dropping=false;
    }
    if (dropping){
        drilling=false;
    }
    if (flocal<.03f and myVel[2]>0){
        memcpy(positionTarget,myPos,12);
        positionTarget[2]-=1;
    }
    if (!game.getNumSamplesHeld()){// don't drop off with no samples
        dropping=false;
    }
    if (not drilling){// don't drill if we aren't drilling
        game.stopDrill();
    }

    
    
	#define destination positionTarget// This (next 20 or so lines) is movement code.
	// It is fairly strange - we will go over exactly how it works eventually
    #define fvector usefulVec
    
    #define ACCEL .014f
    // mathVecSubtract(fvector, destination, myPos, 3);// Gets the vector from us to the target
    mathVecSubtract(fvector, destination, myPos, 3);
    flocal=0.0333333f/(.05f+mathVecMagnitude(fvector,3));// Just storing this value as a functional boolean
    scale(myVel,.2f+flocal);
    mathVecSubtract(fvector,fvector,myVel,3);
    scale(fvector,.27f-.09f*flocal);
    if (geyserOnMe){
        //  flocal=mathVecMagnitude(fvector,3)/15;
        //  fvector[0]/=flocal;
        //  fvector[1]/=flocal;
        scale(fvector,15/mathVecMagnitude(fvector,3));
        fvector[2]=0.01f;
    }
    api.setVelocityTarget(fvector);
}
float dist(float* vec1, float* vec2) {
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}
void fixEdge(int coorVec[2]){
    if (coorVec[1]==0){
        coorVec[1]=1;
    }
    if (coorVec[0]==0){
        coorVec[0]=1;
    }
}
void scale (float* vec, float scale) {// This function scales a length-3 vector by a coeff.
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}