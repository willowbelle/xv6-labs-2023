#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXSZ 512

// Finite state machine state definitions
enum state
{
  S_WAIT,         // Waiting for argument input, initial state or current character is a space
  S_ARG,          // Inside an argument
  S_ARG_END,      // Argument ended
  S_ARG_LINE_END, // Argument followed by a newline, e.g., "arg\n"
  S_LINE_END,     // Space followed by a newline, e.g., "arg  \n"
  S_END           // End of input, EOF
};

// Character type definitions
enum char_type
{
  C_SPACE,
  C_CHAR,
  C_LINE_END
};

// Function to determine character type
enum char_type get_char_type(char c)
{
  switch (c)
  {
  case ' ':
    return C_SPACE;
  case '\n':
    return C_LINE_END;
  default:
    return C_CHAR;
  }
}

// State transition function
enum state transform_state(enum state cur, enum char_type ct)
{
  switch (cur)
  {
  case S_WAIT:
    if (ct == C_SPACE)
      return S_WAIT;
    if (ct == C_LINE_END)
      return S_LINE_END;
    if (ct == C_CHAR)
      return S_ARG;
    break;
  case S_ARG:
    if (ct == C_SPACE)
      return S_ARG_END;
    if (ct == C_LINE_END)
      return S_ARG_LINE_END;
    if (ct == C_CHAR)
      return S_ARG;
    break;
  case S_ARG_END:
  case S_ARG_LINE_END:
  case S_LINE_END:
    if (ct == C_SPACE)
      return S_WAIT;
    if (ct == C_LINE_END)
      return S_LINE_END;
    if (ct == C_CHAR)
      return S_ARG;
    break;
  default:
    break;
  }
  return S_END;
}

// Clear argument list after a certain index
void clear_argv(char *argv[MAXARG], int start_index)
{
  for (int i = start_index; i < MAXARG; ++i)
  {
    argv[i] = 0;
  }
}

int main(int argc, char *argv[])
{
  if (argc - 1 >= MAXARG)
  {
    fprintf(2, "xargs: too many arguments.\n");
    exit(1);
  }

  char lines[MAXSZ];
  char *p = lines;
  char *x_argv[MAXARG] = {0}; // Argument pointer array, all initialized to null pointers

  // Store original arguments
  for (int i = 1; i < argc; ++i)
  {
    x_argv[i - 1] = argv[i];
  }

  int arg_start = 0;                 // Argument start index
  int arg_end = 0;                   // Argument end index
  int arg_count = argc - 1;          // Current argument index
  enum state current_state = S_WAIT; // Initial state set to S_WAIT

  while (current_state != S_END)
  {
    // Read character from input, exit if no more input
    if (read(0, p, sizeof(char)) != sizeof(char))
    {
      current_state = S_END;
    }
    else
    {
      current_state = transform_state(current_state, get_char_type(*p));
    }

    if (++arg_end >= MAXSZ)
    {
      fprintf(2, "xargs: arguments too long.\n");
      exit(1);
    }

    switch (current_state)
    {
    case S_WAIT: // Move argument start pointer forward
      ++arg_start;
      break;
    case S_ARG_END: // Argument ended, store argument address in x_argv array
      x_argv[arg_count++] = &lines[arg_start];
      arg_start = arg_end;
      *p = '\0'; // Replace with string terminator
      break;
    case S_ARG_LINE_END: // Store argument address and execute command
      x_argv[arg_count++] = &lines[arg_start];
      // Fall through to handle line ending
    case S_LINE_END: // Line ended, execute command
      arg_start = arg_end;
      *p = '\0';
      if (fork() == 0)
      {
        exec(argv[1], x_argv);
      }
      arg_count = argc - 1;
      clear_argv(x_argv, arg_count);
      wait(0);
      break;
    default:
      break;
    }

    ++p; // Move to next character storage position
  }
  exit(0);
}
