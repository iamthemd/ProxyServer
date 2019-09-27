/*
 * server.c
 *
 *  Created on: 27-Sep-2019
 *      Author: mdwalker
 */

#include "header.h"

typedef struct sockaddr SA;

int main(int argc, char **argv)
{
	char buffer[LENGTH];
	int sock_fd, client_fd, server_fd;
	struct sockaddr_in client_addr, server_addr;
	socklen_t client_len;
	struct hostent *hp;
	int port, optval = 1;

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
		printf("========================= Received Request =========================\n");
		printf("%s\n", buffer);
		printf("========================= ================ =========================\n");

		char reqMethod[50];
		char URL[500];
		char rest[4000];

		sscanf(buffer, "%s %s %s", reqMethod, URL, rest);
		printf("The URL is %s\n", URL);

		char *host = strstr(URL, "www");
		char *temp_host = malloc(strlen(host) + 1);
		char *temp_host2 = malloc(strlen(host) + 1);
		strcpy(temp_host, host);
		strcpy(temp_host2, host);
		printf("The Host is %s\n", host);

		char * f = index(host, ':');
		if (f == NULL)
		{
			host = (char *) strtok(host, "/");
		}
		else
		{
			host = (char *) strtok(host, ":");
		}

		//extract port number
		char *tmp = NULL;
		tmp = (char *) strtok(temp_host, "/");
		tmp = (char *) strstr(temp_host, ":");

		printf("\nPort: %s\nHost: %s\n", tmp, host);

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return -1; /* check errno for cause of error */
		}
		if ((hp = gethostbyname(host)) == NULL)
		{
			perror("Error: gethosbyname() \n");
			return -2; /* check h_errno for cause of error */
		}

		bzero((char *) &server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		bcopy((char *) hp->h_addr_list[0], (char *) &server_addr.sin_addr.s_addr,
				hp->h_length);

		if (tmp == NULL)
		{
			server_addr.sin_port = htons(80);
		}
		else
		{
			int tok;
			++tmp;
			tok = atoi(tmp);
			server_addr.sin_port = htons(tok);
		}

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
		int response_len = 0;
		ssize_t readByte;
		while ((readByte = read(server_fd, buffer, LENGTH)) > 0)
		{
			response_len += readByte;
			printf("response size : %d, data : %s", response_len, buffer);
			if ((nwritten = write(client_fd, buffer, readByte)) <= 0)
			{
				perror("client socket write failed\n");
				return -1; /* errorno set by write() */
			}
			bzero(buffer, LENGTH);
		}
		close(client_fd);
		close(server_fd);
	}
	return 0;
}

