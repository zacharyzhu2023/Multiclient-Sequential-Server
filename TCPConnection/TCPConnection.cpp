#include "TCPConnection.h"
#pragma once

/**
<summary>
Function to print out time in HH:MM:SS:MMM format in local time.
</summary>
<returns></returns>
*/
void TCPConnection::printTime()
{
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);
	printf("%02d:%02d:%02d:%03d", currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds);
	DWORD end = GetTickCount();
	printf(" (%03d ms): ", end - _lastEvent);
	_lastEvent = end;
}

/**
<summary>
Function to print out chars that can't be printed inherently.
</summary>
<returns></returns>
<param name = "c">Char to be printed</param>
*/
void TCPConnection::printChar(char c)
{
	if (c == char(0))
	{
		printf("<NUL>");
	}
	else if (c == char(1))
	{
		printf("<SOH>");
	}
	else if (c == char(2))
	{
		printf("<STX>");
	}
	else if (c == char(3))
	{
		printf("<ETX>");
	}
	else if (c == char(4))
	{
		printf("<EOT>");
	}
	else if (c == char(5))
	{
		printf("<ENQ>");
	}
	else if (c == char(6))
	{
		printf("<ACK>");
	}
	else if (c == char(21))
	{
		printf("<NAK>");
	}
	else if (c == char(10))
	{
		printf("<LF>");
	}
	else if (c == char(13))
	{
		printf("<CR>");
	}
	else {
		printf("%c", c);
	}
}

/**
<summary>
Print the message sent/received char by char using printChar() method above.
</summary>
<returns></returns>
<param name = "msg">Complete message to print out.</param>
*/
void TCPConnection::printMessage(string msg)
{
	for (unsigned int i = 0; i < msg.length(); i++)
	{
		printChar(msg[i]);
	}
}

/**
<summary>
Timer to delay sending a message
</summary>
<returns></returns>
<param name = "time">Amount of time to delay </param>
*/
void TCPConnection::timeDelay(double time)
{
	while (time >= 1)
	{
		Sleep(1000);
		time -= 1;
	}
}


/**
<summary>
Initialize Winsock for the client--print error if failure on startup
or getting addressinfo.
</summary>
<returns></returns>
*/
void TCPConnection::initWinsock()
{
	_works = WSAStartup(MAKEWORD(2, 2), &_wsaData); // Same as client
	if (_works != 0)
	{
		printf("Error on WSAStartup\n");
		return; // Indicates error encountered
	}
	ZeroMemory(&_hints, sizeof(_hints)); // Fills block of memory w/ 0's
	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_protocol = IPPROTO_TCP;

	/* Getting local address/port */
	_works = getaddrinfo(NULL, DEFAULT_PORT, &_hints, &_result);
	if (_works != 0)
	{
		printf("Error on getaddrinfo\n");
		WSACleanup();
		return;
	}
}

/**
<summary>
Creates a connecting socket and connects to the server from a client.
Prints an error message if error occurs during socket creation.
</summary>
<returns> -1 if an error occurs, 0 if no error occurs </returns>
*/
int TCPConnection::findClientMessager()
{
	_messager = INVALID_SOCKET;
	for (_ptr = _result; _ptr != NULL; _ptr = _ptr->ai_next)
	{

		/* Create the connecting socket */
		_messager = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol); // Create socket
		if (_messager == INVALID_SOCKET)
		{
			printf("Error during socket creation\n");
			WSACleanup();
			return -1;
		}
		printTime();
		printf(": Created connecting socket\n");
		/* Connecting to the server */
		_works = connect(_messager, _ptr->ai_addr, (int)_ptr->ai_addrlen);
		if (_works == SOCKET_ERROR)
		{
			closesocket(_messager);
			_messager = INVALID_SOCKET;
			continue;
		}
		else
		{
			printTime();
			printf(": Connected to the server\n");
		}
		break;
	}
	freeaddrinfo(_result);
	if (_messager == INVALID_SOCKET)
	{
		printf("Error during server connection\n");
		WSACleanup();
		return -1;
	}
	printTime();
	printf("SUCCESS: Client initiated!\n");
	return 0;
}


/**
<summary>
Runs initWinsock() and findClientMessager()
</summary>
<return></return>
*/
void TCPConnection::startClient()
{
	initWinsock();
	findClientMessager();
}

