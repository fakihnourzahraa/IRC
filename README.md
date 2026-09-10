# ft_IRC

_This project has been created as part of the 42 curriculum by nfakih and miwehbe._

# Description

IRC stands for internet relay chat, the aim of this project is to create an IRC server in C++. The server needs to be able to handle clients simultaneously. All input and output operations need to be non-blocking. Additionally we can only use 1 poll. Implemented are join, nickname, username, private messages. Additionally, there are operators who can: kick, invite, set a topic, and change the channel's mode.

# Requirements

- C++98 compiler (g++ / clang++)
- POSIX-compliant system (Linux / macOS)

# Instructions

After cloning the repository, enter the folder, make and run the executable "ircserv" with your desired port and password.

```bash
make        # build the executable
make re     # clean rebuild
make clean  # remove object files
make fclean # remove object files and executable

./ircserv 6667 password
```

Or to show leaks

```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 password
```

# IRC Client

To use this server you need to pick a client, the reference client for this project is irssi.
To use it:

```bash
irssi

/connect 127.0.0.1 6667 password
/nick alice
/quote USER alice 0 * :Alice
```

Using nc is also required for testing

```bash
nc -C 127.0.0.1 6667

PASS password
NICK alice
USER alice 0 * :Alice
```

Additionally, to cross-reference with other servers, we used libera chat:

```bash
nc irc.libera.chat 6667

NICK alice
USER alice 0 * :Alice
```

or

```bash
/connect -tls irc.libera.chat 6697
```

To test loads of messages:

```bash
{                  

  printf "PASS mypass\r\n"

  printf "NICK s\r\n"

  printf "USER s 0 * :s\r\n"

  printf "JOIN #test\r\n"

  for i in $(seq 1 200); do

    printf "PRIVMSG #test :flood %d\r\n" $i

  done

} | nc -C 127.0.0.1 6667
```

# Architecture

```
.
├── main.cpp             # Entry point, argument parsing
├── Server.cpp/.hpp      # Core server: socket setup, poll loop, accept/read/write, client & channel management
├── Client.cpp/.hpp      # Per-client state (nick, user, registration flags) and input/output buffers
├── CommandHandler.cpp/.hpp # Parses dispatched commands and implements IRC command logic
├── Channel.cpp/.hpp     # Channel state: members, operators, invites, modes (i/t/k/o/l)
├── Parser.cpp/.hpp      # Splits a raw line into command + params
├── Replies.cpp/.hpp     # Builds RFC-style numeric reply strings
└── Makefile
```

**Key design points:**
- Single-threaded, non-blocking I/O using a single `poll()` call for listening socket, reads, and writes
- All sockets (listening and client) set to `O_NONBLOCK`
- Per-client input/output buffers to correctly reassemble partial/fragmented TCP messages before parsing
- IRC message parsing follows the RFC 1459 command/parameter format
- Client registration requires `PASS` + `NICK` + `USER` before any other command is accepted

# Commands

Below is every command the server implements, what it does, and how to test it with `nc` (raw IRC protocol) and with irssi (`/command` form).

### PASS

Sends the connection password. Must be sent before the server will finish registration.

```bash
# nc
PASS password

# irssi (set when connecting)
/connect 127.0.0.1 6667 password
```

### NICK

Sets or changes the client's nickname. Fails if the nickname is already taken or contains invalid characters.

```bash
# nc
NICK alice

# irssi
/nick alice
```

### USER

Sends the username and real name to complete registration. Format: `USER <username> <mode> <unused> :<realname>`.

```bash
# nc
USER alice 0 * :Alice

# irssi (irssi sends USER automatically on /connect,
# to send it manually use /quote)
/quote USER alice 0 * :Alice
```

### JOIN

Joins a channel. If the channel doesn't exist yet, it is created and the joining client becomes operator.

```bash
# nc
JOIN #42

# irssi
/join #42
```

### PART

Leaves a channel the client is currently in.

```bash
# nc
PART #42

# irssi
/part #42
```

### PRIVMSG

Sends a private message to a user or a message to a channel.

```bash
# nc - to a channel
PRIVMSG #42 :hello everyone

# nc - to a user
PRIVMSG bob :hey bob

# irssi
/msg #42 hello everyone
/msg bob hey bob
```

### NOTICE

Same as PRIVMSG but the server never sends an automatic error reply back, used mainly for bots/notifications.

```bash
# nc
NOTICE #42 :server maintenance in 5 minutes

# irssi
/notice #42 server maintenance in 5 minutes
```

### TOPIC

Views or changes a channel's topic. Changing it requires operator privileges if mode `+t` is set.

```bash
# nc - view topic
TOPIC #42

# nc - set topic
TOPIC #42 :welcome to 42

# irssi
/topic #42
/topic #42 welcome to 42
```

### INVITE

Invites a user to a channel. Required if the channel is invite-only (`+i`) and the inviter must be operator in that case.

```bash
# nc
INVITE bob #42

# irssi
/invite bob #42
```

### KICK

Removes a member from a channel. Requires operator privileges.

```bash
# nc
KICK #42 bob :bye bob

# irssi
/kick #42 bob bye bob
```

### MODE

Changes a channel's mode. Requires operator privileges.

| Mode | Description                    |
|------|---------------------------------|
| `i`  | Invite-only channel             |
| `t`  | Restrict TOPIC to operators     |
| `k`  | Set/remove channel key (password) |
| `o`  | Give/take operator privilege    |
| `l`  | Set/remove a user limit         |

```bash
# nc
MODE #42 +i
MODE #42 +t
MODE #42 +k secretkey
MODE #42 -k
MODE #42 +o bob
MODE #42 -o bob
MODE #42 +l 10
MODE #42 -l

# irssi
/mode #42 +i
/mode #42 +t
/mode #42 +k secretkey
/mode #42 -k
/mode #42 +o bob
/mode #42 -o bob
/mode #42 +l 10
/mode #42 -l
```

### PING / PONG

Keeps the connection alive. The server replies to PING with PONG.

```bash
# nc
PING :hello

# irssi handles this automatically, no manual command needed
```

### QUIT

Disconnects the client from the server, removing it from all channels.

```bash
# nc
QUIT :leaving

# irssi
/quit leaving
```

# Goals

# Resources

IRC basics:
- https://share.google/Mnfa2t0YNQCpAze39
- https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9

Modes:
- https://matrix-org.github.io/matrix-appservice-irc/latest/irc_modes.html

Poll():
- https://share.google/UitNG0tHD1e5J8zs4

Commands:
- https://share.google/otlf0y7TaZ13hvRNu

# AI Usage

- Clarifying missing pieces
- Testing edge cases
- Initial structure and understanding the project
- Dividing the work
- ReadMe formatting and fixing

# Work Division

- nfakih: Channel, command handlers, teting, and mode
- miwehbe: Client, server, parser, replies, and command handler

*Made with lots of coffee and debugging at 42 Beirut*