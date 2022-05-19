/*
 * @(#)game.c
 *
 * Copyright 1999-2001, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "parashoot" program developed 
 * for the Helio Computing Platform designed by vtech:
 *
 *   http://www.vtechinfo.com/
 *
 * The contents of this file is confidential and  proprietary in nature
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited.
 */

#include "helio.h"

// image resources
#include "game.inc"

// interface
static void GameGetSpritePosition(BYTE, BYTE, SHORT *, SHORT *);
static void GameAdjustLevel(PreferencesType *);
static void GameIncrementScore(PreferencesType *);
static void GameMovePlayer(PreferencesType *);
static void GameMoveParachuters(PreferencesType *);
static void GameRemoveParachuter(PreferencesType *, SHORT, SHORT);

// global variable structure
typedef struct
{
  GfxWindow *winDigits;                     // scoring digits bitmaps
  GfxWindow *winMisses;                     // the lives notification bitmaps

  GfxWindow *winBlades;                     // the blade bitmaps
  BOOLEAN   bladeChanged;                   // do we need to repaint the blade?
  BOOLEAN   bladeOnScreen;                  // is blade bitmap on screen?
  USHORT    bladeOldPosition;               // the *old* position of the blade 

  GfxWindow *winSharks;                     // the shark bitmaps
  BOOLEAN   sharkChanged;                   // do we need to repaint the shark?
  BOOLEAN   sharkOnScreen;                  // is shark bitmap on screen?
  USHORT    sharkOldPosition;               // the *old* position of the shark 

  GfxWindow *winBoats;                      // the boat bitmaps
  BOOLEAN   boatChanged;                    // do we need to repaint boat?
  BOOLEAN   boatOnScreen;                   // is boat bitmap on screen?
  USHORT    boatOldPosition;                // the *old* position of boat 

  GfxWindow *winParachuters;                 // the parachuter bitmaps
  BOOLEAN   parachuteChanged[4][7];         // do we need to repaint parachute
  BOOLEAN   parachuteOnScreen[4][7];        // is parachute bitmap on screen?
  USHORT    parachuteOnScreenPosition[4][7];// the *old* position of parachute 

  GfxWindow *winParachuterDeaths;           // the parachuter death bitmaps
  
  BYTE      gameType;                       // the type of game active
  BOOLEAN   playerDied;                     // has the player died?
  BYTE      moveDelayCount;                 // the delay between moves
  BYTE      moveLast;                       // the last move performed
  BYTE      moveNext;                       // the next desired move  
} GameGlobals;

// globals reference
static GameGlobals *gbls;

/**
 * Initialize the Game.
 */  
void   
GameInitialize()
{
  // create the globals object
  gbls = (GameGlobals *)pmalloc(sizeof(GameGlobals));

  // initialize and load the "bitmap" windows
  {
    SHORT i, j;

    gbls->winDigits                    = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winDigits->width             = bitmap00Width;
    gbls->winDigits->height            = bitmap00Height;
    gbls->winDigits->memSize           = bitmap00Size;
    gbls->winDigits->memory            = (void *)bitmap00;
    gbls->winMisses                    = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winMisses->width             = bitmap01Width;
    gbls->winMisses->height            = bitmap01Height;
    gbls->winMisses->memSize           = bitmap01Size;
    gbls->winMisses->memory            = (void *)bitmap01;
    gbls->winBlades                    = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winBlades->width             = bitmap02Width;
    gbls->winBlades->height            = bitmap02Height;
    gbls->winBlades->memSize           = bitmap02Size;
    gbls->winBlades->memory            = (void *)bitmap02;
    gbls->winSharks                    = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winSharks->width             = bitmap03Width;
    gbls->winSharks->height            = bitmap03Height;
    gbls->winSharks->memSize           = bitmap03Size;
    gbls->winSharks->memory            = (void *)bitmap03;
    gbls->winBoats                     = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winBoats->width              = bitmap04Width;
    gbls->winBoats->height             = bitmap04Height;
    gbls->winBoats->memSize            = bitmap04Size;
    gbls->winBoats->memory             = (void *)bitmap04;
    gbls->winParachuters               = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winParachuters->width        = bitmap05Width;
    gbls->winParachuters->height       = bitmap05Height;
    gbls->winParachuters->memSize      = bitmap05Size;
    gbls->winParachuters->memory       = (void *)bitmap05;
    gbls->winParachuterDeaths          = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winParachuterDeaths->width   = bitmap06Width;
    gbls->winParachuterDeaths->height  = bitmap06Height;
    gbls->winParachuterDeaths->memSize = bitmap06Size;
    gbls->winParachuterDeaths->memory  = (void *)bitmap06;

    gbls->bladeChanged                 = TRUE;
    gbls->bladeOnScreen                = FALSE;
    gbls->bladeOldPosition             = 0;
    
    gbls->sharkChanged                 = TRUE;
    gbls->sharkOnScreen                = FALSE;
    gbls->sharkOldPosition             = 0;

    gbls->boatChanged                  = TRUE;
    gbls->boatOnScreen                 = FALSE;
    gbls->boatOldPosition              = 0;

    for (i=0; i<4; i++) {
      for (j=0; j<7; j++) {
        gbls->parachuteChanged[i][j]          = TRUE;
        gbls->parachuteOnScreen[i][j]         = FALSE;
        gbls->parachuteOnScreenPosition[i][j] = 0;
      }
    }
  }
}

