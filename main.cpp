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
       P
*/
//Declare any variables shared between functions here
float sampleVals[3]; //example samples
float nextMineSqr[3];
float plusX;
float plusY;
bool drillUp;
bool goingUp;
bool specialCaseLeft;

void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	sampleVals[0] = 0.3f;
	sampleVals[1] = 0.3f;
	sampleVals[2] = 0.6f;
	nextMineSqr[0] = 0.0f;
    nextMineSqr[1] = 0.0f;
    nextMineSqr[2] = 0.0f;
    plusX = 0;
    plusY = 0;
	drillUp = false;
	goingUp = true;
	specialCaseLeft = true;
}

//function for what to add to X
float addX(float xSampVals[3]){//inputs an array of values & outputs a +x and a +y and a +z as an array nextMineSqr
    //goingUp doesn't affect this function
    //checking the 2nd sample
    if(xSampVals[1] == 0.3f){//if second sample is a .3
        plusX +=1; //go right 1
    }
    if(xSampVals[1] == 0.6f && xSampVals[2] == 0.3f){//if second sample is .6 and third is .3
        //if third is .6 then you don't need to move on the x-axis
        plusX += -1; //go left 1
    }
    //if its 1 then do nothing
    return plusX;
}
//function for what to add to Y
float addY(float ySampVals[3]){
    //goingUp does affect this function
    if(ySampVals[0] == 0.3f && !(ySampVals[1] == 0.1f || ySampVals[2] == 0.1f)) { //if P is on the top row of 0.3 squares
        plusY += (goingUp)? 2 : -2; //move up/down 2
        
        
    }
    if(ySampVals[0] == ySampVals[1] == ySampVals[2] == 0.6f) { //if all samples are 0.6
        plusY += (goingUp)? 1 : -1; //move up/down 1
    }
    
    return plusY;
}
//function for special cases
float specialCase(float specialCaseSamp[3]){
    if(specialCaseSamp[0] == 0.3f && specialCaseSamp[1] == 0.1f && specialCaseSamp[2] == 0.6f){
        specialCaseLeft = true;
    }
    nextMineSqr[0] += (specialCaseLeft)? 2:-2
    game.dropSample(0);
    
}
bool isSpecialCase(float specialSampVals[3]){
    return (specialSampVals[0] == 0.3f && specialSampVals[1] == 0.1f && specialSampVals[2] == 0.6f) or 
    (specialSampVals[0] == 0.3f && specialSampVals[1] == 0.6f && specialSampVals[2] == 0.1f);
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	 //checking to see if we've hit a special case

	if(isSpecialCase(sampleVals)){ //if it is a special case
	    //code in special functions for finding 10 in special cases (also known as U1 & U2)
	    specialCase(sampleVals);
	}
	else{ //if it isn't a special case
	    //add in addX and addY functions and go on to drill the ten
	    addX(sampleVals);
	    addY(sampleVals);
        nextMineSqr[0] = plusX;
        nextMineSqr[1] = plusY;
	    DEBUG(("+X = %f",nextMineSqr[0]));
	    DEBUG(("+Y = %f",nextMineSqr[1]));
	}
}