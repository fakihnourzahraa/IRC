# ft_irc - Command & Reply

## Reply Priority Tiers

### 1 - Mandatory
| Code | Name | Why it's mandatory |
|---|---|---|
| 001 | RPL_WELCOME | Registration isn't "done" from the client's view without it; most clients (incl. HexChat) won't consider themselves connected |
| 403 | ERR_NOSUCHCHANNEL | JOIN/messaging a nonexistent channel - near-certain test |
| 331 / 332 | RPL_NOTOPIC / RPL_TOPIC | JOIN must send one of these; TOPIC is a required operator command |
| 353 / 366 | RPL_NAMREPLY / RPL_ENDOFNAMES | Without this pair, client shows an empty user list on JOIN |
| 401 | ERR_NOSUCHNICK | PRIVMSG to nonexistent user |
| 432 | ERR_ERRONEUSNICKNAME | Invalid nick input, commonly tested |
| 433 | ERR_NICKNAMEINUSE | Nickname collision, near-certain test |
| 461 | ERR_NEEDMOREPARAMS | Used by nearly every command's param check |
| 464 | ERR_PASSWDMISMATCH | Wrong password - one of your two required CLI args |
| 471 | ERR_CHANNELISFULL | Tied to required MODE flag `l` |
| 472 | ERR_UNKNOWNMODE | MODE is graded flag-by-flag; unrecognized flag needs a defined response |
| 473 | ERR_INVITEONLYCHAN | Tied to required MODE flag `i` |
| 475 | ERR_BADCHANNELKEY | Tied to required MODE flag `k` |
| 482 | ERR_CHANOPRIVSNEEDED | Core of operator vs regular-user grading - explicitly tested per-command |

### 2 - Should implement

| Code | Name | Notes |
|---|---|---|
| 002 / 003 / 004 | RPL_YOURHOST / RPL_CREATED / RPL_MYINFO | Sent alongside 001 as the welcome burst; cheap to add once 001 exists |
| 421 | ERR_UNKNOWNCOMMAND | Any garbage command; easy to add, easy to test with `nc` |
| 431 | ERR_NONICKNAMEGIVEN | `NICK` with zero params; distinct from 432 |
| 441 | ERR_USERNOTINCHANNEL | KICK target not present in channel |
| 442 | ERR_NOTONCHANNEL | PART/TOPIC/etc. from someone not in the channel |
| 443 | ERR_USERONCHANNEL | Double-JOIN, or inviting someone already there |
| 462 | ERR_ALREADYREGISTERED | Sending `USER` twice; trivial since you already track `usernameSet` |

## PASS

**What it does:** Gives the server password.

**Errors:**
- `461 ERR_NEEDMOREPARAMS` - no password given
- `464 ERR_PASSWDMISMATCH` - wrong password
- possibly an error if `PASS` is sent after registration is already complete

**On success:** Password is accepted internally. No special success numeric is normally required immediately.

---

## NICK

**What it does:** Sets or changes the client's nickname.

**Errors:**
- `431 ERR_NONICKNAMEGIVEN` - no nickname given
- `432 ERR_ERRONEUSNICKNAME` - invalid nickname
- `433 ERR_NICKNAMEINUSE` - nickname already in use
- `461 ERR_NEEDMOREPARAMS` - depending on implementation

**On success:** Nickname is changed/set. If this completes registration together with `PASS` + `USER`, send the `001`-`004` welcome burst.

---

## USER

**What it does:** Sets username and real name.

**Errors:**
- `461 ERR_NEEDMOREPARAMS` - missing parameters
- `462 ERR_ALREADYREGISTERED` - already registered

**On success:** Username/realname are stored. If registration is now complete, send `001`-`004`.

---

## JOIN

**What it does:** Joins a channel.

**Errors:**
- `461` — missing channel
- `403 ERR_NOSUCHCHANNEL`
- `442 ERR_NOTONCHANNEL` - in relevant cases
- `471 ERR_CHANNELISFULL`
- `473 ERR_INVITEONLYCHAN`
- `475 ERR_BADCHANNELKEY`
- `443 ERR_USERONCHANNEL` - in relevant invite cases

**On success:** Client joins channel; broadcast `JOIN`; send topic (`331`/`332`); send names (`353`); send end of names (`366`).

---

## PART

**What it does:** Leaves a channel.

**Errors:**
- `461` — missing channel
- `403 ERR_NOSUCHCHANNEL`
- `442 ERR_NOTONCHANNEL`

**On success:** Client leaves; `PART` message is sent/broadcast.

---

## PRIVMSG

**What it does:** Sends a message to a user or channel.

**Errors:**
- `411 ERR_NORECIPIENT` - no target
- `412 ERR_NOTEXTTOSEND` - no message
- `401 ERR_NOSUCHNICK` - target user doesn't exist
- `403 ERR_NOSUCHCHANNEL`
- `404 ERR_CANNOTSENDTOCHAN` - cannot send to channel

**On success:** Message is delivered to target. No numeric success reply is normally needed.

---

## NOTICE

**What it does:** Sends a notice to a user or channel.

**Errors:** Normally no error numeric is sent - this is the important difference from `PRIVMSG`.

**On success:** Notice is delivered; no success numeric required.

---

## TOPIC

**What it does:** Reads or changes the channel topic.

**Errors:**
- `461` - missing channel
- `403` - channel doesn't exist
- `442` - not on channel
- `482 ERR_CHANOPRIVSNEEDED` - not operator when topic is restricted

**On success:**
- If reading: `331` (no topic) or `332` (topic exists)
- If changing: topic is broadcast

---

## INVITE

**What it does:** Invites a user to a channel.

**Errors:**
- `461` - missing params
- `401` - user doesn't exist
- `403` - channel doesn't exist
- `442` - inviter isn't on channel
- `482` - operator privileges needed, when required
- `443 ERR_USERONCHANNEL` - user already there

**On success:** Invitation is recorded; invitation notification is sent to invited user.

---

## KICK

**What it does:** Removes a user from a channel.

**Errors:**
- `461` - missing params
- `403` - channel doesn't exist
- `442` - kicker isn't on channel
- `482` - not operator
- `441 ERR_USERNOTINCHANNEL` - target isn't in channel

**On success:** `KICK` message is broadcast; target is removed.

---

## MODE

**What it does:** Changes or queries channel modes.

**Errors:**
- `461` - missing parameters
- `403` - channel doesn't exist
- `472 ERR_UNKNOWNMODE` - unknown mode
- `482` - not operator, when required
- mode-specific errors depending on argument

**On success:** Mode is changed and a `MODE` message is sent/broadcast; querying modes returns the current modes.

---

## QUIT

**What it does:** Disconnects the client.

**Errors:** Usually no error reply; a malformed optional message generally isn't an error.

**On success:** `QUIT` message is broadcast, client is removed, socket is closed.

---
