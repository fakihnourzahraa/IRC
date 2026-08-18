# ft_irc — Work Split Plan

## Reference client
**Decide and write it here before starting:**
weechat

## checklist (both people)
- Only **one** `poll()` (or equivalent) in the entire codebase - no exceptions, no second one 
- `poll()` must be called before **every** `accept` / `read` / `recv` / `write` / `send` - nothing reads or writes outside of it
- Never use `errno` (e.g. `EAGAIN`) to decide whether to retry a read/write
- `fcntl()` used **only** as `fcntl(fd, F_SETFL, O_NONBLOCK)` - any other flag/usage is forbidden

## Person A - Networking

- `Server` class: create the socket, bind, listen, and run the single `poll()` loop
- Listen on all network interfaces (not just localhost) on the given port
- Accept new client connections, set every fd (listening + client) to non-blocking
- Per-client receive buffering: accumulate bytes across multiple reads and reassemble full commands (handles the partial-packet case, like the `nc -C` / ctrl+D test)
- Raw message parsing: split a complete line into a command name and its parameters, following IRC line format
- Command dispatch table: map each command name to the function that handles it
- Registration flow: handle `PASS`, `NICK`, `USER`, and send the welcome numeric replies once registration is complete
- Handle `PING` / `PONG` and `QUIT`, including clean client disconnection and cleanup
- Disconnect handling: close and remove the fd from the poll array whether the client quits normally, is killed unexpectedly, or `nc` is killed mid-command - the rest of the server must keep running fine for everyone else
- Numeric reply formatting helpers

## Person B - Channels & Commands

- `Channel` class: holds member list, operator list, topic, modes, channel key, user limit
- Handle `JOIN` and `PART`
- Handle `PRIVMSG` for both direct messages (user to user) and channel messages
- Handle `NOTICE` — behaves like `PRIVMSG`, but must **never** trigger an automatic error reply back to the sender, even if the target doesn't exist
- Handle `TOPIC`, `INVITE`, `KICK`
- Handle `MODE` and its sub-flags: `i`, `t`, `k`, `o`, `l`
- Channel-related numeric replies (names list, topic replies, etc.)
- Operator vs. regular-user permission checks for all of the above — every flag/command gets tested individually during defense, so partial support still costs partial points
- Disconnect cleanup: when a client disconnects (normally or unexpectedly), remove them from every channel they were a member or operator of

## Shared

- Agree on the `Client` and `Channel` class interfaces before implementing them separately
- Makefile skeleton

---

## File Structure

### `main.cpp` (shared)
- Validate `argc`/`argv` - must be called as `./ircserv <port> <password>`
- Validate the port is a valid number within an acceptable range (1024–65535)
- Construct a `Server` object with the port and password
- Call `server.run()` - this is what actually starts everything: creates the socket, binds, listens, sets up `poll()`, and enters the main loop waiting for events
- Wrap startup in error handling (try/catch or return-code checks) so a bad argument or setup failure prints a clear error instead of crashing

### `Server.hpp` / `Server.cpp` (Person A)
This is the heart of the program - the equivalent of `main_loop` / `do_select` / `check_fd` from the .tar.

- Setup: create the listening socket, bind it, start listening, and set it non-blocking so `accept()`/`recv()` never block indefinitely
- Per-client handling: when a new connection comes in, accept it, set the new fd non-blocking, add it to the poll array, and create a `Client` object to track it
- The loop: one `poll()` call per iteration watching all fds (listening socket + every connected client) at once
- Dispatch: when a client fd has data ready, read it, hand it to the parser, and route any complete commands to the right handler
- Owns the server password and checks it during registration

### `Client.hpp` / `Client.cpp` (Person A)

- The client's file descriptor fd
- Identity fields set during registration: nickname, username, realname
- Input buffer: accumulates received bytes until a full line (ending in `\r\n`) is found, then hands that line off for parsing
- Output buffer (mandatory, not optional): queues outgoing data when the socket isn't ready to write immediately. This is directly tested at eval — a client is suspended (`^Z`), another client floods the channel, then the suspended client resumes and must receive everything that was queued, with no memory leaks in the process
- Registration progress tracking (password accepted, nickname set, username set, fully registered)

### `Parser.hpp` / `Parser.cpp` (Person A)
Handles turning raw bytes into usable commands, since TCP has no concept of "one command" on its own — everything arrives as a stream of bytes that has to be split at `\r\n` boundaries.

- Extract complete lines from a client's buffered input, leaving any incomplete trailing data for the next read
- Split each complete line into a command name and its list of parameters, following IRC's parameter rules (including the `:` trailing-parameter syntax for multi-word arguments)

### `Channel.hpp` / `Channel.cpp` (Person B)
Holds channel state and handles all channel-related commands directly

- State: member list, operator list, topic, invite-only flag, topic-restriction flag, channel key, user limit, list of invited clients
- `JOIN` / `PART`: add or remove a member, enforce invite-only/key/limit checks on join, clean up an empty channel
- `PRIVMSG` (channel case): forward a message to every member except the sender
- `NOTICE` (channel case): same forwarding behavior as `PRIVMSG`, but never generates an error reply if something's wrong
- `TOPIC`: view or change the topic, respecting the topic-restriction mode
- `INVITE`: add a client to the invite list so they can join an invite-only channel
- `KICK`: remove a member, restricted to operators
- `MODE`: apply and remove the `i`/`t`/`k`/`o`/`l` flags, restricted to operators
- Permission checks: verify operator status before any operator-only action

### `Replies.hpp` / `Replies.cpp` (both)
the numeric replies
Covers replies such as:
- `001` — Welcome
- `331` — No topic set
- `332` — Topic
- `353` / `366` — Names list / end of names
- `401` — No such nick/user
- `403` — No such channel
- `421` — Unknown command
- `441` — User not in channel
- `443` — User already on channel
- `471` — Channel is full
- `472` — Unknown mode
- (plus any other error/reply codes needed for registration, e.g. wrong password, nickname in use, not registered)