/**
 * Reset the Game.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameReset(PreferencesType *prefs, BYTE gameType)
{
  // turn off all the "bitmaps"
  FormDrawForm(gameForm);

  // turn on all the "bitmaps"
  {
    GfxRegion region    = { {   0,   0 }, {   0,   0 } };
    GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };
    SHORT     i;

    //
    // draw the score
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = 8 * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
    }

    //
    // draw the misses
    //

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0,
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 44;
    scrRegion.extent.y  = 16;
    region.topLeft.x    = 2 * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // draw the miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    //
    // draw the blades
    //

    for (i=0; i<2; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteBlade, 0, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 36;
      scrRegion.extent.y  = 6;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;  
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the blade bitmap!
      GfxCopyRegion(gbls->winBlades, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }
    
    // 
    // draw the sharks
    //

    for (i=0; i<5; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteShark, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 32;
      scrRegion.extent.y  = 16;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the shark bitmap!
      GfxCopyRegion(gbls->winSharks, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    //
    // draw the parachuters
    //

    for (i=0; i<5; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteParachuter, i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 18;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = i * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the parachute bitmap!
      GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    for (i=0; i<6; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteParachuter, 7 + i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 18;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = i * scrRegion.extent.x;
      region.topLeft.y    = 1 * scrRegion.extent.y;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the parachute bitmap!
      GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    for (i=0; i<7; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteParachuter, 14 + i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 18;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = i * scrRegion.extent.x;
      region.topLeft.y    = 2 * scrRegion.extent.y;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the parachute bitmap!
      GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    for (i=0; i<2; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteParachuter, 21 + i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 18;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = i * scrRegion.extent.x;
      region.topLeft.y    = 3 * scrRegion.extent.y;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the parachute bitmap!
      GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    //
    // draw the parachuters deaths :))
    //

    for (i=0; i<6; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteParachuterDeath, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 24;
      scrRegion.extent.y  = 10;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the parachuter bitmap!
      GfxCopyRegion(gbls->winParachuterDeaths, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    // 
    // draw the boat
    //

    for (i=0; i<3; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteBoat, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 32;
      scrRegion.extent.y  = 14;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the boat bitmap
      GfxCopyRegion(gbls->winBoats, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }
  }

  // wait a good two seconds :))
  TmrWaitTime(2000);

  // turn off all the "bitmaps"
  FormDrawForm(gameForm);

  // reset the preferences
  GameResetPreferences(prefs, gameType);}

/**
 * Reset the Game preferences.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameResetPreferences(PreferencesType *prefs, BYTE gameType)
{
  SHORT i, j;

  // now we are playing
  prefs->game.gamePlaying                   = TRUE;
  prefs->game.gamePaused                    = FALSE;
  prefs->game.gameWait                      = TRUE;
  prefs->game.gameAnimationCount            = 0;

  // reset score and lives
  prefs->game.gameScore                     = 0;
  prefs->game.gameLives                     = 3;

  // reset parashoot specific things
  prefs->game.parashoot.gameType            = gameType;
  prefs->game.parashoot.gameLevel           = 1;
  prefs->game.parashoot.bonusAvailable      = TRUE;
  prefs->game.parashoot.bonusScoring        = FALSE;

  prefs->game.parashoot.bladeWait           = 0;
  prefs->game.parashoot.bladePosition       = 0;
  prefs->game.parashoot.sharkWait           = 0;
  prefs->game.parashoot.sharkPosition       = 0;

  prefs->game.parashoot.boatPosition        = 0;
  prefs->game.parashoot.boatNewPosition     = 0;
  for (i=0; i<4; i++) {
    prefs->game.parashoot.parachuteCount[i] = 0;
    memset(prefs->game.parashoot.parachutePosition[i], (UBYTE)0, sizeof(USHORT) * 7);
    memset(prefs->game.parashoot.parachuteWait[i], (UBYTE)0, sizeof(USHORT) * 7);
  }
  prefs->game.parashoot.parachuteDeathPosition = 0;

  // reset the "backup" and "onscreen" flags
  gbls->bladeChanged                        = TRUE;
  gbls->sharkChanged                        = TRUE;
  gbls->boatChanged                         = TRUE;
  for (i=0; i<4; i++) {
    for (j=0; j<7; j++) {
      gbls->parachuteChanged[i][j]          = TRUE;
      gbls->parachuteOnScreen[i][j]         = FALSE;
    }
  }

  gbls->gameType                            = gameType;
  gbls->playerDied                          = FALSE;
  gbls->moveDelayCount                      = 0;
  gbls->moveLast                            = moveNone;
  gbls->moveNext                            = moveNone;
}

/**
 * Process key input from the user.
 * 
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */  
void   
GameProcessKeyInput(PreferencesType *prefs, UWORD keyStatus)
{
  // the helio device does not have a very nice "key" pattern so
  // playing games using the keys may not be an easy task :) the
  // following is coded as a "prototype", maybe someone will use
  // the "key" capabilities. :))
  //
  // the system is hardcoded as follows:
  //
  //   address book | pageUp    = move left
  //   to do list   | pageDown  = move right
  //
  // :P enjoy
  //
  // -- Aaron Ardiri, 2000

#define ctlKeyLeft  (keyBitHard1 | keyBitPageUp)
#define ctlKeyRight (keyBitHard2 | keyBitPageDown)

  keyStatus &= (ctlKeyLeft  |
                ctlKeyRight);

  // did they press at least one of the game keys?
  if (keyStatus != 0) {

    // if they were waiting, we should reset the game animation count
    if (prefs->game.gameWait) { 
      prefs->game.gameAnimationCount = 0;
      prefs->game.gameWait           = FALSE;
    }

    // great! they wanna play
    prefs->game.gamePaused = FALSE;
  }

  // move left
  if (
      ((keyStatus & ctlKeyLeft) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveLeft)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.parashoot.boatPosition > 0) {
      prefs->game.parashoot.boatNewPosition = 
        prefs->game.parashoot.boatPosition - 1;
    }
  }

  // move right
  else
  if (
      ((keyStatus & ctlKeyRight) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveRight)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.parashoot.boatPosition < 2) {
      prefs->game.parashoot.boatNewPosition = 
        prefs->game.parashoot.boatPosition + 1;
    }
  }
}

