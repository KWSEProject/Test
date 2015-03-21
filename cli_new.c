#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_BUF 1024

int cmd_srv;		// for command connection
int data_cli, data_srv;	// for data connection

void convert_addr_to_str(char*, unsigned int, char*);
void My_itoa(int, char*);

int main(int argc, char *argv[])
{
	int clilen, len;			// length of client		
	char *hostport = NULL;		// address & port string
	char buf[MAX_BUF];	// buffer
	char c_len[16];
	unsigned int port;		// random port
	socklen_t addr_size;	// check address size
	struct sockaddr_in cmd_srv_addr;	// struct for command connection
	struct sockaddr_in data_srv_addr, data_cli_addr;
	// structs for data connection

	memset(buf, 0, sizeof(buf));
	memset(&cmd_srv_addr, 0, sizeof(cmd_srv_addr));
	memset(&data_srv_addr, 0, sizeof(data_srv_addr));
	memset(&data_cli_addr, 0, sizeof(data_cli_addr));

	/* command connection */
	if((cmd_srv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		write(STDOUT_FILENO, "socket() err.\n", 14);
		exit(1);
	}

	// initialize command connection's struct
	cmd_srv_addr.sin_family = AF_INET;
	cmd_srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	cmd_srv_addr.sin_port = htons(atoi(argv[2]));

	// connect
	connect(cmd_srv, (struct sockaddr*)&cmd_srv_addr, sizeof(cmd_srv_addr));

	read(STDIN_FILENO, buf, sizeof(buf));	// read instruction


	/* data connection */
	data_srv = socket(AF_INET, SOCK_STREAM, 0);	// create socket
	
	data_srv_addr.sin_family = AF_INET;
	data_srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	data_srv_addr.sin_port = 0;

	// bind
	bind(data_srv, (struct sockaddr*)&data_srv_addr, sizeof(data_srv_addr));

	// get port number which kernel give
	addr_size = sizeof(data_srv_addr);
	getsockname(data_srv, (struct sockaddr*)&data_srv_addr, &addr_size);
	port = ntohs(data_srv_addr.sin_port);	// get random port

	/* converting & send inst, port */
	convert_addr_to_str("127.0.0.1", port, hostport);	// converting
	write(cmd_srv, hostport, strlen(hostport));	// send ip & port
	write(STDOUT_FILENO, "200 PORT command successful.\n", 29);

	// listen
	listen(data_srv, 5);

	// accept
	clilen = sizeof(data_cli_addr);
	data_cli = accept(data_srv, (struct sockaddr*)&data_cli_addr, &clilen);

	/* transport */
	write(data_cli, buf, sizeof(buf));	// send inst.
	memset(buf, 0, sizeof(buf));
	read(data_cli, buf, sizeof(buf));	// receive result
	write(STDOUT_FILENO, "150 ASCII mode data connection.\n", 32);
	write(STDOUT_FILENO, buf, sizeof(buf));	// show result

	len = strlen(buf);
	My_itoa(len, c_len);
	write(STDOUT_FILENO, "226 Transfer complete.\n". 22);
	write(STDOUT_FILENO, "OK. ", 4);
	write(STDOUT_FILENO, c_len, sizeof(len));
	write(STDOUT_FILENO, "bytes received.\n", 16);

	printf("leekayoung\n");
	/* close connection */
	close(cmd_srv);
	close(data_cli);
}

char* convert_addr_to_str(char* ip_addr, unsigned int port, char* str)
{
	char copy_ip[32] = {0,};	// copy ip address
	char *temp = NULL;		// for strtok
	int i, doub = 1;
	int copy_port;			// copy port
	int bin[16];			// binary
	int 1st_port = 0, 2nd_port = 0;	// 8bits port

	strcpy(copy_ip, ip_addr);	// copy original to array
	copy_port = port;		// copy port to copy_port

	/* strtok by "." & strcat by "," */
	temp = strtok(copy_ip, ".");
	strcat(str, temp);
	strcat(str, ",");
	for(i = 0; i < 3; i++)
	{
		temp = strtok(NULL, ".");
		strcat(str,temp);
		strcat(str, ",");
	}

	/* make binary port*/
	for(i = 0; i < 16; i++)
	{
		bin[i] = copy_port % 2;	// store rest of_port / 2
		copy_port /= 2;		// restore copy_port / 2 to copy_port
	}

	/* first 8bits port */
	for(i = 0; i < 8; i++)
	{
		1st_port += bin[i] * doub;
		doub *= 2;	// make twice for making decimal
	}

	/* second 8bits port */
	doub = 1;
	for(i = 8; i < 16; i++)
	{
		2nd_port += bin[i] * doub;
		doub *= 2;	// make twice for making decimal
	}

	/* using My_itoa & strcat by "," */
	My_itoa(2nd_port, temp);
	strcat(str, temp);
	strcat(str, ",");
	My_itoa(1st_port, temp);
	strcat(temp);
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
