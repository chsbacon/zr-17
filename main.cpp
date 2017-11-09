#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
float positionTarget[3];
int targetCoordinates[3];
float zeroVec[3];
float myState[13];
float usefulVec[3];
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
#define TWODROPS false
#define MAXDRILLS 2
int siteCoords[3];
bool newLoc;
bool dropping;
bool drilling;
bool twoDrops;
//are described in their names, and act as length-3 float arrays
int samples;
float vcoef;
bool moving;
void init(){
    newLoc=true;
    // zeroVec[0]=zeroVec[1]=zeroVec[2]=0;
	memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
	#define SPEEDCONST .35f
    #define DERIVCONST 2.35f
    api.setPosGains(SPEEDCONST,0,DERIVCONST);
    //api.setAttGains(0.7f,0.1f,3.f);
    //api.setAttGains(0.f,0.f,0.f);
    dropping=false;
    samples=0;
	drilling=false;
	twoDrops=false;
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
    }
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
    twoDrops=true;
    float maxDist=100;//Sets this large
    float modPos[3];
    memcpy(modPos,myPos,12);
    for (int i=0;i<2;i++){
        modPos[i]+=(myPos[i]-usefulVec[i])*6*geyserOnMe;
    }    
        
    if (newLoc and !game.checkSample() and not drilling){
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
                    int temp [2];
                    temp[0]=i;
                    temp[1]=j;
                    if ((score<maxDist 
                    and game.getDrills(usefulIntVec)<1 
                    and not game.isGeyserHere(usefulIntVec) 
                    and i*i+j*j>8 and i*i<16 and j*j<16 
                    and dist(enPos,usefulVec)>.22f) and game.getTerrainHeight(temp) < 0.50f){
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
    
    
    float rotConst;
    rotConst=0;
    
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    if (not dropping){//Second to last condition is redundant
        DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
        DEBUG(("%i %i", mySquare[0],mySquare[1]));
        game.square2pos(siteCoords,positionTarget);
        //adjust positiontarget to the corner of a square
        for (int i=0;i<2;i++){
            //positionTarget[i]+=0.033f*(siteCoords[i]>0?1:-1)*(siteCoords[i]%2>0?1:-1)*(geyserOnMe?1:-1);//can use xor for codesize
            positionTarget[i]+=0.032f*((siteCoords[i]>0)^(siteCoords[i]!=2)^(geyserOnMe)?1:-1)*(1-(siteCoords[i]*siteCoords[i]==1)*.2f);
            //positionTarget[i] += 0.005f;
        }
        //set this to go to the surface
        float h = game.getTerrainHeight(positionTarget);
        DEBUG(("HEIGHT : %f", h));
        #define DRILL_DIST 0.13f
        positionTarget[2] = (h-DRILL_DIST);
        //positionTarget[2]=0.35f;
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
         and (siteCoords[0]==mySquare[0] 
         and siteCoords[1]==mySquare[1])
        ) and not game.getDrillError()
        and (myPos[2]>(h-0.15f) and myPos[2]<(h-0.11f))){
            usefulVec[0]=-myAtt[1];usefulVec[1]=myAtt[0];usefulVec[2]=myAtt[2]*-5;
            api.setAttitudeTarget(usefulVec);
            
            // usefulVec[0]=0;
            // usefulVec[1]=0;
            // usefulVec[2]=0;
            // api.setAttRateTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            else{
                rotConst=.1f;
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
            rotConst=-.1f;
        }
        
        
       
    }
    //otherwise, drop off our samples
    else{
        memcpy(positionTarget,myPos,12);
        scale(positionTarget,.23f/mathVecMagnitude(myPos,3));
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]+=1;
        rotConst=.1f;
    }
    //PRINTVEC("myQuatAtt",myQuatAtt);
    myQuatAtt[3]*=-1;//inverts rotation - now rotates fundamental basis rotation vector (k-hat) to our basis
    zeroVec[2]=rotConst;
    api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    zeroVec[2]=0;
    if (drilling){
        api.setAttRateTarget(usefulVec);
    }
    //if our drill breaks or we get a geyser, stop the current drill
    if (game.getDrillError() 
    or geyserOnMe 
    or game.getDrills(mySquare)>MAXDRILLS-1 or game.getFuelRemaining()<0.01f){
        if (TWODROPS and samples>4){
            dropping=true;
            samples=-100;
        }
        if (!TWODROPS and game.getNumSamplesHeld()>3){
            dropping=true;
        }
        game.stopDrill();
        newLoc=true;
        drilling=false;
    }
    if (game.getNumSamplesHeld()>2 and ((api.getTime()>157 and api.getTime()<163) or (game.getFuelRemaining() < .16f and game.getFuelRemaining() > .8f))){
        dropping=true;
        drilling=false;
        game.stopDrill();
    }

    
    
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    float distance,flocal,fvector[3];
    #define ACCEL .014f
    //mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    distance = mathVecNormalize(fvector, 3);
    mathVecSubtract(fvector, destination, myPos, 3);
    scale(myVel,.26f);
    mathVecSubtract(fvector,fvector,myVel,3);
    scale(fvector,.24f);
    if (geyserOnMe){
        flocal=mathVecMagnitude(fvector,3)/.2f;
        fvector[0]/=flocal;
        fvector[1]/=flocal;
        fvector[2]=0.01f;
    }
    //In order to avoid bumping into terrain, will block any movement in XY plane initiallu
    //Once at a reasonable height has been reached, will not set velocity target in XY plane to 0
    //Instead sets velocity in Z direction to 0 until target square has been entered
    
    if (myPos[2] > 0.34 && newLoc) {
        fvector[0] = 0;
        fvector[1] = 0;
        fvector[2] = ((0.34-myPos[2])-myVel[2]);
    }/*
    int mySqr[3];
    int destSqr[3];
    game.pos2square(myPos, mySqr);
    game.pos2square(destination, destSqr);
    if (newLoc and myPos[2] <= 0.34) {
        if (!(mySqr[0] == destSqr[0] && mySqr[1] == destSqr[1])) {
            fvector[2]=0;
        }
    }*/
    if (game.getFuelRemaining()<0.025f || mathVecMagnitude(myVel, 3) > 0.005) {
        for (int i = 0; i < 3; i++) {
            fvector[i] = 0;
        }   
    }
    api.setVelocityTarget(fvector);
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