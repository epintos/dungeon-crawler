/* dungeonCrawlerBack.c
 * 
 * Trabajo práctio especial Programación Imperativa 2010 - 1er cuatrimestre
 * 
 * Grupo:
 * 		Esteban Pintos
 * 		Agustín Scigliano
 * 		Matías De Santi
 * */

#include "structs.h"
#include "dungeonCrawlerBack.h"

#define BLOCK 5


static int readsImportantPoints(FILE * file, gameDataType * data);
static int validatesText(FILE * file, char * text);
static int readsProfession(gameDataType * data, FILE * file, int i);
static char * readsDescription(FILE * file);
static int readEnemysID(FILE * file);
static int readsAllRooms(gameDataType * data, FILE * file);
static int readsRooms(gameDataType * data, FILE * file, int i);
static int validatesEnemys(gameDataType * data, FILE * file, int i);
static int validatesDoors(gameDataType * data, int * doorcounter,
			FILE * file, int i);
static int validatesDestiny(gameDataType * data);
static int readsAllProfessions(gameDataType * data, FILE * file);
static int findsRepeatedId(int ID, int arraylength, gameDataType * data,
			char * check);
static int searchesDestiny(gameDataType * data, int dest);
static int readsEnemys(FILE * file, gameDataType * data, int i);
static int validatesMinAndMaxDP(gameDataType * data, FILE * file, int i);
static int readsAllEnemys(FILE * file, gameDataType * data);
static int validatesInicialEndRoom(gameDataType * data, int startRoom,
			int finalRoom);
static void freeMonsterToProfessionDamage(gameDataType * data);
static void freeMonsters(gameDataType * data);
static void freeProfessions(gameDataType * data);
static void freeRooms(gameDataType * data);
static void defineMonsterHP ( gameDataType *data, int monsterPosition );
static void fightMonster ( avatarType *avatar, gameDataType *data,
	int actRoom, int (*waitForEnter) (void),
	void (*printHPChanges) (int, gameDataType *, avatarType *, int, int, int),
int enemyQty, int avatarAttacks);
int creates3DMatrix(int numOfEnemys, int *** monsterToProfessionDamage,
	gameDataType *data);


/**
 * \brief Simulates the fight between the Avatar and one enemy.
 * 
 * \return Returns 0 if the Avatar was killed.
 * \return Returns -1 if the Avatar has to fight with another enemy.
 * \return Returns any other value if the avatar has defeated all enemys.
 * 
 * Receives as parameters the functions encharged of printing HP changes
 * during fight and the one encharged of printing the enemys left in the room*/
int fight(gameDataType * data, avatarType * avatar, int (*waitForEnter) (void),
void (*printHPChanges) (int, gameDataType *, avatarType *, int, int, int),
void (*printActualMonster)(gameDataType *, int monsterPosition) )
{
	int i, ALREADYVISITED=0, actRoom;
	int avatarAttacks=1;
	double whoStarts;
/*Validates the parameters are correctly passed. If not, exits with code 1*/
	if(data==NULL || avatar==NULL)
		exit(1);
		
	/*If avatarAttacks==1 avatar attacks*/
	actRoom=avatar->actualRoom;
	if(data->rooms[actRoom].enemyQty==0)
		return avatar->HP;
		
	for( i=data->rooms[actRoom].enemyQty; i>0 && !ALREADYVISITED; i--)
	{
		/*Atacks first the "last" enemy in the room**/
		if( data->rooms[actRoom].enemysID[i-1]!=-1 )
		{
			defineMonsterHP(data,data->rooms[actRoom].enemysID[i-1]);
			(*printActualMonster)(data,data->rooms[actRoom].enemysID[i-1]);
			(*waitForEnter)();
			whoStarts=randomTurn();
				if( whoStarts>0.5 ) /*Decides who starts fighting**/
					avatarAttacks=0;
			fightMonster(avatar,data,actRoom,(*waitForEnter),
				(*printHPChanges),i,avatarAttacks); 
			if(avatar->HP<=0)
			{
				return 0;
			}
			else
			{
				(data->rooms[actRoom].enemyQty)--;
				data->rooms[actRoom].enemysID[i-1]=-1;
				return -1;
			}
		}
		else
		ALREADYVISITED=1;
	}
	return avatar->HP;
}

/** Simulates the fight between the avatar and ONE enemy.
 * 
 * Modifies avatar's and enemy's HP.*/
