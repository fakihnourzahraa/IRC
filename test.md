
## Setup

**Terminal 1 — server:**
```bash
make
./ircserv 6667 test123
```

**Terminal 2 — alice:**
```bash
irssi
/connect 127.0.0.1 6667 test123
/nick alice
```

**Terminal 3 — bob:**
```bash
irssi
/connect 127.0.0.1 6667 test123
/nick bob
```

---

## 1. Registration & auth

| # | Client | Command | Expected |
|---|---|---|---|
| 1.1 | new client | connect with wrong password, then `/nick x` `/quote USER x 0 * :x` | `464` password mismatch |
| 1.2 | alice | (already done above) | `001`-`004` welcome burst |
| 1.3 | bob | `/nick alice` (before registering own nick) | `433` nickname in use |
| 1.4 | bob | `/nick bob` then register | succeeds, `001` welcome |

---

## 2. JOIN / PART / NAMES / TOPIC

| # | Client | Command | Expected |
|---|---|---|---|
| 2.1 | alice | `/join #test` | alice becomes operator (`@alice`) |
| 2.2 | bob | `/join #test` | joins as regular user; alice sees JOIN broadcast |
| 2.3 | bob | `/topic` | "no topic is set" |
| 2.4 | alice | `/topic #test Welcome to the test channel` | topic change broadcast to both |
| 2.5 | bob | `/topic` | shows new topic |
| 2.6 | bob | `/part #test` | alice sees PART; bob removed from names list |
| 2.7 | bob | `/join #test` | rejoins as regular user (NOT operator again) |

---

## 3. Messaging

| # | Client | Command | Expected |
|---|---|---|---|
| 3.1 | bob | type `hello alice` directly in `#test` | alice receives it in channel |
| 3.2 | alice | `/msg bob hey bob` | bob receives it in a private query window |
| 3.3 | alice | `/msg ghostuser123 hi` | `401` no such nick |
| 3.4 | alice | `/notice #test this is a notice` | bob sees it styled differently; no error either way |

---

## 4. INVITE / KICK

| # | Client | Command | Expected |
|---|---|---|---|
| 4.1 | bob | `/part #test` | leaves |
| 4.2 | alice | `/invite bob #test` | bob gets invite notification |
| 4.3 | bob | `/join #test` | rejoins successfully |
| 4.4 | bob | `/kick #test alice bye` | `482` not channel operator; alice stays |
| 4.5 | alice | `/kick #test bob bye` | bob removed, both see KICK broadcast |
| 4.6 | bob | `/join #test` | rejoins for further tests |

---

## 5. MODE


