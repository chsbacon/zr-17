// WONT COMPILE ON ITS OWN : Just the search algorithm changes to be inserted in the actual code
game.pos2square(myPos,mySquare);
    
    // @ USEFUL VEC IS NOW THE 3D POSITION OF OUR SQUARE @
    game.square2pos(mySquare, usefulVec);
    
    bool geyserOnMe = game.isGeyserHere(mySquare);
    
    // must be larger than all distances we check
    float minDist = 100;

                  
    // selects the best square to drill
    if (newLoc and !game.checkSample() and not drilling) {
        for (int i = -6; i<6; i++){ // checks all clumps of 4 squares (blocks)
        // and sees which is both closest to us and in the center.
            for (int j = -8; j<8; j++){
                if (i*j !=0 and (i < -2 or i > 2 or j < -2 or j > 2)) {
                    usefulIntVec[0] = i;
                    usefulIntVec[1] = j;
                    if (game.getTerrainHeight(usefulIntVec) < 0.44f && game.getDrills(usefulIntVec) < 1) {
                        float sqrPos [3];
                        game.square2pos(usefulIntVec, sqrPos);
                        sqrPos[2] = game.getTerrainHeight(usefulIntVec);
                        if (dist(usefulVec, sqrPos) < minDist && dist(enPos, sqrPos) > 0.35f) {
                            minDist = dist(usefulVec, sqrPos);
                            memcpy(siteCoords, usefulIntVec, 8);
                        }
                    }
                    
                }
            }
        }
        // once we have selected a new drill square, we set this flag so that
        // we don't immediately pick a different one
        newLoc = false;
    }