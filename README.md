# Custom Shell Implementation

A fully-featured Unix-like shell written in C, supporting built-in commands, process management, I/O redirection, piping, and background job control.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Compilation](#compilation)
- [Running the Shell](#running-the-shell)
- [Built-in Commands](#built-in-commands)
- [Advanced Features](#advanced-features)
- [Signal Handling](#signal-handling)
- [Project Structure](#project-structure)
- [Requirements](#requirements)

## 🎯 Overview

This shell is a custom command-line interpreter that provides a Unix-like shell environment with support for:
- Custom built-in commands
- Process execution and management
- Background and foreground job control
- I/O redirection
- Command piping
- Command history with replay functionality
- Signal handling (Ctrl+C, Ctrl+Z)

## ✨ Features

### Built-in Commands
- **hop** - Navigate between directories with support for relative/absolute paths, `~`, and `-`
- **reveal** - List directory contents with optional flags `-a` (show hidden) and `-l` (long format)
- **log** - Display command history (stores last 15 unique commands)
- **replay** - Re-execute a command from history
- **activities** - Display all running and stopped background processes
- **ping** - Send signals to processes by PID
- **fg** - Bring background jobs to foreground
- **bg** - Continue stopped background jobs

### Advanced Capabilities
- **I/O Redirection**: Support for `>`, `>>`, and `<`
- **Piping**: Chain multiple commands using `|`
- **Background Execution**: Run processes in background using `&`
- **Job Control**: Full support for process suspension (Ctrl+Z), resumption, and management
- **Signal Handling**: Proper handling of SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
- **Command Chaining**: Execute multiple commands sequentially using `;`

## 🔨 Compilation

### Prerequisites
- GCC compiler
- POSIX-compliant Unix/Linux system (macOS, Linux)
- Make utility

### Build Instructions

1. **Clone or navigate to the project directory:**
```bash
cd /path/to/shell
```

2. **Compile using make:**
```bash
make
```

This will:
- Compile all source files from the `src/` directory
- Generate object files in the `obj/` directory
- Create the executable `shell.out`

3. **Clean build artifacts (optional):**
```bash
make clean
```

## 🚀 Running the Shell

After successful compilation, run the shell:

```bash
./shell.out
```

You should see a prompt like:
```
<username@hostname:~>
```

To exit the shell, press `Ctrl+D` or the shell will handle EOF.

## 📖 Built-in Commands

### 1. hop (Change Directory)

Navigate between directories with enhanced features.

**Syntax:**
```bash
hop [path1] [path2] ... [pathN]
```

**Examples:**
```bash
hop ~               # Go to shell home directory
hop -               # Go to previous directory
hop /usr/local      # Go to absolute path
hop ../..           # Go to relative path
hop dir1 dir2       # Navigate through multiple directories
```

### 2. reveal (List Directory)

Display directory contents with various options.

**Syntax:**
```bash
reveal [flags] [path]
```

**Flags:**
- `-a` : Show hidden files (starting with `.`)
- `-l` : Long format (one entry per line)

**Examples:**
```bash
reveal              # List current directory
reveal -a           # Show all files including hidden
reveal -l           # Long format listing
reveal -al /home    # Show all files in /home with long format
reveal ~            # List home directory
```

### 3. log (Command History)

Display the command history (stores last 15 unique commands).

**Syntax:**
```bash
log
```

**Note:** The `log` command itself is not stored in history.

### 4. replay (Execute from History)

Re-execute a command from history by its index.

**Syntax:**
```bash
replay <index>
```

**Example:**
```bash
log                 # View command history
replay 3            # Execute the 3rd command from history
```

### 5. activities (Process List)

Display all currently running and stopped background processes, sorted lexicographically by command name.

**Syntax:**
```bash
activities
```

**Output Format:**
```
[PID]: command_name - Running/Stopped
```

### 6. ping (Send Signal)

Send a signal to a process by its PID.

**Syntax:**
```bash
ping <pid> <signal_number>
```

**Example:**
```bash
ping 1234 9         # Send signal 9 (SIGKILL) to process 1234
ping 5678 15        # Send signal 15 (SIGTERM) to process 5678
```

**Note:** Signal number is taken modulo 32.

### 7. fg (Foreground)

Bring a background or stopped job to the foreground.

**Syntax:**
```bash
fg [job_id]
```

**Examples:**
```bash
fg                  # Bring most recent background job to foreground
fg 2                # Bring job with ID 2 to foreground
```

### 8. bg (Background)

Resume a stopped background job.

**Syntax:**
```bash
bg <job_id>
```

**Example:**
```bash
bg 1                # Resume stopped job with ID 1 in background
```

## 🔧 Advanced Features

### I/O Redirection

**Output Redirection:**
```bash
echo "Hello" > file.txt          # Overwrite file
echo "World" >> file.txt         # Append to file
reveal -l > output.txt           # Redirect built-in command output
```

**Input Redirection:**
```bash
sort < input.txt                 # Read input from file
```

**Combined:**
```bash
sort < input.txt > output.txt    # Input from file, output to file
```

### Piping

Chain multiple commands together:

```bash
ls | grep ".c"                   # List only .c files
cat file.txt | sort | uniq       # Sort and remove duplicates
ps aux | grep python             # Find python processes
```

**Note:** Pipes work with both built-in and external commands.

### Background Execution

Run commands in the background using `&`:

```bash
sleep 100 &                      # Run sleep in background
./long_running_script.sh &       # Run script in background
```

The shell displays:
```
[job_id] pid
```

### Command Chaining

Execute multiple commands sequentially using `;`:

```bash
hop /tmp ; reveal ; hop ~        # Multiple commands in sequence
echo "Start" ; sleep 2 ; echo "End"
```

### Combining Features

You can combine multiple features:

```bash
# Piping with redirection
cat file.txt | grep "pattern" > results.txt

# Background with redirection
./script.sh > output.log 2>&1 &

# Command chaining with I/O redirection
echo "Test 1" > log.txt ; echo "Test 2" >> log.txt

# Complex pipeline
ps aux | grep python | sort -k3 -r | head -5
```

## ⚡ Signal Handling

The shell implements robust signal handling:

### Ctrl+C (SIGINT)
- Sends SIGINT to the foreground process (if any)
- Shell itself ignores SIGINT and continues running
- Background processes are not affected

### Ctrl+Z (SIGTSTP)
- Suspends the current foreground process
- Adds the suspended process to the background job list
- Process can be resumed using `fg` or `bg` commands

### Background Process Termination
- Shell automatically reaps terminated background processes
- Displays exit status (normal/abnormal) when background process completes

### Clean Exit (Ctrl+D)
- Terminates all background processes
- Properly cleans up resources
- Displays "logout" message

## 📁 Project Structure

```
shell/
├── Makefile              # Build configuration
├── README.md             # This file
├── requirements.txt      # Python dependencies (for testing)
├── include/              # Header files
│   ├── executor.h        # Command execution declarations
│   ├── parser.h          # Command parsing declarations
│   ├── prompt.h          # Prompt display declarations
├── src/                  # Source files
│   ├── main.c            # Main shell loop and signal handlers
│   ├── parser.c          # Command parsing and history
│   ├── executor.c        # Command execution logic
│   ├── prompt.c          # Prompt generation
├── obj/                  # Object files (generated)
├── tester/               # Test scripts
│   ├── requirements.txt
│   └── test.py
└── shell.out             # Compiled executable (generated)
```

### File Descriptions

- **main.c**: Entry point, main loop, signal handler setup, initialization
- **parser.c**: Tokenization, command parsing, history management (15 commands)
- **executor.c**: Command execution, built-ins, piping, redirection, job control
- **prompt.c**: Dynamic prompt generation with username, hostname, and current directory

## 📦 Requirements

### System Requirements
- Unix-like operating system (Linux, macOS, BSD)
- GCC compiler with C99 support
- POSIX.1-2008 compliant system

### Compilation Flags
```
-std=c99
-D_POSIX_C_SOURCE=200809L
-D_XOPEN_SOURCE=700
-Wall -Wextra -Werror
```

## 🧪 Testing

The project includes test scripts in the `tester/` directory:

```bash
cd tester
pip install -r requirements.txt
python test.py
```

## 💡 Usage Examples

### Basic Usage
```bash
# Start the shell
./shell.out

# Navigate and explore
hop /usr/local/bin
reveal -la
hop ~

# Run commands
ls -l | grep ".txt"
cat file.txt > output.txt

# Background job control
sleep 60 &
activities
fg 1
# Press Ctrl+Z to suspend
bg 1
```

### Advanced Workflow
```bash
# Create and navigate to a directory
mkdir test_dir ; hop test_dir

# Work with files
echo "Line 1" > data.txt
echo "Line 2" >> data.txt
cat data.txt | sort > sorted.txt

# Process management
./heavy_computation.sh &
ping 1234 15
activities
fg 1
```

## 🐛 Known Limitations

- Command history limited to 15 unique commands
- Maximum 100 background jobs (configurable in executor.h)
- Pipes are limited to standard input/output (no stderr redirection in pipes)

## 👨‍💻 Author

Created as part of Operating Systems Network course mini-project.

## 📝 License

This project is created for educational purposes.

---

**Note:** This shell is designed for educational purposes and may not include all features of production shells like bash or zsh.