static void fightMonster ( avatarType *avatar, gameDataType *data,
int actRoom,int (*waitForEnter) (void),
void (*printHPChanges) (int, gameDataType *, avatarType *, int, int, int),
int enemyQty, int avatarAttacks)
{
	int a,b,previousHP,damage;
	while( avatar->HP>0 && data->
		monsters[data->rooms[actRoom].enemysID[enemyQty-1]].HP>0 )
	{
		do
		{
			if( avatarAttacks )
			{
				previousHP=data->
					monsters[data->rooms[actRoom].enemysID[enemyQty-1]].HP;
				damage=randomNumber(data->professions[avatar->ID].MaxDP,
					data->professions[avatar->ID].MinDP);
				if ( (data->monsters[data->rooms[actRoom].
					enemysID[enemyQty-1]].HP - damage ) > 0 )
					(data->monsters[data->rooms[actRoom].
					enemysID[enemyQty-1]].HP) -= damage;
				else
					data->monsters[data->rooms[actRoom].
						enemysID[enemyQty-1]].HP = 0;
			}
			else
			{
				previousHP=avatar->HP;
				a=data->rooms[actRoom].enemysID[enemyQty-1];
				b=avatar->ID;
				damage=randomNumber(data->monsterToProfessionDamage[a][b][1],
					data->monsterToProfessionDamage[a][b][0]);
				if ( (avatar->HP)-damage > 0 ) /* HP can't be negative**/
					(avatar->HP)-=damage;
				else
					avatar->HP=0;
			}
			(*printHPChanges)(avatarAttacks, data, avatar, enemyQty,
				previousHP, actRoom);
			avatarAttacks=!avatarAttacks;
		}
		while(avatar->HP>0 && data->monsters[data->rooms[actRoom].
			enemysID[enemyQty-1]].HP>0 && (*waitForEnter)());
	}
}


int nameToStruct( avatarType *avatar, char *auxName )
{
	int lengthAuxName;
	lengthAuxName=strlen(auxName);
	if ( ((avatar->name) = malloc( lengthAuxName+1 ))!=NULL )
	{
		/*copies the name from auxName to avatar->name**/
		strcpy(avatar->name,auxName); 
		return 0;
	}
	else
		return 1;

}


FILE * opensFile(int argc, char ** argv, unsigned int * seed)
{
	int passedSeed;
	FILE * file=NULL;
	if(argc<2 || argc>3)
		return NULL;
	else if(argc==2) /*Only pased by parameter the file to open**/
	{
		file=fopen(argv[1], "r");
		*seed=time(NULL);
	}
	else
	{
		file=fopen(argv[1], "r");
		if((passedSeed=atoi(argv[2]))>0)
			*seed=passedSeed;
		else
		return NULL;
	}
	srand(*seed);
	return file;
}


int saveRoom( gameDataType *data, int actualRoom)
{
	int chosenRoomsQty=data->dumpActions.visitedRoomsQty;
	char **aux1=NULL;
	char *aux2=NULL;
	if((chosenRoomsQty%BLOCK)==0)
	{
		aux1=realloc(data->dumpActions.visitedRooms,
			(chosenRoomsQty+BLOCK)*sizeof(char*));
		if (aux1!=NULL)
		{
			data->dumpActions.visitedRooms=aux1;
		}
		else
		{
			return 1;
		}
	}
	aux2=malloc( (strlen(data->rooms[actualRoom].name)+1)*sizeof(char));
	if(aux2!=NULL)
	data->dumpActions.visitedRooms[chosenRoomsQty]=aux2;
	else
	return 1;	
	strcpy(data->dumpActions.visitedRooms[chosenRoomsQty],data->
		rooms[actualRoom].name);
	++(data->dumpActions.visitedRoomsQty);
	return 0;
}

int saveDoor( gameDataType *data,  int chosenDoor)
{
	int chosenDoorsQty=data->dumpActions.selectedDoorsQty;
	int *aux=NULL;
	if((chosenDoorsQty%BLOCK)==0)
	{
		aux=realloc(data->dumpActions.selectedDoors,
			(chosenDoorsQty+BLOCK)*sizeof(int));
		if (aux!=NULL)
		{
			data->dumpActions.selectedDoors=aux;
		}
		else
		{
			return 1;
		}
	}
	data->dumpActions.selectedDoors[chosenDoorsQty]=chosenDoor;
	++(data->dumpActions.selectedDoorsQty);
	return 0;
}

float
randomTurn()
{
  float p;
	p=(rand()/((float)RAND_MAX+1));
return p;
}

int
randomNumber(int top, int bottom)
{
	return (rand()%(top-bottom+1)+bottom);
}



void 
defineHP( avatarType *avatar, gameDataType *data)
{
	int avatarPosition=avatar->ID; /*the position in the professionsID vector*/
	avatar->HP=randomNumber(data->professions[avatarPosition].MaxHP,data->
		professions[avatarPosition].MinHP);
		/*Asigns the inicial room read in readsFile to the avatar*/
	avatar->actualRoom=data->importantPoints.inicialRoom; 
}

/** \brief Defines monster's HP.
 * 
 * Receives pointer to data and the position of the struct corresponding
 * to the monster in the Enemy's array*/
