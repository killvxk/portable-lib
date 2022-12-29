====================================
Portable Library of Useful C/++ code
====================================

This directory contains code for many common use cases:

- Bloom filters: Standard, counting and scalable
- Hash tables: Policy based, super-fast (cache friendly)
- Variety of good, fast Hash functions
- Fixed-size memory allocator ('mempools')
- Typesafe templates in "C": Linked lists, vectors, queues
- Single-Producer, Single-Consumer lock-free bounded queue
- Multi-Producer, Multi-Consumer lock-free bounded queue
- Blocking, bounded, producer-consumer queue
- Thread pool (job-handlers) with CPU affinity using pthreads and a
  shared queue across the threads.
- Round-robin work distribution across N threads using pthreads;
  each thread has its own queue enabling work to be queued to
  specific threads.
- Growable, resizable string buffer
- Collection of random number generators (ARC4Random-chacha20,
  XORshift, Mersenne-Twister)

Almost all code is written in Portable C (and some C++).  It is
tested to work on at least Linux 3.x/4.x, Darwin (Sierra, macOS),
OpenBSD 5.9/6.0/6.1. Some of the code has been in production use
for over a decade.

What is available in this code base?
====================================

- Collection of Bloom filters (Simple, Counting, Scalable). The
  Bloom filters can be serialized to disk and read back in mmap
  mode. The serialized code has a strong checksum (SHA256) to
  maintain the integrity of the data when read back.

  The filters share a common interface for add, query and destructor.
  A filter specific constructor returns an opaque pointer.

  All the tests were done with false-positive rate of 0.005.

- Multiple implementation of hash tables:

    * Scalable hash table with policy based memory management and
      locking. It resizes dynamically based on load-factor. It has
      several iterators to safely traverse the hash-table. This uses
      a doubly linked list for collision resolution.

    * A very fast, cache-friendly hash table that uses "linked list of arrays"
      for collision resolution. Each such array has 7 elements. The idea
      is to exploit cache-locality when searching for nodes in the
      same bucket. If the collision chain is more than 7 elements, a
      new array of 7 elements is allocated. The hash table uses short
      "fingerprints" of the hash-key to quickly select the array slot.

    * Open addressed hash table that uses a power-of-2 sized bucket
      list and a smaller power-of-2 sized bucket list for overflow.

- A collection of hash functions for use in hash-tables and other
  places:

    * FNV
    * Jenkins
    * Murmur3
    * Siphash24
    * Metrohash
    * xxHash
    * Superfast hash
    * Hsieh hash
    * Cityhash

  These are benchmarked in the test code *test/t_hashbench.c*.

  If you are going to pick a hash function for use in a hash-table,
  pick one that uses a seed as initializer. This ensures that your
  hash table doesn't suffer DoS attacks. All the code I write uses
  Zilong Tan's superfast hash (*fasthash.c*).

- A portable, thread-safe, user-space implementation of OpenBSD's
  arc4random(3). This uses per-thread random state to ensure that
  there are no locks when reading random data.

- Implementation of Xoroshiro, Xorshift+ PRNG (XS64-Star, XS128+,
  XS1024-Star)

- Wrappers for process and thread affinity -- provides
  implementations for Linux, OpenBSD and Darwin.

- gstring.h: Growable C strings library

- zbuf.h: Buffered I/O interface to zlib.h; this enables callers to
  safely call compress/uncompress using user output functions.

- C++ Code:

    * strmatch.h: Templatized implementations of Rabin-Karp,
      Knuth-Morris-Pratt, Boyer-Moore string match algorithms.

    * mmap.h: Memory mapped file reader and writer; implementations
      for POSIX and Win32 platforms exist.

- Specialized memory management:

    * arena.h: Object lifetime based memory allocator. Allocate
      frequently in different sizes, free the entire allocator once.

    * mempool.h: Very fast, fixed size memory allocator

- OSX Darwin specific code:

    * POSIX un-named semaphores (`sem_init(3)`, `sem_wait(3)`, `sem_post(3)`)
    * Replacement for <time.h> to include POSIX clock_gettime().
      This is implemented using Mach APIs (May not be needed post MacOS
      Sierra).

- Portable routines to read password (POSIX and Win32)

- POSIX compatible wrappers for Win32: mmap(2), pthreads(7),
  opendir(3), inet_pton(3) and inet_ntop(3), sys/time.h

- Portable implementation of getopt_long(3).

Single Header Utilities
-----------------------
- Templates in "C" -- these leverage the pre-processor to create type-safe
  containers for several common data structures:

    * list.h: Single and Doubly linked list (BSD inspired)
    * vect.h: Dynamically growable type-safe "vector" (array)
    * queue.h: Fast, bounded FIFO that uses separate read/write
      pointers
    * syncq.h: Type-safe, bounded producer/consumer queue. Uses
      POSIX semaphores and mutexes.
    * spsc_bounded_queue.h: A single-producer, single-consumer,
      lock-free queue. Requires C11 (stdatomic.h).
    * mpmc_bounded_queue.h: Templatized version of Dmitry Vyukov's
      excellent lock-free algorithm for bounded multiple-producer,
      multiple-consumer queue. Requires C11 (stdatomic.h).
      Performance on late 2013 13" MBP (Core i7, 2.8GHz) with 4
      Producers and 4 Consumers: 236 cyc/producer, 727 cyc/consumer.

