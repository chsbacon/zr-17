
float fueltosquare;
float origin[3];
float myState[12];
float testPosition[3];
float myFuel;

void init(){
    
    //declaring coordinates of  Base Station
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    
    fueltosquare = 5; //how fast the sphere moves between squares (base speed)
    
    testPosition[0] = .6;
    testPosition[1] = .4;
    testPosition[2] = .3;
}

void loop(){
	api.getMyZRState(myState);
	api.setPositionTarget(testPosition);
	myFuel = game.getFuelRemaining();
	//timeLeft < 180 - time2base
	
	
	if(myFuel <= 100 - FuelToGoBTB(myState, fueltosquare, origin)){
	    DEBUG(("Going back to Base station"));
	    api.setPositionTarget(origin);
	}
	
}


float FuelToGoBTB(float myPos[12], float fuel2square, float baseStation[3]){
    //getting magnitude between SPHERE and Base Station
    float magnitude = sqrtf(mathSquare(baseStation[1] - myPos[1]) + mathSquare(baseStation[2] - myPos[2]) + mathSquare(baseStation[3] - myPos[3]));
    DEBUG(("Magnitude is %f", magnitude));
    fuel2square = (fuel2square/2)*(magnitude*10);
    float fuel2base= magnitude*fuel2square; //finding the time it'll take to get back to the Base Station
    fuel2base = 180-fuel2base;
    DEBUG(("The time to get back to the base is %f",fuel2base));
    
    return fuel2base;
    
}