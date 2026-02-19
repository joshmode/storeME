# storeME user guide

welcome to storeME! a simple, persistent key-value store. it's lightweight, fast, and remembers your data across restarts.

## features

- **simple**: just type commands and hit enter.
- **persistent**: data is automatically saved to `wal.log`.
- **fast**: implemented using hash tables.

## getting started

### prerequisites

you'll need a c compiler (like `gcc`) and `make`.

### build

run the following command in your terminal:

```bash
make
```

this will create a `db` executable.

### usage

start the program:

```bash
./db
```

you'll see the welcome message:

```
Thanks for using storeME! Follow me on github @joshmode
Here are the commands you can use:
  SET <key> <value>  - Store a key and its value
  GET <key>          - Retrieve a value by its key
  DELETE <key>       - Remove a key-value pair
  EXIT               - Quit the program

>
```

## commands

### 1. SET
stores a key-value pair.

**syntax:** `SET <key> <value>`

**example:**
```
> SET vibe chill
OK
```

### 2. GET
retrieves the value for a key.

**syntax:** `GET <key>`

**example:**
```
> GET vibe
chill
```

if the key doesn't exist:
```
> GET stress
(nil)
```

### 3. DELETE
removes a key-value pair.

**syntax:** `DELETE <key>`

**example:**
```
> DELETE vibe
OK
```

### 4. EXIT
exits the program.

**syntax:** `EXIT`

**example:**
```
> EXIT
```

## persistence

all modifications (`SET`, `DELETE`) are logged to `wal.log`. avoid editing this file directly to ensure data integrity.

## clean up

to remove compiled files and logs:

```bash
make clean
```

enjoy using storeME!
