#include <iostream>
using namespace std;

#define DWORD int32_t

// The handranks lookup table- loaded from HANDRANKS.DAT.
int HR[32487834];

// This function isn't currently used, but shows how you lookup
// a 7-card poker hand. pCards should be a pointer to an array
// of 7 integers each with value between 1 and 52 inclusive.
int LookupHand(int* pCards)
{
    int p = HR[53 + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
	return HR[p + *pCards++];
}

void LookupSingleHands()
{
	printf("Looking up individual hands...\n\n");

	// Create a 7-card poker hand (each card gets a value between 1 and 52)
	int cards[] = { 2, 6, 12, 14, 23, 26, 29 };
	int retVal = LookupHand(cards);
	printf("Category: %d\n", retVal >> 12);
	printf("Salt: %d\n", retVal & 0x00000FFF);
}


void EnumerateAll7CardHands()
{
	// Now let's enumerate every possible 7-card poker hand
	int u0, u1, u2, u3, u4, u5;
	int c0, c1, c2, c3, c4, c5, c6;
	int handTypeSum[10];  // Frequency of hand category (flush, 2 pair, etc)
	int count = 0; // total number of hands enumerated
	memset(handTypeSum, 0, sizeof(handTypeSum));  // do init..

	printf("Enumerating and evaluating all 133,784,560 possible 7-card poker hands...\n\n");

	// On your mark, get set, go...
	//DWORD dwTime = GetTickCount();

	for (c0 = 1; c0 < 47; c0++) {
		u0 = HR[53+c0];
		for (c1 = c0+1; c1 < 48; c1++) {
			u1 = HR[u0+c1];
 			for (c2 = c1+1; c2 < 49; c2++) {
				u2 = HR[u1+c2];
				for (c3 = c2+1; c3 < 50; c3++) {
					u3 = HR[u2+c3];
 					for (c4 = c3+1; c4 < 51; c4++) {
						u4 = HR[u3+c4];
						for (c5 = c4+1; c5 < 52; c5++) {
							u5 = HR[u4+c5];
 							for (c6 = c5+1; c6 < 53; c6++) {

								handTypeSum[HR[u5+c6] >> 12]++;

								// JMD: The above line of code is equivalent to:
								//int finalValue = HR[u5+c6];
								//int handCategory = finalValue >> 12;
								//handTypeSum[handCategory]++;

								count++;
							}
						}
					}
				}
			}
		}
	}

	//dwTime = GetTickCount() - dwTime;

	printf("BAD:              %d\n", handTypeSum[0]);
	printf("High Card:        %d\n", handTypeSum[1]);
	printf("One Pair:         %d\n", handTypeSum[2]);
	printf("Two Pair:         %d\n", handTypeSum[3]);
	printf("Trips:            %d\n", handTypeSum[4]);
	printf("Straight:         %d\n", handTypeSum[5]);
	printf("Flush:            %d\n", handTypeSum[6]);
	printf("Full House:       %d\n", handTypeSum[7]);	
	printf("Quads:            %d\n", handTypeSum[8]);
	printf("Straight Flush:   %d\n", handTypeSum[9]);

	// Perform sanity checks.. make sure numbers are where they should be
	int testCount = 0;
	for (int index = 0; index < 10; index++)
		testCount += handTypeSum[index];
	if (testCount != count || count != 133784560 || handTypeSum[0] != 0)
	{
		printf("\nERROR!\nERROR!\nERROR!");
		return;
	}

	printf("\nEnumerated %d hands.\n", count);
}


int main(int argc, char* argv[]) {

	printf("Testing the Two Plus Two 7-Card Evaluator\n");
	printf("-----------------------------------------\n\n");

	// Load the HandRanks.DAT file and map it into the HR array
	printf("Loading HandRanks.DAT file...");
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("HandRanks.dat", "rb");
	if (!fin)
	    return false;
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array
	fclose(fin);
	printf("complete.\n\n");

	// Enumerate all 133,784,560 possible 7-card poker hands...
	EnumerateAll7CardHands();
	LookupSingleHands();

	return 0;
}

