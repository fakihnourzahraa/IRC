# ft_IRC

_This project has been created as part of the 42 curriculum by nfakih and miwehbe._

# Description

IRC stands for internet relay chat, the aim of this project is to create an IRC server in C++. The server needs to be able to handle clients simultaneously. All input and output operations need to be non-blocking. Additionally we can only use 1 poll. Implemented are join, nickname, username, private mesages. Additionally, there are operators who can: kick, invite, set a topic, and change the channel's mode.


# Instructions

After cloning the repository, enter the folder, make and run the executable "ircserv" with your desired port and password.

```bash
make

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
irrssi

/connect 127.0.0.1 6667 password
NICK alice
USER alice * 0 :Alice
```
Using nc is also required for testing

```bash
nc -C 127.0.0.1 6667

PASS password
NICK alice
USER alice * 0 :Alice
```
Additionally, to cross-reference with other servers, we used libera chat:

```bash
nc irc.libera.chat 6667

NICK alice
USER alice * 0 :Alice
```
or

```bash
/connect -tls irc.libera.chat 6697

NICK alice
USER alice * 0 :Alice
```

# Goals


# Resources
IRC basics:
https://share.google/Mnfa2t0YNQCpAze39
https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9
Modes: 
https://matrix-org.github.io/matrix-appservice-irc/latest/irc_modes.html
Poll():
https://share.google/UitNG0tHD1e5J8zs4
Commands:
https://share.google/otlf0y7TaZ13hvRNu

# AI Usage
- Clarifying missing pieces
- Testing edge cases 
- Intial structure and understanding the project
- Dividing the work

# Work Division

- nfakih: Channel, command handlers, mode, and bot
- miwehbe: Client, server, parser, replies, and command handler

*Made with lots coffee and debugging at 42 Beirut*