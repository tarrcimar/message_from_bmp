#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <omp.h>

#include "colors.h"

#define PORT 80

#define VERSION "Version: 0.6 \nDate: 2021.04.16 \nAuthor: Tarr Marton"
#define HELP "If a file-name is not specified as an arg, you will have to find it in a directory.\n\
Typing the name of a directory works the same as 'cd <directory>.\n\
Typing 'back' will act as 'cd..'.\n"


/*
   Function: Unwrap

   Dekódolja a Parametersként kapott bmp fájl pixel array részét tartalmazó memóriacímét, valamint az üzenet hosszát.
   A tömb feldolgozása a rendelkezésre álló összes processzormagon párhuzamonsa, közel azonos terheléssel történik.

   Parameters:

   		Pbuff - a bmp fájl pixel array részét tartalmazó memóriacím
   		NumCh - a kódolt üzenet hossza

   Returns:

   		A dekódolt szöveg címe
*/
char* Unwrap(char *Pbuff, int NumCh){
	int NumProcs;
	int NumThreads;

	NumProcs = omp_get_num_procs();
	NumThreads = (NumProcs == 1)?(1):(NumProcs-1);


	if(NumCh == 0){
		free(Pbuff);
    	fprintf(stderr, "No hidden text. The program will now exit!\n");
    	exit(1);
    }
    char *letters = malloc((NumCh - 1) * sizeof(char)); // így nincs benne \n
    //Check for Failure
    if(letters == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }

    //set the mask
    int mask = 0b00000111;
    int character;
    int j = 0;

    
    //Decode the rgb elements from the array
    //on all available threads
    #pragma omp parallel for
		for (int i = 0; i < (NumCh - 1) * 3; i+=3) 
		{
			NumThreads = omp_get_thread_num();
		   	
			character = ((Pbuff[i] & mask)<<6) | ((Pbuff[i+1] & mask)<<3) | ((Pbuff[i + 2] & mask));
			letters[j] = character;
			j++;
		}

    
    return letters;
}


/*

	Function: ReadPixels

	Beolvassa egy tömörítetlen TrueColor bmp fájl tartalmát, majd a pixel array részét elhelyezi egy megfelelő méretű dinamikusan méretű tömbben.
	Amennyiben a folyamat 1 mp-nél tovább tartott, a program le fog állni.

	Parameters:

			fd - file descriptor
			NumCH - rejtett szöveg hossza

	Returns:

			A bmp file pixel array tartalmának címe

	See Also:

			<Unwrap>

*/
char* ReadPixels(int fd, int* NumCh)
{
    char info[54];

    //sleep(2); testing catching interrupt signal
    
    read(fd, info, 54*sizeof(char));

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int offset = *(int*)&info[10];
    int text_length = *(int*)&info[6];

    *NumCh = text_length;

    int padded_row =(width*3 + 3) & (~3); 
    int size = padded_row * height;

    char* data = malloc(size * sizeof(char));
    if(data == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(1);
    }
    //fread(data, sizeof(unsigned char), size, file); 
    read(fd, data, size * sizeof(char));
    
    return data;
}

