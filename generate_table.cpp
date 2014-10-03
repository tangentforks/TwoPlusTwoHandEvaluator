// HandRankSetup.cpp : Sets up the HandRank File for VERY fast Lookups
// by Ray Wotton and the 2+2 list  My code is GPL, use it as you like

// ====
//
// the basic concept here is that every 7-card permutation
// is added to a giant directed graph, which is arranged to
// eliminate as much redundancy as possible. it essentially
// acts as a simple finite state machine.
//
// i'm just cleaning this up so i can read the code
// and figure out the actual format of the numbers so
// i can use the database from pascal and generate it
// from non-windows machines.
//
// --tangentstorm / Oct 03,2014
//
// references:
// http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906
// https://web.archive.org/web/20111103160502/http://www.codingthewheel.com/archives/poker-hand-evaluator-roundup#2p2


#include "poker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char HandRanks[][16] = {
  "BAD!!",//0
  "High Card",//1
  "Pair",//2
  "Two Pair",//3
  "Three of a Kind",//4
  "Straight",//5
  "Flush",//6
  "Full House",//7
  "Four of a Kind",//8
  "Straight Flush"//9
 };

#define LARGE_INTEGER int64_t
#define int64 int64_t


inline int min(int const x, int const y) {
  return y < x ? y : x;
}

int64 IDs[612978];
int HR[32487834];

int numIDs = 1;
int numcards = 0;
int maxHR = 0;
int64 maxID = 0;


int64 MakeID(int64 IDin, int newcard) {
  // returns a 64-bit hand ID, for up to 8 cards, stored 1 per byte.

  int64 ID = 0;
  int suitcount[4 + 1];
  int rankcount[13 + 1];
  int wk[8];  // intentially keeping one as a 0 end
  int cardnum;
  int getout = 0;

  memset(wk, 0, sizeof(wk));
  memset(rankcount, 0, sizeof(rankcount));
  memset(suitcount, 0, sizeof(suitcount));

  // can't have more than 6 cards!
  for (cardnum = 0; cardnum < 6; cardnum++) {
    // leave the 0 hole for new card
    wk[cardnum + 1] =  (int) ((IDin >> (8 * cardnum)) & 0xff);
  }

  // my cards are 2c = 1, 2d = 2  ... As = 52
  newcard--;  // make 0 based!

  // add next card. formats card to rrrr00ss
  wk[0] = (((newcard >> 2) + 1) << 4) + (newcard & 3) + 1;

  for (numcards = 0; wk[numcards]; numcards++) {
    // need to see if suit is significant
    suitcount[wk[numcards] & 0xf]++;
    // and rank to be sure we don't have 4!
    rankcount[(wk[numcards] >> 4) & 0xf]++;
    if (numcards) {
      // can't have the same card twice
      // if so need to get out after counting numcards
      if (wk[0] == wk[numcards]) getout = 1;
    }
  }
  if (getout) return 0; // duplicated another card (ignore this one)

// (MakeID)

  // for suit to be significant, need to have n-2 of same suit
  int needsuited = numcards - 2;
  if (numcards > 4) {
    for (int rank = 1; rank < 14; rank++) {
      // if I have more than 4 of a rank then I shouldn't do this one!!
      // can't have more than 4 of a rank so return an ID that can't be!
      if (rankcount[rank] > 4) return 0;
    }
  }

  // However in the ID process I prefered that
  // 2s = 0x21, 3s = 0x31,.... Kc = 0xD4, Ac = 0xE4
  // This allows me to sort in Rank then Suit order

  // if we don't have at least 2 cards of the same suit for 4,
  // we make this card suit 0.
  if (needsuited > 1) {
    for (cardnum = 0; cardnum < numcards; cardnum++) {  // for each card
      if (suitcount[wk[cardnum] & 0xf] < needsuited) {
	// check suitcount to the number I need to have suits significant
	// if not enough - 0 out the suit - now this suit would be a 0 vs 1-4
	wk[cardnum] &= 0xf0;
      }
    }
  }

// (MakeID)

  // Sort Using XOR.  Netwk for N=7, using Bose-Nelson Algorithm:
  // Thanks to the thread!
#define SWAP(I,J) {if (wk[I] < wk[J]) {wk[I]^=wk[J]; wk[J]^=wk[I]; wk[I]^=wk[J];}}

  SWAP(0, 4); SWAP(1, 5); SWAP(2, 6); SWAP(0, 2); SWAP(1, 3);
  SWAP(4, 6); SWAP(2, 4); SWAP(3, 5); SWAP(0, 1); SWAP(2, 3);
  SWAP(4, 5); SWAP(1, 4); SWAP(3, 6); SWAP(1, 2); SWAP(3, 4);
  SWAP(5, 6);

  // long winded way to put the pieces into a int64
  // cards in bytes --66554433221100
  // the resulting ID is a 64 bit value with each card represented by 8 bits.
  ID =  (int64) wk[0] +
    ((int64) wk[1] << 8) +
    ((int64) wk[2] << 16) +
    ((int64) wk[3] << 24) +
    ((int64) wk[4] << 32) +
    ((int64) wk[5] << 40) +
    ((int64) wk[6] << 48);
  return ID;
}

