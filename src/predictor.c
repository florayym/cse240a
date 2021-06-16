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

uint32_t gmask;
uint32_t gsize;

// Gshare: global history based on index sharing
uint32_t ghr;              // GHR: Global history register
uint8_t *pht;              // BHT: key=index, val=counter (SN, WN, WT, ST)

// Tournament
uint8_t *choicePredictors; // choose which predictor (l/g) to use
uint8_t *lPredictors;
uint32_t *lhistoryTable;
uint32_t gpred_cnt;

// Custom: Agree Predictor
// uint8_t *btb;

// Custom: Bi-mode Predictor
// ghr
// choicePredictors
uint8_t *dirpn;
uint8_t *dirpt;

//-------------------------------------//
//    Predictor Function Declaration   //
//-------------------------------------//

void gshare_init();
void tournament_init();
void custom_init();

uint8_t gshare_pred();
uint8_t tournament_pred();
uint8_t custom_pred();

void gshare_train(uint32_t pc, uint8_t outcome);
void tournament_train(uint32_t pc, uint8_t outcome);
void custom_train(uint32_t pc, uint8_t outcome);

uint8_t updatePredictor(uint8_t outcome, uint8_t predict);

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  gsize = 1 << ghistoryBits;
  gmask = gsize - 1;

  switch (bpType) {
    case GSHARE:
      return gshare_init();
    case TOURNAMENT:
      return tournament_init();
    case CUSTOM:
      return custom_init();
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
      return gshare_pred(pc);
    case TOURNAMENT:
      return tournament_pred(pc);
    case CUSTOM:
      return custom_pred(pc);
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
  switch (bpType) {
    case GSHARE:
      return gshare_train(pc, outcome);
    case TOURNAMENT:
      return tournament_train(pc, outcome);
    case CUSTOM:
      return custom_train(pc, outcome);
    default:
      break;
  }
}

void
gshare_init() {
  ghr = 0; // Initialize to NT

  pht = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(pht, 1, gsize); // Weakly not taken (WN)
}

void
tournament_init() {
  uint32_t lsize = 1 << lhistoryBits;

  ghr = 0; // Initialize to NT
  gpred_cnt = 0;

  pht = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(pht, 1, gsize); // Weakly not taken (WN)

  choicePredictors = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(choicePredictors, 2, gsize); // Initialize to weekly select global predictor

  lPredictors = (uint8_t *)malloc(sizeof(uint8_t) * lsize);
  memset(lPredictors, 1, lsize); // Weakly not taken (WN)

  lhistoryTable = (uint32_t *)calloc(1 << pcIndexBits, sizeof(uint32_t));
}

void
custom_init() {
  ghr = 0;

  choicePredictors = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(choicePredictors, 1, gsize);

  dirpn = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(dirpn, 1, gsize);

  dirpt = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
  memset(dirpt, 1, gsize);
}

// void
// agree_init() {
//   ghr = 0; // Initialize to NT

//   pht = (uint8_t *)malloc(sizeof(uint8_t) * gsize);
//   memset(pht, 1, gsize); // Weakly not agree (WA)

//   btb = (uint8_t *)calloc(gsize, sizeof(uint8_t)); // initialize to never met
// }

