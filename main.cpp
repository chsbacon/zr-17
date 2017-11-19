//Declare any variables shared between functions here
float myState[12];
float testPosition[3];
float myVel[3];
float mySpeed;
float betweenVector[3];

void init(){
	//This function is called once when your code is first loaded.

	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	
	testPosition[0] = 0.6f;
	testPosition[1] = 0.2f;
	testPosition[2] = 0;
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	api.getMyZRState(myState);
	
	//finding and debugging speed
	for(int i = 3; i<6; i++)
	    myVel[i-3] = myState[i];
	mySpeed = mathVecMagnitude(myVel, 3);
	DEBUG(("My speed is: %f", mySpeed));
	
	//finding vector between SPHERE and testPosition
	for(int i =0; i<3; i++)
	    betweenVector[i] = testPosition[i] - myState[i];
	    
	mathVecNormalize(betweenVector, 3);//normalizing vector in between SPHERE and testPosition
    scale(betweenVector, 0.04f);//scaling vector
	DEBUG(("Velocity target is: %f,%f,%f",betweenVector[0],betweenVector[1],betweenVector[2]));
	
	
	
	api.setVelocityTarget(betweenVector);//setting velocity to scaled and normalized vector
}

void scale(float* vec, float scale){
    for (int i=0; i<3; i++)
        vec[i] *= scale;
}