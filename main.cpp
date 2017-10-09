//SmartBotv2.0 3d edition

//Standard defines{
float myState[12];
float enState[12];

#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])

#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])

#define SPHERERADIUS 0.11f
#define SPEEDCONST 0.45f
#define DERIVCONST 2.8f

//}

//Variable declarations{
//float analyzer[2][6];
bool samplesHeld[3];
float zeroVec[3];
float zPlusVec[3];
float drillVec[3];
int nextMineSqr[3];
bool isBlue;
float positionTarget[3];
float sampVals[3];
float searchVals[4];/*
 1
 02
3
*/
bool pregame;
int pregameDrillSqrs[8][3];
float tempVec[3];
float tempVar;
float vcoef;
int stage;
bool drillUp;
bool keepSearching;
float analyzer[2][3];
bool pickUp[2];
int pregameDrillSpot;

bool ifTesting;
int test[3];

void init(){
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	
	//For Red Sphere
	analyzer[0][0] = 0.30f;
	analyzer[0][1] = -0.48f;
	analyzer[0][2] = -0.36;
	//For Blue Sphere
	analyzer[1][0] = -0.30f;
	analyzer[1][1] = 0.48f;
	analyzer[1][2] = -0.36;
	
    memset(zeroVec, 0, 12);
    memset(searchVals, 0, 12);
    
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    api.setAttGains(0.45f,0.1f,2.8f);
    
    isBlue = (myState[1] > 0);
    
    //game.pos2square(myPos, nextMineSquare);
    
    pregameDrillSqrs[0][0] = (isBlue) ? 5 : -5;
    pregameDrillSqrs[0][1] = (isBlue) ? 2 : -2;
    pregameDrillSqrs[0][2] = 8;
    //5, 2
    pregameDrillSqrs[1][0] = (isBlue) ? -4 : 4;
    pregameDrillSqrs[1][1] = (isBlue) ? 7 : -7;
    pregameDrillSqrs[1][2] = 8;
    //-4, 7
    pregameDrillSqrs[2][0] = (isBlue) ? -5 : 5;
    pregameDrillSqrs[2][1] = (isBlue) ? 2 : -2;
    pregameDrillSqrs[2][2] = 8;
    //-5, 2
    pregameDrillSqrs[3][0] = (isBlue) ? -1 : 1;
    pregameDrillSqrs[3][1] = (isBlue) ? 5 : -5;
    pregameDrillSqrs[3][2] = 8;
    //-1, 5
    pregameDrillSqrs[4][0] = (isBlue) ? 2 : -2;
    pregameDrillSqrs[4][1] = (isBlue) ? 7 : -7;
    pregameDrillSqrs[4][2] = 8;
    //2, 7
    pregameDrillSqrs[5][0] = (isBlue) ? 5 : -5;
    pregameDrillSqrs[5][1] = (isBlue) ? 6 : -6;
    pregameDrillSqrs[5][2] = 8;
    //5, 6
    pregameDrillSqrs[6][0] = (isBlue) ? 3 : -3;
    pregameDrillSqrs[6][1] = (isBlue) ? 4 : -4;
    pregameDrillSqrs[6][2] = 8;
    //3, 4
    pregameDrillSqrs[7][0] = (isBlue) ? -6 : 6;
    pregameDrillSqrs[7][1] = (isBlue) ? 5 : -5;
    pregameDrillSqrs[7][2] = 8;
    //-6, 5
    
    //nextMineSqr[0] = 4 + (-8 * !isBlue);
    //nextMineSqr[1] = 4 + (-8 * !isBlue);
    
    pregameDrillSpot = 0;
    drillUp = 1;
    pregame = true;
    stage = 0;
    keepSearching = false;
    
    zPlusVec[0] = 0.0f;
    zPlusVec[1] = 0.0f;
    zPlusVec[2] = -1.0f;
    
    ifTesting = 0;
    test[0] = 4;
    test[1] = 4;
    test[2] = 8;
    
    memcpy(nextMineSqr, test, 12);
    
}

