#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen);                  //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd;
	float ti, rt, th;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;  // get the host address
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                           //create the socket
	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr)); // pass to server address
	bzero(&(ser_addr.sin_zero), 8);
	
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	//perform the transmission and receiving
	ti = str_cli(fp, sockfd, &len, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));   // receive and send
	rt = (len/(float)ti);                           //caculate the average transmission rate
	th = 8*rt/(float)1000;
	
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s), Throughput: %f (Mbps)\n", ti, (int)len, rt, th);

	close(sockfd);
	fclose(fp);

	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen)
{
	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

	n = 0;
	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the data unit size is %d bytes\n",DATALEN);
	printf("the batch size is %d\n",NUMDU); // info of current case
// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

  // copy the file into the buffer.
	fread (buf,1,lsize,fp);

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';				//append the end byte
	gettimeofday(&sendt, NULL);				//get the current time
	while(ci<= lsize) // judge whether is the end of file
	{
		// NUMDU packets to be sent
		for (int i=0; i< NUMDU; i++) {
			// determine message to be sent
			if ((lsize+1-ci) <= DATALEN)
				slen = lsize+1-ci;
			else 
				slen = DATALEN;
			memcpy(sends, (buf+ci), slen);
			//send the packet to server
			n = sendto(sockfd, &sends, slen, 0, addr, addrlen);  
			
			if(n == -1) {
				printf("send error!");//send the data
				exit(1);
			}
			ci += slen;
		}
		
		if ((n=recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t *)&addrlen)) == -1) 			
		{      //receive the packet
			printf("error receiving");
			exit(1);
		}
		
		if (ack.num != 1|| ack.len != 0)
			printf("error in transmission\n");

	}
	
	gettimeofday(&recvt, NULL);
	*len= ci;                //get current time
	
	tv_sub(&recvt, &sendt);  // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
