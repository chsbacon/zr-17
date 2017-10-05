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
float drillVec[3];
int nextMineSqr[3];
bool isBlue;
float positionTarget[3];
float sampVals[3];
float searchVals[3];
bool pregame;
int pregameDrillSqrs[4][3];
float tempVec[3];
float tempVar;
float vcoef;
int stage;
bool drillUp;
bool keepSearching;
float analyzer[2][3];
bool pickUp[2];

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
    
    pregameDrillSqrs[0][0] = (isBlue) ? 4 : -4;
    pregameDrillSqrs[0][1] = (isBlue) ? 4 : -4;
    pregameDrillSqrs[0][2] = 8;
    
    pregameDrillSqrs[1][0] = (isBlue) ? -4 : 4;
    pregameDrillSqrs[1][1] = (isBlue) ? 7 : -7;
    pregameDrillSqrs[1][2] = 8;
    
    pregameDrillSqrs[2][0] = (isBlue) ? -4 : 4;
    pregameDrillSqrs[2][1] = (isBlue) ? 4 : -4;
    pregameDrillSqrs[2][2] = 8;
    
    pregameDrillSqrs[3][0] = (isBlue) ? 4 : -4;
    pregameDrillSqrs[3][1] = (isBlue) ? 7 : -7;
    pregameDrillSqrs[3][2] = 8;
    
    //nextMineSqr[0] = 4 + (-8 * !isBlue);
    //nextMineSqr[1] = 4 + (-8 * !isBlue);
    
    drillUp = 1;
    pregame = true;
    stage = 0;
    keepSearching = false;
    
    ifTesting = false;
    test[0] = 4;
    test[1] = 4;
    test[2] = 8;
}

