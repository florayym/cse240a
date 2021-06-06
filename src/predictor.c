//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// Gshare: global history based on index sharing
uint32_t ghr;      // GHR: Global history register
uint8_t tag;       // Index for Direction Predictor (DIRP)
uint8_t *bht;      // BHT: index=tag, val=counter (SN, WN, WT, ST)
uint8_t dirpBits;  // Number of bits used for DIRP，default 2-bit


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  ghr = 0; // initialize as NT
  bht = (uint8_t *)calloc(1 << ghistoryBits, sizeof(uint8_t));
  dirpBits = 2;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare(pc);
    case TOURNAMENT:
      return tournament(pc);
    case CUSTOM:
      return custom(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  bht[tag] += outcome ? (bht[tag] == (1 << dirpBits) - 1 ? 0 : 1) : (bht[tag] == 0 ? 0 : -1);
  ghr = (ghr << 1 | outcome) & ((1 << ghistoryBits) - 1);
}

uint8_t gshare(uint32_t pc) {
  tag = (pc ^ ghr) & ((1 << ghistoryBits) - 1);
  return bht[tag] >> (dirpBits - 1) ? TAKEN : NOTTAKEN;
}

uint8_t tournament(uint32_t pc) {
  return NOTTAKEN;
}

uint8_t custom(uint32_t pc) {
  return NOTTAKEN;
}

void wrap_up_predictor() {
  free(bht);
}