/*
	Function: BrowseForOpen

	Karakteres felületű tallózás.
	Használata: Kiírja a parancssori felületre a felhasználó alapértelmezett könyvtárának tartalmát (a rejtett konyvtári objektumokat is).
		- A választott könyvtár nevének begépelése, majd az Enter billentyű megnyomása megnyitja a könyvtárat
		- a "back" parancs egyel vissza lép a könyvtár hierarchiában
		- amennyiben a bemenet egy reguláris fájl, azt megprobálja megnyitni

	Returns:

			A bináris olvasásra megnyitott fájl descriptor

*/
int BrowseForOpen(){
	DIR* dir;
	struct dirent* entry;
	struct stat inode;
	char tmp[PATH_MAX];
	char tmp_pw[PATH_MAX];
	int regularfile;
	char* extension;
	char* previous;
	
	
	dir = opendir("/home");
	chdir("/home");
	
	
	struct stat path_stat;
	
	while(1){
		while((entry = readdir(dir)) != NULL){
			stat((*entry).d_name, &inode);
			switch(inode.st_mode & S_IFMT){
				case S_IFDIR:
					printf("%s%s%s\t",BHGRN, (*entry).d_name, WHT);
					break;
				case S_IFREG:
					printf("%s%s%s \t",HWHT,(*entry).d_name, WHT);
					break;	
			}
		}
		
		//Scan the path that should be opened
		printf("\nWhat do you want to open?\n");
		scanf("%s", tmp);
		printf("tmp:%s\n",tmp);

		toupper(tmp[0]);
		printf("tmp:%s\n",tmp);

		getcwd(tmp_pw, sizeof(tmp_pw)); //get the current directory

		char* ptr = strrchr(tmp_pw, '/'); //last occurence of '/' in string
		int size = (ptr-tmp_pw);
		char* pw_dir = malloc(size * sizeof(char));
		if(strlen(pw_dir) == 0) strcat(pw_dir, "/");
		
		strncpy(pw_dir, tmp_pw, size); // copy the appropriate length to pw_dir
		
		//if back is written, execute "cd .."
		if(strcmp(tmp, "back") == 0){
			//chdir(pw);
			chdir(pw_dir);
		}
		else{
			while(chdir(tmp) != 0){
				stat(tmp, &path_stat);
				//if not a directory, check if it's a regular file, if yes then open and return
    			if(S_ISREG(path_stat.st_mode) == 1){
					regularfile = open(tmp, O_RDONLY);
					return regularfile;
    			}
    			//if the input does not exist, ask for another input
				printf("\nDirectory or file does not exist. Choose from the list above!\n");
				scanf("%s", tmp);
			}
		}
		dir = opendir(".");
	}
	closedir(dir);
	
	return 0;
}


/*
	Function: Post

	A dekódolt szöveget egy webszerverre továbbítja

	Parameters:

			neptunID - a hallgató neptun kódja
			message - a dekódolt üzenet
			NumCh - az üzenet hossza

	Returns:

			- 0 - amennyiben a webszerver fogadta az üzenetet
			- 2 - amennyiben nem fogadta, vagy hiba lépett fel
*/
int Post(char* neptunID, char* message, int NumCh){
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	

	int portno = 80;
	char* host = "irh.inf.unideb.hu";
	char* message_fmt = "POST /~vargai/post.php HTTP/1.1\r\n"
						"Host: %s\r\n"
						"Content-Length: %d\r\n"
						"Content-Type: application/x-www-form-urlencoded\r\n\r\n"
						"NeptunID=%s&PostedText=%s\r\n"; // full post payload
	
	
	struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char messageform[4096],response[4096], time[1024];

	
    /* fill in the parameters */
    sprintf(time, "%d.%02d.%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int length = strlen(time) + strlen(neptunID) + NumCh + 1; // length of date string, neptunID and the message
    sprintf(messageform,message_fmt,host,length,neptunID,message); 
    

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) fprintf(stderr, "ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) fprintf(stderr, "ERROR: no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = inet_addr("193.6.135.162");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        fprintf(stderr, "ERROR connecting");

  	send(sockfd, messageform, sizeof(messageform), 0);
  	recv(sockfd, response, sizeof(response), 0);
  	close(sockfd);
  	
  	if(strstr(response, "The message has been received.") != NULL) return 0;
  	else return 2;
  	
}

/*
	Function: WhatToDo

	SIGALARM és SIGINT szignálok kezelése

	Parameters:

			sign - a szignál száma

*/
void WhatToDo(int sign){
	pid_t pid;


	if(sign == SIGALRM){
		fprintf(stderr, "Execution timed out!\n");
        exit(5);
	}
	else if(sign == SIGINT){
		pid = fork();
		if(pid == 0){
			fprintf(stdout, "Execution cannot be stopped by 'Ctrl+C'!\n");
			kill(getpid(), SIGKILL);
		}
	}
}