dungeonCrawler: dungeonCrawlerBack.o dungeonCrawlerFront.o structs.h
	gcc -o dungeonCrawler dungeonCrawlerBack.o dungeonCrawlerFront.o

dungeonCrawlerBack.o: dungeonCrawlerBack.c dungeonCrawlerBack.h
	gcc -c -g -Wall -pedantic dungeonCrawlerBack.c
dungeonCrawlerFront.o: dungeonCrawlerFront.c
	gcc -c -g -Wall -pedantic dungeonCrawlerFront.c
