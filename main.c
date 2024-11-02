#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ENV_CONFIG "PIPE_CONFIG"

struct state {
  bool alt;
  char key;
};

void ps_send(struct state *s) {
  write(STDOUT_FILENO, s, sizeof(*s));
  fflush(NULL);
}

void ps_show(struct state *s) {
  printf("[client] alt: %s, key: %c\n", s->alt ? "True " : "False", s->key);
  fflush(NULL);
}
int main(int argc, char *argv[]) {
  char *config_path = getenv(ENV_CONFIG);
  if (!config_path) {
    int c;
    struct state s = {0};
    while ((c = getopt(argc, argv, "ak:")) != -1) {
      switch (c) {
      case 'a':
        s.alt = true;
        break;
      case 'k':
        s.key = *optarg;
      }
    }
    if (argc != 1)
      ps_send(&s);
    return 0;
  }
  // now it is server side run
  int piperw[2];
  pipe(piperw);
  pid_t pid = fork();
  if (pid == 0) {
    unsetenv(ENV_CONFIG);
    close(piperw[0]);
    dup2(piperw[1], STDOUT_FILENO);
    close(piperw[1]);
    execl("/bin/sh", "sh", config_path, NULL);
  }
  close(piperw[1]);
  char buffer[1024] = {0};
  ssize_t nbytes;
  int have_read = 0;
  do {
    nbytes = read(piperw[0], buffer, 1024);
    if (nbytes == 0) {
      printf("[server] > pipe close\n");
      break;
    }
    have_read++;
    struct state *s = calloc(1, nbytes);
    memcpy(s, buffer, nbytes);
    ps_show(s);
    /* buffer[1023] = '\0'; */
    /* printf("[server] %s", buffer); */
    printf("[server] > read done, have read %d, size %ld\n", have_read, nbytes);
    fflush(NULL);
    free(s);
  } while (nbytes > 0);
  printf("[server] > all read done, have read %d key config\n", have_read);
  close(piperw[0]);
  return 0;
}