/**
 * Process stylus input from the user.
 * 
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 */  
void   
GameProcessStylusInput(PreferencesType *prefs, SHORT x, SHORT y)
{
  GfxRegion region;
  SHORT     i;

  // lets take a look at all the possible "positions"
  for (i=0; i<3; i++) {

    // get the bounding box of the position
    GameGetSpritePosition(spriteBoat, i,
                          &region.topLeft.x, &region.topLeft.y);
    region.extent.x  = 32;
    region.extent.y  = 14;

    // did they tap inside this rectangle?
    if (((x >= region.topLeft.x) && (y >= region.topLeft.y) && 
	 (x <= (region.topLeft.x+region.extent.x)) &&
	 (y <= (region.topLeft.y+region.extent.y)))) {

      // ok, this is where we are going to go :)
      prefs->game.parashoot.boatNewPosition = i;

      // if they were waiting, we should reset the game animation count
      if (prefs->game.gameWait) { 
        prefs->game.gameAnimationCount = 0;
        prefs->game.gameWait           = FALSE;
      }

      // great! they wanna play
      prefs->game.gamePaused = FALSE;
      break;                                        // stop looking
    }
  }	
}

/**
 * Process the object movement in the game.
 * 
 * @param prefs the global preference data.
 */  
void   
GameMovement(PreferencesType *prefs)
{
  SHORT     i, j;
  GfxRegion region  = {{   8,  18 }, { 144, 14 }};

  //
  // the game is NOT paused.
  //

  if (!prefs->game.gamePaused) {

    // animate the blade
    if (prefs->game.parashoot.bladeWait == 0) {
    
      prefs->game.parashoot.bladePosition =
        1 - prefs->game.parashoot.bladePosition;
      prefs->game.parashoot.bladeWait     = 4;
  
      gbls->bladeChanged = TRUE;
    }
    else {
      prefs->game.parashoot.bladeWait--;
    }

    // animate the shark
    if (prefs->game.parashoot.sharkWait == 0) {
    
      prefs->game.parashoot.sharkPosition =
        (prefs->game.parashoot.sharkPosition + 1) % 5;
      prefs->game.parashoot.sharkWait     = 4;
  
      gbls->sharkChanged = TRUE;
    }
    else {
      prefs->game.parashoot.sharkWait--;
    }

    // we must make sure the user is ready for playing 
    if (!prefs->game.gameWait) {

      // we cannot be dead yet :)
      gbls->playerDied = FALSE;

      // are we in bonus mode?
      if ((prefs->game.parashoot.bonusScoring) &&
          (prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        BYTE    str[32];
        GfxFont currFont = GfxGetFont();

        strcpy(str, "    * BONUS PLAY *    ");
        GfxSetFont(gfx_palmosBoldFont);
        GfxDrawString(str, strlen(str), 
                      80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
        GfxSetFont(currFont);
      }
      else 
        GfxFillRegion(GfxGetDrawWindow(), &region, gfx_white);

      // player gets first move
      GameMovePlayer(prefs);
      GameMoveParachuters(prefs);

      // is it time to upgrade the game?
      if (prefs->game.gameAnimationCount >= 
           ((gbls->gameType == GAME_A) ? 0x17f : 0x100)) {

        prefs->game.gameAnimationCount = 0;
        prefs->game.parashoot.gameLevel++;

        // upgrading of difficulty?
        if (
            (gbls->gameType                  == GAME_A) &&
            (prefs->game.parashoot.gameLevel > 12)
           ) {

          gbls->gameType                   = GAME_B;
          prefs->game.parashoot.gameLevel -= 2; // give em a break :)
        }
      } 

      // has the player died in this frame?
      if (gbls->playerDied) {

        SHORT     index;
        GfxRegion region    = { {   0,   0 }, {   0,   0 } };
        GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

        // erase the previous shark (wherever it was)
        if (gbls->sharkOnScreen) {

          index = gbls->sharkOldPosition;

          // what is the rectangle we need to copy?
          GameGetSpritePosition(spriteShark, index, 
                                &scrRegion.topLeft.x, &scrRegion.topLeft.y);
          scrRegion.extent.x  = 32;
          scrRegion.extent.y  = 16;
          region.topLeft.x    = index * scrRegion.extent.x; 
          region.topLeft.y    = 0;
          region.extent.x     = scrRegion.extent.x;
          region.extent.y     = scrRegion.extent.y;

          // invert the shark bitmap!
          GfxCopyRegion(gbls->winSharks, GfxGetDrawWindow(),
                        &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
          gbls->sharkOnScreen = FALSE;
        }

        //
        // animate the chasing and eating of the parachuter
        //

        for (i=prefs->game.parashoot.parachuteDeathPosition; i<6; i++) {

          // do twice, first time = draw, second time = erase
          for (j=0; j<2; j++) {

            // draw parachuter in water
            {
              // what is the rectangle we need to copy?
              GameGetSpritePosition(spriteParachuterDeath, i, 
                                    &scrRegion.topLeft.x, &scrRegion.topLeft.y);
              scrRegion.extent.x  = 24;
              scrRegion.extent.y  = 10;
              region.topLeft.x    = i * scrRegion.extent.x; 
              region.topLeft.y    = 0;
              region.extent.x     = scrRegion.extent.x;
              region.extent.y     = scrRegion.extent.y;

              // invert the parachuter bitmap!
              GfxCopyRegion(gbls->winParachuterDeaths, GfxGetDrawWindow(),
                            &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxInvert);
            }

            // draw shark chasing parachuter
            if (i > 0) {

              // what is the rectangle we need to copy?
              GameGetSpritePosition(spriteShark, (i-1), 
                                    &scrRegion.topLeft.x, &scrRegion.topLeft.y);
              scrRegion.extent.x  = 32;
              scrRegion.extent.y  = 16;
              region.topLeft.x    = (i-1) * scrRegion.extent.x; 
              region.topLeft.y    = 0;
              region.extent.x     = scrRegion.extent.x;
              region.extent.y     = scrRegion.extent.y;

              // invert the shark bitmap!
              GfxCopyRegion(gbls->winSharks, GfxGetDrawWindow(),
                            &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxInvert);
            }
            if (j == 1) continue; // we only want sound in first pass

            // play death sound
            SndPlaySndEffect(SNDRES2_BEEP);
            TmrWaitTime(500);
          }
        }

        // lose a life :(
        prefs->game.gameLives--;

        // no more lives left: GAME OVER!
        if (prefs->game.gameLives == 0) {

          // return to main screen
          EvtAppendEvt(EVT_INLAY_SELECT,0,INLAY_EXIT,0,NULL);

          prefs->game.gamePlaying = FALSE;
        }

        // reset player position and continue game
        else {
          GameAdjustLevel(prefs);
          prefs->game.parashoot.bonusScoring = FALSE;
          prefs->game.gameWait               = TRUE;
        }
      }
    }

    // we have to display "GET READY!"
    else {

      // flash on:
      if ((prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        BYTE    str[32];
        GfxFont currFont = GfxGetFont();

        strcpy(str, "    * GET READY *    ");
        GfxSetFont(gfx_palmosBoldFont);
        GfxDrawString(str, strlen(str), 
                      80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
        GfxSetFont(currFont);
      }

      // flash off:
      else
        GfxFillRegion(GfxGetDrawWindow(), &region, gfx_white);
    }

    // update the animation counter
    prefs->game.gameAnimationCount++;
  }

  //
  // the game is paused.
  //

  else {

    BYTE    str[32];
    GfxFont currFont = GfxGetFont();

    strcpy(str, "    *  PAUSED  *    ");
    GfxSetFont(gfx_palmosBoldFont);
    GfxDrawString(str, strlen(str), 
                  80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
    GfxSetFont(currFont);
  }
}

/**
 * Draw the game on the screen.
 * 
 * @param prefs the global preference data.
 */
void   
GameDraw(PreferencesType *prefs)
{
  SHORT     i, j, index;
  GfxRegion region    = { {   0,   0 }, {   0,   0 } };
  GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

  // 
  // DRAW INFORMATION/BITMAPS ON SCREEN
  //

  // draw the score
  {
    short base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = index * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
      base /= 10;
    }
  }

  // draw the misses that have occurred :( 
  if (prefs->game.gameLives < 3) {
  
    index = 2 - prefs->game.gameLives;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 44;
    scrRegion.extent.y  = 16;
    region.topLeft.x    = index * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // draw the miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
  }

  // no missed, make sure none are shown
  else {
  
    index = 2;  // the miss with *all* three misses

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 44;
    scrRegion.extent.y  = 16;
    region.topLeft.x    = index * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // invert the three miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
  }

  // draw the blade on the screen (only if it has changed)
  if (gbls->bladeChanged) {

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteBlade, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 36;
    scrRegion.extent.y  = 6;
    region.topLeft.x    = 0;  
    region.topLeft.y    = 0;  
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // 
    // erase the previous blade 
    // 

    if (gbls->bladeOnScreen) {

      index = gbls->bladeOldPosition;

      // what is the rectangle we need to copy?
      region.topLeft.x = index * scrRegion.extent.x; 
      region.topLeft.y = 0;
      region.extent.x  = scrRegion.extent.x;
      region.extent.y  = scrRegion.extent.y;

      // invert the blade bitmap!
      GfxCopyRegion(gbls->winBlades, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      gbls->bladeOnScreen = FALSE;
    }

    // 
    // draw the blade at the new position
    // 

    index = prefs->game.parashoot.bladePosition;

    // what is the rectangle we need to copy?
    region.topLeft.x = index * scrRegion.extent.x; 
    region.topLeft.y = 0;
    region.extent.x  = scrRegion.extent.x;
    region.extent.y  = scrRegion.extent.y;

    // save this location, record blade is onscreen
    gbls->bladeOnScreen    = TRUE;
    gbls->bladeOldPosition = index;

    // draw the blade bitmap!
    GfxCopyRegion(gbls->winBlades, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    // dont draw until we need to
    gbls->bladeChanged = FALSE;
  }

  // draw the shark on the screen (only if it has changed)
  if (gbls->sharkChanged) {

    // 
    // erase the previous shark 
    // 

    if (gbls->sharkOnScreen) {

      index = gbls->sharkOldPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteShark, index, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 32;
      scrRegion.extent.y  = 16;
      region.topLeft.x    = index * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // invert the shark bitmap!
      GfxCopyRegion(gbls->winSharks, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      gbls->sharkOnScreen = FALSE;
    }

    // 
    // draw the shark at the new position
    // 

    index = prefs->game.parashoot.sharkPosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteShark, index, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 32;
    scrRegion.extent.y  = 16;
    region.topLeft.x    = index * scrRegion.extent.x; 
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // save this location, record shark is onscreen
    gbls->sharkOnScreen    = TRUE;
    gbls->sharkOldPosition = index;

    // draw the shark bitmap!
    GfxCopyRegion(gbls->winSharks, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    // dont draw until we need to
    gbls->sharkChanged = FALSE;
  }

  // draw the parachutes
  for (i=0; i<4; i++) {

    // process each row individually
    for (j=0; j<prefs->game.parashoot.parachuteCount[i]; j++) {

      // draw the parachute on the screen (only if it has changed)
      if (gbls->parachuteChanged[i][j]) {

        //
        // erase the previous parachuter
        //
 
        if (gbls->parachuteOnScreen[i][j]) {

          index = (i * 7) +  // take into account 'layers'
                  gbls->parachuteOnScreenPosition[i][j];

          // what is the rectangle we need to copy?
          GameGetSpritePosition(spriteParachuter, index,
                                &scrRegion.topLeft.x, &scrRegion.topLeft.y);
          scrRegion.extent.x  = 18;
          scrRegion.extent.y  = 20;
          region.topLeft.x    = (index % 7) * scrRegion.extent.x;
          region.topLeft.y    = i * scrRegion.extent.y;
          region.extent.x     = scrRegion.extent.x;
          region.extent.y     = scrRegion.extent.y;

          // invert the old parachute bitmap!
          GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                        &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
        }

        //
        // draw the parachuter at the new position
        //

        index = (i * 7) +  // take into account 'layers'
                prefs->game.parashoot.parachutePosition[i][j];

        // what is the rectangle we need to copy?
        GameGetSpritePosition(spriteParachuter, index,
                              &scrRegion.topLeft.x, &scrRegion.topLeft.y);
        scrRegion.extent.x  = 18;
        scrRegion.extent.y  = 20;
        region.topLeft.x    = (index % 7) * scrRegion.extent.x;
        region.topLeft.y    = i * scrRegion.extent.y;
        region.extent.x     = scrRegion.extent.x;
        region.extent.y     = scrRegion.extent.y;

        // save this location, record parachuter is onscreen
        gbls->parachuteOnScreen[i][j]         = TRUE;
        gbls->parachuteOnScreenPosition[i][j] = prefs->game.parashoot.parachutePosition[i][j];

        // draw the parachute bitmap!
        GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                      &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

        // dont draw until we need to
        gbls->parachuteChanged[i][j] = FALSE;
      }
    }
  }

  // draw boat (only if it has changed)
  if (gbls->boatChanged) {

    // 
    // erase the previous boat
    // 

    if (gbls->boatOnScreen) {

      index = gbls->boatOldPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteBoat, index, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 32;
      scrRegion.extent.y  = 14;
      region.topLeft.x    = index * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // invert the old boat bitmap
      GfxCopyRegion(gbls->winBoats, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      gbls->boatOnScreen  = FALSE;
    }

    // 
    // draw boat at the new position
    // 

    index = prefs->game.parashoot.boatPosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteBoat, index, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 32;
    scrRegion.extent.y  = 14;
    region.topLeft.x    = index * scrRegion.extent.x; 
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // save this location, record boat is onscreen
    gbls->boatOnScreen    = TRUE;
    gbls->boatOldPosition = index;

    // draw the boat bitmap!
    GfxCopyRegion(gbls->winBoats, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    // dont draw until we need to
    gbls->boatChanged = FALSE;
  }
}

/**
 * Terminate the game.
 */
void   
GameTerminate()
{
  // clean up windows/memory
  pfree(gbls->winDigits);
  pfree(gbls->winMisses);
  pfree(gbls->winBlades);
  pfree(gbls->winSharks);
  pfree(gbls->winBoats);
  pfree(gbls->winParachuters);
  pfree(gbls->winParachuterDeaths);
  pfree(gbls);
}

/**
 * Get the position of a particular sprite on the screen.
 *
 * @param spriteType the type of sprite.
 * @param index the index required in the sprite position list.
 * @param x the x co-ordinate of the position
 * @param y the y co-ordinate of the position
 */
static void
GameGetSpritePosition(BYTE  spriteType, 
                      BYTE  index, 
                      SHORT *x, 
                      SHORT *y)
{
  switch (spriteType) 
  {
    case spriteDigit: 
         {
           *x = 6 + (index * 9);
           *y = 38;
         }
         break;

    case spriteMiss: 
         {
           *x = 109;
           *y = 102;
         }
         break;

    case spriteBlade: 
         {
           *x = 119;
           *y = 38;
         }
         break;

    case spriteShark: 
         {
           SHORT positions[][2] = {
                                   {  78, 116 },
                                   {  46, 116 },
                                   {  31, 128 },
                                   {  60, 128 },
                                   {  88, 120 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteBoat: 
         {
           SHORT positions[][2] = {
                                   {  15, 98 },
                                   {  47, 98 },
                                   {  77, 98 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteParachuter: 
         {
           SHORT positions[][2] = {
                                   { 124, 55 }, // 1st wave of falling
                                   { 112, 54 },
                                   {  96, 63 },
                                   {  91, 76 },
                                   {  89, 91 },
                                   {   0,  0 },
                                   {   0,  0 },
                                   { 111, 49 }, // 2nd wave of falling
                                   {  98, 51 },
                                   {  85, 51 },
                                   {  71, 63 },
                                   {  63, 75 },
                                   {  57, 90 },
                                   {   0,  0 },
                                   { 106, 40 }, // 3rd wave of falling
                                   {  88, 44 },
                                   {  74, 45 },
                                   {  60, 50 },
                                   {  45, 63 },
                                   {  34, 74 },
                                   {  26, 90 },
                                   { 124, 71 }, // hanging in space
                                   { 109, 71 },
                                   {   0,  0 },
                                   {   0,  0 },
                                   {   0,  0 },
                                   {   0,  0 },
                                   {   0,  0 }
                                  };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteParachuterDeath: 
         {
           SHORT positions[][2] = {
                                   {  86, 114 },
                                   {  55, 113 },
                                   {  23, 113 },
                                   {  43, 127 },
                                   {  71, 127 },
                                   { 118, 123 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    default:
         break;
  }
}

/**
 * Adjust the level (reset positions)
 *
 * @param prefs the global preference data.
 */
static void 
GameAdjustLevel(PreferencesType *prefs)
{
  // player should stay were the were
  prefs->game.parashoot.boatNewPosition = prefs->game.parashoot.boatPosition;
  gbls->boatChanged                     = TRUE;

  // player is not dead
  gbls->playerDied                      = FALSE;
}

/**
 * Increment the players score. 
 *
 * @param prefs the global preference data.
 */
static void 
GameIncrementScore(PreferencesType *prefs)
{
  SHORT      i, index;
  GfxRegion  region     = { {   0,   0 }, {   0,   0 } };
  GfxRegion  scrRegion  = { {   0,   0 }, {   0,   0 } };

  // adjust accordingly
  prefs->game.gameScore += prefs->game.parashoot.bonusScoring ? 2 : 1;

  // redraw score bitmap
  {
    SHORT base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = index * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
      base /= 10;
    }
  }

  // play the sound
  SndPlaySndEffect(SNDRES5_BEEP);
  TmrWaitTime(125);

  // is it time for a bonus?
  if (
      (prefs->game.gameScore >= 300) &&
      (prefs->game.parashoot.bonusAvailable)
     ) {

    // little fan-fare :)) - the "veewoo" sound was the best i could find :((
    SndPlaySndEffect(SNDRES_VEEWOO);
    TmrWaitTime(500);

    // apply the bonus!
    if (prefs->game.gameLives == 3) 
      prefs->game.parashoot.bonusScoring = TRUE;
    else
      prefs->game.gameLives = 3;

    prefs->game.parashoot.bonusAvailable = FALSE;
  }
}

/**
 * Move the player.
 *
 * @param prefs the global preference data.
 */
static void
GameMovePlayer(PreferencesType *prefs) 
{
  //
  // where does boat want to go today?
  //

  // current position differs from new position?
  if (prefs->game.parashoot.boatPosition != 
      prefs->game.parashoot.boatNewPosition) {

    // need to move left
    if (prefs->game.parashoot.boatPosition > 
        prefs->game.parashoot.boatNewPosition) {

      gbls->moveNext = moveLeft;
    }

    // need to move right
    else
    if (prefs->game.parashoot.boatPosition < 
        prefs->game.parashoot.boatNewPosition) {

      gbls->moveNext = moveRight;
    }
  }

  // lets make sure they are allowed to do the move
  if (
      (gbls->moveDelayCount == 0) || 
      (gbls->moveLast != gbls->moveNext) 
     ) {
    gbls->moveDelayCount = 
     ((gbls->gameType == GAME_A) ? 4 : 3);
  }
  else {
    gbls->moveDelayCount--;
    gbls->moveNext = moveNone;
  }

  // which direction do they wish to move?
  switch (gbls->moveNext)
  {
    case moveLeft:
         {
           prefs->game.parashoot.boatPosition--;
           gbls->boatChanged = TRUE;
         }
         break;

    case moveRight:
         {
           prefs->game.parashoot.boatPosition++;
           gbls->boatChanged = TRUE;
         }
         break;

    default:
         break;
  }

  gbls->moveLast = gbls->moveNext;
  gbls->moveNext = moveNone;

  // do we need to play a movement sound? 
  if (gbls->boatChanged)  
    SndPlaySndEffect(SNDRES0_BEEP);
}

/**
 * Move the parachuters.
 *
 * @param prefs the global preference data.
 */
static void
GameMoveParachuters(PreferencesType *prefs) 
{
  // only do this if the player is still alive
  if (!gbls->playerDied) {

    SHORT i, j, k, pos;

    // process the parachuters!
    for (i=0; i<3; i++) {

      // each row individually
      j = 0;
      while (j<prefs->game.parashoot.parachuteCount[i]) {

        BOOLEAN removal = FALSE;

        if (prefs->game.parashoot.parachuteWait[i][j] == 0) {

          BYTE hangFactor = (gbls->gameType == GAME_A) ? 8 : 4;

          // can the parachuter get stuck?
          if (
              (i == 0) && 
              (prefs->game.parashoot.parachutePosition[i][j] == 1) &&
              (prefs->game.parashoot.parachuteCount[3] == 0)       &&
              ((DeviceRandom(0) % hangFactor) == 0)
             ) {

            // add the 'hanging' parachuter
            prefs->game.parashoot.parachuteCount[3]       = 1;
            prefs->game.parashoot.parachutePosition[3][0] = 0;
            prefs->game.parashoot.parachuteWait[3][0]     =
              (gbls->gameType == GAME_A) ? 6 : 5;
            gbls->parachuteChanged[3][0]          = TRUE;
            gbls->parachuteOnScreen[3][0]         = FALSE;
            gbls->parachuteOnScreenPosition[3][0] = 0;

            // remove the parachuter
            GameRemoveParachuter(prefs, i, j); removal = TRUE;
          }
 
          // normal progression, down to the water
          else {
          
            BOOLEAN ok;

            // lets make sure it is not moving into a parachuter in front of us?
            ok = TRUE;
            for (k=0; k<prefs->game.parashoot.parachuteCount[i]; k++) {

              ok &= (
                     (prefs->game.parashoot.parachutePosition[i][j]+1 !=
                      prefs->game.parashoot.parachutePosition[i][k])
                    );
            }

            // the coast is clear, move!
            if (ok) {

              prefs->game.parashoot.parachutePosition[i][j]++;
              prefs->game.parashoot.parachuteWait[i][j] =
                (gbls->gameType == GAME_A) ? 6 : 4;
              gbls->parachuteChanged[i][j] = TRUE;
 
              // has the parachuter fallen into the water?
              if (prefs->game.parashoot.parachutePosition[i][j] == 5+i) {

                gbls->playerDied |= TRUE;
                prefs->game.parashoot.parachuteDeathPosition = i;

                // remove the parachuter
                GameRemoveParachuter(prefs, i, j); removal = TRUE;
              }

              // play a movement sound
              SndPlaySndEffect(SNDRES1_BEEP);
            }
          }
        }
        else {
          prefs->game.parashoot.parachuteWait[i][j]--;

          pos = 2 - prefs->game.parashoot.boatPosition;

          // lets check if the parachuter is being saved by the boat?
          if (i == pos) {

            if (prefs->game.parashoot.parachutePosition[i][j] == (4 + pos)) {

              // increase score
              GameIncrementScore(prefs);

              // we need to remove the parachuter
              GameRemoveParachuter(prefs, i, j); removal = TRUE;
            }
          }
        }

        if (!removal) j++;
      }
    }

    // process the hanging parachuter
    if (prefs->game.parashoot.parachuteCount[3] != 0) {

      if (prefs->game.parashoot.parachuteWait[3][0] == 0) {

        BYTE    freeFactor = (gbls->gameType == GAME_A) ? 4 : 3;
        BOOLEAN ok;

        // primary condition = theres space, in right pos and RANDOM :P
        ok = (
              (prefs->game.parashoot.parachuteCount[0] < 5)        &&
              (prefs->game.parashoot.parachutePosition[3][0] == 1) &&
              ((DeviceRandom(0) % freeFactor) == 0)
             ); 

        // lets make sure no 'parachuter' is in the way of exiting?
        for (i=0; i<prefs->game.parashoot.parachuteCount[0]; i++) {
          ok &= (
                 (prefs->game.parashoot.parachutePosition[0][i] != 2) &&
                 (prefs->game.parashoot.parachutePosition[0][i] != 3)
                ); 
        }

        // can the parachuter get free?
        if (ok) {

          // we need to remove the hanging parachuter
          GameRemoveParachuter(prefs, 3, 0);

          // add a new parachuter to the first falling fleet
          pos = prefs->game.parashoot.parachuteCount[0]++;

          prefs->game.parashoot.parachutePosition[0][pos] = 3;
          prefs->game.parashoot.parachuteWait[0][pos]     = 
            (gbls->gameType == GAME_A) ? 6 : 4;
          gbls->parachuteChanged[0][pos]          = TRUE;
          gbls->parachuteOnScreen[0][pos]         = FALSE;
          gbls->parachuteOnScreenPosition[0][pos] = 0;

          // play a movement sound
          SndPlaySndEffect(SNDRES1_BEEP);
        }

        // keep swinging
        else {
          prefs->game.parashoot.parachutePosition[3][0] = 
            (prefs->game.parashoot.parachutePosition[3][0] + 1) % 2;
          prefs->game.parashoot.parachuteWait[3][0]     =
            (gbls->gameType == GAME_A) ? 6 : 5;
          gbls->parachuteChanged[3][0] = TRUE;
        }
      }
      else {
        prefs->game.parashoot.parachuteWait[3][0]--;
      }
    }

    // new parachuter appearing on screen?
    {
      BOOLEAN ok;
      BYTE    birthFactor            = (gbls->gameType == GAME_A) ? 8 : 4;
      BYTE    maxOnScreenParachuters = prefs->game.parashoot.gameLevel;
      BYTE    totalOnScreen, new;

      totalOnScreen = (prefs->game.parashoot.parachuteCount[0] + 
                       prefs->game.parashoot.parachuteCount[1] + 
                       prefs->game.parashoot.parachuteCount[2]);
      new = DeviceRandom(0) % 3;

      // we must be able to add a parachuter (based on level)
      ok = (
            (totalOnScreen < maxOnScreenParachuters) &&
            (prefs->game.parashoot.parachuteCount[new] < (5+new)) &&
            ((DeviceRandom(0) % birthFactor) == 0)
           );

      // lets check that there is no parachuter at index = 0;
      for (i=0; i<prefs->game.parashoot.parachuteCount[new]; i++) {
        ok &= (prefs->game.parashoot.parachutePosition[new][i] != 0);
      }

      // lets add a new parachuter
      if (ok) {
        pos = prefs->game.parashoot.parachuteCount[new]++;
        prefs->game.parashoot.parachutePosition[new][pos] = 0;
        prefs->game.parashoot.parachuteWait[new][pos]     = 
          (gbls->gameType == GAME_A) ? 6 : 4;
        gbls->parachuteChanged[new][pos]          = TRUE;
        gbls->parachuteOnScreen[new][pos]         = FALSE;
        gbls->parachuteOnScreenPosition[new][pos] = 0;

        // play a movement sound
        SndPlaySndEffect(SNDRES1_BEEP);
      }
    }
  }
}

/**
 * Remove a parachuter from the game.
 *
 * @param prefs the global preference data.
 * @param rowIndex the index of the parachuter path.
 * @param parachuteIndex the index of the parachuter to remove.
 */
static void 
GameRemoveParachuter(PreferencesType *prefs, 
                     SHORT           rowIndex, 
                     SHORT           parachuteIndex)                  
{
  SHORT     index;
  GfxRegion region    = { {   0,   0 }, {   0,   0 } };
  GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

  // 
  // remove the bitmap from the screen
  //
 
  if (gbls->parachuteOnScreen[rowIndex][parachuteIndex]) {

    index = (rowIndex * 7) +  // take into account 'layers'
            gbls->parachuteOnScreenPosition[rowIndex][parachuteIndex];

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteParachuter, index,
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 18;
    scrRegion.extent.y  = 20;
    region.topLeft.x    = (index % 7) * scrRegion.extent.x;
    region.topLeft.y    = rowIndex * scrRegion.extent.y;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // invert the old parachute bitmap!
    GfxCopyRegion(gbls->winParachuters, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
  }

  //
  // update the information arrays
  //

  // we will push the 'parachute' out of the array
  //
  // before: 1234567---  after: 1345672---
  //          ^     |                 |
  //                end point         end point

  prefs->game.parashoot.parachuteCount[rowIndex]--;

  // removal NOT from end?
  if (prefs->game.parashoot.parachuteCount[rowIndex] > parachuteIndex) {

    SHORT i, count;

    count = prefs->game.parashoot.parachuteCount[rowIndex] - parachuteIndex;

    // shift all elements down
    for (i=parachuteIndex; i<(parachuteIndex+count); i++) {
      prefs->game.parashoot.parachutePosition[rowIndex][i] = prefs->game.parashoot.parachutePosition[rowIndex][i+1];
      prefs->game.parashoot.parachuteWait[rowIndex][i]     = prefs->game.parashoot.parachuteWait[rowIndex][i+1];
      gbls->parachuteChanged[rowIndex][i]                  = gbls->parachuteChanged[rowIndex][i+1];
      gbls->parachuteOnScreen[rowIndex][i]                 = gbls->parachuteOnScreen[rowIndex][i+1];
      gbls->parachuteOnScreenPosition[rowIndex][i]         = gbls->parachuteOnScreenPosition[rowIndex][i+1];
    }
  }
}
