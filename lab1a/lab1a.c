#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

//var used to keep track of single child
pid_t child_pid = -1;

// var used to remember original terminal attributes.
struct termios saved_attributes;

void reset_input_mode(void)
{
  tcsetattr(0, TCSANOW, &saved_attributes);
}

void set_input_mode(void)
{
  struct termios tattr;
  //char *name;

  // Make sure stdin is a terminal.
  if (!isatty(0))
    {
      fprintf(stderr, "Not a terminal.\n");
      exit(1);
    }

  // Save the terminal attributes so we can restore them later.
  tcgetattr(0, &saved_attributes);
  atexit(reset_input_mode);

  // Set the funny terminal modes.
  tcgetattr(0, &tattr);
  //tattr.c_lflag &= ~(ICANON|ECHO); // Clear ICANON and ECHO.
  tattr.c_iflag = ISTRIP;	/* only lower 7 bits	*/
  tattr.c_oflag = 0;		/* no processing	*/
  tattr.c_lflag = 0;		/* no processing	*/
  
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &tattr);
}

void getexitstatus()
{
	int extsatus;
	waitpid(child_pid, &extsatus, 0);
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(extsatus), WEXITSTATUS(extsatus));

}

void simple_signal_handler(int signum) 
{
	if(signum == SIGPIPE)
	{
	    exit(1);
	}
	else if (signum == SIGINT)
	{
	    kill(child_pid, SIGINT);
	}
}

#define SHELL 's'
const char lf='\n';
const char cr='\r';

int main(int argc, char *argv[])
{
    set_input_mode();
    
    struct option long_options[] =
    {
        {"shell", no_argument, NULL, SHELL},
        {0,0,0,0}
    };
    
    while(1)
    {
        int ret = getopt_long(argc, argv, "", long_options, NULL);
        if (ret == -1) 
        {
            break;
        }
        switch (ret) 
        {
            case SHELL:
            {
                int to_child_pipe[2];
                int from_child_pipe[2];
                atexit(getexitstatus);
                signal(SIGPIPE, simple_signal_handler);
                signal(SIGINT, simple_signal_handler);
                
                if (pipe(to_child_pipe) == -1)
                {
                    fprintf(stderr, "pipe() failed!\n");
                    exit(1);
                }
                if (pipe(from_child_pipe) == -1)
                {
                    fprintf(stderr, "pipe() failed!\n");
                    exit(1);
                }
                
                child_pid = fork();
                
                //parent process
                if (child_pid > 0) 
                { 
                    close(to_child_pipe[0]);
                    close(from_child_pipe[1]);
                    
                    char buffer[2048];
                    int count = 0;
                    
                    struct pollfd fds[2];
                    fds[0].fd = 0;
                    fds[1].fd = from_child_pipe[0];
                    fds[0].events = POLLIN | POLLHUP | POLLERR;
                    fds[1].events = POLLIN | POLLHUP | POLLERR;
                    
                    int prs = 0;
                    while (1)
                    {
                        prs = poll(fds, 2, -1);
                        if (prs > 0)
                        {
                            if ((fds[0].revents&POLLIN) == POLLIN)
                            {
                                count = read(0, buffer, 2048);
                                int index = 0;
                                char tbuf;
                                while ( index < count)
                                {
                                    tbuf = buffer[index];
                                    
                                    if(tbuf == 0x03)
                                    {
                                        printf("^C\n");
                                        kill(child_pid, SIGINT);
                                    }
                                    if(tbuf == 0x04)
                                    {
                                        printf("^D\n");
                                        close(to_child_pipe[1]);
                                        close(from_child_pipe[0]);
                                        kill(child_pid, SIGHUP);
                                        exit(0);
                                    }
                                    if(tbuf == '\n' || tbuf == '\r')
                                    {
                                        write(1, &cr, 1);
                                        write(1, &lf, 1);
                                        write(to_child_pipe[1], &lf, 1);
                                    }
                                    else
                                    {
                                        write(1, &tbuf, 1);
                                        write(to_child_pipe[1], &tbuf, 1);
                                    }
                                    index++;
                                }
                            }
                            else if ((fds[1].revents&POLLIN) == POLLIN)
                            {
                                count = read(from_child_pipe[0], buffer, 2048);
                                int index = 0;
                                char tbuf;
                                while ( index < count)
                                {
                                    tbuf = buffer[index];
                                    if(tbuf == '\n')
                                    {
                                        write(1, &cr, 1);
                                        write(1, &lf, 1);
                                    }
                                    else
                                    {
                                        write(1, &tbuf, 1);
                                    }
                                    index++;
                                }
                            }
                            else if(((fds[0].revents&POLLHUP) == POLLHUP) || ((fds[1].revents&POLLHUP) == POLLHUP))
                            {
                                kill(child_pid, SIGHUP);
                                exit(0);
                            }
                            else if(((fds[0].revents&POLLERR) == POLLERR) || ((fds[1].revents&POLLERR) == POLLERR))
                            {
                                kill(child_pid, SIGINT);
                                exit(1);
                            }
                        }
                    }
                }  
                //child process
                else if (child_pid == 0) { 
                    close(to_child_pipe[1]);
                    close(from_child_pipe[0]);
                    dup2(to_child_pipe[0], 0);
                    dup2(from_child_pipe[1], 1);
                    dup2(from_child_pipe[1], 2);
                    close(to_child_pipe[0]);
                    close(from_child_pipe[1]);
                    
                    char *execvp_argv[2];
                    char execvp_filename[] = "/bin/bash";
                    execvp_argv[0] = execvp_filename;
                    execvp_argv[1] = NULL;
                    if (execvp(execvp_filename, execvp_argv) == -1)
                    {
                        fprintf(stderr, "execvp() failed!\n");
                        exit(1);
                    }
                }
                else { //fork() failed
                    fprintf(stderr, "fork() failed!\n");
                    exit(1);
                }
                break;
            }
            default:
            {
                fprintf(stderr, "Unrecognized argument, correct usage includes: --shell");
                exit(1);
            }
        }
    }
    
    char cbuf;
    while (1)
    {
        if (read(0, &cbuf, 1) < 0)
	    {
            fprintf(stderr, "Read Error");
            exit(1);
    	}
    	
        if(cbuf == 0x04)
            break;
        //if(cbuf == 0x03)
        //   break;
        
        if(cbuf == '\n' || cbuf == '\r')
        {
            write(1, &cr, 1);
            write(1, &lf, 1);
        }
        
        else
            write(1, &cbuf, 1);
    }
    exit(0);
}