static void defineMonsterHP ( gameDataType *data, int monsterPosition )
{
	data->monsters[monsterPosition].HP=
		randomNumber(data->monsters[monsterPosition]
		.MaxHP,data->monsters[monsterPosition].MinHP);
}


void
dumpActions( gameDataType *data, char *fileName )
{
	int i;
	FILE *save;
	char name[42]; /* Two extra spaces for ./"*/
	sprintf(name,"./%s",fileName);
	save=fopen(name,"w");
	if(save==NULL)
	exit(1);
	fputs("Semilla: ",save);
	fprintf(save, "%u\n", data->dumpActions.seed);
	for (i = 0; i < (data->dumpActions.selectedDoorsQty); i++)
	{
		fputs(data->dumpActions.visitedRooms[i],save);
		fputc('\t',save);
		fprintf(save, "%d\n", data->dumpActions.selectedDoors[i]);
	}
	fclose(save);
}

/** Inicializes dumpActions struct components*/
void
inicializeDumpActions( gameDataType *data )
{
		data->dumpActions.selectedDoorsQty=0;
		data->dumpActions.visitedRoomsQty=0;
		data->dumpActions.selectedDoors=NULL;
		data->dumpActions.visitedRooms=NULL;
}

/*READ TEXT*/

/** \brief Reads the important points section. Saves corresponding data
 *  in the struct.
 * 
 * \return Returns 1 if there was an error*/
static int readsImportantPoints(FILE * file, gameDataType * data)
{
	int startroom;
	int c;
	
	/*Validates <PuntosImportantes>**/
	if(validatesText(file, "<PuntosImportantes>"))
		return 1;
	
	/*Validates inicialRoom**/
	if(validatesText(file, "<HabitacionInicioID>"))
		return 1;
		
	if(fscanf(file, "%d", &c)==1)
	{
		data->importantPoints.inicialRoom=c;
		startroom=c;
	}
	else
		return 1;
	if(validatesText(file, "</HabitacionInicioID>"))
		return 1;
	
	/*Validates endRoom**/
	
	if(validatesText(file, "<HabitacionSalidaID>"))
		return 1;
	if(fscanf(file, "%d", &c)==1)
	{
		if(c!=startroom) /*Validates inicialroom!=endroom**/
		{
			data->importantPoints.endRoom=c;
		}
		else
		return 1;
	}
	else
		return 1;
	
	if(validatesText(file, "</HabitacionSalidaID>"))
		return 1;
	
	/*Validates </PuntosImportantes>**/
	
	if(validatesText(file, "</PuntosImportantes>"))
		return 1;
	
	return 0;
}

/** \brief Reads from File the ammount of characters that where passed by
 * parameter to the function. Then compares the read text with the parameter.
 * 
 * \param Function gets a pointer to file and a text to read from file.
 * 
 *  \return Returns 1 if there was an error. 0 if everything went OK*/
static int validatesText(FILE * file, char * text)
{
	char *read=NULL;
	char fmt[6];
	int ERROR=0;
	int length=strlen(text);
	read=malloc(length*sizeof(char)+1);
	sprintf(fmt, "%%%ds", length);
	fscanf(file, fmt, read);
	if((strcmp(read, text))!=0)
	{
		ERROR=1;
	}
	free(read);
	return ERROR;
}

/**\brief Reads ONE profession. Saves corresponding data into struct.
 * 
 * \param i represents the position in the profession'sarray in which the
 *  information read must be saved.
 * \n EXAMPLE OF USE:\n
 * 			for(i=0; i<professionQty; i++)\n
 * 			{\n
 * 			readsProfession(data, file, i);\n
 * 			}\n
 * Where professionQty is the number of elements that the array where the
 * data will be saved has.
 * 
 * \return Returns 1 if there was an error*/
