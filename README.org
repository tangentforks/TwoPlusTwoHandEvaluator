* TwoPlusTwo Hand Evaluator

This is the famous Two Plus Two hand evaluator for 5-to-7-card poker hands, created by Ray Wotton with help from many others in a [[http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906][giant thread the Two plus Two poker forums]] and popularized by an old [[https://web.archive.org/web/20111103160502/http://www.codingthewheel.com/archives/poker-hand-evaluator-roundup#2p2][coding the wheel blog post]].

It's basically an elaborate caching system for an [[http://www.suffecool.net/poker/evaluator.html][ealier evaluator by Cactus Kev and Paul Senzee]].

* About this fork

This fork ([[https://github.com/tangentstorm/TwoPlusTwoHandEvaluator]]) was extracted from the [[https://github.com/tangentstorm/XPokerEval][XPokerEval]] distribution (which contains a bunch of hand evaluators compiled by the coding the wheel guy).

I added a =Makefile= and made some minor changes to to improve readability and allow compilation on non-windows platforms (it works on freebsd, anyway).

My own main contribution is =rankhand.pas=, a small example program made with [[http://freepascal.org/][free pascal]] that shows how to use the =HandRanks.dat= file that =generate_table.cpp= creates.

With all the people who had touched the c and c++ code, I found it difficult to decipher exactly how the cards and hands were represented (Wotton's code uses a completely different coding system than the one in =poker.h=).


* License

The pascal program is my own, and available for use under the extremely liberal *zlib license*. I wrote the pascal program from scratch and it has no dependencies on or relation to any of the C/C++ code. The only requirement is the generated database file.

Wotton's code contains the self-contradictory statement "My code is GPL, use it as you like". The Cactus Kev / Paul Senzee code has a copyright but no license.
