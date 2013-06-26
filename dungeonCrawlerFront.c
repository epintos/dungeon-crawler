/* dungeonCrawlerFront.c
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

#define LIMIT_ENGLISH 127
#define CLEAN_BUFFER while ( getchar()!='\n')

static int readAvatarName( avatarType *avatar, char *auxName);
static void asignAvatarName ( avatarType *avatar );
static void printTitle();
static int welcomeMenu( avatarType *avatar, gameDataType *data);
static void defineAvatar( avatarType *avatar, gameDataType *data );
static void printProfessionsMenu( gameDataType * data );
static void printAvatarFeatures( avatarType *avatar, gameDataType *data);
static void printActualState(avatarType * avatar, gameDataType * data);
static void printAvailableDoors(avatarType * avatar , gameDataType * data);
static void chooseDoor ( avatarType * avatar, gameDataType *data);
static void play ( avatarType *avatar, gameDataType *data);
static void chooseProfession ( avatarType * avatar, gameDataType *data );
static void printActualMonster (gameDataType *data, int monsterPosition);
static int readingFile(FILE * file, gameDataType * data);
static FILE * openingFile(int argc, char ** argv, unsigned int * seed);
static void startGame( avatarType *avatar, gameDataType *data,int argc,
char ** argv,FILE*file);
static void printsHPChanges(int avatarAttacks, gameDataType * data,
avatarType * avatar, int i, int previousHP, int room);
static int waitForEnter();
static void printTreasureFound( avatarType *avatar, gameDataType *data);
static void printQuestion ( gameDataType *data, avatarType *avatar);
int main(int argc, char** argv);

int
main(int argc, char** argv)
{
	gameDataType data;
	avatarType avatar;
	FILE * file=NULL;
	startGame(&avatar,&data,argc,argv,file);
	return 0;
}

/** If file does not open prints message and exits. Else reads calls 
 * the function that reads file and then calls the function that prints
 * message*/
static void startGame( avatarType *avatar, gameDataType *data,int argc,
	char ** argv,FILE*file)
{
	int NOTERROR=1,option,FLAG=0;
	if((file=openingFile(argc, argv, &(data->dumpActions.seed)))==NULL)
	{
		printf("Error al intentar abrir el archivo. Compruebe que el archivo existe\n");
		exit(1);
	}
	else
	{
		/*Reads and prints where ocurried error*/
		if( ( NOTERROR=readingFile(file, data) ) ) 
		{
			printTitle();
			do
			{
				if (FLAG)
				printf("Seleccione alguna de las siguientes opciones:\n");
				option=welcomeMenu(avatar,data);
				FLAG=1;
				waitForEnter();
			}
			while( option!=1 );
		}	
	}
	fclose(file);	
	if( NOTERROR )
		freeMemory(data, avatar);
}

/**Calls a function encharged of reading the file passed by parameter
 * If file does not open prints message and returns NULL.
 * Else returns a pointer to FILE*/
static FILE * openingFile(int argc, char ** argv, unsigned int * seed)
{
	FILE * file;
	if((file=opensFile(argc, argv, seed))==NULL)
	{
		printf("Ha ocurrido un error al intentar abrir el archivo \
pasado como parametro o al guardar la semilla.\
Revise que el archivo exista y que este bien pasado y la semilla sea un\
numero mayor que 0\n");
		return NULL;
	}
	return file;
}

/**Prints message according to the error returned by readsFile*/
static int readingFile(FILE * file, gameDataType * data)
{
	int ERROR=0;
	switch(readsFile(file, data))
	{
		case -1:printf("Los punteros pasados no son vÃ¡lidos\n");
				ERROR=1;
				break;
		case 1: printf("Error al leer puntos importantes\n");
				ERROR=1;
				break;
		case 2: printf("Error al leer las profesiones\n");
				ERROR=1;
				break;
		case 3: printf("Error al leer los enemigos\n");
				ERROR=1;
				break;
		case 4: printf("Error al leer las habitaciones\n");
				ERROR=1;
				break;
		case 5: printf("Error al validar las habitaciones de entrada y salida\n");
				ERROR=1;
				break;
	}
	return !ERROR;
}