void loop(){
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	game.getSamplesHeld(samplesHeld);
	//DEBUG((""));
    if(ifTesting){
        if(game.getDrillError()){
            game.stopDrill();
        }
        if(game.getNumSamplesHeld() == 3){
            memcpy(positionTarget, zeroVec, 12);
            api.setAttitudeTarget(zPlusVec);//point at -z
            if(game.atBaseStation()){
                //api.setVelocityTarget(zeroVec);
                game.dropSample(0);
                game.dropSample(1);
                game.dropSample(2);
            }
            goto end;
            //break;
        }
        DEBUG(("%d, %d", nextMineSqr[0], nextMineSqr[1]));
        game.square2pos(nextMineSqr, positionTarget);
        positionTarget[2] -= 0.04f + SPHERERADIUS/2.0f;
        memcpy(tempVec, myAtt, 12);
        tempVec[2] = 0.0f;
        api.setAttitudeTarget(tempVec);
        DEBUG(("Samps (%d)", game.getNumSamplesHeld()));
        DEBUG(("%f, %f, %f", positionTarget[0], positionTarget[1], positionTarget[2]));
        //DEBUG(("%d (%f) %d %d", dist(myPos, positionTarget) <= 0.03f, dist(myPos, positionTarget), mathVecMagnitude(myVel, 3) < 0.01f , !game.getDrillEnabled()));
        if(dist(myPos, positionTarget) < .03f and mathVecMagnitude(myVel, 3) < 0.01f and myState[11] < 0.04f and !game.getDrillEnabled() and myState[2] > 0.5f and fabsf(myAtt[2]) < 0.06f){
            DEBUG(("Starting Drill"));
            game.startDrill();
        }
        else if(game.getDrillEnabled()){
            DEBUG(("Drilling"));
            drillVec[0] = myState[7];
            drillVec[1] = -1 * myState[6];
            drillVec[2] = 0;
            api.setAttitudeTarget(drillVec);
            if(game.checkSample()){
                game.pickupSample();
                game.getConcentrations(sampVals);
                DEBUG(("Num samples: %d", game.getNumSamplesHeld()));
                if(game.getNumSamplesHeld() > 2){
                    DEBUG(("Stopping Drill"));
                    game.stopDrill();
                    
                }
            }
        }
        goto end;
    }
    if(api.getTime() >= 150 and game.getNumSamplesHeld() > 0){
        game.stopDrill();
        memcpy(positionTarget, myPos, 12);
        mathVecNormalize(positionTarget, 3);
        scale(positionTarget, 0.14f);
    }
    else{
        DEBUG(("STAGE(%d)", stage));
        switch(stage){
            case 0://Pregame drilling
                if(pregameDrillSpot == 0){
                    pregameDrillSpot = drillAtSqr(&pregameDrillSqrs[0][0]);
                    break;
                }
                game.getAnalyzer(pickUp);
                if(!pickUp[isBlue] and game.hasAnalyzer() - 1 != isBlue){
                    memcpy(positionTarget, &analyzer[isBlue], 12);
                }
                else{
                    if(drillAtSqr(&pregameDrillSqrs[pregameDrillSpot][0])){
                        game.getConcentrations(sampVals);
                        if(sampVals[game.getNumSamplesHeld()] > 0.1f){
                            if(sampVals[game.getNumSamplesHeld()] == 1.0f){
                                stage = 2;
                                break;
                            }
                            game.dropSample(0); game.dropSample(1); game.dropSample(2);//drop all samples
                            searchVals[0] = sampVals[game.getNumSamplesHeld()];
                            DEBUG(("ENDING PREGAME"));
                            memcpy(nextMineSqr, &pregameDrillSqrs[pregameDrillSpot][0], 12);
                            stage = 1;
                            break;
                        }
                        else{
                            pregameDrillSpot++;
                        }
                    }
                }
                break;
            case 1://Finding the 10
                if(searchVals[1] == 0.0f){
                    if(drillAtSqr(nextMineSqr)){
                        game.getConcentrations(sampVals);
                        if(sampVals[1] == 1.0f){
                            stage = 2;
                            break;
                        }
                        nextMineSqr[0] += (isBlue) ? -1 : 1;
                        searchVals[1] = sampVals[0];
                        game.dropSample(0);
                    }
                }
                else if(searchVals[2] == 0.0f){
                    if(drillAtSqr(nextMineSqr)){
                        game.getConcentrations(sampVals);
                        if(sampVals[1] == 1.0f){
                            stage = 2;
                            break;
                        }
                        nextMineSqr[0] += (isBlue) ? 1 : -1;
                        nextMineSqr[1] += (isBlue) ? 1 : -1;
                        searchVals[2] = sampVals[0];
                        game.dropSample(0);
                    }
                }
                else if(searchVals[3] == 0.0f){
                    if(drillAtSqr(nextMineSqr)){
                        game.getConcentrations(sampVals);
                        if(sampVals[1] == 1.0f){
                            stage = 2;
                            break;
                        }
                        nextMineSqr[0] += (isBlue) ? 1 : -1;
                        nextMineSqr[1] += (isBlue) ? -2 : 2;
                        searchVals[3] = sampVals[0];
                        game.dropSample(0);
                    }
                }
                else{
                    //finding the 10 with given spots
                }
                break;
            case 2://Mining the 10
                DEBUG(("It's a 10 bb"));
                DEBUG(("Oh bb a triple"));
                else if(game.isGeyserHere(nextMineSqr) and api.getTime() <= 120){
                    nextMineSqr[0] *= -1;
                    nextMineSqr[1] *= -1;
                }
                if(game.getDrillError()){
                    game.stopDrill();
                }
                if(game.getNumSamplesHeld() == 3){
                    memcpy(positionTarget, zeroVec, 12);
                    api.setAttitudeTarget(zPlusVec);//point at -z
                    if(game.atBaseStation()){
                        //api.setVelocityTarget(zeroVec);
                        game.dropSample(0);
                        game.dropSample(1);
                        game.dropSample(2);
                    }
                    break;
                }
                DEBUG(("%d, %d", nextMineSqr[0], nextMineSqr[1]));
                game.square2pos(nextMineSqr, positionTarget);
                positionTarget[2] -= 0.04f + SPHERERADIUS/2.0f;
                memcpy(tempVec, myAtt, 12);
                tempVec[2] = 0.0f;
                api.setAttitudeTarget(tempVec);
                DEBUG(("Samps (%d)", game.getNumSamplesHeld()));
                DEBUG(("%f, %f, %f", positionTarget[0], positionTarget[1], positionTarget[2]));
                //DEBUG(("%d (%f) %d %d", dist(myPos, positionTarget) <= 0.03f, dist(myPos, positionTarget), mathVecMagnitude(myVel, 3) < 0.01f , !game.getDrillEnabled()));
                if(dist(myPos, positionTarget) < .03f and mathVecMagnitude(myVel, 3) < 0.01f and myState[11] < 0.04f and !game.getDrillEnabled() and myState[2] > 0.5f and fabsf(myAtt[2]) < 0.06f){
                    DEBUG(("Starting Drill"));
                    game.startDrill();
                }
                else if(game.getDrillEnabled()){
                    DEBUG(("Drilling"));
                    drillVec[0] = myState[7];
                    drillVec[1] = -1 * myState[6];
                    drillVec[2] = 0;
                    api.setAttitudeTarget(drillVec);
                    if(game.checkSample()){
                        game.pickupSample();
                        game.getConcentrations(sampVals);
                        DEBUG(("Num samples: %d", game.getNumSamplesHeld()));
                        if(game.getNumSamplesHeld() > 2){
                            DEBUG(("Stopping Drill"));
                            game.stopDrill();
                            
                        }
                    }
                }
                break;
        }
    }
    end: 1 + 1;
    if(!game.getDrillEnabled() and myState[11] > 0.037f){
        api.setAttRateTarget(zeroVec);
    }
    
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
    
    //positionTarget[2] *= -1.0f;
    nextMineSqr[2] = 8;
    api.setPositionTarget(positionTarget);
}

