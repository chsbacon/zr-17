#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]));
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
bool dropping;
bool drilling;
//are described in their names, and act as length-3 float arrays
int corner;
bool valArray[12][8];
int tenCoords[2];
bool tenFound;
bool concFound;

void init(){

	#define SPEEDCONST .35f
    #define DERIVCONST 2.35f
    api.setPosGains(SPEEDCONST,0,DERIVCONST);
    dropping=false;
	drilling=false;
	concFound=false;
	memset(valArray,true,96);
	tenFound=false;
}

void loop(){
    float flocal;
    float positionTarget[3];
    float myState[12];
    float usefulVec[3];
    int usefulIntVec[2];
    int mySquare[3];
    float enState[12];
    int nextSquare[2];
    bool geyserOnMe = game.isGeyserHere(mySquare);
    float maxDist=100;//Sets this large
    
    bool onSite=((mySquare[0]==siteCoords[0])
        bitand (mySquare[1]==siteCoords[1]));

    if (game.checkSample()){
        game.dropSample(4);
        game.pickupSample();
    }
    if (game.atBaseStation()){
        for (int i=0;i<5;i++){
            game.dropSample(i);
        }
        dropping=false;
    }
    api.getMyZRState(myState);

    api.getOtherZRState(enState);//Makes sure our data on where they are is up to date
    game.pos2square(myPos,mySquare);
    

    if (!game.hasAnalyzer()){
        positionTarget[0]=.3f;
        positionTarget[1]=-.48f;
        if (myPos[1]>0){
            scale(positionTarget,-1);
        }
        positionTarget[2]=-.16f;
    }
    else if (!tenFound){
        float terrain = game.analyzeTerrain();
        DEBUG(("%f",terrain));
        if (terrain>.9f){
            game.pos2square(myPos,tenCoords);
            tenFound = true;
        }
        else{
           
            if (myPos[1]<0){
                mySquare[0] = -mySquare[0];
                mySquare[1] = -mySquare[1];
            }
            valArray[mySquare[0]+6-(mySquare[0]>0)][mySquare[1]-1]=false;
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
            siteCoords[0]/=valCount;
            siteCoords[1]/=valCount;
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
            }
            while (!valArray[siteCoords[0]+6-(siteCoords[0]>-1)][siteCoords[1]-1]){
                siteCoords[0]+=1;
                if (siteCoords[0]>6){
                    siteCoords[0]=-6;
                    siteCoords[1]++;
                }//removing the x=0 skip here shouldn't break things because of how the boolean subtraction is structured in the while condition (ends up same as prev) but may be wrong
                if (siteCoords[1]>8){
                    siteCoords[1]=1;
                }
            }
            if (myPos[1]<0){
                siteCoords[1]*=-1;
                siteCoords[0]*=-1;
            }
            game.square2pos(siteCoords,positionTarget);
            DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
            positionTarget[2] = 0.0f;
            
        }
    }
    
    //drill if we have less than 5 samples and we either have enough fuel or we're close to the surface and don't have many samples already, drill
    else if (not dropping){
        memcpy(nextSquare,mySquare,8);
        float maxDist=1000;
        for (int i=-6;i<7;i++){
            if (i){
                for (int j=-8;j<9;j++){
                    if (j){
                        usefulIntVec[0]=i;usefulIntVec[1]=j;
                        if (usefulIntVec[1]*tenCoords[1]<0){
                            usefulIntVec[0]*=-1;
                            usefulIntVec[1]*=-1;
                        }
                        bool onPos = (mySquare[0]==usefulIntVec[0])
                            bitand (mySquare[1]==usefulIntVec[1]);
                        if (((drilling bitand onPos) bitor !game.getDrills(usefulIntVec))){
                            game.square2pos(usefulIntVec,usefulVec);
                            usefulVec[2]=game.getTerrainHeight(usefulIntVec);
                            
                            flocal=dist(usefulVec,myPos)+intDist(usefulIntVec,tenCoords)-.5f*(usefulVec[2]>=game.getTerrainHeight(mySquare));
                            if ((flocal<maxDist) bitand (dist(enPos,usefulVec)>.3f) bitand !game.isGeyserHere(usefulIntVec)){
                                if (drilling bitand !onPos){
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
        game.pos2square(positionTarget,siteCoords);
        
        
        
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
        DEBUG(("%i %i", nextSquare[0],nextSquare[1]));
        
        
        //if we are on the right square and all the conditions line up, start spinning and drilling
        if ((mathVecMagnitude(myVel,3)<.01f
        bitand ((mathVecMagnitude(myRot,3)<.035f) bitor drilling)
        bitand onSite)){
            usefulVec[0]=-myAtt[1];
            usefulVec[1]=myAtt[0];
            usefulVec[2]=0;
            
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            drilling=true;
            
        }
        else{
            usefulVec[0]=myAtt[0];
            usefulVec[1]=myAtt[1];
            usefulVec[2]=0;
            
            DEBUG(("Slowing"));
            drilling=false;
        }
        api.setAttitudeTarget(usefulVec);
        
        
       
    }
    //otherwise, drop off our samples
    else {
        memcpy(positionTarget,myPos,12);
        if (myPos[2]>.29f){
            positionTarget[2]=.05f;
        }
        else{
            //maybe take out the mathvecMagnitude expression for codesize
            scale(positionTarget,(.23f)/mathVecMagnitude(positionTarget,3));//go to a position that is .09 in the same direction at the enemy. In other words, between them and the origin.
        }
        float dropOffAtt[3];
        dropOffAtt[0] = 0.0f;
        dropOffAtt[1] = 0.0f;
        dropOffAtt[2] = -1.0f;
        api.setAttitudeTarget(dropOffAtt);

    }
    
    if (game.getDrillError() 
    bitor geyserOnMe 
    bitor (game.getDrills(mySquare)>MAXDRILLS-1)){
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

    if (not drilling){//don't drill if we aren't drilling
        game.stopDrill();
    }
    
    if(game.getFuelRemaining() < 0.03f){
        positionTarget[2] = 0;
    }
    
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    #define fvector usefulVec
    
    #define ACCEL .014f
    //mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    mathVecSubtract(fvector, destination, myPos, 3);
    flocal=0.05f/(.05f+mathVecMagnitude(fvector,3));//Just storing this value as a functional boolean
    scale(myVel,0.2f+flocal);
    mathVecSubtract(fvector,fvector,myVel,3);
    scale(fvector,.25f-.09f*flocal);
    if (geyserOnMe){
        fvector[2]=0;
        // flocal=mathVecMagnitude(fvector,3)/15;
        // fvector[0]/=flocal;
        // fvector[1]/=flocal;
        
        scale(fvector,15.0f/mathVecMagnitude(fvector,3));
        //fvector[2]=0.05f;
    }
    else if (drilling){
        fvector[2]=.5f*(positionTarget[2]-myPos[2]);
        for (int i=0;i<2;i++){
            fvector[i]=((((nextSquare[i]>mySquare[i])-(mySquare[i]>nextSquare[i]))
                *0.038f+positionTarget[i]-myPos[i])/(12.0f-4.0f*game.getDrills(mySquare)));
        }
        
    }
    else if (!tenFound and game.hasAnalyzer()){
        fvector[2]=.05f*(.2f-myPos[2]);
        mathVecNormalize(fvector,3);
        scale(fvector,.075f);
        mathVecAdd(fvector,myVel,fvector,3);
        scale(fvector,.33333f);
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
int intDist(int a1[2],int a2[2]){
    return ((a1[0]-a2[0])*(a1[0]-a2[0])+(a1[1]-a2[1])*(a1[1]-a2[1]));
}