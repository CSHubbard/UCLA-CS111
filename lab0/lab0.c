//Cody Hubbard
//CodySpHubbard@Gmail.com
//004483389

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'

void simple_signal_handler() 
{
    fprintf(stderr, "SIGSEGV caught\n");
    exit(4);
}

int main(int argc, char *argv[])
{
    struct option long_options[] =
    {
        {"input", required_argument, NULL, INPUT},
        {"output", required_argument, NULL, OUTPUT},
        {"segfault", no_argument, NULL, SEGFAULT},
        {"catch", no_argument, NULL, CATCH},
        {0,0,0,0}
    };

////////////////////////options
    int ret;
    
    while(1)
    {
    ret = getopt_long(argc, argv, "", long_options, NULL);
        if (ret == -1) 
        {
            break;
        }
        switch (ret) 
        {
            case INPUT:
            {
                int ifd = open(optarg, O_RDONLY);
                if (ifd >= 0) {
                	close(0);
                	dup(ifd);
                	close(ifd);
                }
               if (ifd < 0) {
                    fprintf(stderr, "Error opening file\n");
                    exit(2);
                }
                break;
            }
            case OUTPUT:
            {
                int ofd = creat(optarg, 0666);
                if (ofd >= 0) {
                	close(1);
                	dup(ofd);
                	close(ofd);
                }
               if (ofd < 0) {
                    fprintf(stderr, "Error creating file\n");
                    exit(3);
                }
                break;
            }
            case SEGFAULT:
            {
                char* sf = NULL;
                char rip = *sf;
				rip = rip;
            }
            case CATCH:
            {
                signal(SIGSEGV, simple_signal_handler);
                break;
            }
            default:
            {
                fprintf(stderr, "Unrecognized argument, correct usage includes: --input=filename --output=filename --catch --segfault");
                exit(1);
            }
        }
    }

///////////stuff for read/write
char* buf;
int readstatus;
int err_read;
    while(1)
    {
        readstatus = read(0, buf, sizeof(char));
        if (readstatus == 0)
        {  exit(0); }
        else if (readstatus < 0)
        {
            err_read = errno;
            fprintf(stderr, "\nRead error\n");
            exit(err_read);
        }
        else
        {
            write(1,buf,sizeof(char));
        }
    }
}


