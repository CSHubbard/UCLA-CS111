//Cody Hubbard
//004843389
//CodySpHubbard@gmail.com
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include "mraa.h"
#include "mraa/aio.h"
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>

#define PERIOD 'p'
#define SCALE 's'
#define LOG 'l'
#define ID 'i'
#define HOST 'h'

//GLOBALS
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int bufferSize = 32;
mraa_aio_context temperature_Pin;
mraa_gpio_context button_Pin;
int sock;

//jank condition variables
int button_State = 0;
int on_opt = 1;
int prs = 0;
int input_found = 0;

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

// void button_Pressed()
// {
    // button_State = 1;
// }

//sick hack for clearning my character arrays
void clear_memory(char* array, int size)
{
    int i = 0;
    for (i=0; i < size; i++)
    {
        array[i] = 0;
    }
}

//poll stuff
void* poll_function()
{
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    // poll for stdin commands
    while (1)
    {
        while (!input_found)
        {
            pthread_mutex_lock(&lock1);
            prs = poll(fds, 2, -1);
            if (prs > 0)
            {
                input_found = 1;
            }
            pthread_mutex_unlock(&lock1);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    //optarg variables
    int snoozetime = 1;
    int log_opt = 0;
    int lfd = -1;
    char temp_Scale = 'F';
    int report_opt = 1;
	char* usr_id = "";
	char* host = "";
	int port_num = 0;
    
    struct option long_options[] =
    {
        {"period", required_argument, NULL, PERIOD},
        {"scale", required_argument, NULL, SCALE},
        {"log", required_argument, NULL, LOG},
		{"id", required_argument, NULL, ID},
		{"host", required_argument, NULL, HOST},
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
            case PERIOD:
            {
                snoozetime = atoi(optarg);
                break;
            }
            case SCALE:
            {
                if ((optarg[0] != 'F') & (optarg[0] != 'C'))
                {
                    fprintf(stderr, "Unrecognized Scale, correct usage includes: --scale=[C|F]\n");
                    exit(1);
                }
                temp_Scale = optarg[0];
                break;
            }
            case LOG:
            {
                log_opt = 1;
                lfd = creat(optarg, 0666);
                if (lfd < 0) {
                    fprintf(stderr, "Error creating file.\n");
                    exit(1);
                }
                break;
            }
			case ID:
			{
				usr_id = optarg;
				break;
			}
			case HOST:
			{
				host = optarg;
				break;
			}
            default:
            {
                fprintf(stderr, "Unrecognized argument, correct usage includes: --log=[filename], --scale=[C|F], --period=[seconds], --ID=[9 digit id num], --HOST=[hostname]\n");
                exit(1);
            }
        }
    }
    //get port number
	port_num = atoi(argv[argc - 1]);
	if (port_num == 0)
	{
		fprintf(stderr, "Error!: No port number.\n");
        exit(1);
	}
	
    //running variables
    temperature_Pin = mraa_aio_init(1); 
    //button_Pin = mraa_gpio_init(62);
    float current_Temp;
    time_t readTime;
    struct tm* readTimePTR;
    
    char buffer[bufferSize];
    int buffCount = 0;
    
    //mraa_gpio_isr(button_Pin, MRAA_GPIO_EDGE_RISING, &button_Pressed, NULL);
    
    char input_buff[2048];
    int input_Count = 0;
	
	//TLS Connection Stuff
	SSL* ssl;
    SSL_CTX *ctx;
	const SSL_METHOD *method;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
    SSL_library_init();
	method = SSLv23_client_method();
	ctx = SSL_CTX_new(method);
	ssl = SSL_new(ctx);
	
	//socket for connecting to server
    struct sockaddr_in sock_addr;
	struct hostent *serv_ent;
	
	serv_ent = gethostbyname(host);
	if (serv_ent == NULL)
	{
		fprintf(stderr, "Error! Host not found.\n %s\n", host);
		exit(1);
	}
	
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		fprintf(stderr, "Error! Could not create socket.\n");
		exit(1);
	}
	
	bzero(&sock_addr, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons( port_num );
	sock_addr.sin_addr.s_addr = *(long *) (serv_ent->h_addr);
	if (connect(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) < 0) 
	{
		fprintf(stderr, "Error! Could not connect to server.\n");
		exit(1);
	}
	//end socket
	
	SSL_set_fd(ssl, sock);
	SSL_connect(ssl);
	//end TLS
    
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*1);
    int thread_ret = pthread_create(&tid[0], NULL, poll_function, NULL);
    if(thread_ret != 0)
    {
        fprintf(stderr, "Error!: Thread create failure.\n");
        exit(1);
    }
    
    //dprintf(sock, "ID=%s\n", usr_id);
    char idbuf[32];
    int idbufCount = 0;
	idbufCount = sprintf(idbuf, "ID=%s\n", usr_id);
	SSL_write(ssl, idbuf, idbufCount);
	
	if (log_opt)
    {
		write(lfd, "ID=", 3);
        write(lfd, usr_id, 9);
		write(lfd, "\n", 1);
    }
	
    while(1)
    {
        current_Temp = mraa_aio_read(temperature_Pin);
        //mraa_gpio_dir(button_Pin, MRAA_GPIO_IN);
        
        readTime = time(NULL);
        readTimePTR = localtime(&readTime);
        
        float R = 1023.0/current_Temp-1.0;
        R = R0*R;
        float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
        if (temp_Scale == 'F')
        {        
            temperature = temperature*(1.8) + 32; //convert to F
        }

        if (on_opt == 0)
        {
            buffCount = sprintf(buffer, "%.2d:%.2d:%.2d SHUTDOWN\n", readTimePTR->tm_hour, readTimePTR->tm_min, readTimePTR->tm_sec);
            //write(1, buffer, buffCount);
			//write(sock, buffer, buffCount);
			SSL_write(ssl, buffer, buffCount);
            if (log_opt)
            {
                write(lfd, buffer, buffCount);
            }
            exit(0);
        }
        if ((report_opt == 1) & (on_opt==1))
        {
            buffCount = sprintf(buffer, "%.2d:%.2d:%.2d %0.1f\n", readTimePTR->tm_hour, readTimePTR->tm_min, readTimePTR->tm_sec, temperature);
            //write(1, buffer, buffCount);
			//write(sock, buffer, buffCount);
			SSL_write(ssl, buffer, buffCount);
            if (log_opt)
            {
                write(lfd, buffer, buffCount);
            }
        }

        char tbuf[16];
        clear_memory(tbuf, 16);
        int index = 0;
        int tbuf_pos = 0;
        
        if (prs > 0)
        {  
            pthread_mutex_lock(&lock1);
            //input_Count = read(sock, input_buff, 2048);
			input_Count = SSL_read(ssl, input_buff, 2048);
            index = 0;
            tbuf_pos = 0;
            pthread_mutex_unlock(&lock1);
            
            while (index < input_Count)
            {
                tbuf[tbuf_pos] = input_buff[index];
                if ((tbuf[tbuf_pos] == '=')|(tbuf[tbuf_pos] == '\n')/*|(tbuf[tbuf_pos] == ' ')*/)
                {
                    if (strcmp(tbuf, "SCALE=") == 0)
                    {
                            tbuf_pos++;
                            index++; 
                            tbuf[tbuf_pos] = input_buff[index];
                            tbuf_pos++;
                            index++;
                            tbuf[tbuf_pos] = input_buff[index];

                        if (strcmp(tbuf, "SCALE=F\n") == 0)
                        {
                            temp_Scale = 'F';
							//write(sock, "SCALE=F\n", 8);
                            //write(1, "SCALE=F\n", 8);
                            if (log_opt == 1)
                            {
                               write(lfd, "SCALE=F\n", 8); 
                            }
                        }
                        else if (strcmp(tbuf, "SCALE=C\n") == 0)
                        {
                            temp_Scale = 'C';
							//write(sock, "SCALE=C\n", 8);
                            //write(1, "SCALE=C\n", 8);
                            if (log_opt == 1)
                            {
                               write(lfd, "SCALE=C\n", 8);
                            }
                        }
                    }
                    
                    else if (strcmp(tbuf, "PERIOD=") == 0)
                    {
                        char temp_num[2] = {' ',' '};
                        int temp_num_indx = 0;
                        while(tbuf[tbuf_pos] != '\n')
                        {
                            tbuf[tbuf_pos] = input_buff[index];
                            if (isdigit(tbuf[tbuf_pos]) > 0)
                            {
                                temp_num[temp_num_indx] = tbuf[tbuf_pos];
                                temp_num_indx++;
                            }
                            tbuf_pos++;
                            index++;
                        }
                        tbuf_pos++;
                        if (temp_num_indx >= 1)
                        {
                            //write(1, "PERIOD=", 7);
                            //write(1, temp_num, temp_num_indx);
                            //write(1, "\n", 1);
							//write(sock, "PERIOD=", 7);
                            //write(sock, temp_num, temp_num_indx);
                            //write(sock, "\n", 1);
							temp_num[1] = ' ';
                            if (log_opt == 1)
                            {
                                write(lfd, "PERIOD=", 7);
                                write(lfd, temp_num, temp_num_indx);
                                write(lfd, "\n", 1);
                            }
                            snoozetime = atoi(temp_num);
                        }
                        else
                        {
                            //write(1, "Incorrect Command\n", 18);
							//write(sock, "Incorrect Command\n", 18);
                            clear_memory(tbuf, 16);
                        }
                    }
                    
                    else if (strcmp(tbuf, "STOP\n") == 0)
                    {
                        report_opt = 0;
                        //write(1, "STOP\n", 5);
						//write(sock, "STOP\n", 5);
                        if (log_opt == 1)
                        {
                           write(lfd, "STOP\n", 5); 
                        }
						clear_memory(tbuf, 16);
                    }
                    else if (strcmp(tbuf, "START\n") == 0)
                    {
                        report_opt = 1;
                        //write(1, "START\n", 6);
						//write(sock, "START\n", 6);
                        if (log_opt == 1)
                        {
                           write(lfd, "START\n", 6);
                        }
						clear_memory(tbuf, 16);
                    }
                    else if (strcmp(tbuf, "OFF\n") == 0)
                    {
                        on_opt = 0;
                        //write(1, "OFF\n", 4);
						//write(sock, "OFF\n", 4);
                        if (log_opt == 1)
                        {
                           write(lfd, "OFF\n", 4);
                        }
						clear_memory(tbuf, 16);
                        break;
                    }
					else if (strcmp(tbuf, "LOG ") == 0)
                    {
						if (log_opt == 1)
                        {
							write(lfd, input_buff, input_Count);
						}
						clear_memory(tbuf, 16);
                        break;
                    }
                    else
                    {
						//write(sock, "Incorrect Command\n", 18);
                        //write(1, "Incorrect Command\n", 18);
                        //clear_memory(tbuf, 16);
                    }
                    tbuf_pos = -1;
                }
                tbuf_pos++;
                index++;
            }
            pthread_mutex_lock(&lock1);
            input_found = 0;
			prs = 0;
			clear_memory(tbuf, 16);
			pthread_mutex_unlock(&lock1);
        }
        
        sleep(snoozetime);
    }
}