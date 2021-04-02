#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "colors.h"

#define PORT 80

#define VERSION "Version: 0.4 \nDate: 2021.03.28 \nAuthor: Tarr Marton"
#define HELP "If a file-name is not specified as an arg, you will have to find it in a directory.\n\
Typing the name of a directory works the same as 'cd <directory>.\n\
Typing 'back' will act as 'cd..'.\n"


//second
char* Unwrap(char *Pbuff, int NumCh){
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
    for (int i = 0; i < (NumCh - 1) * 3; i+=3) //NumCh ha kell \n
    {
        character = ((Pbuff[i] & mask)<<6) | ((Pbuff[i+1] & mask)<<3) | ((Pbuff[i + 2] & mask));
        letters[j] = character;
        j++;
    }

    //terminate the string
    //letters[NumCh-1] = '\n';
    
    return letters;
}

char* ReadPixels(int fd, int* numCh)
{
    char info[54];
    
    read(fd, info, 54*sizeof(char));

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int offset = *(int*)&info[10];
    int text_length = *(int*)&info[6];

    *numCh = text_length;
    printf("NumCh:%d\n", text_length);

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
					printf("%s%s%s\t",BLU, (*entry).d_name, WHT);
					break;
				case S_IFREG:
					printf("%s%s%s \t",RED,(*entry).d_name, WHT);
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


int Post(char* neptunID, char* message, int NumCh){
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
  	printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	

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
    
    printf("%s\nlength:%d\ntime:%s", messageform, length, time);

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


int main(int argc, char* argv[]){
    char c;
    int file = 0;
    char* extension;
    int num = 0;
    int response;
    
    if(argc == 2){
    	extension = strrchr(argv[1], '.');
    }

    if(argc == 1){
        file = BrowseForOpen();
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
		printf("%s\n",hidden_text);
		free(pixel_array);
		free(hidden_text);
    }
    else if(argc == 2 && strcmp(argv[1],"--version") == 0){
        printf("%s\n", VERSION);
        exit(1);
    }
    else if(argc == 2 && strcmp(argv[1], "--help") == 0){
        printf("%s\n", HELP);
        exit(1);
    }
    else if(argc == 2 && strcmp(extension + 1, "bmp") == 0){
        file = open(argv[1], O_RDONLY);
        if(file == 0){
        	fprintf(stderr, "File could not be opened. The program will now exit.\n");
        	exit(1);
        }
        char* pixel_array = ReadPixels(file, &num);
    	char* hidden_text = Unwrap(pixel_array, num);
        printf("%s\n",hidden_text);
        /*
		response = Post("G8R7ZQ", hidden_text, num);
		if(response == 0) printf("Post successful! Text sent.\n");
		else if(response != 0) fprintf(stderr, "Text could not be posted!\n");
        */
		free(pixel_array);
		free(hidden_text);
    }

    return EXIT_SUCCESS;
}
