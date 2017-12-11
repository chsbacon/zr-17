// Macros / pointers
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

int targetSquare[3];

// behaviour flags
bool dropping;
bool drilling;

int samples;
int corner;

// we only store half of the possible ten squares
#define TEN_ZONE_HEIGHT 8
#define TEN_ZONE_WIDTH 12
bool concArray[TEN_ZONE_WIDTH][TEN_ZONE_HEIGHT];

int tenCoords[2];
bool tenFound;
bool concFound;

void init() {
    // movement parameters
    #define SPEEDCONST 0.35f
    #define DERIVCONST 2.35f
    api.setPosGains(SPEEDCONST, 0, DERIVCONST);
    
    // initialize everything
    dropping = false;
    drilling = false;
	concFound = false;
	samples = 0;
	memset(concArray, true, 96);
	tenFound = false;
}

void loop() {
    // multi-purpose variable
    float flocal;
    
    // where we are going
    float positionTarget[3];
    
    // never used as (0,0,0) even though it is sometimes that value
    float zeroVec[3];
    
    float myState[13];
    api.getMySphState(myState);
    
    float enState[12];
    api.getOtherZRState(enState);
    
    float usefulVec[3];
    int usefulIntVec[2];
    int mySquare[3];

    memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
    
    
    // if a sample is ready for pick-up
    if (game.checkSample()) {
        // dumps a sample if we are already full, otherwise does nothing
        game.dropSample(4);
        
        //pick up the sample and record it
        samples += (bool)(game.pickupSample());
    }
    
    
    if (game.atBaseStation()) {
        // drop off
        for (int i=0; i<5; i++){
            game.dropSample(i);
        }
        dropping = false;
    }
    
    // turn quaternion attitude into nice normal vector
    float myAtt[3];
    zeroVec[0] -= 1;
    api.quat2AttVec(zeroVec, myQuatAtt, myAtt);
    zeroVec[0] += 1;
    
    game.pos2square(myPos, mySquare);
    // store the center of our current square in usefulVec
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    
    float maxDist = 100;
    
    // are we on our target square?
    bool onSite = (mySquare[0]==targetSquare[0] and mySquare[1]==targetSquare[1]);
    
    int nextSquare[2];
    
    // get the analyzer
    if (!game.hasAnalyzer()){
        // coordinates of one analyzer
        positionTarget[0] = 0.3f;
        positionTarget[1] = -0.48f;
        // if we are closer to the other analyzer, go for it instead
        if (myPos[1] > 0){
            scale(positionTarget, -1);
        }
        // the height of the analyzers
        positionTarget[2] = -0.16f;
    }
    // searching
    else if (!tenFound){
        // concentration of the square below us
        float concentration = game.analyzeTerrain();
        DEBUG(("%f", concentration));
        
        // if we are above the ten
        if (concentration > 0.9f) {
            // take note
            game.pos2square(myPos, tenCoords);
            tenFound = true;
        }
        else {
            // reflect onto the top half if not already there
            if (myPos[1] < 0){
                mySquare[0] *= -1;
                mySquare[1] *= -1;
            }
            // rule this out as a possible ten
            // we need to map squares to positions in concArray
            // special code to handle the lack of a zero column in the game
            concArray[mySquare[0] + (TEN_ZONE_WIDTH/2) - (mySquare[0] > 0)]
                [mySquare[1] - 1] = false;
            
            int valCount = 0;
            memset(targetSquare, 0, 8);
            for (int i = -TEN_ZONE_WIDTH; i <= TEN_ZONE_WIDTH; i++) {
                // don't check zero column
                if (i) {
                    for (int j = 1; j <= TEN_ZONE_HEIGHT; j++) {
                        // store the square being evaluated
                        usefulIntVec[0] = i;
                        usefulIntVec[1] = j;
                        int sqDist = intDist(usefulIntVec, mySquare);
                        
                        // the mirror of the square being evaluated
                        usefulIntVec[0] *= -1;
                        usefulIntVec[1] *= -1;
                        int sqDist2 = intDist(usefulIntVec,mySquare);
                        
                        if (sqDist > sqDist2){
                            sqDist = sqDist2;
                        }
                        
                        // sqDist now contains the squared distance between
                        // us and the closer of the square being examined and
                        // its mirror
                        
                        bool inRadius = sqDist <= (concentration>0.5f ? 3 : 7);
                        // if we are over a 3 or a 6
                        if (concentration > 0.15f) {
                            concFound=true;
                            inRadius=!inRadius;
                        }
                        if (inRadius){
                            concArray[i+6-(i>0)][j-1]=false;
                        }
                        if (concArray[i+6-(i>0)][j-1]){
                            targetSquare[0]+=i;
                            targetSquare[1]+=j;
                            valCount++;
                        }
                    }
                }
            }
            //targetSquare[0]+=(targetSquare[0]-mySquare[0]*valCount);
            //targetSquare[1]+=(targetSquare[1]-mySquare[1]*valCount);
            targetSquare[0]/=valCount;targetSquare[1]/=valCount;
            if (!targetSquare[0]){
                targetSquare[0]++;
            }
            if (!concFound){
                if (concArray[0][0]){
                    targetSquare[0]=-5;
                    targetSquare[1]=2;
                }
                else if (concArray[11][0]){
                    targetSquare[0]=4;
                    targetSquare[1]=2;
                }
                else if (concArray[11][7]){
                    targetSquare[0]=4;
                    targetSquare[1]=7;
                }
            }
            while (!concArray[targetSquare[0]+6-(targetSquare[0]>-1)][targetSquare[1]-1]){
                targetSquare[0]+=1;
                if (targetSquare[0]>6){
                    targetSquare[0]=-6;
                    targetSquare[1]++;
                }//removing the x=0 skip here shouldn't break things because of how the boolean subtraction is structured in the while condition (ends up same as prev) but may be wrong
                if (targetSquare[1]>8){
                    targetSquare[1]=1;
                }
            }
            if (myPos[1]<0){
                targetSquare[1]*=-1;
                targetSquare[0]*=-1;
            }
            game.square2pos(targetSquare,positionTarget);
            DEBUG(("%i %i", targetSquare[0],targetSquare[1]));
            positionTarget[2]=0.f;
            
        }
    }
    
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    else if ((not dropping)){
        memcpy(nextSquare,mySquare,8);
        float maxDist=1000;
        for (int i=-6;i<7;i++){
            if (i){
                for (int j=-8;j<9;j++){
                    if (j){
                        usefulIntVec[0]=i;usefulIntVec[1]=j;
                        if (((drilling and mySquare[0]==usefulIntVec[0] and mySquare[1]==usefulIntVec[1]) or !game.getDrills(usefulIntVec))){
                            game.square2pos(usefulIntVec,usefulVec);
                            usefulVec[2]=game.getTerrainHeight(usefulIntVec);
                            // if (usefulIntVec[1]<0){
                            //     usefulIntVec[0]*=-1;
                            //     usefulIntVec[1]*=-1;
                            // }
                            flocal=dist(usefulVec,myPos)+intDist(usefulIntVec,tenCoords)-.1f*(usefulIntVec[2]>game.getTerrainHeight(mySquare));
                            if (flocal<maxDist and dist(enPos,usefulVec)>.3f and !game.isGeyserHere(usefulIntVec)){
                                if (drilling and (mySquare[0]!=usefulIntVec[0] or mySquare[1]!=usefulIntVec[1])){
                                    memcpy(nextSquare,usefulIntVec,8);
                                    maxDist=flocal;
                                }
                                else{
                                    memcpy(positionTarget,usefulVec,12);
                                    if (!drilling){
                                        maxDist=flocal;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (game.getNumSamplesHeld()>2){
            memset(nextSquare,0,8);
        }
        //adjust positiontarget to the corner of a square
        game.pos2square(positionTarget,targetSquare);
        
        
        
        //positionTarget[2]=myPos[2];//vertical movement to avoid terrain
        if (!onSite){// and (game.getTerrainHeight(mySquare)<game.getTerrainHeight(targetSquare))){
            positionTarget[2]=.27f;
            DEBUG(("O"));
            if (myPos[2]>.29f){
                DEBUG(("U"));
                memcpy(positionTarget,myPos,8);
            }
        }
        else{
            DEBUG(("D"));
            positionTarget[2]=game.getTerrainHeight(targetSquare)-.13f;
        }
        DEBUG(("%i %i", targetSquare[0],targetSquare[1]));
        DEBUG(("%i %i", mySquare[0],mySquare[1]));
        DEBUG(("%i %i", nextSquare[0],nextSquare[1]));
        
        
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
        and (onSite))
        //and not game.getDrillError()
        ){//and (myPos[2]-positionTarget[2]<0.02f and myPos[2]-positionTarget[2]>-0.02f)){
            usefulVec[0]=-myAtt[1];usefulVec[1]=myAtt[0];usefulVec[2]=0;//myAtt[2]*-5;
            
            // usefulVec[0]=0;
            // usefulVec[1]=0;
            // usefulVec[2]=0;
            // api.setAttRateTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            //zeroVec[2]=.04f;
            drilling=true;
            
        }
        else{
            usefulVec[0]=myAtt[0];usefulVec[1]=myAtt[1];usefulVec[2]=0;
            
            // memcpy(usefulVec,myRot,12);
            // scale(usefulVec,-0.6f);
            // api.setAttRateTarget(usefulVec);
            DEBUG(("Slowing"));
            drilling=false;
            //zeroVec[2]=-.04f;
        }
        api.setAttitudeTarget(usefulVec);
        
        
       
    }
    //otherwise, drop off our samples
    else{
        memcpy(positionTarget,myPos,12);
        if (myPos[2]>.29f){
            positionTarget[2]=.05f;
        }
        else{
            //maybe take out the mathvecMagnitude expression for codesize
            scale(positionTarget,(.23f)/mathVecMagnitude(positionTarget,3));//go to a position that is .09 in the same direction at the enemy. In other words, between them and the origin.
        }
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        //zeroVec[2]=0;//.01f;//Slow down
        
        
    }
    
    //PRINTVEC("myQuatAtt",myQuatAtt);
    // myQuatAtt[3]*=-1;//inverts rotation - now rotates fundamental basis rotation vector (k-hat) to our basis
    // api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    // if (drilling){
    //     api.setAttRateTarget(usefulVec);
    // }
    //if our drill breaks or we get a geyser, stop the current drill
    flocal=game.getFuelRemaining();
    if (game.getNumSamplesHeld()>1 and ((!(int)((api.getTime()-161)/4)))){// or (flocal<.18f and flocal> .12f))){//at the end of the game, drop off what we have
        dropping=true;
    }
    if (game.getDrillError() 
    or geyserOnMe 
    or game.getDrills(mySquare)>MAXDRILLS-1
    or dropping){
        DEBUG(("Broke"));
        if (game.getNumSamplesHeld()>3){
            dropping=true;
        }
        drilling=false;
    }
    
    // if (flocal<.03f and myVel[2]>0){
    //     memcpy(positionTarget,myPos,12);
    //     positionTarget[2]-=1;
    // }
    if (!game.getNumSamplesHeld()){//don't drop off with no samples
        dropping=false;
    }
    if (not drilling){//don't drill if we aren't drilling
        game.stopDrill();
    }

    
    
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    #define fvector usefulVec
    
    #define ACCEL .014f
    //mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    mathVecSubtract(fvector, destination, myPos, 3);
    flocal=0.05f/(.05f+mathVecMagnitude(fvector,3));//Just storing this value as a functional boolean
    scale(myVel,.2f+flocal);
    mathVecSubtract(fvector,fvector,myVel,3);
    scale(fvector,.25f-.09f*flocal);
    if (geyserOnMe){
        fvector[2]=0;
        // flocal=mathVecMagnitude(fvector,3)/15;
        // fvector[0]/=flocal;
        // fvector[1]/=flocal;
        scale(fvector,15/mathVecMagnitude(fvector,3));
        //fvector[2]=0.05f;
    }
    if (!tenFound and game.hasAnalyzer()){
        fvector[2]=.05f*(.2f-myPos[2]);
        mathVecNormalize(fvector,3);
        //fvector[2]=.1f*(.27f-myPos[2]);
        //while (mathVecMagnitude(fvector,3)>.039f){
        scale(fvector,.03f);
        //}
    }
    if (drilling){
        fvector[2]=.5f*(positionTarget[2]-myPos[2]);
        for (int i=0;i<2;i++){
            fvector[i]=(((nextSquare[i]>mySquare[i])-(mySquare[i]>nextSquare[i]))*.038f+positionTarget[i]-myPos[i])/(10-4.f*game.getDrills(mySquare));
        }
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

void scale (float* vec, float scale) {//This function scales a length-3 vector by a coeff.
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}

/**
 * @param a1 - a square
 * @param a2 - another square
 * @return the distance between a1 and a2, squared
 */
int intDist(int a1[2],int a2[2]) {
    return ((a1[0]-a2[0])*(a1[0]-a2[0])+(a1[1]-a2[1])*(a1[1]-a2[1]));
}