#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myQuatAtt (&myState[6])
#define myRot (&myState[10])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])//These are pointers. They will have the values that
#define MAXDRILLS 3
int siteCoords[2];
bool dropping;
bool drilling;
//are described in their names, and act as length-3 float arrays
int samples;
int corner;
bool guarding;
float enScore;
bool valArray[12][8];
int tenCoords[2];
bool tenFound;
bool concFound;
void init(){
    enScore=0;

	
	#define SPEEDCONST .35f
    #define DERIVCONST 2.35f
    api.setPosGains(SPEEDCONST,0,DERIVCONST);
    //api.setAttGains(0.7f,0.1f,3.f);
    //api.setAttGains(0.f,0.f,0.f);
    dropping=false;
    samples=0;
    guarding=false;
	drilling=false;
	concFound=false;
	memset(valArray,true,96);
	tenFound=false;
}

void loop(){
    float flocal;
    float positionTarget[3];
    float zeroVec[3];
    float myState[13];
    float usefulVec[3];
    int usefulIntVec[2];
    int mySquare[3];
    float enState[12];
    memset(zeroVec, 0.0f, 12);//Sets all places in an array to 0
    float enDeltaScore=game.getOtherScore()-enScore;
    enScore=enScore+enDeltaScore;
    if (game.checkSample()){
        game.dropSample(4);
        samples+=(bool)(game.pickupSample());
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
    float maxDist=100;//Sets this large
    
    if (enDeltaScore==3.5f){
        game.pos2square(enPos,siteCoords);
        siteCoords[0]*=-1;
        siteCoords[1]*=-1;
    }
    
    bool onSite=(mySquare[0]==siteCoords[0] and mySquare[1]==siteCoords[1]);
    if (!game.hasAnalyzer()){
        positionTarget[0]=.3f;
        positionTarget[1]=-.48f;
        if (myPos[1]>0){
            scale(positionTarget,-1);
        }
        positionTarget[2]=-.16f;
    }
    else if (!tenFound){
        float terrain=game.analyzeTerrain();
        DEBUG(("%f",terrain));
        if (terrain>.9f){
            game.pos2square(myPos,tenCoords);
            tenFound=true;
        }
        else{
            valArray[mySquare[0]+6-(mySquare[0]>0)][mySquare[1]-1]=false;
            if (myPos[1]<0){
                mySquare[0]*=-1;
                mySquare[1]*=-1;
            }
            int valCount=0;
            memset(siteCoords,0,8);
            for (int i=-6;i<7;i++){
                if (i){
                    for (int j=1;j<9;j++){
                        usefulIntVec[0]=i;usefulIntVec[1]=j;
                        int sqDist=intDist(usefulIntVec,mySquare);
                        usefulIntVec[0]*=-1;
                        usefulIntVec[1]*=-1;
                        int sqDist2=intDist(usefulIntVec,mySquare);
                        if (sqDist>sqDist2){
                            sqDist=sqDist2;
                        }
                        bool inRad=sqDist<=(terrain>.5f?3:7);
                        if (terrain>.15f){
                            concFound=true;
                            inRad=!inRad;
                        }
                        if (inRad){
                            valArray[i+6-(i>0)][j-1]=false;
                        }
                        if (valArray[i+6-(i>0)][j-1]){
                            siteCoords[0]+=i;
                            siteCoords[1]+=j;
                            valCount++;
                        }
                    }
                }
            }
            //siteCoords[0]+=(siteCoords[0]-mySquare[0]*valCount);
            //siteCoords[1]+=(siteCoords[1]-mySquare[1]*valCount);
            siteCoords[0]/=valCount;siteCoords[1]/=valCount;
            if (!siteCoords[0]){
                siteCoords[0]++;
            }
            if (!concFound){
                if (valArray[0][0]){
                    siteCoords[0]=-5;
                    siteCoords[1]=2;
                }
                else if (valArray[11][0]){
                    siteCoords[0]=4;
                    siteCoords[1]=2;
                }
                else if (valArray[11][7]){
                    siteCoords[0]=4;
                    siteCoords[1]=7;
                }
            }
            while (!valArray[siteCoords[0]+6-(siteCoords[0]>0)][siteCoords[1]-1]){
                siteCoords[0]+=1;
                if (siteCoords[0]>6){
                    siteCoords[0]=-6;
                    siteCoords[1]++;
                }
                if (!siteCoords[0]){
                    siteCoords[0]++;
                }
                if (siteCoords[1]>8){
                    siteCoords[1]=1;
                }
            }
            if (myPos[1]<0){
                siteCoords[0]*=-1;
                siteCoords[1]*=-1;
            }
            
            game.square2pos(siteCoords,positionTarget);
            DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
            positionTarget[2]=.27f;
            
        }
    }
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    else if ((not dropping)){
        float maxDist=1000;
        for (int i=-6;i<7;i++){
            if (i){
                for (int j=-8;j<9;j++){
                    if (j){
                        usefulIntVec[0]=i;usefulIntVec[1]=j;
                        if (((drilling and mySquare[0]==usefulIntVec[0] and mySquare[1]==usefulIntVec[1]) or !game.getDrills(usefulIntVec))){
                            game.square2pos(usefulIntVec,usefulVec);
                            usefulVec[2]=game.getTerrainHeight(usefulIntVec);
                            if (usefulIntVec[1]<0){
                                usefulIntVec[0]*=-1;
                                usefulIntVec[1]*=-1;
                            }
                            flocal=dist(usefulVec,myPos)*3+intDist(usefulIntVec,tenCoords)+.05f*(usefulVec[2]>=game.getTerrainHeight(myPos));
                            if (flocal<maxDist and dist(enPos,usefulVec)>.3f){
                                memcpy(positionTarget,usefulVec,12);
                                maxDist=flocal;
                            }
                        }
                    }
                }
            }
        }
        //adjust positiontarget to the corner of a square
        game.pos2square(positionTarget,siteCoords);
        positionTarget[0]+=(positionTarget[0]<0)*0.029f;
        positionTarget[1]+=(positionTarget[1]<0)*0.029f;
        
        
        //positionTarget[2]=myPos[2];//vertical movement to avoid terrain
        if (!onSite and (game.getTerrainHeight(mySquare)<game.getTerrainHeight(siteCoords))){
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
        
        
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        and (mathVecMagnitude(myRot,3)<.035f or drilling)
        and (onSite))
        and not game.getDrillError()
        and (myPos[2]-positionTarget[2]<.02f and myPos[2]-positionTarget[2]>-0.02f)){
            usefulVec[0]=-myAtt[1];usefulVec[1]=myAtt[0];usefulVec[2]=myAtt[2]*-5;
            
            // usefulVec[0]=0;
            // usefulVec[1]=0;
            // usefulVec[2]=0;
            // api.setAttRateTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            zeroVec[2]=.04f;
            drilling=true;
            
        }
        else{
            usefulVec[0]=myAtt[0];usefulVec[1]=myAtt[1];usefulVec[2]=0;
            
            // memcpy(usefulVec,myRot,12);
            // scale(usefulVec,-0.6f);
            // api.setAttRateTarget(usefulVec);
            DEBUG(("Slowing"));
            drilling=false;
            zeroVec[2]=-.04f;
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
            scale(positionTarget,(.23f)/mathVecMagnitude(positionTarget,3));//go to a position that is .09 in the same direction at the enemy. In other words, between them and the origin.        }
        }
        zeroVec[2]-=1;
        api.setAttitudeTarget(zeroVec);
        zeroVec[2]=0;//.01f;//Slow down
        
        
    }
    PRINTVEC("myQuatAtt",myQuatAtt);
    myQuatAtt[3]*=-1;//inverts rotation - now rotates fundamental basis rotation vector (k-hat) to our basis
    api.quat2AttVec(zeroVec,myQuatAtt,usefulVec);
    if (drilling){
        api.setAttRateTarget(usefulVec);
    }
    //if our drill breaks or we get a geyser, stop the current drill
    flocal=game.getFuelRemaining();
    if (game.getDrillError() 
    or geyserOnMe 
    or game.getDrills(mySquare)>MAXDRILLS-1){
        DEBUG(("Broke"));
        if (game.getNumSamplesHeld()>3){
            dropping=true;
        }
        drilling=false;
    }
    if (game.getNumSamplesHeld()>1 and ((!(int)((api.getTime()-161)/4)) or (flocal<.16f and flocal> .12f))){//at the end of the game, drop off what we have
        dropping=true;
        drilling=false;
    }
    if (dropping){
        
    }
    // if (flocal<.03f and myVel[2]>0){
    //     memcpy(positionTarget,myPos,12);
    //     positionTarget[2]-=1;
    // }
    if (!game.getNumSamplesHeld() or mathVecMagnitude(enPos,3)<.16){//don't drop off with no samples
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
    scale(fvector,.2f-.09f*flocal);
    if (geyserOnMe){
        fvector[2]=0;
        // flocal=mathVecMagnitude(fvector,3)/15;
        // fvector[0]/=flocal;
        // fvector[1]/=flocal;
        scale(fvector,15/mathVecMagnitude(fvector,3));
        //fvector[2]=0.05f;
    }
    if (!tenFound and game.hasAnalyzer()){
        mathVecNormalize(fvector,3);
        fvector[2]=.1f*(.27f-myPos[2]);
        while (mathVecMagnitude(fvector,3)>.039f){
            scale(fvector,.99f);
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
int intDist(int a1[2],int a2[2]){
    return ((a1[0]-a2[0])*(a1[0]-a2[0])+(a1[1]-a2[1])*(a1[1]-a2[1]));
}