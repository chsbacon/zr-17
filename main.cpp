//This strategy involves going to precalculated spots and guaruntees finding the concentration on our side
//Normally, should find in a couple of searches but may take longer on very rare occasions
float myPosition[3];  //Current Position of the Sphere
float myVelocity[3];
float myAttitude[3];
float myAttRate[3];
float myState[12];
float blAnaPos[3];
float reAnaPos[3];
float vecBet[3];
float dest[3];
float spots[10][3];
int locSqr [3];
int step;
bool amRed;
float velocity;
int spot;
float concentration;
int square[3];
int targSqr[3];
int currSqr[3];
int centerConcSqr[3];
float targetPos[3];
int locs [4][2];
float concs[4];
int locPos;
void scale (float* vec, float scale) {//This function scales a length-3 vector by a coeff.
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}

void update () {
   // api.getOtherZRState(otherState);
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++) {
        myPosition[i] = myState[i];
    }
    for (int i = 0; i < 3; i++) {
        myVelocity[i] = myState[i+3];
    }
    for (int i = 0; i < 3; i++) {
        myAttitude[i] = myState[i+6];
    }
    for (int i = 0; i < 3; i++) {
        myAttRate[i] = myState[i+9];
    }
    velocity = mathVecMagnitude(myVelocity, 3);
}
void init(){
    centerConcSqr[2] = -1;
    locPos = 0;
    spot = 0;
	update();
	step = 1;
	blAnaPos[0] = -0.3;
	blAnaPos[1] = 0.48;
	blAnaPos[2] = -0.16;
	reAnaPos[2] = -0.16;
	for (int i = 0; i < 2; i++) {
	    reAnaPos[i] = -1*blAnaPos[i];
	}
    if (myPosition[0] > 0) {
        for (int i = 0; i < 3; i++) {
    	    dest[i] = blAnaPos[i];
    	}
        amRed = false;
    } else {
        amRed = true;
        for (int i = 0; i < 3; i++) {
    	    dest[i] = reAnaPos[i];
    	}
    }
    square[0] = -4;
    square[1] = 6;
    square[2] = 0;
    game.square2pos(square, spots[0]);
    
    square[0] = 1;
    square[1] = 7;
    //square[2] = 0;
    game.square2pos(square, spots[1]);
    
    square[0] = 5;
    square[1] = 7;
    //square[2] = 0;
    game.square2pos(square, spots[2]);
    
    square[0] = 2;
    square[1] = 4;
    //square[2] = 0;
    game.square2pos(square, spots[3]);
    
    square[0] = -3;
    square[1] = 3;
    //square[2] = 0;
    game.square2pos(square, spots[4]);
    
    square[0] = -5;
    square[1] = -2;
    //square[2] = 0;
    game.square2pos(square, spots[5]);
    
    square[0] = -6;
    square[1] = -5;
    //square[2] = 0;
    game.square2pos(square, spots[6]);
    
    square[0] = -6;
    square[1] = 3;
    //square[2] = 0;
    game.square2pos(square, spots[7]);
    
    square[0] = -6;
    square[1] = 8;
    //square[2] = 0;
    game.square2pos(square, spots[8]);
    
    square[0] = 8;
    square[1] = 8;
    //square[2] = 0;
    game.square2pos(square, spots[9]);
    if (amRed) {
        for (int i = 0; i < 10; i++) {
            spots[i][0]*=-1;
            spots[i][1]*=-1;
        }
    }
    //Bottom Right
    locs[0][0] = 1;
    locs[0][1] = 1;
    
    //Top Right
    locs[1][0] = 1;
    locs[1][1] = -1;
    
    //Top Left
    locs[2][0] = -1;
    locs[2][1] = -1;
    
    //Bottom Left
    locs[3][0] = -1;
    locs[3][1] = 1;
    
    //DEBUG
    /*for (int i = 0; i < 10; i++) {
        game.pos2square(spots[i], square);
        DEBUG(("SPOT %i: %i, %i, %i", i, square[0], square[1], square[2]));
    }*/
}

