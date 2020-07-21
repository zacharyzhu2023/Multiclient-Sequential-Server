#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_LENGTH 512
#define DEFAULT_PORT "27015"
#define MAX_CLIENTS 50

using namespace std;
using namespace std::chrono;

/*
Table containing the SOCKET, connected status,
receiveACK, and sentACK
*/
struct clients_b {
	bool connected;
	SOCKET client;
	bool sentACK;
	bool receiveACK;
};

class TCPConnection
{
public:

	/* Variables: Helpful string representations of EOT, ENQ, ACK */
	string EOT = string(1, char(4));
	string ENQ = string(1, char(5));
	string ACK = string(1, char(6));

	/* Initialization Methods */
	void initWinsock();
	int findClientMessager();
	void startClient();
	void startServer();

	/* Constructor */
	/**
	<summary>
	Initializes variables based on isServer/sendDelay.
	</summary>
	<returns></returns>
	<param name = "isServer">True if server, false if client</param>
	<param name = "sendDelay">How long to delay sending a message</param>
	<param name = "msg1">First default message</param>
	<param name = "msg2">Second default message</param>
	*/
	TCPConnection(bool isServer, double sendDelay, string msg1, string msg2)
	{
		setIsServer(isServer);
		setSendDelay(sendDelay);
		setSender(!isServer);
		setMessage1(msg1);
		setMessage2(msg2);
	}


	/* Printing Information */
	void printTime();
	static void printChar(char c);
	static void printMessage(string msg);

	/* Accessor Methods */

	/** <summary>Return  whether the most recent message sent by a client/server was ACK.</summary> */
	bool getSentACK() { return _sentACK; }
	/** <summary>Return  whether the most recent message received by a client/server was ACK.</summary> */
	bool getReceiveACK() { return _receiveACK; }
	/** <summary>Return whether the most recent operation by a client/server was successful.</summary> */
	int getWorks() { return _works; }
	/** <summary>Return whether second send/receive by a client/server was successful.</summary> */
	int getWorks2() { return _works2; }
	/** <summary>Return the number of messages sent.</summary> */
	int getSendCounter() { return _sendCounter; }
	/** <summary>Return delay to send a message.</summary> */
	double getSendDelay() { return _sendDelay; }
	/** <summary>Return the SOCKET table for given client/server.</summary> */
	clients_b* getClients() { return _clients; }
	/** <summary>Return SOCKET info given position in table.</summary> */
	clients_b getOneClient(int x) { return _clients[x]; }
	/** <summary>Return SOCKET ID for given table entry.</summary> */
	SOCKET getOneClientClient(int x) { return _clients[x].client; }
	/** <summary>Return whether given socket in table is connected.</summary >*/
	bool getClientConnected(int x) { return _clients[x].connected; }
	/** <summary>Return whether given socket most recent message sent was ACK.</summary> */
	bool getClientSendACK(int x) { return _clients[x].sentACK; }
	/** <summary>Return whether given socket most recent message received was ACK.</summary> */
	bool getClientReceiveACK(int x) { return _clients[x].receiveACK; }
	/** <summary>Return client/server's messager socket.</summary> */
	SOCKET getMessager() { return _messager; }
	/** <summary>Return server's listener socket.</summary> */
	SOCKET getListener() { return _listener; }
	/** <summary>Return timing for most recent event for given server/client</summary> */
	DWORD getLastEvent() { return _lastEvent; }
	/** <summary>Return socket address info for the server</summary> */
	sockaddr_in getServer() { return _server; }
	/** <summary>Return number of clients server was introduced to.</summary> */
	int getNumClients() { return _numClients; }
	/** <summary>Return number of clients server is currently connected to.</summary> */
	int getNumClientsConnected() { return _numClientsConnected; }

	/* Setter Methods */
	/** <summary>Set the messager socket for a given client/server.</summary>*/
	void setMessager(SOCKET messager) { _messager = messager; }
	/** <summary>Set whether a client/server was most recently a sender or a receiver.</summary>*/
	void setSender(bool sender) { _sender = sender; }
	/** <summary>Set delay to send a message.</summary>*/
	void setSendDelay(double sendDelay) { _sendDelay = sendDelay; }
	/** <summary>Set a default for the first message.</summary>*/
	void setMessage1(string msg1) { _msg1 = msg1; }
	/** <summary>Set a default for the second message.</summary>*/
	void setMessage2(string msg2) { _msg2 = msg2; }
	/** <summary>Change the most recent event that occurred.</summary>*/
	void setLastEvent(DWORD event) { _lastEvent = event; }
	/** <summary>Set whether or not given connection is server.</summary>*/
	void setIsServer(bool b) { _isServer = b; }
	/** <summary>Set a socket in the _clients table.</summary>*/
	void setOneClientClient(int x, SOCKET messager) { _clients[x].client = messager; }
	/** <summary>Set a sendACK status for a given socket in the _clients table.</summary>*/
	void setOneClientSendACK(int x, bool s) { _clients[x].sentACK = s; }
	/** <summary>Set a connected status for a given socket in the _clients table.</summary>*/
	void setOneClientConnected(int x, bool c) { _clients[x].connected = c; }
	/** <summary>Set a receiveACK status for a given socket in the _clients table.</summary>*/
	void setOneClientReceiveACK(int x, bool r) { _clients[x].receiveACK = r; }
	/** <summary>Decrease # clients connected for a given server by 1.</summary>*/
	void decrementClientsConnected() { _numClientsConnected--; }

	/* Send Messages */
	int sendMessage(int x, string msg);
	int sendEOT(int x);
	int sendENQ(int x);
	int sendACK(int x);

	/* Receive Messages */
	int receiveMessage(int x);


	/* Other Methods */
	static void timeDelay(double timeDelay);
	void makeClient(int x, SOCKET s);

	/* Shutdown & Cleanup */
	int clientShutdown();

private:
	clients_b _clients[MAX_CLIENTS];
	WSADATA _wsaData; // Structure contains info about windows socket implmenetation
	int _works = 0, _works2 = 0;
	int _numClientsConnected = 0, _numClients = 0;
	struct addrinfo* _result = NULL, * _ptr = NULL, _hints; // Contains sockaddr structure
	bool _isServer = false, _sender = true, _sentACK = false, _receiveACK = false;
	int _sendCounter = 0, _receiveCounter = 0;
	bool _oneReceive = true, _oneSend = true;
	string _msg1, _msg2;
	double _sendTime = 0, _receiveTime = 0;
	double _sendDelay;
	sockaddr_in _server;

	/*const char NUL = char(0), SOH = char(1), STX = char(2), ETX = char(3), LF = char(10),
		EOT = char(4), ENQ = char(5), ACK = char(6), NAK = char(21), CR = char(13); */

	SOCKET _messager, _listener;
	string _rMessage;
	string _sMessage;
	DWORD _lastEvent = GetTickCount();
};