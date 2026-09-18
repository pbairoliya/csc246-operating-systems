# CSC 246 — Operating Systems

Coursework from **NC State University, Fall 2022** (`@date` headers run 31 Aug – 17 Nov 2022).
C99 and Java. Published later, so commit dates are the publication date, not when the work was done.

Seven assignments that keep solving the same problem with a different concurrency primitive each
time, which turns out to be the fastest way to learn what each one is actually for.

## The assignments

### hw0 — a shell, without the standard library
`stash.c` (200 lines) parses a command line, `fork`/`execvp`s it, wires up redirection, and
implements `cd` and `exit` as builtins. The constraint was no libc string handling, so
`customStrLen` and a hand-rolled `atoi` are in there. `exclude.c` filters a line out of a stream
using only raw `read`/`write` syscalls — no stdio, no buffering you didn't write.

### hw1 — processes, pipes, and message queues
`maxsum.c` splits maximum-contiguous-subarray across N worker processes with `fork` and `pipe`,
reducing results in the parent. `server.c`/`client.c` run a Lights Out puzzle over **POSIX message
queues**, with a `SIGINT` handler that flips a `volatile sig_atomic_t` instead of doing real work
in a signal context.

### hw2 — System V shared memory
The same puzzle again on `sys/shm.h`, with undo and report commands. `Maxsum.java` is the Java
threads counterpart, for contrast.

### hw3 — semaphores
`maxsum-sem.c`: pthreads plus POSIX semaphores in an explicit producer/consumer arrangement.

### hw4 — monitors
`hall.c` is a monitor — a mutex guarding occupancy state, with condition variables for the waiting
— exercised by ten driver scenarios. The Java side (`Global`, `Kitchen`, `Ordered`, `TakeAll`) is
four different strategies for avoiding deadlock in a dining-philosophers-shaped problem.

### hw5 — a threaded server, and the same problem on a GPU
`scrabbleServer.c` (586 lines) is a multithreaded TCP Scrabble server: BSD sockets, a thread per
client, shared board state. `maxsum.cu` is the CUDA port — the header records 1.668s real time
on an RTX 2070.

### hw6 — authentication
`Server.java` and `Client.java` do RSA public-key challenge/response against a `passwd.txt` of
user → public key records, using `KeyFactory` and `X509EncodedKeySpec`, then play scrabble scoring
over the authenticated channel.

> The course's private-key fixtures are **not** included here. Generate your own keypair and add
> the public half to `passwd.txt` to run hw6.

## Building

There is no build system — that was the point of the course. Each program compiles directly:

```bash
gcc -Wall -std=c99 -pthread hw3/maxsum-sem.c -o maxsum-sem
javac hw6/Server.java && java Server 8080
nvcc hw5/maxsum.cu -o maxsum-cuda     # needs a CUDA toolchain
```

Verification is by `expected*.txt` fixtures rather than a test framework.
