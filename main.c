/*
 * server.c
 *
 *  Created on: 27-Sep-2019
 *      Author: mdwalker
 */

#include "header.h"

typedef struct sockaddr SA;

int server();
int client();

int main(int argc, char **argv)
{
	return server();
}

int client()
{
	int sock;
	struct sockaddr_in server;
	char buffer[BUFSIZ], server_reply[BUFSIZ], HTML[MAX], temp[1000];

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	puts("Connected\n");

	//keep communicating with server

	while (1)
	{
		printf("Enter message : ");
		scanf("%s", temp);
		//GET
		strcpy(buffer, "GET ");
		strcat(buffer, temp);
		strcat(buffer, " HTTP/1.1\r\n\r\n");
		int nWrittern;
		//Send some data
		if ((nWrittern = write(sock, buffer, strlen(buffer))) < 0)
		{
			perror("Request write failed\n");
			exit(1);
		}
		int nRead = 0, i, count = 0;

		while (1)
		{

			nRead = read(sock, server_reply, BUFSIZ); //reads the html from the proxy

			for (i = 0; i < strlen(server_reply); i++) //copies incoming bytes to msg2
			{
				HTML[count] = server_reply[i];
				count++;
			}
			bzero(&server_reply, sizeof(server_reply));
			//memset(server_reply, 0, n);
			if ((nRead <= 0)) //or n = 0 that stops the proxy being sent
				break; //then reading stops!
		}
		//Receive a reply from the server
		puts("Server reply :");
		puts(HTML);
	}

	close(sock);
	return 0;
}

int server()
{
	char buffer[LENGTH];
	int sock_fd, client_fd, server_fd;
	struct sockaddr_in client_addr, server_addr;
	socklen_t client_len;
	struct hostent *hp;
	int port, optval = 1;
	port = 8888;

// check arguments
//  if (argc != 2) {
//    fprintf(stderr, "Usage: %s port number\n", argv[0]);
//    exit(0);
//  }
	port = 8888; //atoi(argv[1]);

	// Create a socket
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket create failed\n");
		return -1;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int)) < 0)
	{
		perror("Address already in use\n");
		return -1;
	}

	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short) port);

	// bind socket
	if (bind(sock_fd, (SA *) &server_addr, sizeof(server_addr)) < 0)
	{
		perror("socket bind failed\n");
		return -1;
	}

	// listen socket
	if (listen(sock_fd, 1) < 0)
	{
		perror("Socket listen faield");
		return -1;
	}

	while (1)
	{
		client_len = sizeof(client_addr);
		// Accepting client connection
		if ((client_fd = accept(sock_fd, (SA *) &client_addr, &client_len)) < 0)
		{
			perror("Accept failed\n");
			exit(1);
		}

		recv(client_fd, buffer, LENGTH, 0);
		printf(
				"========================= Received Request =========================\n");
		printf("%s\n", buffer);
		printf(
				"========================= ================ =========================\n");

		// GET http://goidirectory.nic.in/index.php HTTP/1.0\r\n\r\n
		char reqMethod[50];
		char URL[500];
		char rest[4000];
		char *query = malloc(4000);
		;

		sscanf(buffer, "%s %s %s", reqMethod, URL, rest);

		char *temp_host = malloc(1000);
		char *temp_host2 = malloc(1000);
		char * f = malloc(1000);
		char *hostWWW = malloc(1000);
		char * host = malloc(1000);

		hostWWW = strstr(URL, "www");
		if (hostWWW == NULL)
		{
			temp_host = strstr(URL, "http:");
			temp_host += 7;
			query = strchr(temp_host, '/');
			if (query != NULL)
			{
				int index = (int) (query - temp_host);
				strncpy(temp_host2, temp_host, index);
				strcpy(host, temp_host2);
			}
			else
			{
				strcpy(host, temp_host);
			}
			printf("The Host is %s\n", host);
		}
		else
		{
			query = strchr(hostWWW, '/');
			if(query != NULL)
			{
				int index = (int) (query - hostWWW);
				strncpy(temp_host2, hostWWW, index);
				strcpy(host, temp_host2);
			}
			else
			{
				strcpy(host, hostWWW);
			}
			printf("The Host is %s\n", host);
		}

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return -1;
		}
		if ((hp = gethostbyname(host)) == NULL)
		{
			perror("Error: gethosbyname() \n");
			return -1;
		}

		bzero((char *) &server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		bcopy((char *) hp->h_addr_list[0], (char *) &server_addr.sin_addr.s_addr, hp->h_length);
			server_addr.sin_port = htons(80);

		if (connect(server_fd, (SA *) &server_addr, sizeof(server_addr)) < 0)
		{
			perror("Web Connect failed\n");
			return -1;
		}

		//send GET request to server
		ssize_t nwritten;
		char reqBuffer[REQBUFFLEN];
		strcpy(reqBuffer, "GET ");
		strcat(reqBuffer, URL);
		strcat(reqBuffer, " HTTP/1.0\r\n\r\n");
		size_t size = strlen(reqBuffer);
		if ((nwritten = write(server_fd, reqBuffer, size)) <= 0)
		{
			perror("Web socket write failed\n");
			exit(1);
		}

		//receive reply
		int response_len = 0, written_length;
		ssize_t readByte;
		while ((readByte = read(server_fd, buffer, LENGTH)) > 0)
		{
			response_len += readByte;
			printf("response size : %d, data : %s", response_len, buffer);
			if ((nwritten = write(client_fd, buffer, readByte)) <= 0)
			{
				perror("client socket write failed\n");
				return -1;
			}
			else
			{
				written_length += nwritten;
			}
			bzero(buffer, LENGTH);
		}

		close(client_fd);
		close(server_fd);
	}
	return 0;
}