uint8_t
gshare_pred(uint32_t pc) {
  uint32_t index = (pc ^ ghr) & gmask;
  return pht[index] >> (COUNTER - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
tournament_pred(uint32_t pc) {
  uint8_t predict = NOTTAKEN;
  uint8_t selector = choicePredictors[ghr] >> (COUNTER - 1);
  if (selector) {
    predict = pht[ghr];
    gpred_cnt++;
  } else {
    predict = lPredictors[lhistoryTable[pc & ((1 << pcIndexBits) - 1)]];
  }
  return predict >> (COUNTER - 1) ? TAKEN : NOTTAKEN;
}

uint8_t
custom_pred(uint32_t pc) {
  uint8_t predict = NOTTAKEN;
  uint32_t pcindex = pc & gmask;
  uint32_t index = pcindex ^ ghr;
  uint8_t choicePredict = choicePredictors[pcindex] >> (COUNTER - 1);

  if (choicePredict) {
    predict = dirpt[index];
  } else {
    predict = dirpn[index];
  }
  return predict >> (COUNTER - 1) ? TAKEN : NOTTAKEN;
}

// uint8_t
// agree_pred(uint32_t pc) {
//   uint32_t pcindex = pc & gmask;
//   uint8_t biasBit = btb[pcindex];
//   uint32_t index = (pc ^ ghr) & gmask;
//   uint8_t agree = pht[index];
//   return agree >> (COUNTER - 1) ? biasBit : !biasBit;
// }

void
gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t index = (pc ^ ghr) & gmask;
  ghr = (ghr << 1 | outcome) & gmask;
  if (outcome) {
    pht[index] += pht[index] == (1 << COUNTER) - 1 ? 0 : 1;
  } else {
    pht[index] -= pht[index] == 0 ? 0 : 1;
  }
}

void
tournament_train(uint32_t pc, uint8_t outcome) {
  uint8_t MAX = (1 << COUNTER) - 1;
  uint32_t pcindex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lindex = lhistoryTable[pcindex];

  uint8_t gCorrect = pht[ghr] >> (COUNTER - 1) == outcome;
  uint8_t lCorrect = lPredictors[lindex] >> (COUNTER - 1) == outcome;

  int action = gCorrect - lCorrect;
  if (action > 0) {
    choicePredictors[ghr] += choicePredictors[ghr] == MAX ? 0 : 1;
  } else if (action < 0) {
    choicePredictors[ghr] -= choicePredictors[ghr] == 0 ? 0 : 1;
  }

  if (outcome) {
    pht[ghr] += pht[ghr] == MAX ? 0 : 1;
    lPredictors[lindex] += lPredictors[lindex] == MAX ? 0 : 1;
  } else {
    pht[ghr] -= pht[ghr] == 0 ? 0 : 1;
    lPredictors[lindex] -= lPredictors[lindex] == 0 ? 0 : 1;
  }

  ghr = (ghr << 1 | outcome) & gmask;
  lhistoryTable[pcindex] = (lhistoryTable[pcindex] << 1 | outcome) & ((1 << lhistoryBits) - 1);
}

void
custom_train(uint32_t pc, uint8_t outcome) {
  uint8_t predict = NOTTAKEN;
  uint32_t pcindex = pc & gmask;
  uint32_t index = pcindex ^ ghr;
  uint8_t choicePredict = choicePredictors[pcindex] >> (COUNTER - 1);

  ghr = (ghr << 1 | outcome) & gmask;

  if (choicePredict) {
    predict = dirpt[index];
    dirpt[index] = updatePredictor(outcome, predict);
  } else {
    predict = dirpn[index];
    dirpn[index] = updatePredictor(outcome, predict);
  }

  if (!(choicePredict != outcome && predict == outcome)) {
    choicePredictors[pcindex] = updatePredictor(outcome, choicePredictors[pcindex]);
  }
}

uint8_t
updatePredictor(uint8_t outcome, uint8_t predict) {
  if (outcome) {
    predict += predict == (1 << COUNTER) - 1 ? 0 : 1;
  } else {
    predict -= predict == 0 ? 0 : 1;
  }
  return predict;
}

// void
// agree_train(uint32_t pc, uint8_t outcome) {
//   uint32_t pcindex = pc & gmask;
//   uint32_t index = (pc ^ ghr) & gmask;
//   if (btb[pcindex] == outcome) {
//     pht[index] += pht[index] == (1 << COUNTER) - 1 ? 0 : 1;
//   } else {
//     pht[index] -= pht[index] == 0 ? 0 : 1;
//   }
//   btb[pcindex] = outcome;
// }

void
wrap_up_predictor() {
  switch (bpType) {
    case GSHARE: {
      free(pht);
      break;
    }
    case TOURNAMENT: {
      free(pht);
      free(choicePredictors);
      free(lPredictors);
      free(lhistoryTable);
      break;
    }
    case CUSTOM: {
      free(choicePredictors);
      free(dirpt);
      free(dirpn);
      break;
    }
    default:
      break;
  }
}