//Game specific functions{
bool drillAtSqr(int* sqr){
    if(game.getDrillError()){
        game.stopDrill();
    }
    DEBUG(("%d, %d", sqr[0], sqr[1]));
    game.square2pos(sqr, positionTarget);
    positionTarget[2] -= 0.04f + SPHERERADIUS/2.0f;
    memcpy(tempVec, myAtt, 12);
    tempVec[2] = 0.0f;
    api.setAttitudeTarget(tempVec);
    DEBUG(("%f, %f, %f", positionTarget[0], positionTarget[1], positionTarget[2]));
    //DEBUG(("%d (%f) %d %d", dist(myPos, positionTarget) <= 0.03f, dist(myPos, positionTarget), mathVecMagnitude(myVel, 3) < 0.01f , !game.getDrillEnabled()));
    if(dist(myPos, positionTarget) < .03f and mathVecMagnitude(myVel, 3) < 0.01f and myState[11] < 0.04f and !game.getDrillEnabled() and myState[2] > 0.5f and fabsf(myAtt[2]) < 0.06f){
        DEBUG(("Starting Drill"));
        game.startDrill();
    }
    else if(game.getDrillEnabled()){
        DEBUG(("Drilling"));
        drillVec[0] = myState[7];
        drillVec[1] = -1 * myState[6];
        drillVec[2] = 0;
        api.setAttitudeTarget(drillVec);
        if(game.checkSample()){
            DEBUG(("sampsBeforePickup(%f,%f,%f)", searchVals[0], searchVals[1], searchVals[2]));
            game.pickupSample();
            game.getConcentrations(sampVals);
            DEBUG(("sampsAfterPickup(%f,%f,%f)", searchVals[0], searchVals[1], searchVals[2]));
            DEBUG(("Stopping Drill"));
            game.stopDrill();
            return true;
        }
    }
    /*else if(myState[11] > 0.037f){
        api.setAttRateTarget(zeroVec);
    }*/
    return false;
}

//Vector math functions{
    float dist(float* vec1, float* vec2){
        float ansVec[3];
        mathVecSubtract(ansVec, vec1, vec2, 3);
        return mathVecMagnitude(ansVec, 3);
    }
    float angle(float* vec1, float* vec2){
        return acosf(mathVecInner(vec1,vec2,3)/(mathVecMagnitude(vec1,3)*mathVecMagnitude(vec2,3)));
    }
    void scale(float* vec, float scale){
        for (int i=0; i<3; i++)
            vec[i] *= scale;
    }
    
//}