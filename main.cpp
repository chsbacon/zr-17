/*

STILL INCOMPLETE

Going up:
    P
   /
  v
  Q-->R

Going down:
  Q-->R
  ^
   \
    P
*/
//Declare any variables shared between functions here
float sampleVals[3]; //example samples
float plusXYZ[3];
float plusX;
float plusY;
bool drillUp;
bool specialCase;
bool goingUp;

void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	sampleVals[0] = 0.3f;
	sampleVals[1] = 0.3f;
	sampleVals[2] = 0.6f;
	
	plusXYZ[0] = 0.0f;
    plusXYZ[1] = 0.0f;
    plusXYZ[2] = 0.0f;
    
    plusX = 0;
    plusY = 0;
    
	drillUp = false;
	
	specialCase = false;
	
	goingUp = true;
}

float addX(float xSampVals[3]){//inputs an array of values & outputs a +x and a +y and a +z as an array plusXYZ
    //goingUp doesn't affect this function
    //checking the 2nd sample
    if(xSampVals[1] == 0.3f){//if second sample is a .3
        plusX =1; //go right 1
    }
    if(xSampVals[1] == 0.6f && xSampVals[2] == 0.3f){//if second sample is .6 and third is .3
        //if third is .6 then you don't need to move on the x-axis
        plusX = -1; //go left 1
    }
    //if its 1 then do nothing
    return plusX;
}
float addY(float ySampVals[3]){
    //goingUp does affect this function
    if(ySampVals[0] == 0.3f && !(ySampVals[1] == 0.1f || ySampVals[2] == 0.1f)) { //if P is on the top row of 0.3 squares
        plusY = (goingUp)? 2 : -2; //move up/down 2
        
        
    }
    if(ySampVals[0] == ySampVals[1] == ySampVals[2] == 0.6f) { //if all samples are 0.6
        plusY = (goingUp)? 1 : -1; //move up/down 1
    }
    
    return plusY;
}

bool isSpecialCase(float specialSampVals[3]){
    /*if(specialSampVals[0] == .3 && specialSampVals[1] == .1 && specialSampVals[2] == .6){
        specialCase = true;
    }
    if(specialSampVals[0] == .3 && specialSampVals[1] == .6 && specialSampVals[2] == .1){
        specialCase = true;
    }
    return specialCase;*/
    return (specialSampVals[0] == 0.3f && specialSampVals[1] == 0.1f && specialSampVals[2] == 0.6f) or 
    (specialSampVals[0] == 0.3f && specialSampVals[1] == 0.6f && specialSampVals[2] == 0.1f);
}





void loop(){
	//This function is called once per second.  Use it to control the satellite.
	 //checking to see if we've hit a special case

	if(isSpecialCase(sampleVals)){ //if it is a special case
	    //code in special functions for finding 10 in special cases (also known as U1 & U2)
	    
	}
	else{ //if it isn't a special case
	    //add in addX and addY functions and go on to drill the ten
	    addX(sampleVals);
	    addY(sampleVals);
	    
        plusXYZ[0] = plusX;
        plusXYZ[1] = plusY;
	    
	    DEBUG(("+X = %f",plusXYZ[0]));
	    DEBUG(("+Y = %f",plusXYZ[1]));
	}
}