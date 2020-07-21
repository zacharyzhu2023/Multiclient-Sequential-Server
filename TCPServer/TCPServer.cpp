#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <TCPConnection.h>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 50

/** <summary>
Create and start server; look for clients to connect to while sending/receiving messages with
clients that are currently connected. Raise errors if encountered.
</summary>
*/
int main() {

	/* Defining the necessary variables */
	int len, receiveWorks, sendWorks;
	bool active = TRUE, isServer = true;
	SOCKET client_fd;
	string msg1 = string(1, char(6)), msg2 = "random message";
	double sendDelay = 2.0;

	/* Start Server */
	TCPConnection testServer(isServer, sendDelay, msg1, msg2);
	testServer.startServer();

	/* SEND AND RECEIVE */
	while (active) {

		/* Accept clients */
		len = sizeof(testServer.getServer());
		client_fd = accept(testServer.getListener(), (struct sockaddr*)&testServer.getServer(), &len);

		/* Check that the client was valid */
		if (client_fd != INVALID_SOCKET) {

			/* Adding client to the table */
			testServer.makeClient(testServer.getNumClients(), client_fd);
		}

		/* Adding in a delay */
		Sleep(1);

		/* Send and receive data */
		if (testServer.getNumClientsConnected() > 0)
		{

			/* Looping through all the clients */
			for (int cc = 0; cc < testServer.getNumClients(); cc++)
			{
				if (testServer.getClientConnected(cc))
				{
					/* Receive message(s) */
					receiveWorks = testServer.receiveMessage(cc);
					/* Send data */
					if (receiveWorks > 0)
					{
						if (testServer.getClientSendACK(cc))
						{
							do {
								receiveWorks = testServer.receiveMessage(cc);
							} while (receiveWorks < 0);
							receiveWorks = testServer.receiveMessage(cc);
						}
						Sleep(10);
						sendWorks = testServer.sendMessage(cc, msg1);
						if (testServer.getClientReceiveACK(cc))
						{
							sendWorks = testServer.sendMessage(cc, msg2);
						}
					}

					/* Client has closed connection */
					else if (receiveWorks == 0)
					{
						testServer.setOneClientConnected(cc, false);
						testServer.decrementClientsConnected();
					}
				}
			}
		}
	}
	/* Cleanup and shutdown */
	printf("Is the shutdown reached?\n");
	closesocket(testServer.getListener());
	WSACleanup();
	return 0;
}

/*
	Note to self:
	- Shutdown condition is never reached
		- Should it be reachable? If so, when?
		- Want to maintain the flexibility to accept new connections...
			- When's a good time to ask if server is allowed to shut down?
*/