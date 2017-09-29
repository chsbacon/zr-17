#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]))
float positionTarget[3];
int targetCoordinates[3];
float dist[3];
float zeroVec[3];
float myState[12];
float usefulVec[3];
int usefulIntVec[3];
int mySquare[3];
float enState[12];
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9]) //This is a comment
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])//These are pointers. They will have the values that
int siteCoords[2];
bool newLoc;
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
    api.setAttGains(0.45f,0.1f,2.8f);
	
}

void loop(){
    api.getMyZRState(myState);
    myPos[2]=0.f;
    api.getOtherZRState(enState);//Makes sure our data on where they are is up to date
    enPos[2]=0;
    game.pos2square(myPos,mySquare);
    float maxDist=100;//Sets this large
    if (newLoc){
        for (int i=-8;i<9;i++){//This checks all of the grid spaces, and sees which is both
        //closest to us and in the center. You should understand this search structure - it's important!
            for (int j=-10;j<11;j++){
                if ((i!=0) and (j!=0)){
                    //DEBUG(("%i %i",i,j));
                    usefulIntVec[0]=i;usefulIntVec[1]=j;usefulIntVec[2]=0;
                    game.square2pos(usefulIntVec,usefulVec);
                    mathVecSubtract(usefulVec,myPos,usefulVec,3);
                    usefulVec[2]=0;
                    float score=mathVecMagnitude(usefulVec,3)-10*(mathVecMagnitude(enPos,3)>0.4f xor i*i+j*j<3);
                    if (score<maxDist and game.getDrills(usefulIntVec)<1 and i*i+j*j<7){
                        siteCoords[0]=i;siteCoords[1]=j;
                        DEBUG(("Changed %f", score));
                        maxDist = score;
                    }
                }
            }
        }
        newLoc=false;
    }
    DEBUG(("%i %i", siteCoords[0],siteCoords[1]));
    if (mathVecMagnitude(enPos,3)<.3f){//If they are close to the center
        vcoef=.120f;
        //Go to the position that is between them and the center, at .04 distance
        //so that they can't get into the zone
        memcpy(positionTarget,enPos,12);
        scale(positionTarget,.04f/mathVecMagnitude(enPos,3));
        game.stopDrill();
        newLoc=true;
        
    }
    else{
        vcoef=.170f;
        game.square2pos(siteCoords,positionTarget);
        positionTarget[2]=myState[2];
        if (mathVecMagnitude(myVel,3)<.008 and (mathVecMagnitude(myRot,3)<.04 or game.getDrillEnabled()) and distsquared(myState,positionTarget)<0.0016 and game.getNumSamplesHeld()<3 and not game.getDrillError()){
            usefulVec[0]=myState[7];usefulVec[1]=-1*myState[6];usefulVec[2]=0;
            api.setAttitudeTarget(usefulVec);
            if (!game.getDrillEnabled()){
                game.startDrill();
            }
            DEBUG(("Got it"));
            
        }
        else{
            api.setAttRateTarget(zeroVec);
        }
        if (game.getDrillError() or mySquare[0]!=siteCoords[0] or mySquare[1]!=siteCoords[1]){
            game.stopDrill();
        }
        if (game.checkSample()){
            game.pickupSample();
            game.stopDrill();
            newLoc=true;
        }
        if (game.atBaseStation()){
            game.dropSample(0);
            game.dropSample(1);
            game.dropSample(2);
        }
    }
    
    
    
    positionTarget[2]=0;
	#define destination positionTarget//This (next 20 or so lines) is movement code.
	//It is fairly strange - we will go over exactly how it works eventually
    float distance,flocal,fvector[3];
    #define ACCEL .0175f
    mathVecSubtract(fvector, destination, myPos, 3);//Gets the vector from us to the target
    distance = mathVecNormalize(fvector, 3);
    if (distance > 0.05f) {//If not close, decide on our velocity
        flocal = vcoef;
        //flocal = 10.f;
        //flocal=velocity+accel*6.f;
        //if ( ( fabs(flocal - velocity) > 0.008f)){
            //DEBUG(("VELOCITY CONTROL : vel = %f  des_vel = %f", velocity, flocal));
            if (flocal*flocal/ACCEL>distance-.02f){//Cap on how fast we go
                flocal = sqrtf(distance*ACCEL)-.02f;
                //DEBUG(("Slower"));
            }
            scale(fvector, flocal);
            api.setVelocityTarget(fvector);
        //}
    }
    else{//If close:
        api.setPositionTarget(destination);
        //for (int i = 0; i<9; i++) {
            //game.hasItem(i)==-1?
        //}
    }
}
float distsquared(float pos1[3],float pos2[3]){//This gives the squared distance between to coordinates
    float sum=0;
    for (int i=0;i<2;i++){
        sum+=(pos1[i]-pos2[i])*(pos1[i]-pos2[i]);
    }
    return (sum);
}
int distsquared(int pos1[3],int pos2[3]){//This gives the squared distance between to coordinates
    int sum=0;
    for (int i=0;i<2;i++){
        sum+=(pos1[i]-pos2[i])*(pos1[i]-pos2[i]);
    }
    return (sum);
}
void scale (float* vec, float scale) {//This function scales a length-3 vector by a coeff.
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}