static int readsProfession(gameDataType * data, FILE * file, int i)
{
	char * aux;
	int c, min;
	
	if(validatesText(file, "<Profesion>"))
		return 1;
	
	/*Validates ID**/
	if(validatesText(file, "<ID>"))
		return 1;
	
	if(fscanf(file, "%d", &c)==1 && 
		findsRepeatedId(c, i, data, "Professions")==-1)
	{ /* The IF condition validates the ID is a number and isn't repeated**/
		data->professions[i].ID=c;
	}
	else
		return 1;
	
	if(validatesText(file, "</ID>"))
		return 1;
	
	/*Validates Nombre**/
	if(validatesText(file, "<Nombre>"))
		return 1;
	
		aux=readsDescription(file);
		if(aux!=NULL)
		{
			data->professions[i].name=aux;
		}
		else
			return 1;
			
	if(validatesText(file, "/Nombre>"))
		return 1;
	
	 /*Validates MinHP**/
	if(validatesText(file, "<MinHP>"))
		return 1;
	if(fscanf(file, "%d", &c)==1 && c>0)
	{
		data->professions[i].MinHP=c;
		min=c;
	}
	else
		return 1;
		
	if(validatesText(file, "</MinHP>"))
		return 1;
	
	 /*Validates MaxHP**/
	if(validatesText(file, "<MaxHP>"))
		return 1;
	if(fscanf(file, "%d", &c)==1 && c>0 && c>=min)
	{
		data->professions[i].MaxHP=c;
	}
	else
		return 1;
	if(validatesText(file, "</MaxHP>"))
		return 1;
	
	 /*Validates MinDP**/
	if(validatesText(file, "<MinDP>"))
		return 1;
	if(fscanf(file, "%d", &c)==1 && c>0)
	{
		data->professions[i].MinDP=c;
		min=c;
	}
	else
		return 1;
	if(validatesText(file, "</MinDP>"))
		return 1;
	
	 /*Validates MaxDP**/
	if(validatesText(file, "<MaxDP>"))
		return 1;
	if(fscanf(file, "%d", &c)==1 && c>0 && c>=min)
	{
		data->professions[i].MaxDP=c;
	}
	else
		return 1;
	if(validatesText(file, "</MaxDP>"))
		return 1;
	
	if(validatesText(file, "</Profesion>"))
		return 1;
	
	return 0;
}

/** \brief Reads text from file until it finds a "<".
 * 
 * Returns a pointer to the read text. NULL if there was an error*/
static char * readsDescription(FILE * file)
{
	char * description=NULL;
	char * aux=NULL;
	int ERROR=0;
	int c, i=0, counter=0;
	c=fgetc(file);
	while(c!='<' && ERROR==0)
	{
		if((i%BLOCK)==0)
		{
			aux=realloc(description, (i+BLOCK)*sizeof(char)+1);
			if(aux!=NULL)
			{
				description=aux;
			}
			else
			ERROR=1;
		}
		if(c!='\n')
		description[i++]=c;
		c=fgetc(file);
		counter++;
	}
	if(counter!=0)
	{
		description[i]=0;
		description=realloc(description, i*sizeof(char)+1);
	}
	else
	ERROR=1;
	if(ERROR)
	return NULL;
	
	return description;
}

/** \brief Reads and expects a number between <ID> and </ID>.
 * \return ID in case there was no ERROR. Returns -1 if there
 * was an ERROR**/
static int readEnemysID(FILE * file)
{
	int ID;
	
	if(validatesText(file, "<Enemigo>"))
		return -1;
	if(validatesText(file, "<ID>"))
		return -1;
		
	if(fscanf(file, "%d", &ID)==1 && ID>=0)
	{
		if(validatesText(file, "</ID>") || validatesText(file, "</Enemigo>"))
			return -1;
	}
	else
		return -1;
	return ID;
}

/** \brief Reads ONE room. Saves read data into corresponding struct.
 * 
 * \param "i" represents the position of the structure correspondig to
 *  the actual room in which the data must be saved.
 * \nEXAMPLE OF USE:\n
 * 			for(i=0; i<roomsQty; i++)\n
 * 			{\n
 * 			readsRooms(data, file, i);\n
 * 			}\n
 * Where roomsQty is the ammount of elements that the array where the data
 * will be saved has.
 * 
 * Returns 1 if error*/
static int readsRooms(gameDataType * data, FILE * file, int i)
{
	int ID, j=0, doorcounter=0;
	char * aux;
	
	if(validatesText(file, "<Habitacion>"))
		return 1;
	
	/*Validates Room ID**/
	if(validatesText(file, "<ID>"))
		return 1;
	if(fscanf(file, "%d", &ID)==1)
	{
		data->rooms[i].ID=ID;
	}
	else
		return 1;
	if(validatesText(file, "</ID>"))
		return 1;
	
	/*Validates room name**/
	if(validatesText(file, "<Nombre>"))
		return 1;
		
		aux=readsDescription(file);
		if(aux!=NULL)
		{
			data->rooms[i].name=aux;
		}
		else
			return 1;
			
	if(validatesText(file, "/Nombre>"))
		return 1;
	
	/*Validates Description**/
	if(validatesText(file, "<Descripcion>"))
		return 1;
		
		aux=readsDescription(file);
		if(aux!=NULL)
		{
			data->rooms[i].description=aux;
		}
		else
			return 1;
	if(validatesText(file, "/Descripcion>"))
		return 1;
	
	/*Validates doors if any.**/
	j=0;
	if(validatesText(file, "<Puertas>")==0)
	{
		if(validatesDoors(data, &doorcounter, file, i))
			return 1;
	}
	else
	fseek(file, -strlen("<Puertas>"), SEEK_CUR);
	
	/*Validates enemys if any**/
	if(validatesText(file, "<Enemigos>")==0)
	{
		if(validatesEnemys(data, file, i))
			return 1;
	}
	else
	fseek(file, -strlen("<Enemigos>"), SEEK_CUR);
	if(validatesText(file, "</Habitacion>"))
		return 1;
	return 0;
}