/**Prints the monster that comes next in the room*/
void
printActualMonster (gameDataType *data, int monsterPosition)
{
	printf("Aparecio un %s, el cual no esta dispuesto a dejarte pasar.\
 Su cantidad de puntos de vida es %d.\n",data->monsters[monsterPosition].name,\
data->monsters[monsterPosition].HP);
}

/**Reads the profession that the player wants*/
static void chooseProfession ( avatarType * avatar, gameDataType *data )
{
	int scanfValue,FLAG=0;
	int chosenProfession;
	printf("\nElija una profesion:\n");
	do
	{
		if (FLAG)/* It will enter if there is an error in the input*/
			printf("Ingrese un numero de profesion valido:\n");

		scanfValue=scanf("%d",&chosenProfession);
		CLEAN_BUFFER;
		FLAG=1;
	}
	while( !( chosenProfession<=(data->professionQty) && chosenProfession>0
		&& scanfValue==1) );
		/*Asigning the position according to the chosen profession*/
	avatar->ID=chosenProfession-1; 
}

/** Prints the corresponding message for the attack.
 * 
 * The "i" refers to the enemy that must be printed next*/
static void printsHPChanges(int avatarAttacks, gameDataType * data,
	avatarType * avatar, int i, int previousHP, int room)
{
	int a;
	a=data->rooms[avatar->actualRoom].enemysID[i-1];
	if(avatarAttacks)
	{ 
		printf("Ataco %s. El %s tenia %d de vida. Ahora tiene %d.\n",
			avatar->name,data->monsters[a].name,previousHP,
			data->monsters[data->rooms[room].enemysID[i-1]].HP);
	}
	else
	{
		printf("Ataco el %s. La vida que tenia %s era %d, ahora tiene %d.\n",
			data->monsters[a].name,avatar->name, previousHP, avatar->HP);
	}
}

/** Waits for the user to enter an '\n' to continue Fighting*/
static int waitForEnter()
{
	int c,FLAG=0;
	do
	{
		if ( FLAG) /* It will enter if there is an error in the input*/
			printf("Ingrese un caracter valido\n");
		c=getchar();		
		if( c!='\n')
		{ 
			CLEAN_BUFFER;
		}
		FLAG=1;
	}
	while(c!='\n');
	return 1;
}

/** Calls the functions fight, continue fight, choosedoor and
 * print available doors while the avatar is allive*/
static void
play ( avatarType *avatar, gameDataType *data)
{
	int NOTDEAD=1,WIN=0, STATE,WIZARD=0;
	inicializeDumpActions(data);
	defineAvatar(avatar,data); /*Name, Profession, HP*/
	printAvatarFeatures(avatar,data);
	do
	{
		printActualState(avatar,data); /*prints Avatar actual Room*/
		if ( data->rooms[avatar->actualRoom].enemyQty!=0 ) 
		/*Treasure only in rooms with enemies*/
		{
			printTreasureFound(avatar,data);
		}
		while((STATE=fight(data,avatar, waitForEnter, printsHPChanges,
			printActualMonster))==-1)
		{
			printf("**Has ganado la batalla**\n");
			waitForEnter();
		}
		if(STATE==0)
		{
			NOTDEAD=0;
			printf("\n**Has perdido, GAME OVER**\n");
		}
		/* Appear only if HP <= 20% of MaxHP*/
		else if ( avatar->HP <= data->professions[avatar->ID].MaxHP*0.2 &&
				avatar->actualRoom!=data->importantPoints.endRoom 
				&& WIZARD < 3 ) 
		{
			printQuestion(data,avatar);
			waitForEnter();
			++WIZARD; /*wizard can only appear 3 times*/
		}			
		if ( NOTDEAD && avatar->actualRoom!=data->importantPoints.endRoom && data->rooms[avatar->actualRoom].doorQty!=0)
		{
			printAvailableDoors(avatar,data); /*prints doors or win*/
			chooseDoor(avatar,data);
		}
		
		else if ( NOTDEAD && avatar->actualRoom==data->importantPoints.endRoom)
			WIN=1;
		else if ( data->rooms[avatar->actualRoom].doorQty == 0 && STATE!=0)
		{
			/*if room has no exit, player loses*/
			printf("**Esta habitacion no tiene salida, GAME OVER**\n");
			NOTDEAD=0;
		}
	}
	while ( NOTDEAD==1 && !WIN );
	if (NOTDEAD)
		printAvailableDoors(avatar,data);			
}


