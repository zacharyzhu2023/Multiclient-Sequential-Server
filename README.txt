Model for a Multiclient TCP Server 
- Using C++ socket programming and Microsoft's Winsock library
- Configurations of IOCTL socket enable handling of multiple clients
    - Handles the clients sequentially
- TCPConnection is built as a static library: contains utility functions and common
  functions used by TCPClient and TCPServer
- TCPClient/TCPServer use the TCPConnection class to initialize, send/receive messages
- To launch, use .exe files for client/server
