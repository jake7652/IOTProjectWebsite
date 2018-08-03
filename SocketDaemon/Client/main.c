// Client side C/C++ program to demonstrate Socket programming
2
#include <stdio.h>
3
#include <sys/socket.h>
4
#include <stdlib.h>
5
#include <netinet/in.h>
6
#include <string.h>
7
#include <arpa/inet.h>
8
#include <unistd.h>
9
#include <sys/wait.h>
10
#define PORT 8080
11
#define BUF_LEN 2048
12
​
13
//trims off the newlines characters from the fgets read
14
char * fTrim (char s[]) {
15
  int i = strlen(s)-1;
16
  if ((i > 0) && (s[i] == '\n'))
17
    s[i] = '\0';
18
  return s;
19
}
20
​
21
static int exec_prog(const char **argv)
22
{
23
    pid_t   my_pid;
24
    int     status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;
25
​
26
    if (0 == (my_pid = fork())) {
27
            if (-1 == execve(argv[0], (char **)argv , NULL)) {
28
                    perror("child process execve failed [%m]");
29
                    return -1;
30
            }
31
    }
32
​
33
#ifdef WAIT_FOR_COMPLETION
34
    timeout = 1000;
35
​
36
    while (0 == waitpid(my_pid , &status , WNOHANG)) {
37
            if ( --timeout < 0 ) {
38
                    perror("timeout");
39
                    return -1;
40
            }
41
            sleep(1);
42
    }
43
​
44
    printf("%s WEXITSTATUS %d WIFEXITED %d [status %d]\n",
45
            argv[0], WEXITSTATUS(status), WIFEXITED(status), status);
46
​
47
    if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
48
            perror("%s failed, halt system");
49
            return -1;