/**
<summary>
Initialize Winsock. Then, create, bind, and start listening from a listener
socket, printing any errors if encountered along the way.
</summary>
<return></return>
*/
void TCPConnection::startServer()
{
	/* Initialize Winsock */
	int start, i = 1;
	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = INADDR_ANY;
	_server.sin_port = htons(stoi(DEFAULT_PORT));
	start = WSAStartup(MAKEWORD(2, 2), &_wsaData);
	if (start != 0)
	{
		printf("Error on WSAStartup: %d\n", start);
	}
	/* Create socket that will connect to server */
	_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listener == INVALID_SOCKET)
	{
		printf("Error creating socket to connect to server: %ld\n", WSAGetLastError());
		WSACleanup();
	}
	setsockopt(_listener, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i));

	/* Bind the socket */
	start = bind(_listener, (sockaddr*)&_server, sizeof(_server));
	if (start == SOCKET_ERROR)
	{
		printf("Error on bind: %d\n", WSAGetLastError());
		closesocket(_listener);
		WSACleanup();
	}
	/* Create the listener socket */
	start = listen(_listener, 5);
	unsigned long b = 1;

	/* Configurations for non-blocking*/
	ioctlsocket(_listener, FIONBIO, &b);
	if (start == SOCKET_ERROR)
	{
		printf("Error on listen: %d\n", WSAGetLastError());
		closesocket(_listener);
		WSACleanup();
	}
	printTime();
	printf("SUCCESS: Server initiated!\n");
}



/**
<summary>
Account for the desired time delay of message. Send the message specified to the desired
socket for both client/server. Print the time message was sent.
</summary>
<returns> Returns -1 if error encountered, return length of message if successful </returns>
<param name = "x">The SOCKET number</param>
<param name = "message">Message to be sent</param>
*/
int TCPConnection::sendMessage(int x, string message)
{
	timeDelay(_sendDelay);
	_works = send(_clients[x].client, message.c_str(), message.length(), 0);
	printTime();

	if (_works == SOCKET_ERROR)
	{
		printf("Error on sending message\n");
		closesocket(_clients[x].client);
		WSACleanup();
		return -1;
	}
	else
	{
		if (_isServer)
		{
			cout << "SERVER sends to client " << _clients[x].client << " : ";
		}
		else
		{
			cout << "CLIENT sends to server: ";
		}
		printMessage(message);
		printf("\n");
	}
	_sender = false;
	_clients[x].sentACK = (message == string(1, char(6)));
	_sendCounter++;
	return _works;
}

/**
<summary>
Use sendMessage() to send EOT character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "x">The SOCKET number</param>
*/
int TCPConnection::sendEOT(int x)
{
	return sendMessage(x, EOT);
}

/**
<summary>
Use sendMessage() to send ENQ character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "x">The SOCKET number</param>
*/
int TCPConnection::sendENQ(int x)
{
	return sendMessage(x, ENQ);
}

/**
<summary>
Use sendMessage() to send ACK character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "x">The SOCKET number</param>
*/
int TCPConnection::sendACK(int x)
{
	return sendMessage(x, ACK);
}

/**
<summary>
Client/Server attempts to receive a message. Print time and message if successful.
If received 0 bytes, print/indicate a disconnection.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "x">The SOCKET number</param>
*/
int TCPConnection::receiveMessage(int x)
{
	char receiveMsg[DEFAULT_LENGTH];
	clients_b client = getOneClient(0);
	int rWorks = recv(_clients[x].client, receiveMsg, (int)strlen(receiveMsg), 0);
	if (rWorks > 0)
	{
		printTime();
		if (_isServer)
		{
			cout << "SERVER received from client " << _clients[x].client << " : ";
		}
		else
		{
			cout << "CLIENT received from server: ";
		}
		string msg;
		msg.assign(receiveMsg, rWorks);
		printMessage(msg);
		printf("\n");
		_sender = true;
		_clients[x].receiveACK = (msg == ACK);
		return rWorks;
	}
	else if (rWorks == 0)
	{
		printTime();
		if (_isServer)
		{
			cout << "Client " << _clients[x].client << " disconnected\n";
		}
		else
		{
			cout << "SERVER disconnected.\n";
		}
		return 0;
	}
	return -1;
}

/**
<summary>
Create a new client in the _clients table, based on the number
of clients already present in the table and the SOCKET ID.
</summary>
<returns></returns>
<param name = "x">The position in _clients table </param>
<param name = "z">The socket ID</param>
*/
void TCPConnection::makeClient(int x, SOCKET s)
{
	setOneClientClient(x, s);
	setOneClientConnected(x, true);
	setOneClientSendACK(x, false);
	setOneClientReceiveACK(x, false);
	if (_isServer)
	{
		_numClients++;
		_numClientsConnected++;
		printTime();
		cout << "New client: " << s << endl;
	}
}

/**
<summary>
Function to shutdown a client, raising error if one is encoutnered.
</summary>
<returns>-1 if error encountered, 0 otherwise </returns>
*/
int TCPConnection::clientShutdown()
{
	/* Shutdown connection for sending */
	_works = shutdown(getMessager(), SD_SEND);
	if (_works == SOCKET_ERROR) {
		printf("Error on shutdown: %d\n", WSAGetLastError());
		closesocket(getMessager());
		WSACleanup();
		return 1;
	}

	/* Shutdown and cleanup */
	closesocket(getMessager());
	WSACleanup();
	printTime();
	printf(": Closed and shutdown\n");

	/* Safe exit */
	printf("Enter any number to exit: ");
	int ok;
	cin >> ok;
	return 0;
}