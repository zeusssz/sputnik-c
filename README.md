![Sputnik](https://github.com/user-attachments/assets/53a1dd79-90c3-4d5c-9165-6778d9715da8) # Sputnik

A simple multithreaded HTTP server implemented in C. This server is capable of serving static files from a designated directory and handling multiple client connections concurrently. 
I certainly plan on improving this project, as I plan on using it for one of my projects.

## Features

- Serve static files (HTML, CSS, JS) from the `www/` directory.
- Multithreaded to handle multiple clients simultaneously.
- Simple logging to track server activity.
- Configurable via the `Makefile`.

## Project Structure

```
root/
├── src/
│   ├── main.c
│   ├── server.c
│   └── client.c
├── include/
│   ├── server.h
│   └── client.h
├── www/
├── logs/
└──Makefile
```

## Prerequisites

- GCC or any compatible C compiler.
- `make` utility for building the project.

## Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/my_http_server.git
   cd my_http_server
   ```

2. **Build the server**:
   ```bash
   make
   ```

## Usage

1. **Run the server**:
   ```bash
   make run
   ```

   The server will start on port `8080`. You can change the port by editing the `run` rule in the `Makefile`.

2. **Access the server**:
   Open a web browser and navigate to `http://localhost:8080` to see the served static files.

## Configuration

- **Static Files**: Place your static files (HTML, CSS, JavaScript, etc.) in the `www/` directory.
- **Logs**: Server logs are stored in the `logs/` directory.

## Cleaning Up

- **Remove binaries and object files**:
  ```bash
  make clean
  ```

- **Remove log files**:
  ```bash
  make clean-logs
  ```

## Contributing

Feel free to fork this repository, open issues, and submit pull requests. Contributions are greatly appreciated.