/**\brief Reads the enemys present in a room.
 * 
 * \param "i" in the parameters represents the position in the room's
 * array where the data must be saved.
 * \nEXAMPLE OF USE:\n
 * 			for(i=0; i<enemyQty; i++)\n
 * 			{\n
 * 			validatesEnemy(data, file, i);\n
 * 			}\n
 * Where enemyQty is the ammount of enemys present in a room.
 * 
 * The variable memory saves the place in which the enemy is
 * located in the enemy's vector. i.e.If the enemy with ID 5 was 3rd in
 * the enemy's array, then memory will have the number 2.
 * 
 * Returns 1 if error*/
static int validatesEnemys(gameDataType * data, FILE * file, int i)
{
	int ERROR=0;
	int c, numofenemys=0, j=0, memory;
	int * aux2;
	
	if(validatesText(file, "<Cantidad>"))
		return 1;
	if(fscanf(file, "%d", &c)==1)
	{
		numofenemys=c;
	}
	else
		return 1;
	if(validatesText(file, "</Cantidad>"))
		return 1;
		if(numofenemys>0)
		{
			
			aux2=malloc(numofenemys*sizeof(int));
			data->rooms[i].enemyQty=numofenemys;
			if(aux2!=NULL)
			{
				data->rooms[i].enemysID=aux2;
			}
			for(j=0; j<numofenemys && ERROR==0; j++)
			{
				c=readEnemysID(file);
				if(c>=0 && (memory=findsRepeatedId(c, data->monstersQty,
					data, "Monsters"))!=-1)
				{
					c=memory;
					data->rooms[i].enemysID[j]=c;
				}
				else
				{
					ERROR=1;
				}
			}
			}	
		if(validatesText(file, "</Enemigos>"))
			return 1;
		return ERROR;
}

/** \brief Reads all the doors a room can have.
 * 
 * Receives an indicator of where should the data be saved in the 
 * room's array ("i" parameter). Receives a pointer to a counter that
 * saves the amount of doors present in the room.
 * 
 * \nEXAMPLE OF USE:\n
 * 				for(i=0; i<roomQty; i++)\n
 * 				{\n
 * 					-------- (any other lines that might come before)\n
 * 					if(there are doors)\n
 * 					{\n
 * 						validatesDoors(......, i);\n
 * 					}\n
 * 					-------- (any other lines that might come after)\n
 * 				}\n
 * 
 * Returns 1 if ERROR*/

static int validatesDoors(gameDataType * data, int * doorcounter,
	FILE * file, int i)
{
	int dest, j=0;
	char * aux;
	doorType * ptr=NULL;
	
	while(validatesText(file, "<Puerta>")==0)
		{
			if((j%BLOCK)==0) /*Creates doors array*/
			{
				ptr=realloc(ptr, (j+BLOCK)*sizeof(doorType));
				if(ptr!=NULL)
				{
					data->rooms[i].doors=ptr;
				}
				else
				return 1;
			}
			if(validatesText(file, "<Nombre>"))
				return 1;
			aux=readsDescription(file);
			if(aux!=NULL)
			{
				data->rooms[i].doors[j].name=aux;
			}
			else
				return 1;
			if(validatesText(file, "/Nombre>") || 
				validatesText(file, "<Destino>"))
				return 1;
			if(fscanf(file, "%d", &dest)==1)
			{
				data->rooms[i].doors[j].destiny=dest;
			}
			else
			return 1;
			if(validatesText(file, "</Destino>") || 
				validatesText(file, "</Puerta>"))
				return 1;
			j++;
			(*doorcounter)++;
		}
		data->rooms[i].doorQty=(*doorcounter);
		fseek(file, -strlen("<Puerta>"), SEEK_CUR);
		if(validatesText(file, "</Puertas>"))
			return 1;

		return 0;
}

/** \brief Validates that the destiny of the doors in a room EXISTS.
 * 
 * \return Returns 1 if error.*/
static int validatesDestiny(gameDataType * data)
{
	int i, j, ERROR=0, place;
	
	for(i=0; i<data->roomsQty && ERROR==0;i++)
	{
		for(j=0; j<data->rooms[i].doorQty && ERROR==0; j++)
		{
			if((place=searchesDestiny(data, data->rooms[i].doors[j].destiny))
				==-1)
			{
				ERROR=1;
			}
			else
			{
				data->rooms[i].doors[j].destiny=place;
			}
		}
	}
	return ERROR;
}

/** \brief Reads all professions. This means that it will only read the ammount
 * of existing professions in the file (validates the present text before
 * and after their definition) and then calls a specific function encharged
 * of reading each individual profession.
 * 
 * Returns 1 if ERROR.*/
