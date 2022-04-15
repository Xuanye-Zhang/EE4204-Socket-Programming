#include "headsock.h"

void str_ser(int sockfd, struct sockaddr *addr, int addrlen);             // transmitting and receiving function

void compareFiles();	// check if the file received is correct by comparing with original

int main(void)
{
	int sockfd, ret;
	struct sockaddr_in my_addr;
  
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          //create socket
	if (sockfd<0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret<0)
	{
		printf("error in binding");
		exit(1);
	}
	
	while(1) { // run forever to wait for client data
		printf("waiting for data\n");
		str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));                //receive client packet and response to client
		compareFiles();	// compare the transfered file	
	}

	close(sockfd);

	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[NUMDU*DATALEN]; // every time n * datalen
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;

	while(!end)
	{
   
		if ((n= recvfrom(sockfd, &recvs, 4*DATALEN, 0, addr, (socklen_t *)&addrlen))==-1)  //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
			
		if (recvs[n-1] == '\0')									//if it is the end of the file
		{
		  end = 1;
		  n --;
		}
		memcpy((buf+lseek), recvs, n); // received data into buffer
		lseek += n;
		
		ack.num = 1;
		ack.len = 0;
		if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1)//send the ack
		  {
			  printf("send error!");								
			  exit(1);
		  }
	}

	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}

void compareFiles() 
{ 
	FILE *fp1;
	FILE *fp2;
	
	if((fp1 = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File fp1 doesn't exit\n");
		exit(0);
	}
	
	if((fp2 = fopen ("myUDPreceive.txt","r+t")) == NULL)
	{
		printf("File fp2 doesn't exit\n");
		exit(0);
	}
	
    // fetching character of two file 
    // in two variable ch1 and ch2 
    char ch1 = getc(fp1); 
    char ch2 = getc(fp2); 
  
    // error keeps track of number of errors 
    // pos keeps track of position of errors 
    // line keeps track of error line 
    int error = 0, pos = 0, line = 1; 
  
    // iterate loop till end of file 
    while (ch1 != EOF && ch2 != EOF) 
    { 
        pos++; 
  
        // if both variable encounters new 
        // line then line variable is incremented 
        // and pos variable is set to 0 
        if (ch1 == '\n' && ch2 == '\n') 
        { 
            line++; 
            pos = 0; 
        } 
  
        // if fetched data is not equal then 
        // error is incremented 
        if (ch1 != ch2) 
        { 
            error++; 
            printf("Line Number : %d \tError"
               " Position : %d \n", line, pos); 
        } 
  
        // fetching character until end of file 
        ch1 = getc(fp1); 
        ch2 = getc(fp2); 
    } 
  
    printf("Total Errors : %d\t \n", error); 
} 
