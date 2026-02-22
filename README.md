# storeME: Persistent Key-Value Database

> A lightweight, persistent key-value database engineered in C, focusing on memory safety, efficient file I/O operations, and reliable state recovery.

## System Architecture & Data Flow

```text
[ Client Request ] 
       │
       ▼
( Memory Manager ) ──► Validates Input & Allocates Memory
       │
       ▼
[ Hash Map Index ] ──► In-memory state for O(1) lookups
       │
       ▼
( File I/O Stream) ──► Serializes data safely to disk
       │
       ▼
[ .TXT Storage   ] ──► Persistent state (Restored on reboot)