int SaveID(int64 ID) {
  // this inserts a hand ID into the IDs array.

  if (ID == 0) return 0; // don't use up a record for a 0!

  // take care of the most likely first goes on the end...
  if (ID >= maxID) {
    if (ID > maxID) { // greater than create new else it was the last one!
      IDs[numIDs++] = ID;  // add the new ID
      maxID = ID;
    }
    return numIDs - 1;
  }

  // find the slot (by a pseudo bsearch algorithm)
  int low = 0;
  int high = numIDs - 1;
  int64 testval;
  int holdtest;

  while (high - low > 1) {
    holdtest = (high + low + 1) / 2;
    testval = IDs[holdtest] - ID;
    if (testval > 0) high = holdtest;
    else if (testval < 0) low = holdtest;
    else return holdtest;   // got it!!
  }
  // it couldn't be found so must be added to the current location (high)
  // make space...  // don't expect this much!
  memmove(&IDs[high + 1], &IDs[high], (numIDs - high) * sizeof(IDs[0]));

  IDs[high] = ID;   // do the insert into the hole created
  numIDs++;
  return high;
}


int DoEval(int64 IDin) {
  // converts a 64bit handID to an absolute ranking.

  // I guess I have some explaining to do here...
  // I used the Cactus Kevs Eval ref http://www.suffecool.net/poker/evaluator.html
  // I Love the pokersource for speed, but I needed to do some tweaking
  // to get it my way and Cactus Kevs stuff was easy to tweak ;-)
  int result = 0;
  int cardnum;
  int wkcard;
  int rank;
  int suit;
  int mainsuit = 20;  // just something that will never hit...
  // TODO: need to eliminate the main suit from the iterator
  //int suititerator = 0;
  int suititerator = 1; // changed as per Ray Wotton's comment at http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906&page=0&fpart=18&vc=1
  int holdrank;
  int wk[8];  // "work" intentially keeping one as a 0 end
  int holdcards[8];
  int numevalcards = 0;

  // See Cactus Kevs page for explainations for this type of stuff...
  const int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };

  memset(wk, 0, sizeof(wk));
  memset(holdcards, 0, sizeof(holdcards));

  if (IDin) { // if I have a good ID then do it...
    for (cardnum = 0; cardnum < 7; cardnum++) {
      // convert all 7 cards (0s are ok)
      holdcards[cardnum] =  (int) ((IDin >> (8 * cardnum)) & 0xff);
      if (holdcards[cardnum] == 0) break;	// once I hit a 0 I know I am done
      numevalcards++;
      // if not 0 then count the card
      if (suit = holdcards[cardnum] & 0xf) {
	// find out what suit (if any) was significant and remember it
	mainsuit = suit;
      }
    }

// (DoEval)

    for (cardnum = 0; cardnum < numevalcards; cardnum++) {
      // just have numcards...
      wkcard = holdcards[cardnum];

      // convert to cactus kevs way!!
      // ref http://www.suffecool.net/poker/evaluator.html
      //   +--------+--------+--------+--------+
      //   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
      //   +--------+--------+--------+--------+
      //   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
      //   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
      //   cdhs = suit of card
      //   b = bit turned on depending on rank of card

      rank = (wkcard >> 4) - 1;	 // my rank is top 4 bits 1-13 so convert
      suit = wkcard & 0xf;  // my suit is bottom 4 bits 1-4, order is different, but who cares?
      if (suit == 0) {
	// if suit wasn't significant though...
	suit = suititerator++;   // Cactus Kev needs a suit!
	if (suititerator == 5)	 // loop through available suits
	  suititerator = 1;
	if (suit == mainsuit) {   // if it was the sigificant suit...  Don't want extras!!
	  suit = suititerator++;    // skip it
	  if (suititerator == 5)	  // roll 1-4
	    suititerator = 1;
	}
      }
      // now make Cactus Kev's Card
      wk[cardnum] = primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
    }

// (DoEval)
    // James Devlin: replaced all calls to Cactus Kev's eval_5cards with calls to
    // Senzee's improved eval_5hand_fast

    switch (numevalcards) {  // run Cactus Keys routines
    case 5 :  holdrank =     eval_5hand_fast(wk[0],wk[1],wk[2],wk[3],wk[4]);
      break;
      // if 6 cards I would like to find Result for them
      // Cactus Key is 1 = highest - 7362 lowest
      // I need to get the min for the permutations
    case 6 :
      holdrank = eval_5hand_fast(wk[0],wk[1],wk[2],wk[3],wk[4]);
      holdrank = min( holdrank,
		      eval_5hand_fast(wk[0],wk[1],wk[2],wk[3],wk[5]));
      holdrank = min( holdrank,
		      eval_5hand_fast(wk[0],wk[1],wk[2],wk[4],wk[5]));
      holdrank = min( holdrank,
		      eval_5hand_fast(wk[0],wk[1],wk[3],wk[4],wk[5]));
      holdrank = min( holdrank,
		      eval_5hand_fast(wk[0],wk[2],wk[3],wk[4],wk[5]));
      holdrank = min( holdrank,
		      eval_5hand_fast(wk[1],wk[2],wk[3],wk[4],wk[5]));
      break;
    case 7 : holdrank = eval_7hand(wk);
      break;
    default : // problem!!  shouldn't hit this...
      printf("    Problem with numcards = %d!!\n", numcards);
      break;
    }

// (DoEval)
    // I would like to change the format of Catus Kev's ret value to:
    // hhhhrrrrrrrrrrrr   hhhh = 1 high card -> 9 straight flush
    //                    r..r = rank within the above	1 to max of 2861
    result = 7463 - holdrank;  // now the worst hand = 1

    if      (result < 1278) result = result -    0 + 4096 * 1;  // 1277 high card
    else if (result < 4138) result = result - 1277 + 4096 * 2;  // 2860 one pair
    else if (result < 4996) result = result - 4137 + 4096 * 3;  //  858 two pair
    else if (result < 5854) result = result - 4995 + 4096 * 4;  //  858 three-kind
    else if (result < 5864) result = result - 5853 + 4096 * 5;  //   10 straights
    else if (result < 7141) result = result - 5863 + 4096 * 6;  // 1277 flushes
    else if (result < 7297) result = result - 7140 + 4096 * 7;  //  156 full house
    else if (result < 7453) result = result - 7296 + 4096 * 8;  //  156 four-kind
    else                    result = result - 7452 + 4096 * 9;  //   10 str.flushes
  }
  return result;  // now a handrank that I like
}


