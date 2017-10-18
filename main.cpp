
float timetosquare;
int timeLeft;
float origin[3];
float myState[12];
float oldPos[3];
float speed[3];
int atBase;
float testPosition[3];

void init(){
    timeLeft = 0;
    
    //declaring coordinates of  Base Station
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    
    timetosquare = 2.5f; //how fast the sphere moves between squares (base speed)
    
    testPosition[0] = .2;
    testPosition[1] = .4;
    testPosition[2] = .2;
    
}

void loop(){
	api.getMyZRState(myState);
	for(int i = 0; i < 3; i++) {
	    speed[i] = myState[i] - oldPos[i];
	}
	for(int i = 0; i < 3; i++) {
	    oldPos[i] = myState[i];
	}
	api.setPositionTarget(testPosition);
	//timeLeft < 180 - time2base
	
	
	
	if(timeLeft >= TimeToGoBTB(myState, origin)){
	    DEBUG(("Going back to Base station"));
	    api.setPositionTarget(origin);
	    DEBUG(("Speed: %f, %f, %f", speed[0], speed[1], speed[2]));
	    atBase = if(game.atBaseStation()) 1 ? 0;
	    DEBUG(("%d", atBase));
	}
	
	timeLeft++; //adding one to timer each second
}


float TimeToGoBTB(float myPos[12], float baseStation[3]){
    //getting magnitude between SPHERE and Base Station
    float magnitude = sqrtf(mathSquare(baseStation[1] - myPos[1]) + mathSquare(baseStation[2] - myPos[2]) + mathSquare(baseStation[3] - myPos[3]));
    DEBUG(("Magnitude is %f", magnitude));
    float time2square = 18 * exp(0.57 * magnitude) + 7.3;
    float time2base= magnitude*time2square; //finding the time it'll take to get back to the Base Station
    time2base = 180-time2base;
    DEBUG(("The time to get back to the base is %f",time2base));
    
    return time2base;
    
}