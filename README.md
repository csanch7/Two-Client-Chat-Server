# Chat Application

A simple client-server chat system that allows two users to communicate through a relay server.

## Features

- **Two-user chat**: Server relays messages between exactly two connected clients
- **Thread-based architecture**: Each client connection runs in its own thread
- **Message framing**: Messages are prefixed with length for reliable transmission
- **Graceful shutdown**: Proper connection termination handling
- **Configurable server**: Can specify port via command line or prompt

## Project Structure

```
chat-app/
├── chatClient.c          # Client implementation
├── chatServer.c         # Server implementation
├── clientServer.h       # Common header file
└── README.md           # This file
```

## Prerequisites

- GCC compiler
- POSIX-compliant system (Linux, macOS, BSD)
- Basic understanding of terminal usage

## Building

### Using Make (Recommended)

```bash
make           # Build both client and server
make clean     # Clean build artifacts
```

### Manual Compilation

```bash
# Build server
gcc -Wall -Wextra -pthread -o chatServer chatServer.c

# Build client
gcc -Wall -Wextra -pthread -o chatClient chatClient.c
```

## Usage

### Starting the Server

1. **Default mode** (prompts for port):
```bash
./chatServer
```

2. **With port specified**:
```bash
./chatServer 8080
```

The server will:
- Bind to the specified port
- Wait for two clients to connect
- Relay messages between them
- Display connection status and received messages

### Starting Clients

1. **Default mode** (prompts for server and port):
```bash
./chatClient
```

2. **Using command line arguments** (if modified in code):
```bash
# You can modify the code to accept command-line arguments
# Currently prompts interactively
```

The client will:
- Prompt for server address (default: `localhost`)
- Prompt for port number
- Establish connection to server
- Start send and receive threads
- Display messages from the other user

## How It Works

### Protocol
1. **Message format**: `[4-byte length (network byte order)][message data]`
2. **Connection**: TCP sockets with proper error handling
3. **Threading**: Separate threads for sending and receiving

### Server Architecture
```
Main Thread
    │
    ├── Accepts Client 1
    │
    ├── Accepts Client 2
    │
    ├── Creates Thread 1 (handles Client 1 → Client 2)
    │
    └── Creates Thread 2 (handles Client 2 → Client 1)
```

### Client Architecture
```
Main Thread
    │
    ├── Creates Send Thread (stdin → socket)
    │
    └── Creates Receive Thread (socket → stdout)
```

## Communication Flow

1. **Connection Phase**:
   - Server starts and waits for two connections
   - Clients connect one after another
   - Server pairs clients together

2. **Messaging Phase**:
   ```
   Client A types message → Server → Client B
   Client B types message → Server → Client A
   ```

3. **Disconnection**:
   - Either client can disconnect by pressing Ctrl+D (EOF)
   - Server detects disconnection and shuts down remaining connection
   - Both threads clean up resources

## Key Functions

### Server (`chatServer.c`)
- `doServer()`: Main server logic, accepts connections
- `handleClient()`: Thread function for message relaying
- `getServerFileDescriptor()`: Sets up listening socket

### Client (`chatClient.c`)
- `sendThread()`: Reads from stdin and sends to server
- `recvThread()`: Receives from server and prints to stdout
- `attemptToConnectToServer()`: Establishes connection


## Security Considerations

⚠️ **For educational purposes only** ⚠️

This implementation is not suitable for production use because:
- No encryption (consider using TLS/SSL)
- No authentication mechanism
- No input validation/sanitization
- Buffer overflow vulnerabilities may exist
- No protection against DoS attacks

## Troubleshooting

### Debug Mode
Compile with debugging symbols:
```bash
gcc -g -Wall -pthread -o chatServer chatServer.c
gdb ./chatServer
```

## Extending the Application

Possible enhancements:
1. **Multi-room support**: Allow more than 2 clients
2. **Command system**: Add `/users`, `/help`, `/whisper`
3. **File transfer**: Send files between clients
4. **Encryption**: Implement TLS/SSL
5. **Graphical interface**: Add GUI using GTK or Qt
6. **Message persistence**: Store chat history

## License

Educational/Personal Use - See source code headers for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request


---
**Note**: This is a learning project. Always test in a controlled environment before deployment.