int main(int argc, char* argv[]) {
  int IDslot, card = 0, count = 0;
  int64 ID;

  clock_t timer = clock();   // remember when I started

  // Store the count of each type of hand (One Pair, Flush, etc)
  int handTypeSum[10];

  // Clear our arrays
  memset(handTypeSum, 0, sizeof(handTypeSum));
  memset(IDs, 0, sizeof(IDs));
  memset(HR, 0, sizeof(HR));


  // step through the ID array - always shifting the current ID and
  // adding 52 cards to the end of the array.
  // when I am at 7 cards put the Hand Rank in!!
  // stepping through the ID array is perfect!!

  int IDnum;
  int holdid;

// main()

  printf("\nGetting Card IDs!\n");

  // Jmd: Okay, this loop is going to fill up the IDs[] array which has
  // 612,967 slots. as this loops through and find new combinations it
  // adds them to the end. I need this list to be stable when I set the
  // handranks (next set)  (I do the insertion sort on new IDs these)
  // so I had to get the IDs first and then set the handranks
  for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++) {
    // start at 1 so I have a zero catching entry (just in case)
    for (card = 1; card < 53; card++) {
      // the ids above contain cards upto the current card.  Now add a new card
      ID = MakeID(IDs[IDnum], card);   // get the new ID for it
      // and save it in the list if I am not on the 7th card
      if (numcards < 7) holdid = SaveID(ID);
    }
    printf("\rID - %d", IDnum);	  // show progress -- this counts up to 612976
  }

