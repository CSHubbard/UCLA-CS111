#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>    
#include <sys/socket.h>    
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <mcrypt.h>

// var used to remember original terminal attributes.
struct termios saved_attributes;

MCRYPT efd;
MCRYPT defd;

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

void cryptexit()
{
	mcrypt_generic_deinit(efd);
	mcrypt_module_close(efd);

	mcrypt_generic_deinit(defd);
	mcrypt_module_close(defd);
}

#define PORT 'p'
#define LOG 'l'
#define ENCRYPT 'e'

const char lf='\n';
const char cr='\r';

int main(int argc, char *argv[])
{
    set_input_mode();

    int log_opt = 0;
    int lfd = -1;
    int enc_opt = 0;
    int prt_opt = -1;
	int shouldkill=0;
    
    int sock;
    struct sockaddr_in server;
    
    struct option long_options[] =
    {
        {"port", required_argument, NULL, PORT},
        {"log", required_argument, NULL, LOG},
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
                 
                sock = socket(AF_INET , SOCK_STREAM , 0);
                if (sock == -1)
                {
                    fprintf(stderr, "Error! Could not create socket");
                    exit(1);
                }
                int temp_port = atoi(optarg);
                server.sin_family = AF_INET;
                server.sin_port = htons( temp_port );
             
                if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0)
                {
                    fprintf(stderr, "Error! Connect failed.\n");
                    exit(1);
                }
                
                break;
            }
            case LOG:
            {
                log_opt = 1;
                lfd = creat(optarg, 0666);
                if (lfd < 0) {
                    fprintf(stderr, "Error creating file\n");
                    exit(1);
                }
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
                fprintf(stderr, "Unrecognized argument, correct usage includes: --port=port#, --log=filename, --encrypt=keyfile\n");
                exit(1);
            }
        }
    }
    
    if (prt_opt < 0)
    {
        fprintf(stderr, "Error! The port command is mandatory. Correct usage: --port=port#\n");
        exit(1);
    }
    
    struct pollfd fds[2];
    fds[0].fd = 0;
    fds[1].fd = sock;
    fds[0].events = POLLIN | POLLHUP | POLLERR;
    fds[1].events = POLLIN | POLLHUP | POLLERR;
    
    char logbuffer[2048];
    int logcount = 0;
    
    char buffer[2048];
    int count = 0;
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
						shouldkill=1;
						//exit(0);
                    }
                    if(tbuf == 0x04)
                    {
                        printf("^D\n");
						shouldkill=1;
						//exit(0);
                    }
                    if(tbuf == '\n' || tbuf == '\r')
                    {
                        write(1, &cr, 1);
                        write(1, &lf, 1);
                        if(enc_opt) {
                            buffer[index]=lf;
                            mcrypt_generic(efd, &tbuf, 1);
                            write(sock, &tbuf, 1);
							buffer[index] = tbuf;
                        } 
                        else
                            write(sock, &lf, 1);
                    }
                    else 
                    {
						write(1, &tbuf, 1);
                        if(enc_opt) {
							mcrypt_generic(efd, &tbuf, 1); 
							buffer[index] = tbuf;
						}
                        write(sock, &tbuf, 1);
                    }
                    index++;
                }
                    
                 if (log_opt)
                {
                    buffer[count] = '\0';
                    logcount = sprintf(logbuffer, "SENT %d bytes: %s \n", 1, buffer);
                    write(lfd, logbuffer, logcount);
                }
				
				if ( shouldkill==1)
					exit(0);
                
            }
            else if ((fds[1].revents&POLLIN) == POLLIN)
            {
                count = read(sock, buffer, 2048);
                int index = 0;
                char tbuf;
                
                if (log_opt)
                {
                    buffer[count] = '\0';
                    logcount = sprintf(logbuffer, "RECEIVED %d bytes: %s\n",(int) (count*sizeof(char)), buffer);
                    write(lfd, logbuffer, logcount);
                }
                
                if(enc_opt)
			        mdecrypt_generic(defd, buffer, count);
                
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
                exit(0);
            }
            else if(((fds[0].revents&POLLERR) == POLLERR) || ((fds[1].revents&POLLERR) == POLLERR))
            {
                exit(1);
            }
        }
    }
}