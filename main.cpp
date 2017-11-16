//Declare any variables shared between functions here
float place[3];
float myState[12];
float mainVector[3];

void init(){
	//This function is called once when your code is first loaded.

	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	
	place[0] = 0.3;
	place[1] = -0.48;
	place[2] = -0.16;
	
	
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	api.getMyZRState(myState);
	
	mathVecSubtract(mainVector, place, myState, 3);
	float distance = mathVecMagnitude(mainVector, 3);
	
    mainVector[0] -= 0.1;
    mainVector[1] += 0.1;
    mainVector[2] += 0.1;
	
	if(distance > .4)
	    api.setVelocityTarget(mainVector);

	else
	   api.setPositionTarget(place);
	
}
