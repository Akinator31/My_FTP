# my_ftp

An FTP server written in C++20, based on the [RFC 959](https://datatracker.ietf.org/doc/html/rfc959).

Handles multiple simultaneous connections using `poll()` and supports anonymous authentication.

## Build

The project uses CMake and ships with a Makefile:

```
make        # build the project
make clean  # remove object files
make fclean # full cleanup
make re     # rebuild from scratch
```

The `myftp` binary is generated at the project root.

## Usage

```
./myftp <port> <path>
```

- `port` – port the server listens on
- `path` – root directory of the FTP server (home directory)

```
./myftp 4242 /tmp
./myftp --help
```

## Supported commands

| Command | Description                                      |
|---------|--------------------------------------------------|
| `USER`  | Specify the username for authentication          |
| `PASS`  | Specify the password for authentication          |
| `CWD`   | Change the current working directory             |
| `CDUP`  | Change to the parent directory                   |
| `QUIT`  | Close the connection                             |
| `PWD`   | Print the current working directory              |
| `NOOP`  | No operation (keeps the connection alive)        |
| `HELP`  | Display help information for one or all commands |
| `DELE`  | Delete a file on the server                      |
| `PASV`  | Enter passive mode                               |
| `PORT`  | Specify address and port for active mode         |
| `LIST`  | List files in the current or specified directory |
| `RETR`  | Retrieve (download) a file from the server       |
| `STOR`  | Store (upload) a file to the server              |

> Only anonymous login is supported (`USER Anonymous` with no password).

## Project structure

```
src/
├── main.cpp                  # Entry point
├── Server/
│   ├── Server.h++/cpp        # Main loop, accepts incoming connections
│   └── Commands.h++/cpp      # FTP command handlers
├── Client/
│   ├── Client.h++/cpp        # Represents a connected client (session, state, replies)
│   └── DataTransferManager.h++/cpp  # Manages active/passive data transfers
├── FtpSession/               # Manages control/data sockets for a session
├── Socket/                   # RAII wrapper around POSIX sockets
├── Poller/                   # Abstraction over poll()
├── Errors/                   # Custom error handling
└── Utils/                    # Argument parsing, path validation
```

## Tests

Unit tests rely on [Criterion](https://github.com/Snaipe/Criterion):

```
make tests_run
```

## Requirements

- C++20
- CMake >= 3.20
- Compiled with `-Wall -Wextra -Werror`
