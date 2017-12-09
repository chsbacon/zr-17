//Declare any variables shared between functions here

float otherstate[12];
float mystate[12];
float vec[3];
float aux;
int counter;
float distance;
float vecbet[3];
float vec2[3];
float vec3[3];
float vec4[3];
void init(){
    counter=0;
	
}

void loop(){
    
    
    api.getMyZRState(mystate);
    api.getOtherZRState(otherstate);
    
    for(int i = 0; i < 3; i++) {
        vec2[i] = mystate[i];
        vec3[i] = otherstate[i];
    }
    
    if(api.getTime() < 1) {
        
        vec3[2] = 0;
        mathVecSubtract(vecbet,vec3,vec2,3);
        distance = mathVecMagnitude(vecbet,3);
        
        for(int i=0;i<3;i++)
            vecbet[i]=vecbet[i]*1000;
        api.setForces(vecbet);
        
    } else if(api.getTime() < 2) {
        
        vec3[2] = 0;
        mathVecSubtract(vecbet,vec3,vec2,3);
        distance = mathVecMagnitude(vecbet,3);
        
        for(int i=0;i<3;i++)
            vecbet[i]=vecbet[i]*1000;
        
        for(int i = 0; i < 3; i++) 
            vecbet[i] *= -1;
        api.setForces(vecbet);
        
    } else {
        
        vec4[0] = vec2[0] - vec3[0];
        vec4[1] = vec2[1] - vec3[1];
        
        vec3[0] -= vec4[0];
        vec3[1] -= vec4[1];
        
        vec3[2] = 0.4;
        api.setPositionTarget(vec3);
        
    }
}

