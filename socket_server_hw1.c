#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

#define MAX_BUF_SIZE 2048

char clientip[32];
unsigned short clientport;

void getIPaddress(int newfd)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(newfd, (struct sockaddr *) (&addr), (&addr_size));
    if (res < 0) exit(-1);
    bzero(clientip, 32);
    strcpy(clientip, inet_ntoa(addr.sin_addr));
    clientport = addr.sin_port;
    return;
}

struct ecs36b_student
{
  char sname[128];
  char tID[32];
  char sID[32];
  char uID[32];
  char ssession[32];
};

int main(int argc, char *argv[])
{
  int listenfd = 0, connfd = 0, n = 0;
  struct sockaddr_in serv_addr; 

  char sendBuff[MAX_BUF_SIZE];
  time_t ticks; 

  if (argc != 5)
    {
      fprintf(stderr, "./%s server_name port master_file log_file\n", argv[0]);
      return 0;
    }
  FILE *ecs36b_f = fopen(argv[3], "r");
  if (ecs36b_f == NULL) return -1;

  FILE *log_f = fopen(argv[4], "a");
  if (log_f == NULL) return -1;

  char buf[1024];
  int flag1 = 1;

  struct ecs36b_student es[1024];
  int es_index = 0;

  bzero(es, sizeof(struct ecs36b_student) * 1024);

  while(flag1)
    {
      bzero(buf, 1024);
      int rc = fscanf(ecs36b_f, "%[^\n\r]", buf);

      if (rc == EOF)
	{
	  flag1 = 0; // leaving the loop
	}
      else
	{
	  char *buf_ptr = buf;

	  sscanf(buf_ptr, "%[^\t]", es[es_index].sname);
	  buf_ptr += (strlen(es[es_index].sname) + strlen("\t"));
	  sscanf(buf_ptr, "%[^\t]", es[es_index].tID);
	  buf_ptr += (strlen(es[es_index].tID) + strlen("\t"));
	  sscanf(buf_ptr, "%[^\t]", es[es_index].sID);
	  buf_ptr += (strlen(es[es_index].sID) + strlen("\t"));
	  sscanf(buf_ptr, "%[^\t]", es[es_index].uID);
	  buf_ptr += (strlen(es[es_index].uID) + strlen("\t"));
	  sscanf(buf_ptr, "%[^\t]", es[es_index].ssession);

	  fgetc(ecs36b_f); // get rid of end of line sumbol and get ready for next line
	  es_index++;
	}
    }
  printf("es_index = %d\n", es_index);
  fclose(ecs36b_f);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons((short) atoi(argv[2])); 

  bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 

  while(1)
    {
      connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
 
      ticks = time(NULL);
      bzero(sendBuff, MAX_BUF_SIZE);
      snprintf(sendBuff, sizeof(sendBuff), "Hi, this is %s -- %.24s\r\n",
	       argv[1], ctime(&ticks));
      write(connfd, sendBuff, strlen(sendBuff));
      getIPaddress(connfd);
      fprintf(stderr, "just sent an acknowledge to [%s %d]\n", clientip, clientport);

      char recvBuff[1024];
      if ((n = read(connfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
	  recvBuff[n] = 0;
	  fprintf(stderr, "got [%s]\n", recvBuff);
        }

      int i;
      for (i = 0; i < es_index; i++)
	{
	  if (strcmp(recvBuff, es[i].sID) == 0)
	    {
	      fprintf(stderr, "%s %s %s %s %s\n",
		      es[i].sname, es[i].tID, es[i].sID, es[i].uID, es[i].ssession);

	      ticks = time(NULL);
	      char buftime[64];
	      bzero(buftime, 64);
	      sprintf(buftime, "%s", ctime(&ticks));
	      buftime[strlen(buftime)] = '\0';

	      char snbuf[2048];
	      bzero(snbuf, 2048);
	      sprintf(snbuf,
		      "{\"status\": \"successful\", \"name\": %s, \"sID\": \"%s\", \"uID\": \"%s\", \"session\": \"%s\", \"time\": \"%.24s\", \"ip address\": \"%s\", \"port\": \"%d\", \"code\": \"%ld\"}",
		      es[i].sname, es[i].sID, es[i].uID, es[i].ssession, buftime,
		      clientip, clientport, random());

	      fprintf(log_f, "%s\n", snbuf);
	      printf("%s\n", snbuf);
	      snprintf(sendBuff, sizeof(sendBuff), "%s", snbuf);
	      write(connfd, sendBuff, strlen(sendBuff));
	      fprintf(log_f, "{\"status\": \"done\"}\n");
	      fflush(log_f);
	      sleep(1);
	      break;
	    }
	}

      if (i == es_index)
	{
	  ticks = time(NULL);
	  char buftime[64];
	  bzero(buftime, 64);
	  sprintf(buftime, "%s", ctime(&ticks));
	  buftime[strlen(buftime)] = '\0';

	  char snbuf[2048];
	  bzero(snbuf, 2048);
	  sprintf(snbuf,
		  "{\"status\": \"ID not found\", \"sID\": \"%s\", \"time\": \"%.24s\", \"ip address\": \"%s\", \"port\": \"%d\", \"code\": \"%ld\"}",
		  recvBuff, buftime, clientip, clientport, random());
	  fprintf(log_f, "%s\n", snbuf);
	  printf("%s\n", snbuf);
	  snprintf(sendBuff, sizeof(sendBuff), "%s", snbuf);
	  write(connfd, sendBuff, strlen(sendBuff));
	  fprintf(log_f, "{\"status\": \"done\"}\n");
	  fflush(log_f);
	  sleep(1);
	}
      close(connfd);
      sleep(1);
    }
}
