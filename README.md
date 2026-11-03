# Novi

Novi is a small interpreted language written in C++. The first milestone is a calculator that can read expressions such as `1 + 2 * 3` and print `7`.

## Build and run in VS Code

The project uses the Apple Clang compiler installed with the macOS Command Line Tools. In VS Code, press **⌘⇧B** to build. To build and start Novi, open the Command Palette (**⇧⌘P**), choose **Tasks: Run Task**, then select **Run Novi**.

Type a line and press Enter. Type `exit` to quit.

## VS Code

Open this folder in VS Code and install Microsoft's **C/C++** extension if it is not already installed. The project task compiles `src/main.cpp` with the macOS SDK headers.

## Learning milestones

1. Read one line and tokenize numbers and `+ - * / ( )`.
2. Parse expressions with precedence and parentheses.
3. Evaluate the expression and report readable errors.
4. Add variables and a `print` statement.

The initial implementation is intentionally a calculator. We will add each language feature in small steps so the code remains understandable.
