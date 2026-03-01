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

| Command | Description              |
|---------|--------------------------|
| `USER`  | Set the username         |
| `PASS`  | Set the password         |
| `CWD`   | Change working directory |
| `CDUP`  | Move to parent directory |
| `QUIT`  | Close the connection     |

> Only anonymous login is supported (`USER Anonymous` with no password).

## Project structure

```
src/
├── main.cpp                  # Entry point
├── Server/                   # Main loop, accepts incoming connections
│   └── Commands/             # FTP command handlers
├── Client/                   # Represents a connected client (session, state, replies)
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
