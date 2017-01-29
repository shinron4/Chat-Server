#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define MAXLEN 1024                   //Maximum length of the array defined.

int fd;
pthread_t tid;

void handler(int pid){
	write(fd, "-q", 3);
	close(fd);
	exit(1);
}

void *serverResponse(void *ptr){
	int n;
	char buffer[MAXLEN];
	while((n = read(fd, buffer, MAXLEN)) > 0){
		write(1, "\n", 2);
		write(1, buffer, n);
		write(1, "\n> ", 4);
	}
	return 0;
}

int main(int argc, char *argv[]){
	int  in_file, i = 0, id;
	struct sockaddr_in ser_addr;     //Address of the server.
	struct hostent *ser_addr_attr;   //Attributes of the server name.
	char buffer[MAXLEN], name[15];   			
	signal(SIGQUIT, handler);

	 //Address of the server not provided.
	if(argc < 3){
		printf("error : server address not provided\n");
		exit(1);
	}

	//Resolving the hostname.
	if(!(ser_addr_attr = gethostbyname(argv[1]))){
		printf("error : hostname can't be resolved\n");
		exit(1);
	}

	//Creating the socket.
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("error");
		exit(1);
	}

	//Filling the server address structure.
	bzero((char *)&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[2]));
	bcopy(ser_addr_attr->h_addr, (char *)&ser_addr.sin_addr.s_addr, ser_addr_attr->h_length);
	
	//Connecting to the server.
	if(connect(fd, &ser_addr, sizeof(ser_addr)) < 0){
		perror("error");
		exit(1);
	}

	//Reading the reply.
	if((i = read(fd, buffer, MAXLEN)) < 0){
		perror("error");
		exit(1);
	}

	//Client limit exceeds.
	if(buffer[0] == 'e'){
		write(1, "error : client limit exceed\n", 29);
		exit(1);
	}

	//Client successfully connected
	else if(buffer[0] == 'h'){
		write(1, &buffer[2], strlen(buffer) - 1);
		sscanf(&buffer[2], "%d%s", &id, name);
	}

	//Start the client.
	write(1, "> ", 3);
	pthread_create(&tid, NULL, &serverResponse, NULL);
	while((i = read(0, buffer, MAXLEN)) > 0){
		int j = 1;
		char req[MAXLEN], user[MAXLEN], k = 0;
		buffer[i - 1] = '\0';
		switch(buffer[0]){

			//Query command.
			case 'l': while((buffer[j] == ' ' || buffer[j] == '\t'))j++;
					  if(buffer[j] == '\0')
					  	sprintf(req, "-l");
					  else if(buffer[j] != '-' ){
					  	write(1, "l : Invalid format\n", 20);
					  	break;
					  }
					  else{
					  	j++;
					  	while(isalnum(user[k] = buffer[j]))j++, k++;
					  	user[k] = '\0';
					  	if((buffer[j] != ' ' && buffer[j] != '\t' && buffer[j] != '\0') || k > 10 || k == 0){
					  		write(1, "l : Invalid user\n", 18);
					  		break;
					  	}
					  	else if(buffer[j] == ' ' || buffer[j] == '\t')
					  		while((buffer[j] == ' ' || buffer[j] == '\t'))j++;
					  		if(buffer[j] != '\0'){
							  	write(1, "l : Invalid format\n", 20);
							  	break;
						 	}
					  	sprintf(req, "-l %s", user);
					  }
					  write(fd, req, strlen(req) + 1);
					  break;

			//Sending message command.
			case 'm': while(buffer[j] == ' ' || buffer[j] == '\t')j++;
					  if(buffer[j] != '-'){
					  	write(1, "m : Invalid format\n", 20);
					  	break;
					  }
					  j++;
					  while(isalnum(user[k] = buffer[j]))j++, k++;
					  user[k] = '\0';
					  if((buffer[j] != ' ' && buffer[j] != '\t' && buffer[j] != '\0') || k > 10 || k == 0){
					  	write(1, "m : Invalid user\n", 18);
					  	break;
					  }
					  while(buffer[j] == ' ' || buffer[j] == '\t')j++;
					  if(buffer[j] == '\0'){
					  	write(1, "m : No message\n", 16);
					    break;
					  }
					  k = 0;
					  while(user[k] != '\0' && isdigit(user[k])) k++;
					  if(user[k] == '\0'){
					  	if(atoi(user) == id){
					  		write(1, "error : Sender and receiver same\n", 34);
							break;
					  	}
					  	sprintf(req, "-m 1 %s %s %s", name, user, &buffer[j]);
					  }
					  else{
					  	if(!strcmp(user, name)){
					  		write(1, "error : Sender and receiver same\n", 34);
							break;
					  	}
					  	sprintf(req, "-m 0 %s %s %s", name, user, &buffer[j]);
					  }
					  write(fd, req, strlen(req) + 1);
					  break;

			//Not a valid command.
			default : write(1, "error : Invalid command\n", 25);
					  break;
		}
		write(1, "> ", 3);
	}
	return 0;
}