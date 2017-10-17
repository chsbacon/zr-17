#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
float positionTarget[3];
int targetCoordinates[3];
float zeroVec[3];
float myState[12];
float usefulVec[3];
int usefulIntVec[3];
int mySquare[3];
float enState[12];
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])//These are pointers. They will have the values that
#define MAXDRILLS 3
int siteCoords[3];
bool newLoc;
bool dropping;
//are described in their names, and act as length-3 float arrays

float vcoef;
void init(){
    newLoc=true;
    vcoef=.154;//A coefficient for our movement speed
    // zeroVec[0]=zeroVec[1]=zeroVec[2]=0;
	memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
	#define SPEEDCONST 0.45f
    #define DERIVCONST 2.8f
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.6f,0.1f,2.8f);
    dropping=false;
	
}

void loop(){
    game.pickupSample();
    if (game.atBaseStation()){
        for (int i=0;i<5;i++){
            game.dropSample(i);
        }
        dropping=false;
    }
    api.getMyZRState(myState);
    api.getOtherZRState(enState);//Makes sure our data on where they are is up to date
    game.pos2square(myPos,mySquare);
    game.square2pos(mySquare,usefulVec);
    float maxDist=100;//Sets this large
    float modPos[3];
    memcpy(modPos,myPos,12);
    for (int i=0;i<2;i++){
        modPos[i]+=(myPos[i]-usefulVec[i])*6*game.isGeyserHere(mySquare);
        DEBUG(("%f",(myPos[i]-usefulVec[i])*6*game.isGeyserHere(mySquare)));
    }    
        
    if (newLoc and !game.checkSample()){
        DEBUG(("reselecting"));
        for (int i=-8;i<9;i++){//This checks all of the grid spaces, and sees which is both
        //closest to us and in the center. You should understand this search structure - it's important!
            for (int j=-10;j<11;j++){
                if ((i!=0) and (j!=0)){
                    //DEBUG(("%i %i",i,j));
                    usefulIntVec[0]=i;usefulIntVec[1]=j;usefulIntVec[2]=0;
                    game.square2pos(usefulIntVec,usefulVec);
                    usefulVec[2]=0.34f;
                    float score=dist(usefulVec,modPos);
                    if (score<maxDist 
                    and game.getDrills(usefulIntVec)<MAXDRILLS 
                    and not game.isGeyserHere(usefulIntVec) 
                    and i*i+j*j>8){
                        siteCoords[0]=i;siteCoords[1]=j;
                        //DEBUG(("Changed %f", score));
                        maxDist = score;
                    }
                }
            }
        }
        newLoc=false;
    }
    vcoef=.170f;
    if (game.getNumSamplesHeld()>2 and ((api.getTime()>156 and api.getTime()<159) or (game.getFuelRemaining() < .15f and game.getFuelRemaining() > .12f))){
        dropping=true;
    }
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    if (game.getNumSamplesHeld()<5 and not dropping){//Second to last condition is redundant
        DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
        DEBUG(("%i %i", mySquare[0],mySquare[1]));
        game.square2pos(siteCoords,positionTarget);
        //adjust positiontarget to the corner of a square
        for (int i=0;i<2;i++){
            //positionTarget[i]+=0.033f*(siteCoords[i]>0?1:-1)*(siteCoords[i]%2>0?1:-1)*(game.isGeyserHere(mySquare)?1:-1);//can use xor for codesize
            positionTarget[i]+=0.033f*((siteCoords[i]>0)^(siteCoords[i]%2>0)^(game.isGeyserHere(mySquare))?-1:1);
        }
        //set this to go to the surface
        positionTarget[2]=0.35f;
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if (mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.04f 
        or game.getDrillEnabled()) 
        and not game.getDrillError() 
        and (siteCoords[0]==mySquare[0] 
        and siteCoords[1]==mySquare[1])){
            usefulVec[0]=myAtt[1];usefulVec[1]=-myAtt[0];usefulVec[2]=0;
            api.setAttitudeTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
        }
        else{
            memcpy(usefulVec,myAtt,12);
            usefulVec[2]=0;
            api.setAttitudeTarget(usefulVec);
        }
       
    }
    //otherwise, drop off our samples
    else{
        memcpy(positionTarget,myPos,12);
        scale(positionTarget,.23f/mathVecMagnitude(myPos,3));
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]+=1;
    }
    //if our drill breaks or we get a geyser, stop the current drill
    if (game.getDrillError() 
    or ((mySquare[0]!=siteCoords[0] 
    or mySquare[1]!=siteCoords[1]) 
    or mathVecMagnitude(myVel,3)>0.009f) 
    or game.getNumSamplesHeld()==5 
    or game.isGeyserHere(mySquare) 
    or game.getDrills(mySquare)>MAXDRILLS-1){
        game.stopDrill();
        newLoc=true;
    }
    

    
    
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    float distance,flocal,fvector[3];
    #define ACCEL .0175f
    mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    distance = mathVecNormalize(fvector, 3);
    if (distance > 0.05f) {//If not close, decide on our velocity
        flocal = vcoef;
        if (flocal*flocal/ACCEL>distance-.015f){//Cap on how fast we go
            flocal = sqrtf(distance*ACCEL)-.015f;
            //DEBUG(("Slower"));
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