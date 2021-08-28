#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main() {
        int ptoc[2];
        int ctop[2];
        pipe(ptoc);
        pipe(ctop);
        pid_t child;
        int count = 0;
        clock_t start = clock();
        if ((child = fork()) == 0) {
            close(0);
            dup(ptoc[0]);
            while ((clock() - start) < (start + 5 * CLOCKS_PER_SEC)) {
                // === rcv char from parent
                char buf = 0;
                read(0, &buf, 1);
                assert(buf == 0x42);

                // ==== send char to parent
                char buf_send = 1;
                write(ctop[1], &buf_send, 1);
            }
            // End of loop, send kill signal
            char buf_send = 0xff;
            write(ctop[1], &buf_send, 1);

        } else {
            close(ptoc[0]);

            while (1) {
                // send char to child
                char send = 0x42;
                write(ptoc[1], &send, 1);

                // ==== receive char from child
                char rcv_buf;
                read(ctop[0], &rcv_buf, 1);
                if (rcv_buf == -1) {
                    break;
                }
                assert(rcv_buf == 1);
                count++;
            }
            int wstatus;
            wait(&wstatus);
            printf("exchanges per second: %d\n", count / 5);
        }
        return 0;
}