/** Asks the player for the door to move or dumpActions.
 * If dumpActions is introduced then it reads the file that must be created
 * and calls dumpActions function*/
static void chooseDoor ( avatarType * avatar, gameDataType *data)
{
	int chosenDoor,FLAG=0,c, position=0;
	char chosenOption[40]={0}; /*Necessary space for "dumpActions" and file name*/
	int actRoom=avatar->actualRoom;
	if ( saveRoom(data,actRoom)) /*Saves the name of the actual room*/
	{	
		printf("Error de Memoria\n");
		exit(1);
	}
	printf("\nElija una puerta o ejecute el dumpActions:\n");
	do
	{
		position=0;
		if ( FLAG==1) /* It will enter if there is an error in the input*/
			printf("Ingrese una opcion valida:\n");
		else if( FLAG==2)
			printf("\nElija una puerta o ejecute el dumpActions:\n");
		scanf("%11s",chosenOption); /*First read "dumpActions"*/
		if ( !strcmp(chosenOption,"dumpActions") )
		{
			/*Has to be a blank between dumpActions and the filename*/
			if ( (c=getchar())== ' ') 
			{
				while( (c=getchar())!='/' && c!='\n' && position<39)
/* Validates that the player doesn't enter a /. This would produce an error
 * while opening the file*/
					chosenOption[position++]=c;
				if(position>0)
				{
					chosenOption[position]=0;
					dumpActions(data,chosenOption);
					FLAG=2;
				}
				else
					FLAG=1;
			}
			else
				FLAG=1;
			if ( c!='\n')
					CLEAN_BUFFER;
		}
		else
		{
			chosenDoor=atoi(chosenOption);
			FLAG=1;
			CLEAN_BUFFER;
		}			
	} 
	while(  !(chosenDoor <= data->rooms[actRoom].doorQty && chosenDoor >0) );
	/*asign the new room after asking for the door*/
	avatar->actualRoom=data->rooms[actRoom].doors[chosenDoor-1].destiny; 
	if ( saveDoor(data,chosenDoor) ) /*Saves the number of chosen door*/
	{	
		printf("Error de Memoria\n");
		exit(1);
	}
}

/**Prints the available doors in the room*/
static void printAvailableDoors(avatarType * avatar , gameDataType * data)
{
  int i;
  int actRoom=avatar->actualRoom;
  if( (actRoom) == (data->importantPoints.endRoom) )
  printf("***Has ganado el juego! Felicitaciones!***\n");
  else
  {
    printf("Las puertas disponibles son:\n");
    for(i=0 ; i< (data->rooms[actRoom].doorQty); i++)
      {
			printf("\n%d: %s\n",(i+1),data->rooms[actRoom].doors[i].name);
      }
  }
}

/**Prints the actual location of the avatar*/
static void printActualState(avatarType * avatar, gameDataType * data)
{
	int actRoom;
	actRoom=avatar->actualRoom;
	printf("\n%s te encuentras en el cuarto %s. %s.\n\n",
		avatar->name,data->rooms[actRoom].name,
		data->rooms[actRoom].description);
}

/**Prints the the avatar's features. i.e. HP, name, profession*/
static void printAvatarFeatures( avatarType *avatar, gameDataType *data)
{
	printf("\nCaracteristicas de su Avatar:\n");
	printf("Nombre Avatar: %s\nProfesion: %s\nHP:%d\n",\
avatar->name,data->professions[avatar->ID].name,avatar->HP);
}

/** Calls the function readAvatarName while the name inserted is not valid.
 * If there is a memory error, exits*/
