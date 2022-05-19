/*
 * @(#)helio.h
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

#ifndef _HELIO_H
#define _HELIO_H

// system includes
#include <system.h>
#include "resource/gfx.h"

// application constants
#define GAME_FPS  8                  // 8 fps (adequate for animations)

typedef struct
{
  struct 
  {
    BOOLEAN   gamePlaying;              // is there a game in play?
    BOOLEAN   gamePaused;               // is the game currently paused?
    BOOLEAN   gameWait;                 // are we waiting for user? 
    USHORT    gameAnimationCount;       // a ticking counter
    USHORT    gameScore;                // the score of the player
    USHORT    highScore[2];             // a high score list (score only)
    USHORT    gameLives;                // the number of lives the player has

    struct 
    {
      BYTE    gameType;                 // what type of game are we playing?
      USHORT  gameLevel;                // what level are we at?
      BOOLEAN bonusAvailable;           // is there a bonus available?
      BOOLEAN bonusScoring;             // are we currently in BONUS mode?

      USHORT  bladeWait;                // the delay between blade positions
      USHORT  bladePosition;            // the position of the blade

      USHORT  sharkWait;                // the delay between shark positions
      USHORT  sharkPosition;            // the position of the shark

      USHORT  boatPosition;             // the position of the boat
      USHORT  boatNewPosition;          // the desired position of the boat

      USHORT  parachuteCount[4];        // the number of parachutes on screen
      USHORT  parachuteWait[4][7];      // the delay between parachute moves
      USHORT  parachutePosition[4][7];  // the position of the parachute

      USHORT  parachuteDeathPosition;   // the index of death?
    } parashoot;

  } game;
  
} PreferencesType;

// local includes
#include "device.h"
#include "help.h"
#include "game.h"
#include "animate.h"
#include "resource.h"

// functions
extern void    InitApplication(void);
extern BOOLEAN ApplicationHandleEvent(EvtType *);
extern void    ApplicationDisplayDialog(ObjectID);
extern void    EventLoop(void);
extern void    EndApplication(void);
extern BOOLEAN HelioMain(WORD, void *);

#endif 