// main()
  printf("\nSetting HandRanks!\n");

  // this is as above, but will not add anything to the ID list, so it is stable
  for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++) {
    // start at 1 so I have a zero catching entry (just in case)
    for (card = 1; card < 53; card++) {
      ID = MakeID(IDs[IDnum], card);

      if (numcards < 7) {
	// when in the index mode (< 7 cards) get the id to save
	IDslot = SaveID(ID) * 53 + 53;
      } else {
	// if I am at the 7th card, get the equivalence class ("hand rank") to save
	IDslot = DoEval(ID);
      }

      maxHR = IDnum * 53 + card + 53;	// find where to put it
      HR[maxHR] = IDslot; // and save the pointer to the next card or the handrank
    }

    if (numcards == 6 || numcards == 7) {
      // an extra, If you want to know what the handrank when there is 5 or 6 cards
      // you can just do HR[u3] or HR[u4] from below code for Handrank of the 5 or
      // 6 card hand
      // this puts the above handrank into the array
      HR[IDnum * 53 + 53] = DoEval(IDs[IDnum]);
    }

    printf("\rID - %d", IDnum);	// show the progress -- counts to 612976 again
  }

  printf("\nNumber IDs = %d\nmaxHR = %d\n", numIDs, maxHR);  // for warm fuzzys

  timer = clock() - timer;  // end the timer

  printf("Training seconds = %.2f\n", (float)timer/CLOCKS_PER_SEC);

// main()
  LARGE_INTEGER timings, endtimings;	// for high precision timing

  timer = clock();   // now get current time for Testing!

  // another algorithm right off the thread

  int c0, c1, c2, c3, c4, c5, c6;
  int u0, u1, u2, u3, u4, u5;

  // QueryPerformanceCounter(&timings);
  // start High Precision clock
  for (c0 = 1; c0 < 53; c0++) {
    u0 = HR[53+c0];
    for (c1 = c0+1; c1 < 53; c1++) {
      u1 = HR[u0+c1];
      for (c2 = c1+1; c2 < 53; c2++) {
	u2 = HR[u1+c2];
	for (c3 = c2+1; c3 < 53; c3++) {
	  u3 = HR[u2+c3];
	  for (c4 = c3+1; c4 < 53; c4++) {
	    u4 = HR[u3+c4];
	    for (c5 = c4+1; c5 < 53; c5++) {
	      u5 = HR[u4+c5];
	      for (c6 = c5+1; c6 < 53; c6++) {
		handTypeSum[HR[u5+c6] >> 12]++;
		count++;
	      }
	    }
	  }
	}
      }
    }
  }

  //	QueryPerformanceCounter(&endtimings);
  // end the high precision clock

  timer = clock() - timer;  // get the time in this

  for (int i = 0; i <= 9; i++)  // display the results
    printf("\n%16s = %d", HandRanks[i], handTypeSum[i]);

  printf("\nTotal Hands = %d\n", count);

// main()
  //	int64 clocksused = (int64)endtimings.QuadPart - (int64)
  //  timings.QuadPart;  // calc clocks used from the High Precision clock

  // and display the clock results
  //	printf("\nValidation seconds = %.4lf\nTotal HighPrecision Clocks = %I64d\nHighPrecision clocks per lookup = %lf\n", (double)timer/CLOCKS_PER_SEC, clocksused, (double) clocksused /  133784560.0) ;

  // output the array now that I have it!!
  FILE * fout = fopen("HandRanks.dat", "wb");
  if (!fout) {
    printf("Problem creating the Output File!\n");
    return 1;
  }
  fwrite(HR, sizeof(HR), 1, fout);  // big write, but quick

  fclose(fout);

  return 0;
}

///////////////////////////////// end code!!
