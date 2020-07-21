#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <TCPConnection.h>

using namespace std;

/** <summary>
Create and start client; connect with server and then start sending/receiving messages
with it. Raise errors if encountered during any step. Shutdown once done transmitting messages.
</summary>
*/
int __cdecl main(int argc, char** argv)
{

	bool isServer = false;
	string msg1 = string(1, char(4)), msg2 = string(1, char(5));
	double sendDelay = 2.0;
	int sendWorks, receiveWorks;

	TCPConnection testClient(isServer, sendDelay, msg1, msg2);
	testClient.startClient();
	testClient.makeClient(0, testClient.getMessager());

	int loop = 1;
	int counter = 0;
	while (counter < 5) {

		/* Send message */
		if (testClient.getClientReceiveACK(0))
		{
			sendWorks = testClient.sendMessage(0, msg1);
			sendWorks = testClient.sendMessage(0, msg2);
		}
		else
		{
			sendWorks = testClient.sendMessage(0, msg2);
		}
		if (sendWorks == SOCKET_ERROR) {
			return 1;
		}
		/* Receive Message(s) */
		receiveWorks = testClient.receiveMessage(0);
		if (testClient.getClientSendACK(0))
		{
			receiveWorks = testClient.receiveMessage(0);
		}
		counter++;

	}
	testClient.clientShutdown();
}