static int readsAllProfessions(gameDataType * data, FILE * file)
{
		int ERROR=0, numofprof, c, i;
		professionType * aux;
		
		if(validatesText(file, "<Profesiones>") || 
			validatesText(file, "<Cantidad>"))
			return 1;
		
		if(fscanf(file, "%d", &c)==1)
		{
			numofprof=c;
		}
		else
			return 1;
		if(validatesText(file, "</Cantidad>"))
			return 1;
		if(numofprof>0)
		{
			aux=malloc(numofprof*sizeof(professionType));
			if(aux!=NULL)
			data->professions=aux;
			else
			return 0;
		
			for(i=0; i<numofprof && ERROR!=1; i++)
			{
				ERROR=readsProfession(data, file, i);
			}
			data->professionQty=numofprof;
		}
		else
			return 1;
	if(validatesText(file, "</Profesiones>"))
		return 1;
	
	return ERROR;
}

/** \brief Reads all rooms. This means that it will only read the ammount
 * of existing rooms in the file (validates the present text before
 * and after their definition) and then calls a specific function encharged
 * of reading each individual room.
 * 
 * Returns 1 if ERROR*/
static int readsAllRooms(gameDataType * data, FILE * file)
{
		int ERROR=0, c, i, numofrooms;
		roomType * aux2;
		
		if(validatesText(file, "<Laberinto>") || 
			validatesText(file, "<Cantidad>"))
			return 1;
		if(fscanf(file, "%d", &c)==1)
		{
			numofrooms=c;
			data->roomsQty=c;
		}
		else
			return 1;
		if(validatesText(file, "</Cantidad>"))
			return 1;
		if(numofrooms>0)
		{
			aux2=malloc(numofrooms*sizeof(roomType));
			if(aux2!=NULL)
			data->rooms=aux2;
			else
			return 0;
		
			for(i=0; i<numofrooms && ERROR!=1; i++)
			{
				ERROR=readsRooms(data, file, i);
			}
		}
		else
			return 1;
		if(validatesText(file, "</Laberinto>"))
			return 1;
		if(validatesDestiny(data))
			return 1;
	return ERROR;
}

/**\brief Checks if the ID given is present in the given field.
 * 
 * \param "check" expects either "Monsters" or "Professions"
 * 
 * Returns the position where it was found of -1 if not found*/
static int findsRepeatedId(int ID, int arraylength, gameDataType * data,
	char * check)
{
	int c, ERROR=0;
	if(strcmp(check, "Professions")==0)
	{
		for(c=0; c<arraylength && ERROR==0; c++)
		{
			if(ID==data->professions[c].ID)
			{
				return c;
			}
		}
	}
	else if(strcmp(check, "Monsters")==0)
	{
		for(c=0; c<arraylength && ERROR==0; c++)
		{
			if(ID==data->monsters[c].ID)
			{
				return c;
			}
		}
	}
	return -1;
}

/** \brief Loos up for a given ID in the room's array to verify it exists.
 * 
 * \return Returns -1 if not found*/
static int searchesDestiny(gameDataType * data, int dest)
{
	int i;
	for(i=0; i<data->roomsQty; i++)
	{
		if(dest == data->rooms[i].ID)
		return i;
	}
	return -1;
}

/** \brief Rads ONE enemy.
 * 
 * \param "i" parameter represents the position in the enemy's array where
 * the data read must be saved.
 * \nEXAMPLE OF USE:\n
 * 			for(i=0; i<enemyQty; i++)\n
 * 			{\n
 * 			readsEnemys(data, file, i);\n
 * 			}\n
 * 
 * \return Returns 1 if ERROR.*/
static int readsEnemys(FILE * file, gameDataType * data, int i)
{
	int ERROR=0, c, j;
	char * aux=NULL;
	
	if(validatesText(file, "<Enemigo>"))
		return 1;

	/*Validates ID*/
	if(validatesText(file, "<ID>"))
		return 1;
	if(fscanf(file, "%d", &c)==1 && c>=0 && findsRepeatedId(c, i, data, "Monsters")==-1)
	{
		data->monsters[i].ID=c;
	}
	else
		return 1;
	
	if(validatesText(file, "</ID>"))
		return 1;
	
	/*Validates Name*/
	if(validatesText(file, "<Nombre>"))
		return 1;
		
	aux=readsDescription(file);
	if(aux!=NULL)
	{
		data->monsters[i].name=aux;
	}
	else
	return 1;
	if(validatesText(file, "/Nombre>"))
		return 1;
	
	/*Validates MinHP*/
	if(validatesText(file, "<MinHP>"))
		return 1;
	fscanf(file, "%d", &c);
	if(c>0)
	{
		data->monsters[i].MinHP=c;
	}
	else
	return 1;
	if(validatesText(file, "</MinHP>"))
		return 1;
	
	/*Validates MaxHP*/
	if(validatesText(file, "<MaxHP>"))
		return 1;
	fscanf(file, "%d", &c);
	if(c>0 && c>data->monsters[i].MinHP)
	{
		data->monsters[i].MaxHP=c;
	}
	else
	return 1;
	if(validatesText(file, "</MaxHP>"))
		return 1;
	
	for(j=0; j<data->professionQty && ERROR==0; j++)
	{
		ERROR=validatesMinAndMaxDP(data, file, i);
	}
	if(validatesText(file, "</Enemigo>"))
		return 1;

	return ERROR;
}

