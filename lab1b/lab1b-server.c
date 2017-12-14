#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>    
#include <arpa/inet.h> 
#include <signal.h>
#include <sys/wait.h>
#include <poll.h>
#include <mcrypt.h>
#include <fcntl.h>

#define PORT 'p'
#define ENCRYPT 'e'

const char lf='\n';
const char cr='\r';

MCRYPT efd;
MCRYPT defd;

//var used to keep track of single child
pid_t child_pid = -1;

void getexitstatus()
{
	int extsatus;
	waitpid(child_pid, &extsatus, 0);
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", WTERMSIG(extsatus), WEXITSTATUS(extsatus));

}

void cryptexit()
{
	mcrypt_generic_deinit(efd);
	mcrypt_module_close(efd);

	mcrypt_generic_deinit(defd);
	mcrypt_module_close(defd);
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

int main(int argc, char *argv[])
{
    int prt_opt = -1;
    int enc_opt = 0;
    
    int socket_desc, client_sock;
    struct sockaddr_in server, client;
    
    struct option long_options[] =
    {
        {"port", required_argument, NULL, PORT},
		{"encrypt", required_argument, NULL, ENCRYPT},
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
            case PORT:
            {
				
                prt_opt = 1;
                
                //Create socket
                socket_desc = socket(AF_INET, SOCK_STREAM, 0);
                if (socket_desc == -1)
                {
                    fprintf(stderr, "Error! Could not create socket");
                    exit(1);
                }
                     
                //Prepare the sockaddr_in structure
                int temp_port = atoi(optarg);
                server.sin_family = AF_INET;
                server.sin_addr.s_addr = INADDR_ANY;
                server.sin_port = htons( temp_port );
                     
                //Bind
                if(bind(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0)
                {
                    //print the error message
                    fprintf(stderr, "Error! Bind failed.");
                    exit(1);
                }
                //puts("bind done/n");
                     
                //Listen
                listen(socket_desc, 3);
                     
                //Accept and incoming connection
                //puts("Waiting for incoming connections.../n");
                     
                //accept connection from an incoming client
                socklen_t c = sizeof(client);
                client_sock = accept(socket_desc, (struct sockaddr*) &client, (socklen_t*) &c);
                if (client_sock < 0)
                {
                    fprintf(stderr, "Error! Accept failed");
                    exit(1);
                }
                //puts("Connection accepted/n");
                break;
            }
            case ENCRYPT:
            {
                enc_opt = 1;
                char encryption_key[128];
                int kfd = open(optarg, O_RDONLY);
                int keylen = read(kfd, encryption_key, 128);
                
                efd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
                defd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
                if ((efd==MCRYPT_FAILED) || (defd==MCRYPT_FAILED)) {
                    fprintf(stderr, "Encryption error!\n");
                    exit(1);
                }
                
				int i;
                char* IV1= malloc(mcrypt_enc_get_iv_size(efd));
                for (i=0; i< mcrypt_enc_get_iv_size( efd); i++) {
					IV1[i]=0; 
				}
				char* IV2= malloc(mcrypt_enc_get_iv_size(defd));
				for (i=0; i< mcrypt_enc_get_iv_size( defd); i++) {
					IV2[i]=0;
				}
                
				int poserror1, poserror2;
				poserror1=mcrypt_generic_init( efd, encryption_key, keylen, IV1);
				poserror2=mcrypt_generic_init( defd, encryption_key, keylen, IV2);
				if ((poserror1<0) || (poserror2<0)) {
					fprintf(stderr, "Encryption initalization error!\n");
					exit(1);
				}
				
				atexit(cryptexit);
                break;
            }
            default:
            {
                fprintf(stderr, "Unrecognized argument, correct usage includes: --port=port#\n");
                exit(1);
            }
			
        }
    }
    
    if (prt_opt < 0)
    {
        fprintf(stderr, "Error! The port command is mandatory. Correct usage: --port=port# --encrypt=keyfile\n");
        exit(1);
    }
    
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
        fds[0].fd = client_sock;
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
					count = read(client_sock, buffer, 2048);
                    
					if(enc_opt)
					    mdecrypt_generic(defd, buffer, count);

                    int index = 0;
                    char tbuf;
                    while ( index < count)
                    {
                        tbuf = buffer[index];
                                
                        if(tbuf == 0x03)
                        {
                            kill(child_pid, SIGINT);
                        }
                        if(tbuf == 0x04)
                        {
                            close(to_child_pipe[1]);
                            close(from_child_pipe[0]);
                            kill(child_pid, SIGHUP);
                            exit(0);
                        }
                        if(tbuf == '\n' || tbuf == '\r')
                        {
                            write(to_child_pipe[1], &lf, 1);
                        }
                        else
                        {
                            write(to_child_pipe[1], &tbuf, 1);
                        }
                        index++;
                    }
                }
                else if ((fds[1].revents&POLLIN) == POLLIN)
                {   
                    count = read(from_child_pipe[0], buffer, 2048);
					
                    if (count > 0)
                    {
                        if (enc_opt)
                            mcrypt_generic(efd, buffer, count);
                        write(client_sock, buffer, count);
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
        fprintf(stderr, "Error! fork() failed!\n");
        exit(1);
    }
    
    exit(0);
}