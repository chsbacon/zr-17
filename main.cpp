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
    spot = 0;
	//This function is called once when your code is first loaded.
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
	//This function is called once per second.  Use it to control the satellite.
	if (step == 1) { //get analyzer
	    if (game.hasAnalyzer()>0) {
            step++;
        }
	}
	if (step == 2) {
	    for (int i = 0; i < 3; i++) {
	        dest[i] = spots[spot][i];
	    }
	    if (amRed) {
	        dest[0]*=-1;
	        dest[1]*=-1;
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
	if (step == 3) {
	    game.pos2square(spots[spot], square);
	    DEBUG(("FOUND IT AT SQR: %i, %i, %i", square[0], square[1], square[2]));
	    
	}
	mathVecSubtract(vecBet, dest, myPosition, 3);
    scale(myVelocity,.26f);
    mathVecSubtract(vecBet, vecBet, myVelocity, 3);
    scale(vecBet,.24f);
    //DEBUG(("%f, %f, %f", vecBet[0], vecBet[1], vecBet[2]));
    api.setVelocityTarget(vecBet);
    
}
