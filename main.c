#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ENV_CONFIG "PIPE_CONFIG"
#define SERVER_PID "SERVER_PID"

struct state {
  bool alt;
  char key;
};

void ps_send(struct state *s) {
  write(STDOUT_FILENO, s, sizeof(*s));
  volatile double result = 0.0;
  for (int i = 0; i < 1000000; i++) {
    result += i * 0.000001;
  }
  write(STDOUT_FILENO, s, sizeof(*s));
  fflush(NULL);
}

void ps_show(struct state *s) {
  printf("[client] alt: %s, key: %c\n", s->alt ? "True " : "False", s->key);
  fflush(NULL);
}

void void_signal(int signum) {
  (void)signum;
  return;
}

int main(int argc, char *argv[]) {
  char *pid_read = getenv(SERVER_PID);
  if (pid_read) {
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
    if (argc != 1) {
      ps_send(&s);
      pid_t pid = atoi(pid_read);
      kill(pid, SIGUSR1);
      // if error, kill -USR2
    }
    return 0;
  }
  // now it is server side run
  pid_t server_pid = getpid();
  char pid_write[20];
  snprintf(pid_write, sizeof(pid_read), "%d", server_pid);
  setenv(SERVER_PID, pid_write, 1);
  int piperw[2];
  pipe(piperw);
  pid_t pid = fork();
  if (pid == 0) {
    close(piperw[0]);
    dup2(piperw[1], STDOUT_FILENO);
    close(piperw[1]);
    char *config_path = getenv(ENV_CONFIG);
    // set default /etc/xxx if NULL
    execl("/bin/sh", "sh", config_path, NULL);
  }
  close(piperw[1]);
  char buffer[1024] = {0};
  ssize_t nbytes;
  int have_read = 0;
  signal(SIGUSR1, void_signal);
  do {
    /* sleep(1); */
    int rc = usleep(100000);
    nbytes = read(piperw[0], buffer, 1024);
    // we need to be awakened by a signal
    // if sound sleep, context is unreliable
    if (nbytes == 0 || rc == 0) {
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