static void asignAvatarName ( avatarType *avatar )
{
	int readValue=0,FLAG=0;
	char auxName[33]; /* 32 plus the space for the null terminated character*/
	printf("\nIngrese el nombre de su avatar:\n");
	do /* asks for the name till it is okey*/
	{
		if ( FLAG)
			printf("Ingrese un nombre valido:\n");
		readValue=readAvatarName(avatar,auxName);
		FLAG=1;
	}
	while ( readValue==0 );
	if ( nameToStruct(avatar, auxName) )
	{
		printf("Error de Memoria\n");
		exit(1);
	}
}

/** Reads the avatar's name Returns 0 if there was an error*/
static int readAvatarName( avatarType *avatar, char *auxName)
/*Validates if the name is correct and returns it in auxName*/
{
	int i=0,ERROR=0;
	int c;
	while ( (c=getchar())!=EOF && c!='\n' && !ERROR && (i<32) )
	{
		if ( c > LIMIT_ENGLISH ) /*Has to be in the english alphabet*/
			ERROR=1;
		auxName[i]=c;
		++i;
	}
	auxName[i]=0;
	if ( i==0 && c=='\n') /*A name with an enter only it's not possible*/
		ERROR=1;
	else if ( c!='\n') /*This is because if not, stays waiting for the enter*/
		CLEAN_BUFFER;
	return !ERROR;
}

/** Calls functions needed to define the avatar's features*/
static void defineAvatar( avatarType *avatar, gameDataType *data )
{
		asignAvatarName(avatar);
		printProfessionsMenu(data);
		chooseProfession(avatar,data);
		defineHP(avatar,data);/*asigning inicial room to actual room*/
}


/**Prints the welcome Menu. Depending on the option chosen, prints a
 * specific message.
 * 
 * Returns 1 if option chosen was play*/
static int welcomeMenu( avatarType *avatar, gameDataType *data)
{
	int chosenOption, scanfValue,PLAY=0,FLAG=0;
	printf("\n1. Jugar\n");
	printf("2. Instrucciones\n");
	printf("3. Como Jugar\n");
	printf("4. Creditos\n");
	printf("5. Salir\n\n");
	printf("Opcion:\n");
	do
	{
		if (FLAG)
			printf("Ingrese una opcion valida\n");
		scanfValue=scanf("%d",&chosenOption);
		CLEAN_BUFFER;
		FLAG=1;
	} while( !(scanfValue==1 && chosenOption >0 && chosenOption <=5 ));
	switch (chosenOption)
	{
		case 1: play(avatar,data);PLAY=1;break;
		case 2: printf("\nDungeon Crawler consiste en un juego de rol,\
 donde debes tratar de llegar a la habitacion de salida,\
 pero en el camino te toparas con enemigos que intentaran\
 impedirtelo, atacandote y disminuyendo tu vida. Ten cuidado de no\
 quedarte sin salida, ya que perderas. Cuando tu vida este muy\
 baja, el mago misterioso podra aparecer tres veces para ayudarte, pero\
 solo si respondes lo que necesita. Tambien, en las habitaciones podras\
 encontrar objetos, ya sean pociones o armas.\n");break;
		case 3: printf("\nPara jugar sigue las instrucciones del juego\
 utilizando el teclado, y para continuar presione la tecla ENTER.\
 Cuando elijes una puerta puedes guardar la partida escribiendo\
 \"dumpActions\" + el nombre del archivo donde quieres guardar.\n");
 				break;
		case 4: printf("\nDungeon Crawler fue desarrollado por:\nMatias de\
 Santi\nEsteban Pintos\nAgustin Scigliano\n");break;	
		case 5: exit(0);break;
	}
	if ( PLAY )
		return 1;
	else 
		return 0;
}

