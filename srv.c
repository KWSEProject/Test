#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>

#define MAX_BUF 1024

void convert_str_to_addr(char*, unsigned int, char*);

int main(int argc, char *argv[])
{
	int cmd_srv, cmd_cli;	// for command connection
	int data_srv;		// for data connection
	int clilen;		// length
	pid_t pid;		// variable for process ID
	unsigned int port;

	char buf[MAX_BUF];
	char ip_addr[20] = {0,};	// ip address

	struct sockaddr_in cmd_srv_addr, cmd_cli_addr;	// command connection's struct
	struct sockaddr_in data_srv_addr;	// data connection's struct

	/* for ls */
	DIR *dp;
	struct dirent *dirp;

	memset(&cmd_srv_addr, 0, sizeof(cmd_srv_addr));
	memset(&cmd_cli_addr, 0, sizeof(cmd_cli_addr));
	memset(&data_srv_addr, 0, sizeof(data_srv_addr));

	/* command connection */
	cmd_srv = socket(AF_INET, SOCK_STREAM, 0);

	// initialize struct
	cmd_srv_addr.sin_family = AF_INET;
	cmd_srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cmd_srv_addr.sin_port = htons(atoi(argv[1]));

	//bind
	bind(cmd_srv, (struct sockaddr*)&cmd_srv_addr, sizeof(cmd_srv_addr));

	//listen
	listen(cmd_srv, 5);

	while(1)
	{
		// accept
		clilen = sizeof(cmd_cli_addr);
		cmd_cli = accept(cmd_srv, (struct sockaddr*)&cmd_cli_addr,
					&clilen);
		
		if((pid = fork()) == 0)	// chile process
		{
			read(cmd_cli, buf, sizeof(buf));
			write(STDOUT_FILENO, "Client IP address & port: ");
			write(STDOUT_FILENO, buf, sizeof(buf));

			/* convert IP & port */
			convert_str_to_addr(buf, &port, ip_addr);

			/* data connection */
			data_srv = socket(AF_INET, SOCK_STREAM, 0);

			// initialize struct
			data_srv_addr.sin_family = AF_INET;
			data_srv_addr.sin_addr.s_addr = inet_addr(ip_addr);
			data_srv_addr.sin_port = htons(port);

			connect(data_srv, (struct sockaddr*)&data_srv_addr, sizeof(data_srv_addr));

			read(data_srv, buf, sizeof(buf));	// get inst.
			write(STDOUT_FILENO, "Client instruction: ");
			write(STDOUT_FILENO, buf, sizeof(buf));
			write(STDOUT_FILENO, "\n", 1);

			if(!strcmp(buf, "ls\n"))
			{
				dp = opendir(".");	// open current directory

				memset(buf, 0, sizeof(buf));
				while((dirp = readdir(dp)) != NULL)
				{
					strcat(buf, dirp->d_name);
					strcat(buf, "\n");
				}
				write(data_srv, buf, sizeof(buf));
			}
		}
		else
			continue;
		close(data_srv);
		close(cmd_cli);
	}
	printf("KWSEProject!\n");
	exit(0);
}

void convert_str_to_addr(char* str, unsigned int* port, char* ip_addr)
{
	char copy_str[32] = {0,};	// copy string
	char *temp = NULL;	// for strtok

	int 1st_port;
	int 2nd_port;
	int bin[16];
	int i, doub = 1;

	strcpy(copy_str, str);	// strcpy str to copy_str

	/* get ip address */
	temp = strtok(copy_str, ",");
	strcat(ip_addr, temp);
	strcat(ip_addr, ".");
	for(i = 0; i < 3; i++)
	{
		temp = strtok(NULL, ",");
		strcat(ip_addr, temp);
		if(i == 2)
			continue;
		else
			strcat(ip_addr, ".");
	}

	/* get 1st port */
	temp = strtok(NULL, ",");
	1st_port = atoi(temp);

	for(i = 8; i < 16; i++)
	{
		bin[i] = 1st_port % 2;
		1st_port /= 2;
	}

	/* get 2nd port */
	temp = strtok(NULL, "\n");
	2nd_port = atoi(temp);

	for(i = 0; i < 8; i++)
	{
		bin[i] = 2nd_port % 2;
		2nd_port /= 2;
	}

	/* get total port number */
	*port = 0;	// initialize port number
	for(i = 0; i < 16; i++)
	{
		*port += bin[i] * doub;
		doub *= 2;
	}
}

////////////////////////////////////////////////////////
// My_itoa
// -----------------------------------------------------
// Input : int - number, char - string
// Output : void
// Purpose : integer is changed string
////////////////////////////////////////////////////////
void My_itoa(int num, char* str)
{
	int i, j;
	char cp_str[MAX_BUF];		// copy string
	memset(cp_str, 0, sizeof(cp_str));

	for(i = 0; num > 0; i++)
	{
		j = num % 10;
		if(j >= 10)		// if rest is bigger than 10
			str[i] = (j - 10) + 'a';	// store j - 10 + 'a'
			// convert ASCII code
		else
			str[i] = j + '0';	// else, store j - 10 + '0'
		num /= 10;	// repeat
	}

	// copy
	i = 0;
	while(str[i] != 0)
	{
		cp_str[i] = str[i];
		i++;
	}
	cp_str[i] = 0;

	// reverse
	j = 0;
	while(str[j] != 0)
	{
		str[j] = cp_str[i-1];
		i--;
		j++;
	}
}