void loop(){
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
	game.getSamplesHeld(samplesHeld);
	//DEBUG((""));
    if(ifTesting){
        drillAtSqr((test));
        goto end;
    }
    if(api.getTime() >= 157 and game.getNumSamplesHeld() > 0){
        game.stopDrill();
        memcpy(positionTarget, myPos, 12);
        mathVecNormalize(positionTarget, 3);
        scale(positionTarget, 0.14f);
    }
    else{
        if(pregame){//Pregame preparing
            game.getAnalyzer(pickUp);
            if(!samplesHeld[0]){//Drill at starting position
                drillAtSqr((&pregameDrillSqrs[0][0]));
            }
            else if(!pickUp[isBlue] and game.hasAnalyzer() - 1 != isBlue){
                memcpy(positionTarget, &analyzer[isBlue], 12);
            }
            else if(drillAtSqr(&pregameDrillSqrs[1][0])){
                pregame = false;
                game.getConcentrations(sampVals);
                DEBUG(("ENDING PREGAME"));
            }
        }
        else{//Get more bacteria based on that info and drop it off
            //memset(positionTarget, 0.0f, 12);
            //DEBUG(("Samples: %f, %f, %f", sampVals[0], sampVals[1], sampVals[2]));
            DEBUG(("STAGE(%d)", stage));
            
            switch(stage){
                case 0://Selecting best mine spot
                    for(int i = 0; i < 2; i++){
                        if(sampVals[i] == 1.0f){
                            memcpy(nextMineSqr, (&pregameDrillSqrs[i][0]), 8);
                            stage = 4;
                            goto bacteriaFound;
                        }
                        //DEBUG(("BOOLS: %d, %d, >(%d, %d)< ite(%d)", sampVals[i] > 0.1f, sampVals[i] > 0.3f, sampVals[i] >= sampVals[(i = 0) ? 1 : (i = 1) ? 0 : (i = 2) ? 0 : 3], sampVals[i] >= sampVals[(i = 0) ? 2 : (i = 1) ? 2 : (i = 2) ? 1 : 3],i));
                        if(sampVals[i] > 0.1f and sampVals[i] >= sampVals[(i == 0) ? 1 : (i = 1) ? 0 : 3] and sampVals[i] >= sampVals[(i == 0) ? 2 : (i = 1) ? 2 : 3]){
                            memcpy(nextMineSqr, (&pregameDrillSqrs[i][0]), 8);
                            stage = 2;
                            
                            drillUp = (!isBlue) ? i : !i;
                            
                            DEBUG(("SEARCHING AT %d", i));
                            DEBUG(("DrillUp = %d", drillUp));
                            
                            nextMineSqr[0] += (isBlue) ? -1 : 1;
                            nextMineSqr[1] += (drillUp) ? -1 : 1;
                            
                            searchVals[0] = sampVals[i];
                            
                            game.dropSample(1);
                            game.dropSample(2);
                            
                            game.getConcentrations(sampVals);
                            DEBUG(("sampVals(%f, %f, %f)", sampVals[0], sampVals[1], sampVals[2]));
                            goto bacteriaFound;
                        }
                    }
                    //What to do if we have to check 3{
                    stage = 1;
                    memcpy(nextMineSqr, (&pregameDrillSqrs[2][0]), 8);
                    drillUp = !isBlue;
                    game.dropSample(0);
                    game.dropSample(1);
                    game.dropSample(2);
                    
                    break;
                    //}
                    bacteriaFound: break;
                case 1://Pick spot triple drill
                    if(drillAtSqr(nextMineSqr)){
                        game.getConcentrations(sampVals);
                        if(sampVals[0] > 0.1f or sampVals[1] > 0.1f){
                            if(sampVals[0] == 1.0f or sampVals[1] == 1.0f or sampVals[2] == 1.0f){
                                stage = 4;
                                break;
                            }
                            searchVals[0] = sampVals[0];
                            nextMineSqr[0] += (isBlue) ? -1 : 1;
                            nextMineSqr[1] += (drillUp) ? -1 : 1;
                            DEBUG(("sampVals(%f, %f, %f)", sampVals[0], sampVals[1], sampVals[2]));
                            stage = 2;
                            break;
                        }
                        else{
                            memcpy(nextMineSqr, (&pregameDrillSqrs[3][0]), 8);
                            DEBUG(("Going to 4th spot %d, %d", nextMineSqr[0], nextMineSqr[1]));
                            game.dropSample(0);
                            game.dropSample(1);
                            game.dropSample(2);
                            drillUp = isBlue;
                        }
                    }
                    break;
                case 2://Drilling pyramid
                    if(game.getNumSamplesHeld() == 1){
                        if(drillAtSqr(nextMineSqr)){
                            game.getConcentrations(sampVals);
                            if(sampVals[1] == 1.0f){
                                stage = 4;
                                break;
                            }
                            nextMineSqr[0] += (isBlue) ? 2 : -2;
                            searchVals[1] = sampVals[1];
                        }
                    }
                    else if(game.getNumSamplesHeld() == 2){
                        if(drillAtSqr(nextMineSqr)){
                            game.getConcentrations(sampVals);
                            if(sampVals[2] == 1.0f){
                                stage = 4;
                                break;
                            }
                            DEBUG(("sampVals(%f, %f, %f)", sampVals[0], sampVals[1], sampVals[2]));
                            stage = 3;
                            nextMineSqr[0] += (isBlue) ? -1 : 1;
                            nextMineSqr[1] += (drillUp) ? 1 : -1;
                            searchVals[2] = sampVals[2];
                        }
                    }
                    break;
                case 3://Find ten
                    DEBUG(("MINESQUARE(%d, %d)", nextMineSqr[0], nextMineSqr[1]));
                    DEBUG(("SEARCH VALUES(%f, %f, %f)", searchVals[0], searchVals[1], searchVals[2]));
                    if(searchVals[0] == 0.3f){
                        if(searchVals[1] == 0.1f){
                            if(searchVals[2] == 0.3f){//3|1,3
                                DEBUG(("3|1,3"));
                                nextMineSqr[0] += (isBlue) ? 2 : -2;
                                nextMineSqr[1] += (drillUp) ? 1 : -1;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.6f){//3|1,6 special case
                                DEBUG(("3|1,6"));
                                nextMineSqr[0] += (isBlue) ? 2 : -2;
                                //nextMineSqr[1] += ((drillUp) * -2) + 1;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                        if(searchVals[1] == 0.3f){
                            if(searchVals[2] == 0.1f){//3|3,1
                                DEBUG(("3|3,1"));
                                nextMineSqr[0] += (isBlue) ? -2 : 2;//left 2
                                nextMineSqr[1] += (drillUp) ? 1 : -1;//up or down 1
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.6f){//3|3,6
                                DEBUG(("3|3,6"));
                                nextMineSqr[0] += (isBlue) ? 1 : -1;//right 1 
                                nextMineSqr[1] += (drillUp) ? -2 : 2;//up or down 2
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                        if(searchVals[1] == 0.6f){
                            if(searchVals[2] == 0.1f){//3|6,1
                                DEBUG(("3|6,1"));
                                nextMineSqr[0] += (isBlue) ?  -2 : 2;//I dunno
                                //nextMineSqr[1] += ;//I dunno
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.3f){//3|6,3
                                DEBUG(("3|6,3"));
                                nextMineSqr[0] += (isBlue) ? -1 : 1;//left 1
                                nextMineSqr[1] += (drillUp) ? -2 : 2;//up or down 2
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.6f){//3|6,6
                                DEBUG(("3|6,6"));
                                //nextMineSqr[0] += (isBlue) ? 1 : -1;//nothin
                                nextMineSqr[1] += (drillUp) ? -2 : 2;//down 2
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                    }
                    if(searchVals[0] == 0.6f){
                        if(searchVals[1] == 0.1f){
                            if(searchVals[2] == 0.3f){//6|1,3
                                DEBUG(("6|1,3"));
                                nextMineSqr[0] += (isBlue) ? 1 : -1;//right 1
                                nextMineSqr[1] += (drillUp) ? 1 : -1;//up or down 1
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                        if(searchVals[1] == 0.3f){
                            if(searchVals[2] == 0.1f){//6|3,1
                                DEBUG(("6|3,1"));
                                nextMineSqr[0] += (isBlue) ? -1 : 1;//left 1
                                nextMineSqr[1] += (drillUp) ? 1 : -1;//up or down 1
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.3f){//6|3,3
                                DEBUG(("6|3,3"));
                                //nextMineSqr[0] += ;
                                nextMineSqr[1] += (drillUp) ? 1 : -1;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.6f){//6|3,6
                                DEBUG(("6|3,6"));
                                nextMineSqr[0] += (isBlue) ? 1 : -1;
                                //nextMineSqr[1] += ;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 1.0f){//6|3,1
                                DEBUG(("6|3,1"));
                                nextMineSqr[0] += (isBlue) ? 1 : -1;
                                nextMineSqr[1] += (drillUp) ? -1 : 1;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                        if(searchVals[1] == 0.6f){
                            if(searchVals[2] == 0.3f){//6|6,3
                                DEBUG(("6|6,3"));
                                nextMineSqr[0] += (isBlue) ? -1 : 1;;
                                //nextMineSqr[1] += ;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                            if(searchVals[2] == 0.6f){//6|6,6
                                DEBUG(("6|6,6"));
                                //nextMineSqr[0] += ;
                                nextMineSqr[1] += (drillUp) ? -1 : 1;
                                stage = 4;
                                game.dropSample(0);
                                game.dropSample(1);
                                game.dropSample(2);
                                break;
                            }
                        }
                    }
                    break;
                case 4://Mining 10
                    DEBUG(("It's a 10 bb"));
                    game.getConcentrations(sampVals);
                    if(game.getNumSamplesHeld() == 3){
                        memcpy(positionTarget, myPos, 12);
                        mathVecNormalize(positionTarget, 3);
                        scale(positionTarget, 0.14f - SPHERERADIUS);
                    }
                    else if(game.isGeyserHere(nextMineSqr) and api.getTime() <= 120){
                        nextMineSqr[0] *= -1;
                        nextMineSqr[1] *= -1;
                    }
                    else{
                        if(drillAtSqr(nextMineSqr)){
                            game.getConcentrations(sampVals);
                        }
                    }
                    break;
            }
        }
    }
    if(game.atBaseStation()){
        api.setVelocityTarget(zeroVec);
        game.dropSample(0);
        game.dropSample(1);
        game.dropSample(2);
    }
    if(!game.getDrillEnabled() and myState[11] > 0.037f){
        api.setAttRateTarget(zeroVec);
    }
    end: 1 + 1;
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
    DEBUG(("%f, %f, %f", positionTarget[0], positionTarget[1], positionTarget[2]));
    //DEBUG(("%d (%f) %d %d", dist(myPos, positionTarget) <= 0.03f, dist(myPos, positionTarget), mathVecMagnitude(myVel, 3) < 0.01f , !game.getDrillEnabled()));
    if(dist(myPos, positionTarget) < .03f and mathVecMagnitude(myVel, 3) < 0.01f and myState[11] < 0.04f and !game.getDrillEnabled() and myState[2] > 0.5f){
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