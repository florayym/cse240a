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

// Gshare: global history based on index sharing
uint32_t tag;               // Index for Direction Predictor (DIRP)

uint32_t ghr;              // GHR: Global history register
uint8_t *bht;              // BHT: index=tag, val=counter (SN, WN, WT, ST)
uint8_t counterBits;       // Number of bits used for counters, default 2-bit

// Tournament
uint8_t *choicePredictors; // choose which predictor (l/g) to use
uint8_t *lPredictors;
uint32_t *lhistoryTable;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  int gentries = 1 << ghistoryBits;
  ghr = 0; // Initialize to NT
  counterBits = 2; // Default
  bht = (uint8_t *)malloc(sizeof(uint8_t) * gentries);
  memset(bht, 1, gentries); // Weakly not taken (WN)
  switch (bpType) {
    case GSHARE: {
      tag = 0;
      break;
    }
    case TOURNAMENT: {
      choicePredictors = (uint8_t *)malloc(sizeof(uint8_t) * gentries);
      memset(choicePredictors, 2, gentries); // Initialize to weekly select global predictor
      
      lPredictors = (uint8_t *)malloc(sizeof(uint8_t) * (1 << lhistoryBits));
      memset(lPredictors, 1, 1 << lhistoryBits); // Weakly not taken (WN)
      
      lhistoryTable = (uint32_t *)calloc(1 << pcIndexBits, sizeof(uint32_t));
      break;
    }
    case CUSTOM: {
      break;
    }
    default:
      break;
  }
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
  uint32_t idx_g = ghr;
  uint8_t MAX = (1 << counterBits) - 1;
  ghr = (ghr << 1 | outcome) & ((1 << ghistoryBits) - 1);

  switch (bpType) {
  case GSHARE: {
    if (outcome) {
      bht[tag] += bht[tag] == MAX ? 0 : 1;
    } else {
      bht[tag] -= bht[tag] == 0 ? 0 : 1;
    }
    break;
  }
  case TOURNAMENT: {
    uint32_t idx_lhr = pc & ((1 << pcIndexBits) - 1);
    uint32_t idx_l = lhistoryTable[idx_lhr];

    uint8_t gCorrect = bht[idx_g] >> (counterBits - 1) == outcome;
    uint8_t lCorrect = lPredictors[idx_l] >> (counterBits - 1) == outcome;

    int action = gCorrect - lCorrect;
    if (action > 0) {
      choicePredictors[idx_g] += choicePredictors[idx_g] == MAX ? 0 : 1;
    } else if (action < 0) {
      choicePredictors[idx_g] -= choicePredictors[idx_g] == 0 ? 0 : 1;
    }

    if (outcome) {
      bht[idx_g] += bht[idx_g] == MAX ? 0 : 1;
      lPredictors[idx_l] += lPredictors[idx_l] == MAX ? 0 : 1;
    } else {
      bht[idx_g] -= bht[idx_g] == 0 ? 0 : 1;
      lPredictors[idx_l] -= lPredictors[idx_l] == 0 ? 0 : 1;
    }

    lhistoryTable[idx_lhr] = (lhistoryTable[idx_lhr] << 1 | outcome) & ((1 << lhistoryBits) - 1);
    break;
  }
  default:
    break;
  }
}

uint8_t
gshare(uint32_t pc) {
  tag = (pc ^ ghr) & ((1 << ghistoryBits) - 1);
  return bht[tag] >> (counterBits - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
tournament(uint32_t pc) {
  uint8_t predict = NOTTAKEN;
  uint8_t selector = choicePredictors[ghr] >> (counterBits - 1);
  if (selector) {
    predict = bht[ghr];
  } else {
    predict = lPredictors[lhistoryTable[pc & ((1 << pcIndexBits) - 1)]];
  }
  return predict >> (counterBits - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
custom(uint32_t pc) {
  return NOTTAKEN;
}

void
wrap_up_predictor() {
  free(bht);
  free(choicePredictors);
  free(lPredictors);
  free(lhistoryTable);
}