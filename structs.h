/* structs.h
 * 
 * Trabajo práctio especial Programación Imperativa 2010 - 1er cuatrimestre
 * 
 * Grupo:
 * 		Esteban Pintos
 * 		Agustín Scigliano
 * 		Matías De Santi
 * */

#ifndef structs_H_
#define structs_H_

typedef struct
{
	int inicialRoom; /**Has the position of the inicial room*/
	int endRoom;	/**Has the position of the end room*/
} importantPointsType;

typedef struct
{
	unsigned int seed;
	char **visitedRooms;
	int *selectedDoors;
	int visitedRoomsQty; 
	int selectedDoorsQty;
} dumpActionsType;

typedef struct
{
	int ID;
	char * name;
	int MinHP;
	int MaxHP;
	int MinDP;
	int MaxDP;
} professionType;

typedef struct
{
	int ID;
	char *name;
	int DP;
	int HP;
	int actualRoom;
} avatarType;

typedef struct
{
	int ID;
	char * name;
	int MinHP;
	int MaxHP;
	int HP;
	int * MinDP;
	int * MaxDP;
} monsterType;

typedef struct
{
	char * name;
	int destiny;
} doorType;

typedef struct
{
	int ID;
	char * name;
	char * description;
	doorType * doors;
/**Has the position of the corresponding enemyID in the enemy's array*/
	int * enemysID;  
	int  enemyQty;
	int  doorQty;
} roomType;

typedef struct
{
	importantPointsType importantPoints;
	professionType * professions;
	dumpActionsType dumpActions;
	int professionQty;
	monsterType * monsters;
	int monstersQty;
	roomType * rooms;
	int roomsQty;
	int  *** monsterToProfessionDamage;
} gameDataType;

#endif
