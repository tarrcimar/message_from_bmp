#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
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

#define VERSION "Version: 1.0 \nDate: 2021.04.19 \nAuthor: Tarr Marton"
#define HELP "If a file-name is not specified as an arg, you will have to find it in a directory using a command line interface.\n\
Typing the name of a directory works the same as 'cd <directory>.\n\
Typing 'back' will act as 'cd..'.\n"


char* Unwrap(char *Pbuff, int NumCh){

	if(NumCh == 0){
		free(Pbuff);
    	fprintf(stderr, "No hidden text. Make sure you chose the appropriate file!\n");
    	exit(3);
    }

    char *letters = malloc((NumCh + 1) * sizeof(char));
    char *finalletters = malloc(NumCh * sizeof(char));

    //Check for Failure
    if(letters == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(4);
    }

    //set the mask
    int mask = 0b00000111;
    int character;
    int i;

    
    //Decode the rgb elements from the array
    //on all available threads
    #pragma omp parallel for shared(Pbuff) private(i)
		for (i = 0; i < (NumCh * 3);i+=3) 
		{
			character = ((Pbuff[i] & mask)<<6) | ((Pbuff[i+1] & mask)<<3) | ((Pbuff[i + 2] & mask));
			letters[i/3] = character;
		}


	//Double check the length

	for(int i = 0; i < NumCh; i++){
		finalletters[i] = letters[i];
	}

	
    
    return finalletters;
}

char* ReadPixels(int fd, int* numCh)
{
    char info[54]; //length of header(14) + InfoHeader(40) in bytes

    //sleep(2); //testing catching interrupt signal
    
    read(fd, info, 54*sizeof(char));

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int offset = *(int*)&info[10];
    int text_length = *(int*)&info[6];

    *numCh = text_length;

    //calculate the padding
    int padded_row = (width*3 + 3) & (~3); 
    int size = padded_row * height;

    char* data = malloc(size * sizeof(char));

    if(data == NULL){
        fprintf(stderr, "Memory could not be allocated! Program will exit..");
        exit(4);
    }

    read(fd, data, size * sizeof(char)); //read all of the pixel array
    
    return data;
}

int BrowseForOpen(){
	DIR* dir;
	struct dirent* entry;
	struct stat inode;
	char tmp[PATH_MAX];
	char tmp_pw[PATH_MAX];
	int regularfile;
	char* extension;
	
	
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
					printf("%s%s%s \t",UCYN,(*entry).d_name, WHT);
					break;	
			}
		}
		
		//Scan the path that should be opened
		printf("\nWhat do you want to open?\n");
		scanf("%s", tmp);
		extension = strrchr(tmp, '.');

		getcwd(tmp_pw, sizeof(tmp_pw)); //get the current directory

		char* ptr = strrchr(tmp_pw, '/'); //last occurence of '/' in string
		int size = (ptr-tmp_pw);
		char* pw_dir = malloc(size * sizeof(char));
		if(strlen(pw_dir) == 0) strcat(pw_dir, "/");
		
		strncpy(pw_dir, tmp_pw, size); // copy the appropriate length to pw_dir
		
		//if back is written, execute "cd .."
		if(strcmp(tmp, "back") == 0){
			chdir(pw_dir);
		}
		else{
			while(chdir(tmp) != 0){
				stat(tmp, &path_stat);
				//If not a directory, check if it's a regular file, if yes then open and return
    			if(S_ISREG(path_stat.st_mode) == 1){
    				if(strcmp(extension, "bmp") == 0){
    					regularfile = open(tmp, O_RDONLY);
						return regularfile;	
    				}
    				else{
    					fprintf(stderr, "Wrong file format!");
    					exit(9);
    				}
					
    			}
    			//If the input does not exist, ask for another input
				printf("\nDirectory or file does not exist. Choose from the list above!\n");
				scanf("%s", tmp);
			}
		}

		dir = opendir(".");
	}

	closedir(dir);
	
	return 0;
}


int Post(char* neptunID, char* message, int NumCh){
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	

	char* host = "irh.inf.unideb.hu";
	char* message_format = "POST /~vargai/post.php HTTP/1.1\r\n" //message format to be filled in
						"Host: %s\r\n"
						"Content-Length: %d\r\n"
						"Content-Type: application/x-www-form-urlencoded\r\n\r\n"
						"NeptunID=%s&PostedText=%s\r\n"; // full post payload
	
	
    struct sockaddr_in server;
    //Fill the struct
    server.sin_addr.s_addr = inet_addr("193.6.135.162");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);


    int sock;
    char message_to_send[4096],response[4096], time[1024];

	
    //Create the timestamp
    sprintf(time, "%d.%02d.%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int length = strlen(time) + strlen(neptunID) + NumCh + 1; // length of date string, neptunID and the message
    //Create the full message
    sprintf(message_to_send,message_format,host,length,neptunID,message); 
    

    //Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0){
    	fprintf(stderr, "Error opening socket");
    	exit(7);
    }


    char on = 1;
    //Fill socket options
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));


    //Try to connect the socket
    if (connect(sock,(struct sockaddr *)&server,sizeof(server)) < 0){
        fprintf(stderr, "Error connecting");
        exit(8);
    }

  	send(sock, message_to_send, sizeof(message_to_send), 0);
  	recv(sock, response, sizeof(response), 0);
  	close(sock);
  	
  	if(strstr(response, "The message has been received.") != NULL) return 0;
  	else return 6;
  	
}


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