- Portable, inline little-endian/big-endian encode and decode functions
  for fixed-width ordinal types (u16, u32, u64).

- Arbitrary sized bitset (uses largest available wordsize on the
  platform).


Performance Measurements
========================
SPSC Lock-free Bounded Queue
----------------------------
Performance on a late 2018 15" MBP (6-Core i9, 2.9GHz):
    * Q size 1048576: ~120 cyc/producer, ~80 cyc/consumer
    * Q size 128: ~30 cyc/producer, ~ 29 cyc/consumer

MPMC Lock-free bounded Queue
----------------------------
Performance on a late 2018 15" MBP (6-Core i9, 2.9GHz):
    * 6 producers and 6 consumers: ~2300 cyc/producer, ~2400 cyc/consumer
    * 2 producers and 2 consumer: ~515 cyc/producer, ~550 cyc/consumer

Bloom Filters
-------------
Performance on a late 2018 15" MBP (6-Core i9, 2.9GHz):

    * Standard Bloom filter: 155 cycles/add, 148 cycles/search
    * Counting Bloom filter: 157 cycles/add, 150 cycles/search
    * Scalable Bloom filter: 716 cycles/add, 770 cycles/search


Fast Hash Table
---------------
Performance on a 2022 Core i7 on ChromeOS Linux env:

    * insert if not present: 1093 cycles (490 ns/insert)
    * find existing element:  80 cycles (362.90 ns/find)
    * find non-existing element: 235 cycles (252 ns/find)
    * delete existing element: 112 cycles (367.39 ns/del)
    * delete non-existent element: 163 element (216 ns/del)

Memory Allocators
-----------------
Performance on a late 2018 15" MBP (6-Core i9, 2.8GHz):
    * Arena: ~5700 cycles/alloc
    * Mempool: 20 cycles/alloc 33M alloc/sec, 19 cycles/free (27M free/sec)

How is portability achieved?
============================
The code above tries to be portable without use of ``#ifdef`` or
other pre-processor constructs. In cases where a particular platform
does not provide a required symbol or function, a compatibility
header is provided in ``inc/$PLATFORM/``. e.g., Darwin doesn't have
a working POSIX un-named semaphore implementation (``sem_init(3)``);
the file ``inc/Darwin/semaphore.h`` provides a working
implementation of the API. Thus, any program using un-named
semaphores can function without any wrappers or ugly ``ifdef``.

While the compatibility functions and symbols are provided via the
mechanism above, the next question is - "how does one tailor the
build environment to accommodate these peculiarities?". This is
where we leverage features of ``make`` to have a conditional build
environment.

GNUmakefile Tricks and Tips
---------------------------
This library comes with a set of ``GNUmake`` fragments and an
example top-level ``GNUmakefile`` to make building programs easy.

These makefiles are written to be cross-platform and incorporates
many idioms to make building for multiple platforms possible
**without** needing the bloated ``configure`` infrastructure.

For each platform that is supported, ``portablelib.mk`` defines a
set of macros for that platform like so::

    darwin_incdirs += /opt/local/include /usr/local/include
    darwin_ldlibs  += /opt/local/lib/libsodium.a
    darwin_objs    += darwin_cpu.o darwin_sem.o darwin_clock.o

    linux_defs   += -D_GNU_SOURCE=1
    linux_ldlibs += -lpthread
    linux_objs   += linux_cpu.o arc4random.o

    openbsd_ldlibs += -L/usr/local/lib -lsodium -lpthread
    openbsd_objs   += openbsd_cpu.o


Then, these flags are used to set ``CFLAGS`` and ``objs`` via
"double variable expansion"  like so::

    os := $(shell uname -s | tr '[A-Z]' '[a-z]')

    INCDIRS = $($(os)_incdirs) $(TOPDIR)/inc/$(os) $(TOPDIR)/inc

    INCS = $(addprefix -I, $(INCDIRS))
    DEFS = -D__$(os)__=1 $($(os)_defs)

    CFLAGS = -g -O2 $(INCS) $(DEFS)
    LDFLAGS = $($(os)_ldlibs)


In similar fashion, the list of object files to be built is expanded
to include platform specific object files.
This Makefile feature allows us to separate platform specific
peculiarities without the mess of ``autoconf`` and ``automake``.

What is in the *tools/* subdirectory?
=====================================
The *tools* subdirectory has several utility scripts that are useful
for the productive programmer.

mkgetopt.py
-----------
This script generates command line parsing routines from a human readable
specification file. For more details, see *tools/mkgetopt-manual.rst*.
A fully usable example specification is in *tools/example.in*.

depweed.py
----------
Parse ``gcc -MM -MD`` output and validate each of the dependents. If
any dependent file doesn't exist, then the owning ``.d`` file is
deleted. This script is most-useful in a GNUmakefile: instead of
``include $(depfiles)``, one can now do::

    include $(shell depweed.py $(depfiles))

This makes sure that invalid dependencies never make it into the
Makefile.

The sample ``Sample-GNUMakefile`` in the top-dir is a good reference for
incorporating these ideas and library into a larger program.

.. vim: ft=rst:sw=4:ts=4:expandtab:tw=78:
