#include "../kernel/types.h"
#include "user.h"

void piperecursion(int input_fd)
{
  int prime;

  // Read the first number from the input pipe
  if (read(input_fd, &prime, sizeof(int)) <= 0)
  {
    close(input_fd);
    return;
  }

  printf("prime %d\n", prime);

  int pipe_fds[2];
  if (pipe(pipe_fds) < 0)
  {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0)
  {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid > 0)
  {
    // Parent process: filter numbers and write to the next pipe
    close(pipe_fds[0]); // Close unused read end
    int number;

    while (read(input_fd, &number, sizeof(int)) > 0)
    {
      if (number % prime != 0)
      {
        if (write(pipe_fds[1], &number, sizeof(int)) != sizeof(int))
        {
          fprintf(2, "write failed\n");
          exit(1);
        }
      }
    }

    close(pipe_fds[1]); // Close write end after use
    wait(0);            // Wait for child process to finish
  }
  else
  {
    // Child process: continue the pipeline
    close(pipe_fds[1]);         // Close unused write end
    close(input_fd);            // Close the previous pipe's read end
    piperecursion(pipe_fds[0]); // Recursive call with new read end
  }
}

int main()
{
  int pipe_fds[2];
  if (pipe(pipe_fds) < 0)
  {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0)
  {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid > 0)
  {
    // Parent process: write numbers to the pipe
    close(pipe_fds[0]); // Close unused read end
    for (int i = 2; i <= 280; i++)
    {
      if (write(pipe_fds[1], &i, sizeof(int)) != sizeof(int))
      {
        fprintf(2, "write failed\n");
        exit(1);
      }
    }
    close(pipe_fds[1]); // Close write end after use
    wait(0);            // Wait for child process to finish
  }
  else
  {
    // Child process: start the pipeline
    close(pipe_fds[1]);         // Close unused write end
    piperecursion(pipe_fds[0]); // Start recursion with read end
  }

  exit(0);
}
