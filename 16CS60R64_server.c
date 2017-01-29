#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define MAXLEN 1024

struct clientInfo{
	char name[10], time[50];
	int id, online, fd;
};

struct sharedmem_Info{
	struct clientInfo *CI;
	sem_t *lock;
	int *len;
	int idci, idl, idlen;
}shm_cli;

struct messageBuffer{
	int f;
	char from[10], to[10];
	char message[MAXLEN];
};

struct sharedmem_Message{
	struct messageBuffer *Q;
	int *head, *tail;
	sem_t *lock;
	int idq, idh, idt, idl;
}shm_mbuf;

char buffer[MAXLEN]; 	//Buffer defined for storing the data temporarily.

time_t startTime;
pthread_t tid;

//handler function of SIGKILL signal for deallocating the shared memory.
void handler(int pid){
	shmdt(shm_cli.CI);
	shmdt(shm_cli.lock);
	shmdt(shm_cli.len);
	shmdt(shm_mbuf.Q);
	shmdt(shm_mbuf.head);
	shmdt(shm_mbuf.tail);
	shmdt(shm_mbuf.lock);
}

//Thread for handling the message QUEUE.
void *messageQueue(void *ptr){
	struct messageBuffer m;
	char res[MAXLEN + 15];
	int i, dequeue(struct messageBuffer *);
	int logFile = creat("log.txt", 0666);
	char log[MAXLEN];
	while(1){
		sem_wait(shm_mbuf.lock);
		if(dequeue(&m) != -1){
			for(i = 0; i < 5; i++){
				if(shm_cli.CI[i].online && (m.f ? atoi(m.to) == shm_cli.CI[i].id : !strcmp(shm_cli.CI[i].name, m.to))){

					//Sending the message to the receiver

					sprintf(res, "%s : %s", m.from, m.message);
					write(shm_cli.CI[i].fd, res, strlen(res) +1);

					//Writing the information about the message into the log file.
					if(strcmp(m.from, "server")){
						sprintf(log, "%s %s %d %s\n", m.from, shm_cli.CI[i].name, strlen(m.message), m.message);
						write(logFile, log, strlen(log));
					}
					break;
				}
			}
			if(i == 5){

				//The receiver is not online.
				sprintf(res, "Can't send message to : [%s]", m.to);
				for(i = 0; i < 5; i++)
					if(shm_cli.CI[i].online && !strcmp(shm_cli.CI[i].name, m.from)){
						write(shm_cli.CI[i].fd, res, strlen(res) + 1);
					}
			}
		}
		sem_post(shm_mbuf.lock);
	}
	close(logFile);
	return 0;
}

