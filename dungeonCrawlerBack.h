/* dungeonCrawlerBack.h
 * 
 * Trabajo práctio especial Programación Imperativa 2010 - 1er cuatrimestre
 * 
 * Grupo:
 * 		Esteban Pintos
 * 		Agustín Scigliano
 * 		Matías De Santi
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef dungeonCrawlerBack_H_
#define dungeonCrawlerBack_H_



/** \brief Copies the name enetered by the player to the corresponding struct.
Receives a pointer to the string to be saved*/
int nameToStruct( avatarType *avatar, char *auxName );

/** \briefFrees the memory used during the game*/
void freeMemory(gameDataType * data, avatarType * avatar);

/** \briefDefines the HP the avatar will have.
Also saves the inicial room into the avatar's actual room*/
void defineHP(avatarType *avatar, gameDataType *data);

/** \briefSaves in a file passed by parameter the visited rooms and the door
chosen in each room*/
void dumpActions( gameDataType *data, char *fileName );

/** \briefInicializes the dumpActions information*/
void inicializeDumpActions( gameDataType *data );

/** Opens the file given when the program was called.
 * 
 * If a seed was passed as parameter, saves the seed in a struct so it
 * can be used during al the game. Else starts the seed with time(NULL).
 * If the wrong parameters where sent to the game, returns NULL.
 * 
 * Receives the pointer to the seed, the quantity of arguments passed when
 * calling the game and an array with the parameters*/
FILE * opensFile(int argc, char ** argv, unsigned int * seed);

/**Saves the number of door chosen when moving in the map.
 * 
 * Receives pointer to struct and the numer of chosen door.
 * 
 * Returns 1 if there was an error*/
int saveDoor( gameDataType *data,  int chosenDoor);

/**Saves the name of the chosen room to keep a log and save it to the 
 * specified file if dumpActions is called.
 * Receives pointer to struct and the string containing the actual room
 * 
 * Retuns 1 in case there was an error.*/
int saveRoom( gameDataType *data, int actualRoom);

/** \brief Generates a random number in the interval given passed by parameters*/
int randomNumber(int top, int bottom);

/** \brief Generates a random number in the interval [0,1)*/
float randomTurn();

/** Calls all the functions that will read each part of the xml.
 * 
 * Return:
 *  	*-1 if either the file or de data pointer are NULL
 *  	*1 if there was an error while reading important points
 *  	*2 if there was an error while reading professions
 *  	*3 if there was an error while reading enemys
 *  	*4 if there was an error while reading rooms
 *  	*5 if inicial and end rooms are not valid*/
int readsFile(FILE * file, gameDataType * data);

/**
 * Simulates the fight between the Avatar and one enemy.
 * 
 * 	Returns 0 if the Avatar was killed.
 * 	Returns -1 if the Avatar has to fight with another enemy.
 * 	Returns any other value if the avatar has defeated all enemys.
 * 
 * Receives as parameters the functions encharged of printing HP changes
 * during fight and the one encharged of printing the enemys left in the room*/
int fight(gameDataType * data, avatarType * avatar, int (*continueFight) (void), void (*printAttack) (int, gameDataType *, avatarType *, int, int, int),void (*printActualMonster)(gameDataType *, int monsterPosition) );

/** Generates a random number and depending on this number perfoms different
 * accions. It either reduces avatar's HP, increases it or increases the 
 * MaxDP*/
int treasure( avatarType *avatar, gameDataType *data);

#endif