/** \brief Reads all the "<MinDP-X>%d</MinDP-X>" that are present in an enemy's
 * definition.
 * 
 * \param "i" repesents where to save the data in the 3d array.
 * 
 * \return Returns 1 if error*/
static int validatesMinAndMaxDP(gameDataType * data, FILE * file, int i)
{
	int c, positionOfProf, lastread, min;
	
	/*Validates MinDP-X*/
		if(validatesText(file, "<MinDP-"))
			return 1;
		fscanf(file, "%d", &lastread);
		if((positionOfProf=findsRepeatedId(lastread, data->professionQty,
			data, "Professions"))==-1)
			return 1;
		else
		{
			if(validatesText(file, ">"))
				return 1;
			fscanf(file, "%d",&c); /*Reads MinDP*/
			if(c>0)
			{
				data->monsterToProfessionDamage[i][positionOfProf][0]=c;
				min=c;
			}
			if(validatesText(file, "</MinDP-"))
				return 1;
/*Reads the closing of <MinDP-. The number after - must be the same as in <MinDP-*/
			fscanf(file, "%d", &c);
			if(c!=lastread)
			{
				return 1;
			}
			if(validatesText(file, ">"))
				return 1;
		}
		/*Validates MaxDP-X*/
		if(validatesText(file, "<MaxDP-"))
			return 1;
		fscanf(file, "%d", &c);
		if(c!=lastread)
			return 1;
			
		if((positionOfProf=findsRepeatedId(lastread, data->professionQty,
			data, "Professions"))==-1)
			return 1;
		else
		{
			if(validatesText(file, ">"))
				return 1;
			fscanf(file, "%d",&c);
			if(c>0 && c>min)
			data->monsterToProfessionDamage[i][positionOfProf][1]=c;
			else
				return 1;
			if(validatesText(file, "</MaxDP-"))
				return 1;
			fscanf(file, "%d", &c);
			if(c!=lastread)
			return 1;
			if(validatesText(file, ">"))
				return 1;
		}
		return 0;
}

/**\brief Reads ALL enemys present. This means that it will only read the ammount
 * of existing enemys in the file (validates the present text before
 * and after their definition) and then calls a specific function encharged
 * of reading each individual enemy.
 * 
 * Calls a specific function encharged of creating the 3d matrix
 * 
 * \return Returns 1 if error*/
static int readsAllEnemys(FILE * file, gameDataType * data)
{
	
	int *** matrix3=NULL;
	monsterType * aux2;
	int ERROR=0, numOfEnemys=0, i;
	if(validatesText(file, "<Enemigos>") || validatesText(file, "<Cantidad>"))
		return 1;
	fscanf(file, "%d", &numOfEnemys);
	if(validatesText(file, "</Cantidad>"))
		return 1;
	
	
	if(numOfEnemys>0)
	{	
		data->monstersQty=numOfEnemys;
		aux2=malloc(numOfEnemys*sizeof(monsterType));
		if(aux2!=NULL)
		{
			data->monsters=aux2;
		}
		else
		return 1;
/*Creates first dimension of monsterToProfessionDamage matrix*/
		matrix3=malloc(numOfEnemys*sizeof(char *));
		if(matrix3!=NULL)
		{
			data->monsterToProfessionDamage=matrix3;
		}
		else
		return 1;
/*Creates second and third dimension of monsterToProfessionDamage matrix*/
		if(creates3DMatrix(numOfEnemys, data->monsterToProfessionDamage, data)==1)
			return 1;
		for(i=0; i<numOfEnemys && ERROR==0; i++)
		{
			ERROR=readsEnemys(file, data, i);
		}
		if(validatesText(file, "</Enemigos>"))
			return 1;
	}
	else
	return 1;
	
	return ERROR;
}

/** \brief creates 3 dimension matrix to store the damage each enemy does
 * to each proffesion.*/
