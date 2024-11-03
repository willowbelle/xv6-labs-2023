#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

void find(const char *path, const char *filename)
{
  char buf[512];
  int fd;
  struct dirent de;
  struct stat st;

  // Open the directory
  if ((fd = open(path, 0)) < 0)
  {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // Get the status of the file descriptor
  if (fstat(fd, &st) < 0)
  {
    fprintf(2, "find: cannot fstat %s\n", path);
    close(fd);
    return;
  }

  // Ensure the path is a directory
  if (st.type != T_DIR)
  {
    fprintf(2, "usage: find <directory> <filename>\n");
    close(fd);
    return;
  }

  // Prepare the buffer for constructing file paths
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
  {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  char *path_end = buf + strlen(buf);
  *path_end++ = '/';

  // Read directory entries
  while (read(fd, &de, sizeof(de)) == sizeof(de))
  {
    if (de.inum == 0)
    {
      continue;
    }

    // Construct the full path
    memmove(path_end, de.name, DIRSIZ);
    path_end[DIRSIZ] = 0;

    if (stat(buf, &st) < 0)
    {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    // Recursively search directories, ignoring "." and ".."
    if (st.type == T_DIR && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0)
    {
      find(buf, filename);
    }
    else if (strcmp(filename, de.name) == 0)
    {
      printf("%s\n", buf);
    }
  }

  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(2, "usage: find <directory> <filename>\n");
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