void loop(){
    /*if (step==2) {
        for (int i = 0; i < 3; i++) {
	        dest[i] = spots[spot][i];
	    }
	    for (int i = 0; i < 10; i++) {
            game.pos2square(spots[i], square);
            DEBUG(("SPOT %i: %i, %i, %i", i, square[0], square[1], square[2]));
        }
    }*/
    //DEBUG(("SPOT %i, %f, %f, %f, *** %f, %f, %f", spot, dest[0], dest[1], dest[2], spots[spot][0], spots[spot][1], spots[spot][2]));
    update();
	if (step == 1) { //get analyzer
	    if (game.hasAnalyzer()>0) {
            step++;
        }
	}
	if (step == 2) {
	    for (int i = 0; i < 3; i++) {
	        dest[i] = spots[spot][i];
	    }
	    
	    game.pos2square(myPosition, square);
	    int tempSqr[3];
	    game.pos2square(spots[spot], tempSqr);
	    bool temp = true;
	    for (int i = 0; i < 2; i++) {
	        if (square[i] != tempSqr[i]) {
	            temp = false;
	            break;
	        }
	    }
	    if (velocity < 0.04 && temp) {
	        concentration = game.analyzeTerrain();
	        DEBUG(("CONCENTRATION %f", concentration));
	        if (concentration > 0.2) {
	            step++;
	        } else if (concentration > 0) {
	            spot++;
	        }
	    }
	}
	if (step == 3) { //hover around to find center
	    game.pos2square(spots[spot], currSqr);
	    DEBUG(("FOUND IT AT SQR: %i, %i, %i", currSqr[0], currSqr[1], currSqr[2]));
	    if (concentration > 0.4) {
	        locPos=0;
	        step++;
	    } else {
	        for (int i = 0; i < 2; i++) {
	            targSqr[i] = currSqr[i]+locs[locPos][i];
	        }
	        targSqr[2] = -1;
	        game.square2pos(targSqr, targetPos);
	        for (int i = 0; i < 3; i++) {
	            dest[i] = targetPos[i];
	        }
	        
	        game.pos2square(myPosition, square);
    	    bool temp = true;
    	    for (int i = 0; i < 2; i++) {
    	        if (square[i] != targSqr[i]) {
    	            temp = false;
    	            break;
    	        }
    	    }
    	    if (velocity < 0.04 && temp) {
    	        concentration = game.analyzeTerrain();
    	        DEBUG(("CONCENTRATION %f", concentration));
    	        if (concentration > 0.4) {
    	            for (int i = 0; i < 3; i++) {
    	                currSqr[i] = targSqr[i];
    	            }
    	            locPos=0;
    	            step++;
    	        } else if (concentration > 0) {
    	            locPos++;
    	        }
    	    }
	    }
	}
	if (step == 4) {
	    DEBUG(("60 concentration found"));
	    if (concentration > 0.7) {
	        step++;
	    } else {
	        if (locPos<4) {
    	        for (int i = 0; i < 2; i++) {
    	            targSqr[i] = currSqr[i]+locs[locPos][i];
    	        }
    	        targSqr[2] = -1;
    	        game.square2pos(targSqr, targetPos);
    	        for (int i = 0; i < 3; i++) {
    	            dest[i] = targetPos[i];
    	        }
    	        
    	        game.pos2square(myPosition, square);
        	    bool temp = true;
        	    for (int i = 0; i < 2; i++) {
        	        if (square[i] != targSqr[i]) {
        	            temp = false;
        	            break;
        	        }
        	    }
        	    if (velocity < 0.04 && temp) {
        	        concentration = game.analyzeTerrain();
        	        DEBUG(("CONCENTRATION %f", concentration));
        	        if (concentration > 0.8) {
        	            for (int i = 0; i < 3; i++) {
        	                centerConcSqr[i] = targSqr[i];
        	            }
        	            step++;
        	        } else if (concentration > 0) {
        	            concs[locPos] = concentration;
        	            locPos++;
        	        }
        	    }
	        } else {
	            int sqr1[3];
	            int sqr2[3];
	            sqr1[2] = -1;
	            sqr2[2] = -1;
	            bool firFound = false;
	            for (int i = 0; i < 4; i++) {
	                if (concs[i] > 0.4) {
	                    if (!firFound) {
    	                    for (int j = 0; j < 2; j++) {
    	                        sqr1[j] = currSqr[j]+locs[i][j];
    	                    }
    	                    firFound = true;
	                    } else {
	                        for (int j = 0; j < 2; j++) {
    	                        sqr2[j] = currSqr[j]+locs[i][j];
    	                    }
	                    }
	                }
	            }
	            if (sqr1[0] == sqr2[0]) {
	                centerConcSqr[0] = sqr1[0];
	                if (fabsf(sqr1[1]) > fabsf(sqr2[1])) {
	                    if (sqr2[1] < 0) {
	                        centerConcSqr[1] = sqr2[1]-1;
	                    } else {
	                        centerConcSqr[1] = sqr2[1]+1;
	                    }
	                } else {
	                    if (sqr1[1] < 0) {
	                        centerConcSqr[1] = sqr1[1]-1;
	                    } else {
	                        centerConcSqr[1] = sqr1[1]+1;
	                    }
	                }
	            } else {
	                centerConcSqr[1] = sqr1[1];
	                if (fabsf(sqr1[0]) > fabsf(sqr2[0])) {
	                    if (sqr2[0] < 0) {
	                        centerConcSqr[0] = sqr2[0]-1;
	                    } else {
	                        centerConcSqr[0] = sqr2[0]+1;
	                    }
	                } else {
	                    if (sqr1[0] < 0) {
	                        centerConcSqr[0] = sqr1[0]-1;
	                    } else {
	                        centerConcSqr[0] = sqr1[0]+1;
	                    }
	                }
	            }
	            step++;
	        }
	    }
	}
	if (step == 5) {//Begin Drilling and collecting points
	    DEBUG(("FOUND IT 100 Concentration AT SQR: %i, %i, %i", centerConcSqr[0], centerConcSqr[1], centerConcSqr[2]));
	}
	mathVecSubtract(vecBet, dest, myPosition, 3);
    scale(myVelocity,.26f);
    mathVecSubtract(vecBet, vecBet, myVelocity, 3);
    scale(vecBet,.24f);
    //DEBUG(("%f, %f, %f", vecBet[0], vecBet[1], vecBet[2]));
    api.setVelocityTarget(vecBet);
    
}
