# malloc-implementation

A truncated implementation of `malloc` for learning purposes.

This repository provides a simplified version of memory allocation functions (`malloc` and related routines) written in C. The goal is to offer a learning platform for understanding how dynamic memory allocation works under the hood.

## Features

- Basic memory allocation similar to `malloc`
- Accessible and annotated source code for educational use
- Minimal dependencies—standalone C code

## Motivation

Understanding how memory allocators work is a critical part of system programming. By exploring a stripped-down version of `malloc`, learners gain insight into data structures, memory management techniques, and how real-world heap allocators are implemented.

## Usage

1. **Clone the repository:**
    ```sh
    git clone https://github.com/fredricngo/malloc-implementation.git
    ```

2. **Build:**
    ```sh
    gcc -o malloc_test main.c malloc.c
    ```

3. **Run:**
    ```sh
    ./malloc_test
    ```

## Files

- `malloc.c` – Core allocation logic
- `main.c` – Sample/test usage
- (Add any other important files here)

## Contributing

Contributions are welcome! Feel free to submit pull requests, raise issues, or suggest enhancements.

## License

This project is licensed under the MIT License.

---

*Created for educational purposes. This implementation is not suitable for production use.*
