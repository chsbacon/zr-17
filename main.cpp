#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
float positionTarget[3];
int targetCoordinates[3];
float zeroVec[3];
float myState[13];
float usefulVec[3];
float guardPos[3];
int usefulIntVec[3];
int mySquare[3];
float enState[12];
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myQuatAtt (&myState[6])
#define myRot (&myState[10])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])//These are pointers. They will have the values that
#define MAXDRILLS 3
int siteCoords[3];
bool newLoc;

//could improve these flags to save codesize
bool dropping;
bool drilling;
bool guarding;
bool twoDrops;
//are described in their names, and act as length-3 float arrays
int samples;
float vcoef;

void init(){
    newLoc=true;
    // zeroVec[0]=zeroVec[1]=zeroVec[2]=0;
	memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
	#define SPEEDCONST .5f
    #define DERIVCONST 2.5f
    api.setPosGains(SPEEDCONST,0,DERIVCONST);
    //api.setAttGains(0.7f,0.1f,3.f);
    //api.setAttGains(0.f,0.f,0.f);
    dropping=false;
    samples=0;
	drilling=false; 
	twoDrops=false;
	guarding=false;
}

void loop(){
    
    if (game.checkSample()){
        game.dropSample(4);
        game.pickupSample();
        samples+=1;
    }
    if (game.atBaseStation()){
        for (int i=0;i<5;i++){
            game.dropSample(i);
        }
        dropping=false;
        samples=-100;
    }
    
    //calculate guardSpot
    memcpy(guardPos, enPos, 12);
    scale(guardPos, 0.16f/mathVecMagnitude(enPos, 3));
    
    api.getMySphState(myState);
    float myAtt[3];
    zeroVec[0]-=1;
    api.quat2AttVec(zeroVec,myQuatAtt,myAtt);
    zeroVec[0]+=1;
    api.getOtherZRState(enState);//Makes sure our data on where they are is up to date
    game.pos2square(myPos,mySquare);
    game.square2pos(mySquare,usefulVec);
    bool geyserOnMe;
    geyserOnMe=game.isGeyserHere(mySquare);
    if (game.getScore()<7 and geyserOnMe){
        twoDrops=true;
    }
    //What?
    twoDrops=true;
    if(geyserOnMe and game.getNumSamplesHeld() > 4){
        dropping=true;
    }
    float maxDist=100;//Sets this large
    float modPos[3];
    memcpy(modPos,myPos,12);
    for (int i=0;i<2;i++){
        modPos[i]+=(myPos[i]-usefulVec[i])*6*geyserOnMe;
    }    
        
    if (newLoc and !game.checkSample() and not drilling and not guarding){
        DEBUG(("%d",newLoc));
        DEBUG(("reselecting"));
        for (int i=-8;i<9;i++){//This checks all of the grid spaces, and sees which is both
        //closest to us and in the center. You should understand this search structure - it's important!
            for (int j=-10;j<11;j++){
                if ((i!=0) and (j!=0)){
                    //DEBUG(("%i %i",i,j));
                    usefulIntVec[0]=i;usefulIntVec[1]=j;usefulIntVec[2]=0;
                    game.square2pos(usefulIntVec,usefulVec);
                    usefulVec[2]=0.35f;
                    float score=dist(usefulVec,modPos);
                    if (score<maxDist 
                    and game.getDrills(usefulIntVec)<MAXDRILLS 
                    and not game.isGeyserHere(usefulIntVec) 
                    and i*i+j*j>8 
                    and i%2+j%2<=1){
                        siteCoords[0]=i;siteCoords[1]=j;
                        //DEBUG(("Changed %f", score));
                        maxDist = score;
                    }
                }
            }
        }
        newLoc=false;
    }
    vcoef=.120f;
    // if (geyserOnMe){
    //     vcoef+=.04f;
    // }
    if (game.getNumSamplesHeld()>2 and ((api.getTime()==160) or (game.getFuelRemaining() < .13f and game.getFuelRemaining() > .10f))){
        dropping=true;
        drilling=false;
        game.stopDrill();
    }
    
    float rotConst;
    rotConst=0;
    
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already
    if ((((samples%3>0 or samples<0)and twoDrops) or game.getNumSamplesHeld()<5) and not dropping and not guarding){//Second to last condition is redundant
        DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
        DEBUG(("%i %i", mySquare[0],mySquare[1]));
        game.square2pos(siteCoords,positionTarget);
        //adjust positiontarget to the corner of a square
        for (int i=0;i<2;i++){
            //positionTarget[i]+=0.033f*(siteCoords[i]>0?1:-1)*(siteCoords[i]%2>0?1:-1)*(geyserOnMe?1:-1);//can use xor for codesize
            positionTarget[i]+=0.034f*((siteCoords[i]>0)^(siteCoords[i]%2>0)^(geyserOnMe)?-.5f:1);
        }
        //set this to go to the surface
        positionTarget[2]=0.35f;
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
         and (siteCoords[0]==mySquare[0] 
         and siteCoords[1]==mySquare[1]) 
        ) and not game.getDrillError()
        and (myPos[2]>.33f and myPos[2]<.37f)){
            usefulVec[0]=-myAtt[1];usefulVec[1]=myAtt[0];usefulVec[2]=myAtt[2]*-5;
            api.setAttitudeTarget(usefulVec);
            
            
            // scale(usefulVec,.2f/mathVecMagnitude(usefulVec,3));
            // api.setAttRateTarget(usefulVec);
            // usefulVec[0]=0;
            // usefulVec[1]=0;
            // usefulVec[2]=0;
            // api.setAttRateTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            else{
                rotConst=.3f;
            }
            drilling=true;
            
        }
        else{
            usefulVec[0]=myAtt[0];usefulVec[1]=myAtt[1];usefulVec[2]=0;
            api.setAttitudeTarget(usefulVec);
            // memcpy(usefulVec,myRot,12);
            // scale(usefulVec,-0.6f);
            // api.setAttRateTarget(usefulVec);
            DEBUG(("Slowing"));
            drilling=false;
            rotConst=mathVecMagnitude(myRot,3)*-.2f;
        }
        PRINTVEC("myQuatAtt",myQuatAtt);
        myQuatAtt[3]*=-1;//inverts rotation - now rotates fundamental basis rotation vector (k-hat) to our basis
        zeroVec[2]=rotConst;
        api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
        zeroVec[2]=0;
        if (drilling){
            api.setAttRateTarget(usefulVec);
        }
        
       
    }
    //otherwise, drop off our samples
    else{
        if(guarding){
            memcpy(positionTarget, guardPos, 12);
        }
        else{
        memcpy(positionTarget,myPos,12);
        scale(positionTarget,0.23f/mathVecMagnitude(myPos,3));
        }
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]+=1;
        api.setAttRateTarget(zeroVec);
    }
    //if our drill breaks or we get a geyser, stop the current drill
    if (game.getDrillError() 
    //or (mySquare[0]!=siteCoords[0] or mySquare[1]!=siteCoords[1]) 
    //or mathVecMagnitude(myVel,3)>0.009f
    or (samples==6
    or geyserOnMe 
    or game.getDrills(mySquare)>MAXDRILLS-1)){
        if (geyserOnMe and drilling){
            samples-=samples%3;
        }
        game.stopDrill();
        newLoc=true;
        drilling=false;
    }
    
    //if we're closer than them to the guarding position and we're ahead
    if((dist(enPos, guardPos) - dist(myPos, guardPos) > .15) and (game.getScore() - game.getOtherScore() > 2)){
        guarding=true;
    }
    //if we were guarding and they're too close to the surface
    if(guarding and enPos[2] > .28){
        guarding=false;
    }
    
    DEBUG(("Guarding: %d", guarding));

    
    
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    float distance,flocal,fvector[3];
    #define ACCEL .0135f
    mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    distance = mathVecNormalize(fvector, 3);
    if (distance > 0.05f) {//If not close, decide on our velocity
        flocal = vcoef;
        if (flocal*flocal/ACCEL>distance-.02f){//Cap on how fast we go
            flocal = sqrtf(distance*ACCEL)-.02f;
            //DEBUG(("Slower"));
        }
        else if (!geyserOnMe and flocal>ACCEL+mathVecMagnitude(myVel,3)){
            flocal=ACCEL+mathVecMagnitude(myVel,3);
        }
        scale(fvector, flocal);
        api.setVelocityTarget(fvector);
    }
    else{//If close:
        api.setPositionTarget(destination);
    }
}

float dist(float* vec1, float* vec2) {
    float ansVec[3];
    mathVecSubtract(ansVec, vec1, vec2, 3);
    return mathVecMagnitude(ansVec, 3);
}
void scale (float* vec, float scale) {//This function scales a length-3 vector by a coeff.
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}