# ft_irc

## Overview

ft_irc is my implementation of an **Internet Relay Chat (IRC) server** written in **C++98** as part of the 42 curriculum.
The goal of this project is to understand how a real network protocol works internally and how to design a scalable, non-blocking server using low-level system calls.

This README is meant to be read **before looking at the code**.
It explains the architecture, the design decisions, and how the code is organized, so that reading the implementation later becomes much easier.

---

## What This Server Does

The server implements a subset of the IRC protocol (RFC 1459 / RFC 2812 style behavior), including:

* TCP client connections
* Non-blocking I/O using `poll()`
* Authentication (`PASS`, `NICK`, `USER`)
* Channels (`JOIN`, `PART`)
* Private and channel messaging (`PRIVMSG`)
* Standard IRC replies and error handling

This is a **single-server, client-to-server** IRC implementation.
There is no server-to-server communication.

---

## Build & Run

### Build

```sh
make
```

The project is compiled with:

* `-Wall -Wextra -Werror`
* `-std=c++98`

---

### Run

```sh
./ircserv <port> <password>
```

Example:

```sh
./ircserv 6667 pass123
```

---

### Connect (for testing)

Using netcat:

```sh
nc -C localhost 6667
```

Using irssi:

```sh
/connect localhost 6667 pass123
```

The `-C` flag in `nc` is required to preserve IRC CRLF (`\r\n`) line endings.

---

## Project Structure

```text
.
├── includes/        # All class headers
├── srcs/            # All source files
├── Makefile         # Build configuration
├── README.md        # This file
├── en.subject.pdf   # Project subject
```

Headers define responsibilities clearly, and source files implement exactly what is declared in the headers.

---

## How the Server Works (High-Level)

Before reading any file, keep this execution flow in mind:

```text
Client connects
    ↓
Socket accepted
    ↓
User object created
    ↓
Incoming data buffered
    ↓
Full IRC line detected (\r\n)
    ↓
Command parsed
    ↓
Command handler executed
    ↓
Response sent
```

Every major class exists to support one step of this pipeline.

---

## Entry Point (`main.cpp`)

`main.cpp` is intentionally minimal.

Its responsibilities are:

* Validating program arguments
* Creating the server instance
* Starting the main server loop

All logic is delegated to the server class.
If `main.cpp` grows large, it means responsibilities are leaking.

---

## Server Core (Sockets & Event Loop)

The server class is responsible for:

* Creating the listening socket
* Setting sockets to non-blocking mode
* Using `poll()` to monitor all sockets
* Accepting new client connections
* Detecting readable client sockets

Conceptual event loop:

```text
while (server is running)
{
    poll(all sockets);

    if (listening socket is readable)
        accept new client;

    for each readable client socket
        read incoming data;
}
```

### Design Choice

The server uses **no threads**.
All concurrency is handled through I/O multiplexing.
This simplifies state management and avoids race conditions.

---

## User Abstraction

Each connected client is represented by a `User` object.

A user stores:

* Socket file descriptor
* Nickname
* Username
* Authentication state
* Input buffer
* Joined channels

---

### Why Buffers Are Mandatory

TCP does not preserve message boundaries.

This means:

* A single `recv()` may contain half a command
* Or multiple commands at once

The server therefore always follows this logic:

```text
recv() → append to buffer
while buffer contains "\r\n"
    extract one full line
    process command
```

Understanding this buffering logic is essential to understanding the entire codebase.

---

## IRC Command Parsing

IRC commands follow this format:

```text
COMMAND param1 param2 :trailing parameter\r\n
```

Rules:

* Parameters are space-separated
* Everything after `:` is a single parameter
* Commands are case-insensitive

Parsing steps:

1. Extract a full line from the buffer
2. Split command and parameters
3. Normalize the command
4. Dispatch to the correct handler

Example:

```text
PRIVMSG #channel :hello world
```

Becomes:

* Command: `PRIVMSG`
* Parameters: `#channel`, `hello world`

---

## Authentication Flow

Authentication always follows this order:

```text
PASS → NICK → USER
```

A user is not considered **registered** until all required steps are completed.

Internally:

* User state flags track authentication progress
* Commands are rejected if sent too early
* Errors follow IRC numeric reply rules

This is why many command handlers begin with strict state checks.

---

## Channels

Channels are managed separately from users.

A channel typically stores:

* Channel name
* Connected users
* Operators
* Topic (if implemented)

---

### Broadcasting Messages

When a message is sent to a channel:

```text
for each user in channel
    if user != sender
        send message
```

Channels do not write directly to sockets.
They rely on server or user methods to keep ownership clear.

---

## Sending Replies

All outgoing messages (errors, confirmations, messages) follow IRC formatting rules.

Examples:

```text
:server 001 nick :Welcome to the IRC server
:server 433 * nick :Nickname is already in use
```

Key points:

* Message formatting is centralized
* Socket writing is centralized
* No raw `send()` calls scattered across the code

This makes debugging with real IRC clients much easier.

---

## Recommended Reading Order

To understand the code efficiently, read files in this order:

1. `main.cpp`
2. Server class (socket setup and event loop)
3. User class (state management and buffering)
4. Command parsing logic
5. Individual command handlers
6. Channel management
7. Utility helpers

Avoid jumping directly into command handlers before understanding the server loop.

---

## Intentional Constraints

Some design choices are deliberate:

* No threads
* No modern C++ features
* Explicit state validation
* Verbose error handling

These constraints are part of the project and reinforce protocol correctness and server design fundamentals.

---

## Final Note

If something feels strict or verbose in the code, it is usually because:

* The IRC protocol requires it, or
* Network programming demands it

When reading the code, always ask:

> Which step of the IRC flow is this implementing?

Once that is clear, the implementation should make sense.