| # | Client | Command | Expected once fixed |
|---|---|---|---|
| 5.1 | bob | `/mode #test +i` | `482` not operator (bob isn't op) |
| 5.2 | alice | `/mode #test +i` | channel becomes invite-only |
| 5.3 | bob | `/part #test` then `/join #test` | `473` cannot join (+i), since bob has no invite |
| 5.4 | alice | `/invite bob #test` then bob `/join #test` | bob can join despite `+i` |
| 5.5 | alice | `/mode #test -i` | invite-only removed |
| 5.6 | alice | `/mode #test +t` | topic changes now restricted to operators |
| 5.7 | bob | `/topic #test new topic attempt` | `482` not operator |
| 5.8 | alice | `/topic #test new topic attempt` | succeeds (alice is op) |
| 5.9 | alice | `/mode #test -t` | topic restriction removed |
| 5.10 | alice | `/mode #test +k secretpass` | channel now requires a key to join |
| 5.11 | bob | `/part #test` then `/join #test` (no key) | `475` bad channel key |
| 5.12 | bob | `/join #test secretpass` | succeeds with correct key |
| 5.13 | alice | `/mode #test -k` | key requirement removed |
| 5.14 | alice | `/mode #test +o bob` | bob becomes operator too (`@bob` in names) |
| 5.15 | bob | `/kick #test alice bye` | now succeeds — bob is operator |
| 5.16 | alice (rejoin) | `/mode #test -o bob` | bob loses operator status |
| 5.17 | alice | `/mode #test +l 2` | user limit set to 2 |
| 5.18 | third client | `/join #test` (with 2 already in) | `471` channel is full |
| 5.19 | alice | `/mode #test -l` | limit removed, third client can now join |
| 5.20 | alice | `/mode #test +z` (invalid flag) | `472` unknown mode |

---

## 6. Changing nickname mid-session


Setup: alice and bob both already registered and joined `#test` (alice is
operator).

| # | Client | Command | Expected |
|---|---|---|---|
| 6.1 | alice | `/nick alice2` | alice's nick changes; **bob sees a NICK change notice** in `#test` (e.g. `alice is now known as alice2`) |
| 6.2 | bob | `/part #test` then `/join #test` | names list shows `@alice2`, confirming the rename is reflected everywhere, not just the sender's own view |
| 6.3 | alice2 | check own prompt/status bar | now shows `alice2`, not `alice` |
| 6.4 | alice2 | `/nick alice2` (same nick, redundant) | should be a harmless no-op, not an error or duplicate welcome burst |
| 6.5 | bob | `/nick alice2` (collision — alice2 already taken) | `433` nickname in use; bob keeps his old nick |
| 6.6 | bob | `/nick !!!bad` (invalid first character) | `432` erroneous nickname; bob keeps his old nick |
| 6.7 | bob | `/nick ok-nick_2` (valid chars: alnum, `-`, `_`) | succeeds |
| 6.8 | alice2 | `/msg ok-nick_2 hi after rename` | delivered correctly — PRIVMSG must resolve the **current** nickname, not a cached old one |
| 6.9 | alice2 | `/kick #test ok-nick_2 bye` then invite/rejoin | operator permissions must be preserved after alice's own nick change — she should still be able to KICK despite renaming since operator status is tracked per-`Client*`, not by nickname string |
| 6.10 | alice2 | `/nick alice2` then immediately `/quit` | QUIT broadcast should show the **current** nick (`alice2`), not the original registration nick (`alice`) |
| 6.11 | new client | connect, `/nick alice` (the now-freed original nick) | should succeed — freeing a nickname on rename must actually release it for reuse |

> **Why this matters for grading:** several of your data structures key off
> `Client*` pointers rather than nickname strings (operator lists, invite
> lists, channel membership), which is correct — but any code path that
> looks a client up **by nickname** (`findClientByNickname`, used in
> PRIVMSG/INVITE/KICK/NICK-collision-checks) must always see the *current*
> nickname, and the *old* nickname must become immediately available for a
> different client to claim. Test 6.5, 6.8, and 6.11 specifically catch bugs
> where an old nickname is left "reserved" or a new nickname isn't properly
> indexed.

---

## 7. QUIT & reconnect

| # | Client | Command | Expected |
|---|---|---|---|
| 7.1 | bob | `/quit see ya` | alice sees QUIT broadcast; server stays alive |
| 7.2 | bob | reopen irssi, `/connect 127.0.0.1 6667 test123`, `/nick bob`, `/join #test` | rejoins cleanly, no server crash |

---

## 8. Raw protocol / fragmentation (needs `nc`, not irssi)

```bash
nc -C 127.0.0.1 6667
```

| # | Type this (Ctrl+D = split point) | Expected |
|---|---|---|
| 8.1 | `PIN` [Ctrl+D] `G :hello` [Enter] | `:ircserv PONG ircserv :hello` |
| 8.2 | `PA` [Ctrl+D] `SS test123` [Enter], then `NICK carol`, `USER carol 0 * :carol` | full registration succeeds despite the split |
| 8.3 | garbage split across two writes, e.g. `com` [Ctrl+D] `mand` [Enter] | `421` unknown command (or `451` if not yet registered) — no crash |

---

## 9. Resilience

| # | Test | Expected |
|---|---|---|
| 9.1 | Join a channel, then close that client's terminal window directly (no `/quit`) | server and other clients unaffected; new clients can still connect |
| 9.2 | One client stops reading (idle window) while another floods `#test` with many messages | server doesn't block/crash; idle client receives all messages once it's active again |
| 9.3 | Run server under `valgrind --leak-check=full ./ircserv 6667 test123`, repeat the above tests, then quit all clients and Ctrl+C the server | no "definitely lost" bytes reported |