/** prints a welcome message when the game starts*/
static void printTitle()
{
	printf(" _____     __  __     __   __     ______     ______     ____\
__     __   __    \n");
	printf("/\\  __-.  /\\ \\/\\ \\   /\\ \"-.\\ \\   /\\  ___\\   /\\  \
___\\   /\\  __ \\   /\\ \"-.\\ \\   \n");
	printf("\\ \\ \\/\\ \\ \\ \\ \\_\\ \\  \\ \\ \\-.  \\  \\ \\ \\__ \\\
  \\ \\  __\\   \\ \\ \\/\\ \\  \\ \\ \\-.  \\  \n");
	printf(" \\ \\____-  \\ \\_____\\  \\ \\_\\\\\"\\_\\  \\ \\_____\\  \
\\ \\_____\\  \\ \\_____\\  \\ \\_\\\\\"\\_\\ \n");
	printf("  \\/____/   \\/_____/   \\/_/ \\/_/   \\/_____/   \\/_____/\
   \\/_____/   \\/_/ \\/_/\n");
                             
	printf(" ______     ______     ______     __     __     __         _\
_____     ______    \n");
	printf("/\\  ___\\   /\\  == \\   /\\  __ \\   /\\ \\  _ \\ \\   /\\\
 \\       /\\  ___\\   /\\  == \\   \n");
	printf("\\ \\ \\____  \\ \\  __<   \\ \\  __ \\  \\ \\ \\/ \".\\ \\ \
 \\ \\ \\____  \\ \\  __\\   \\ \\  __<   \n");
	printf(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_\\ \\_\\  \\ \\__/\".~\
\\_\\  \\ \\_____\\  \\ \\_____\\  \\ \\_\\ \\_\\ \n");
	printf("  \\/_____/   \\/_/ /_/   \\/_/\\/_/   \\/_/   \\/_/   \\/__\
___/   \\/_____/   \\/_/ /_/\n");
}

/** Prints all professions and their HP and DP*/
static void printProfessionsMenu( gameDataType * data )
{
	int i;
	printf("\nProfesiones disponibles:\n");
	for (i = 0; i < (data->professionQty) ; i++) 
	{
		printf("\nProfesion %d: %s\t\tHP=%d/%d\tDP=%d/%d\n",i+1,
			data->professions[i].name, data->professions[i].MinHP,
			data->professions[i].MaxHP,data->professions[i].MinDP,
			data->professions[i].MaxDP);
	}
}

/** If a treasure is found, this function prints a message telling the
 * player how will he be affected*/
static void printTreasureFound( avatarType *avatar, gameDataType *data)
{
	int whatFound;
	whatFound=treasure (avatar,data);
	switch(whatFound)
	{
		case -1:printf("Has abierto un cofre que tenia una poción\
 veneno, tu HP disminuyo a %d.\n\n",avatar->HP);break;
		case 0:printf("Has abierto un cofre que tenia una nueva arma\
, tu ataque maximo aumento a %d.\n\n",
				data->professions[avatar->ID].MaxDP);break;
		case 1:printf("Has abierto un cofre que tenia una pocion\
 curativa, tu HP aumento a %d.\n\n",avatar->HP);break;
	}
}


/** If the HP falls below a certain percentage of the max value, a magician
 * appaears and tries to help the player. This function prints the messages
 * to the user*/
static void printQuestion ( gameDataType *data, avatarType *avatar)
{
	int chosenAnswer,FLAG=0,scanfValue;
	printf("Un mago misterioso a venido a curarte, pero para esto debes\
 responder una simple pregunta, ¿Cuantas habitaciones has recorrido?: ");
	do
	{
		if (FLAG)
			printf("Ingresa una opcion valida:\n");
		scanfValue=scanf("%d",&chosenAnswer);
		CLEAN_BUFFER;
		FLAG=1;
	}
	while ( scanfValue !=1 );
	if ( chosenAnswer == data->dumpActions.visitedRoomsQty+1 )
	{
		avatar->HP+=(avatar->HP)*0.5;
		printf("\nRespuesta correcta!, tu HP subio a %d\n",avatar->HP);
	}
	else
		printf("\nRespuesta incorrecta! El mago solo aparecera 2 veces"
		"mas en todo el juego, a si que no desaproveches la proxima"
		"oportunidad\n");
}
