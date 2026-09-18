# CSC 246 — Operating Systems

**NC State, Fall 2022.** Seven assignments that solve the *same problem* with a different
concurrency primitive each time. By the end you know exactly what each one is for.

> Commit dates are the publication date; the work is from Fall 2022.

## Max-subarray, five ways

The running joke of the semester. One problem, five different machines underneath it:

| # | Primitive | File |
|---|---|---|
| 1 | `fork` + `pipe` across N processes | `hw1/maxsum.c` |
| 2 | Java threads | `hw2/Maxsum.java` |
| 3 | pthreads + POSIX semaphores | `hw3/maxsum-sem.c` |
| 4 | Monitors — mutex + condition variables | `hw4/hall.c` |
| 5 | **CUDA** | `hw5/maxsum.cu` |

The CUDA file still has my timing note in the header: **1.668s on an RTX 2070.**

## The rest of the greatest hits

🐚 **A shell, with no standard library** — `hw0/stash.c`. Parses, forks, execs, redirects, has
`cd` and `exit` builtins. No libc string functions allowed, so `customStrLen` and a hand-rolled
`atoi` are in there. 200 lines.

📬 **The same puzzle over three IPC mechanisms** — POSIX message queues (`hw1`), System V shared
memory (`hw2`), then sockets. Signal handling done properly: `SIGINT` flips a
`volatile sig_atomic_t` instead of doing real work inside the handler.

🎯 **A multithreaded Scrabble server** — `hw5/scrabbleServer.c`, 586 lines. BSD sockets, a thread
per client, shared board state. Clients view, place across/down, or quit.

🔐 **RSA challenge/response auth** — `hw6`. Server checks a `passwd.txt` of user → public key,
issues a challenge, verifies the signed response, *then* lets you play.

## Running it

No build system — that was the point.

```bash
gcc -Wall -std=c99 -pthread hw3/maxsum-sem.c -o maxsum-sem
javac hw6/Server.java && java Server 8080
nvcc hw5/maxsum.cu -o maxsum-cuda
```

Private-key fixtures are deliberately not in the repo — generate a keypair and add the public half
to `passwd.txt` for hw6.

---

*More: [CSC 230](https://github.com/pbairoliya/csc230-c-software-tools) ·
[CSC 474](https://github.com/pbairoliya/csc474-network-security) ·
[CSC 484](https://github.com/pbairoliya/csc484-game-ai) ·
[where it started](https://github.com/pbairoliya/first-code)*