int creates3DMatrix(int numOfEnemys,
	int *** monsterToProfessionDamage, gameDataType * data)
{
	int * matrix1=NULL;
	int ** matrix2=NULL;
	int i, ERROR=0, w;
	
	for(i=0; i<numOfEnemys && ERROR==0; i++)
		{
			matrix2=malloc(data->professionQty*sizeof(char *));
			if(matrix2!=NULL)
			{
				data->monsterToProfessionDamage[i]=matrix2;
			}
			else
			ERROR=1;
			for(w=0; w<data->professionQty && ERROR==0; w++)
			{
				matrix1=malloc(2*sizeof(int));
				if(matrix1!=NULL)
				{
					data->monsterToProfessionDamage[i][w]=matrix1;
				}
				else
				ERROR=1;
			}
		}
		return ERROR;
}

/**\brief Validates that the inicial and end room exist.
 * Variables memoryinicial and memoryfinal do the same. They both save
 * the position in the room's vector in which the searched room is located.
 * 
 * \return Returns 1 if no error. 0 if there was error*/
static int validatesInicialEndRoom(gameDataType * data, int startRoom,
	int finalRoom)
{
	
	int memoryinicial, memoryfinal;
	if((memoryinicial=searchesDestiny(data, startRoom))!=-1 &&
		(memoryfinal=searchesDestiny(data, finalRoom))!=-1)
	{
		data->importantPoints.inicialRoom=memoryinicial;
		data->importantPoints.endRoom=memoryfinal;
		return 0;
	}
	return 1;
}

int readsFile(FILE * file, gameDataType * data)
{
	int ERROR=0;
	if(file==NULL || data==NULL)
		return -1;
	
	if(validatesText(file, "<Juego>"))
		return 1;
	/*readsImportantPoints returns 1 if there was an error*/
	if(readsImportantPoints(file, data)) 
	{
		ERROR=1;
	}
	/*Reads all professions in text*/
	if(ERROR==0 && readsAllProfessions(data, file)==1) 
	{
		ERROR=2;
	}
	if(ERROR==0 && readsAllEnemys(file, data)==1)
	{
		ERROR=3;
	}
	/*validates and reads Rooms in laberinth*/
	if(ERROR==0 && readsAllRooms(data, file)==1) 
	{
		ERROR=4;
	}
	if(ERROR==0 &&validatesText(file, "</Juego>"))
		return 1;
	
	if(ERROR==0 && validatesInicialEndRoom(data, data->
		importantPoints.inicialRoom, data->importantPoints.endRoom)==1)
	{
		ERROR=5;
	}
	return ERROR;
}

/**\brief Frees memory*/
void freeMemory(gameDataType * data, avatarType * avatar)
{
	freeMonsterToProfessionDamage(data);
	freeRooms(data);
	freeMonsters(data);
	freeProfessions(data);
	free(avatar->name);
	free(data->dumpActions.selectedDoors);
}


static void freeMonsterToProfessionDamage(gameDataType * data)
{
	int i, j;
	
	for(i=0; i<data->monstersQty; i++)
	{
		for(j=0; j<data->professionQty; j++)
		{
			free(data->monsterToProfessionDamage[i][j]);
		}
		free(data->monsterToProfessionDamage[i]);
	}
	free(data->monsterToProfessionDamage);
}

static void freeMonsters(gameDataType * data)
{
	int i;
	
	for(i=0; i<data->monstersQty; i++)
	{
		free(data->monsters[i].name);
		free(data->monsters[i].MinDP);
		free(data->monsters[i].MaxDP);
	}
	free(data->monsters);
}

static void freeProfessions(gameDataType * data)
{
	int i;
	for(i=0; i<data->professionQty; i++)
	{
		free(data->professions[i].name);
	}
	free(data->professions);
}

static void freeRooms(gameDataType * data)
{
	int i, j;
	for(i=0; i<data->roomsQty; i++)
	{
		free(data->rooms[i].name);
		free(data->rooms[i].description);
		for(j=0; j<data->rooms[i].doorQty; j++)
		{
			free(data->rooms[i].doors[j].name);
		}
			free(data->rooms[i].enemysID);
			free(data->rooms[i].doors);
	}
	free(data->rooms);

	for(i=0; i<data->dumpActions.visitedRoomsQty; i++)
	{
		free(data->dumpActions.visitedRooms[i]);
	}
	free(data->dumpActions.visitedRooms);
}

/** \brief Generates a random number and depending on this number perfoms different
 * accions. It either reduces avatar's HP, increases it or increases the 
 * MaxDP*/
int treasure( avatarType *avatar, gameDataType *data)
{
	int chest=randomNumber(70,0);
	if ( chest >=20 && chest <=30 )
	{
		avatar->HP+=avatar->HP*0.2;
		return 1;
	}
	else if ( chest >=0 && chest <=10 )
	{
		data->professions[avatar->ID].MaxDP+=
			data->professions[avatar->ID].MaxDP*0.25;
		return 0;
	}
	else if ( chest >=35 && chest <=40 )
	{
		avatar->HP-=avatar->HP*0.2;
		return -1;
	}
	return 2;
}
