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
#define MAXDRILLS 3
int siteCoords[3];
bool newLoc;
bool dropping;
bool drilling;
//are described in their names, and act as length-3 float arrays
int samples;
float vcoef;
int corner;
float enScore;
void init(){
    enScore=0;
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
}

void loop(){
    float enDeltaScore=game.getOtherScore()-enScore;
    enScore=enScore+enDeltaScore;
    if (game.checkSample()){
        game.dropSample(4);
        game.pickupSample();
        samples+=1;
    }
    if (game.atBaseStation()){
        for (int i=0;i<5;i++){
            game.dropSample(i);
        }
        newLoc=true;
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
    float maxDist=100;//Sets this large
    float modPos[3];
    memcpy(modPos,myPos,12);
    for (int i=0;i<2;i++){
        modPos[i]+=(myPos[i]-usefulVec[i])*6*geyserOnMe;
    }    
    if (samples%4!=2){
        modPos[2]=.2f;//this favors high points
    }
    if (newLoc and !game.checkSample() and not drilling){
        DEBUG(("%d",newLoc));
        DEBUG(("reselecting"));
        for (int i=-6;i<6;i++){//This checks all of the grid spaces, and sees which is both
        //closest to us and in the center. You should understand this search structure - it's important!
            for (int j=-8;j<8;j++){
                if (i*j!=0 and (i<-3 or i>2 or j<-3 or j>2)){
                    //DEBUG(("%i %i",i,j));
                    int heights[4];
                    memset(heights,0,16);
                    for (int a=0;a<4;a++){//Allows to cycle through four points of square
                        usefulIntVec[0]=i+a%2;usefulIntVec[1]=j+a/2;usefulIntVec[2]=0;
                        fixEdge(usefulIntVec);
                        int index=(game.getTerrainHeight(usefulIntVec)-.4f)*12.5f;
                        //DEBUG(("%i",index));
                        heights[index]+=1;
                    }
                    float goodHeight=.4f;
                    for (int other=1;other<4;other++){
                        if (heights[other]==3){
                            heights[0]=3;
                            goodHeight=other*.08f+.4f;//silent fail specific to goodheight
                        }
                    }
                    DEBUG(("%i %i %i %i", heights[0],heights[1],heights[2],heights[3]));
                    if (heights[0]>1){
                        //DEBUG(("GROUP"));
                        for (int a=0;a<4;a++){
                            usefulIntVec[0]=i+a%2;usefulIntVec[1]=j+a/2;usefulIntVec[2]=0;
                            fixEdge(usefulIntVec);
                            game.square2pos(usefulIntVec,usefulVec);
                            usefulVec[2]=game.getTerrainHeight(usefulIntVec);
                            float score=dist(usefulVec,modPos);
                            if ( usefulVec[2]==goodHeight
                            and score<maxDist 
                            and game.getDrills(usefulIntVec)<1 
                            and not game.isGeyserHere(usefulIntVec) 
                            //and i*i+j*j>8 and i*i<16 and j*j<16 
                            and dist(enPos,usefulVec)>.22f){
                                siteCoords[0]=i+a%2;siteCoords[1]=j+a/2;
                                fixEdge(siteCoords);
                                
                                corner=a;
                                
                                //DEBUG(("Changed %f", score));
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
    vcoef=.120f;
    if (geyserOnMe){
        vcoef+=.04f;
    }
    
    
    float rotConst;
    rotConst=0;
    
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    if (not dropping){
        //adjust positiontarget to the corner of a square
        game.square2pos(siteCoords,positionTarget);
        positionTarget[0]+=((corner%2)*-2+1)*0.031f;
        positionTarget[1]+=((corner/2)*-2+1)*0.031f;
        positionTarget[2]=myPos[2];//vertical movement to avoid terrain
        if ((mySquare[0]!=siteCoords[0] or mySquare[1]!=siteCoords[1]) and dist(positionTarget,myPos)>.02f){
            positionTarget[2]=.26f;
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
        
        
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
         and (siteCoords[0]==mySquare[0] 
         and siteCoords[1]==mySquare[1]) 
        ) and not game.getDrillError()
        and (myPos[2]-positionTarget[2]<.02f and myPos[2]-positionTarget[2]>-0.02f)){
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
        if (myPos[2]>.29f){
            memcpy(positionTarget,myPos,8);
        }
        memcpy(positionTarget,myPos,12);
        scale(positionTarget,.23f/mathVecMagnitude(myPos,3));
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]+=1;
        rotConst=.1f;
        
    }
    PRINTVEC("myQuatAtt",myQuatAtt);
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
    or game.getDrills(mySquare)>MAXDRILLS-1){
        DEBUG(("Broke"));
        if (game.getNumSamplesHeld()>3){
            dropping=true;
        }
        game.stopDrill();
        newLoc=true;
        drilling=false;
    }
    if (game.getNumSamplesHeld()>1 and ((api.getTime()>157 and api.getTime()<163) or (game.getFuelRemaining() < .16f and game.getFuelRemaining() > .8f))){
        dropping=true;
        drilling=false;
        game.stopDrill();
    }
    if (game.getNumSamplesHeld()==0){
        dropping=false;
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
    scale(fvector,.25f);
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