int main(int argc, char *argv[]){
	struct sockaddr_in ser_addr, cli_addr;  //Server and client address structures variables.
	int fds, fdc, clilen, j = 0, len, foffline, conns = 0, i;

	//Checking for port number of the server.
	if(argc < 2){
		printf("error : port number not provided\n");
		exit(1);
	}

	//Creating the socket.
	if((fds = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("error");
		exit(1);
	}

	//Filling the address structure of the server.
	bzero((char *)&ser_addr, sizeof(ser_addr));
	bzero((char *)&cli_addr, sizeof(cli_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[1]));
	ser_addr.sin_addr.s_addr = INADDR_ANY;

	//Binding the socket with the server address.
	if(bind(fds, &ser_addr, sizeof(ser_addr))){
		perror("error");
		exit(1);
	}

	//Getting and adding the shared memory for the client info.
	if((shm_cli.idci = shmget(IPC_PRIVATE, 5 * sizeof(struct clientInfo), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_cli.CI = shmat(shm_cli.idci, NULL, 0)) == (struct clientInfo *)-1){
		perror("error");
		exit(1);
	}
	if((shm_cli.idl = shmget(IPC_PRIVATE, sizeof(int), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_cli.lock = shmat(shm_cli.idl, NULL, 0)) == (sem_t *)-1){
		perror("error");
		exit(1);
	}
	if((shm_cli.idlen = shmget(IPC_PRIVATE, sizeof(int), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_cli.len = shmat(shm_cli.idlen, NULL, 0)) == (int *)-1){
		perror("error");
		exit(1);
	}
	//Initializing the shared memory
	bzero((char *)shm_cli.CI, 5 * sizeof(struct clientInfo));
	*shm_cli.len = 0;
	sem_init(shm_cli.lock, 1, 1);

	//Preparing to detatching the shared memory.
	shmctl(shm_cli.idci, IPC_RMID, 0);
	shmctl(shm_cli.idl, IPC_RMID, 0);
	shmctl(shm_cli.idlen, IPC_RMID, 0);

	//Getting and adding shared memory for the message buffer.
	if((shm_mbuf.idq = shmget(IPC_PRIVATE, 20 * sizeof(struct messageBuffer), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.Q = shmat(shm_mbuf.idq, NULL, 0)) == (struct messageBuffer *)-1){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.idl = shmget(IPC_PRIVATE, sizeof(int), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.lock = shmat(shm_mbuf.idl, NULL, 0)) == (sem_t *)-1){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.idh = shmget(IPC_PRIVATE, sizeof(int), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.head = shmat(shm_mbuf.idh, NULL, 0)) == (int *)-1){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.idt = shmget(IPC_PRIVATE, sizeof(int), 0666)) < 0){
		perror("error");
		exit(1);
	}
	if((shm_mbuf.tail = shmat(shm_mbuf.idt, NULL, 0)) == (int *)-1){
		perror("error");
		exit(1);
	}
	//Initializing the shared memory
	bzero((char *)shm_mbuf.Q, 20 * sizeof(struct messageBuffer));
	*shm_mbuf.head = *shm_mbuf.tail = 0;
	sem_init(shm_mbuf.lock, 1, 1);

	//Preparing to detatching the shared memory.
	shmctl(shm_mbuf.idq, IPC_RMID, 0);
	shmctl(shm_mbuf.idl, IPC_RMID, 0);
	shmctl(shm_mbuf.idh, IPC_RMID, 0);
	shmctl(shm_mbuf.idt, IPC_RMID, 0);
	//Initializing the handler.
	signal(SIGKILL, handler);

	//Starting the listening process of the server and checking for queue length.
	if(listen(fds, argc > 2 ? atoi(argv[2]) : 10)){
		perror("error");
		exit(1);
	}

	//Thread serving the message buffer.
	pthread_create(&tid, NULL, &messageQueue, NULL);

	srand(time(NULL));
	//Infinite listening process.
	while(1){
		
		//Accepting the connection from the client.
		if((fdc = accept(fds, &cli_addr, (unsigned int *)&clilen)) == -1){
			perror("error");
			exit(1);
		}

		sem_wait(shm_cli.lock);
		//Reject If client limit exceeds.
		if(*shm_cli.len == 5){
			write(fdc, "e", 2);
			close(fdc);
			sem_post(shm_cli.lock);
			continue;
		}

		for(i = 0; i < 5; i++)
			if(!shm_cli.CI[i].online){
				foffline = i;
				break;
			}

		if(fork() > 0){

			//Filling the client Details in the shared memory.
			sprintf(shm_cli.CI[foffline].name, "client%d", conns++);
			//write(1, shm_cli.CI[foffline].name, 10);
			shm_cli.CI[foffline].id = rand() % 100000;
			shm_cli.CI[foffline].fd = fdc;
			time(&startTime);
			strcpy(shm_cli.CI[foffline].time, asctime(localtime(&startTime)));
			shm_cli.CI[foffline].online = 1;
			*shm_cli.len += 1;
			sprintf(buffer, "h-%d %s %s", shm_cli.CI[foffline].id, shm_cli.CI[foffline].name, shm_cli.CI[foffline].time);
	
			//Null reply on successful Completion.
			write(fdc, buffer, strlen(buffer) + 1);
			sem_post(shm_cli.lock);
		}

		//Forking a new process for serving the client.
		else{
			int i, n, enqueue(struct messageBuffer);
			char *p;
			struct messageBuffer m;
			while((n = read(fdc, buffer, MAXLEN)) > 0){
				p = buffer;
				switch(buffer[1]){

					//Client is quiting.
					case 'q':sem_wait(shm_cli.lock);
							 shm_cli.CI[foffline].online = 0;
							 *shm_cli.len -= 1;
			 				 sprintf(p, "%d : %s gone offline.", shm_cli.CI[foffline].id, shm_cli.CI[foffline].name);
							 sem_wait(shm_mbuf.lock);
							 m.f = 0;
							 strcpy(m.from, "server");
							 for(i = 0; i < 5; i++)
							 	if(shm_cli.CI[i].online){
									strcpy(m.to, shm_cli.CI[i].name);
									strcpy(m.message, p);
									enqueue(m);
								}
							 sem_post(shm_mbuf.lock);
							 sem_post(shm_cli.lock);
							 exit(0);
							 break;

					//Client sends the query to know which clients are online.
					case 'l':sem_wait(shm_cli.lock);
							 if(strlen(&buffer[2]) == 0){
								for(i = 0; i < 5; i++)
									if(shm_cli.CI[i].online){
										sprintf(p , "%d : %s\n", shm_cli.CI[i].id, shm_cli.CI[i].name);
										p += strlen(p);
									}
							 }
							 else{
							 	for(i = 0; i < 5; i++)
							 		if(!strcmp(shm_cli.CI[i].name, &buffer[3]) && shm_cli.CI[i].online){
							 			sprintf(p, "%d : %s\n", shm_cli.CI[i].id, shm_cli.CI[i].name);
							 			p += strlen(p);
							 			break;
							 		}
							 }
							 *p = '\0';
							 write(fdc, buffer, strlen(buffer) + 1);
							 sem_post(shm_cli.lock);
							 break;

					//Sending message to the other clients. 
					case 'm':sscanf(&buffer[3], "%d%s%s%[^\n]", &m.f, m.from, m.to, m.message);
							 sem_wait(shm_mbuf.lock);
							 if(enqueue(m) == -1){
								sprintf(buffer, "Message queue full");
							 	write(fdc, buffer, strlen(buffer) + 1);
							 }
							 sem_post(shm_mbuf.lock);
							 break;
				}
			}
		}
	}
	return 0;
} 

int enqueue(struct messageBuffer e){
	if(*shm_mbuf.tail - *shm_mbuf.head == MAXLEN)
		return -1;
	shm_mbuf.Q[*shm_mbuf.tail].f = e.f;
	strcpy(shm_mbuf.Q[*shm_mbuf.tail].from, e.from);
	strcpy(shm_mbuf.Q[*shm_mbuf.tail].to, e.to);
	strcpy(shm_mbuf.Q[*shm_mbuf.tail].message, e.message);
	(*shm_mbuf.tail) += 1; 
	return 0;
}

int dequeue(struct messageBuffer *x){
	if(*shm_mbuf.head == *shm_mbuf.tail)
		return -1;
	x->f = shm_mbuf.Q[*shm_mbuf.head].f;
	strcpy(x->from, shm_mbuf.Q[*shm_mbuf.head].from);
	strcpy(x->to, shm_mbuf.Q[*shm_mbuf.head].to);
	strcpy(x->message, shm_mbuf.Q[*shm_mbuf.head].message);
	(*shm_mbuf.head) += 1;
	if(*shm_mbuf.head == *shm_mbuf.tail)
		*shm_mbuf.head = *shm_mbuf.tail = 0;
	return 0;
}