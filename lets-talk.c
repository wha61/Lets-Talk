#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <pthread.h>
// #include "list.h"

// create 4 threads and two lists
pthread_t threads[4];
struct List_s* input_list;
struct List_s* output_list;
pthread_mutex_t status_lock;

bool has_input = false;
bool has_send = false;
bool has_received = false;

int local_port;
int remote_port;
char *remote_host;


char* message_encryption_by_key(char* message, int key){
    for(int i=0; i<strlen(message); i++){
        char c = message[i];
        c = c + key;
        if(c=='\0'){
            break;
        }
        message[i] = c;
    }
    return message;
}

char* message_decryption_by_key(char* message, int key){
    for(int i=0; i<strlen(message); i++){
        char c = message[i];
        c = c - key;
        if(c=='\0'){
            break;
        }
        message[i] = c;
    }
    return message;
}

void delete_char(char str[],char target)
{
	int i,j;
	for(i=j=0;str[i]!='\0';i++){
		if(str[i]!=target){
			str[j++]=str[i];
		}
	}
	str[j]='\0';
}

void* keyboard_input_thread(){
    char buffer[5000];

    // char *str = "asdsadsd";
    // // printf("%s\n", str);
    // pthread_mutex_lock(&status_lock);
    // List_add(input_list, str);
    // pthread_mutex_unlock(&status_lock);

    while(true){

        while(has_input){

        }
        fgets(buffer, 5000, stdin);
        char *str = (char *)malloc(strlen(buffer)+1);
        strcpy(str, buffer);
        delete_char(str, '\n');

        pthread_mutex_lock(&status_lock);
        List_add(input_list, message_encryption_by_key(str, 0));
        pthread_mutex_unlock(&status_lock);


        printf("at input: %s\n", (char *) List_curr(input_list));


        has_input = true;

        
    }

    pthread_exit(NULL);

}

void* sender_thread(){
    sleep(1);
    int sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sockfd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    while(true){

        while(!has_input){

        }
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(remote_port);//remote port
        addr.sin_addr.s_addr = inet_addr(remote_host);//remote ip

        
        ssize_t s_ret = sendto(sockfd,(char *) List_curr(input_list),strlen((char *) List_curr(input_list)),0,(struct sockaddr *)&addr,sizeof(addr));
        
        printf("at sender: %s\n", (char *) List_curr(input_list));

        if(s_ret < 0)
        {
            perror("sento failed");
            exit(EXIT_FAILURE);
        }

        // has_send = true;
        has_input = false;
        // while(has_send){

        // }

    }


    pthread_exit(NULL);
}

void* receiver_thread(){

    sleep(1);
    int sockfd1 = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sockfd1 < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //创建ipv4的结构体，这也是服务端的ip和端口号
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local_port);//local port
    //这里需要注意的是inet_addr包含在#include <arpa/inet.h>
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    //2.绑定地址信息结构(sockaddr)
    int ret = bind(sockfd1,(struct sockaddr*)&addr,sizeof(addr));
    if(ret < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while(true){

        // while(!has_send){

        // }   

        char buf[1024] = {0};
        struct sockaddr_in recv_addr;
        socklen_t recvlen = sizeof(recv_addr);
        ssize_t recv_size = recvfrom(sockfd1,buf,sizeof(buf)-1,0,(struct sockaddr *)&recv_addr,&recvlen);
        if(recv_size < 0)
        {
            perror("recvfrom failed");
            return 0;
        }
        //走到这表示接收到消息了
        printf("i am server,i recv : %s,i recv sucess %s:%d\n",buf,inet_ntoa(recv_addr.sin_addr),ntohs(recv_addr.sin_port));

        pthread_mutex_lock(&status_lock);
        List_add(output_list, message_encryption_by_key(buf, 0));
        pthread_mutex_unlock(&status_lock);



        has_received = true;

        while(has_received){

        }



    }

}

void* console_output_thread(){

    // sleep(1);

    // pthread_mutex_lock(&status_lock);
    // printf("%s\n",(char *) List_curr(input_list));
    // pthread_mutex_unlock(&status_lock);


    while(true){



        while(!has_received){

        }  

        pthread_mutex_lock(&status_lock);
        printf("ok i output: %s\n",(char *) List_curr(output_list));
        pthread_mutex_unlock(&status_lock);
        has_received = false;
        // has_send = false;
        
        // has_input = false;
    }

    pthread_exit(NULL);

}

int main(int argc, char *argv[]){



    printf("here is arguments: %s, %s, %s, %s.\n", argv[0], argv[1], argv[2], argv[3]);

    remote_port = atoi(argv[3]);
    local_port = atoi(argv[1]);

    remote_host = (char *)malloc(strlen(argv[2]) + 1);
    strcpy(remote_host, argv[2]);    
    printf("%s\n", remote_host);


    input_list = List_create();
    output_list = List_create();

    int threadStatus = pthread_create(&threads[0], NULL, keyboard_input_thread, NULL);
    
    printf("%d\n", threadStatus);

    if(threadStatus) {
        perror("Failed to create printer thread");
        exit(1);
    };

    threadStatus = pthread_create(&threads[1], NULL, sender_thread, NULL);
    
    printf("%d\n", threadStatus);

    if(threadStatus) {
        perror("Failed to create printer thread");
        exit(1);
    };

    threadStatus = pthread_create(&threads[2], NULL, receiver_thread, NULL);
    
    printf("%d\n", threadStatus);

    if(threadStatus) {
        perror("Failed to create printer thread");
        exit(1);
    };

    threadStatus = pthread_create(&threads[3], NULL, console_output_thread, NULL);
    
    printf("%d\n", threadStatus);

    if(threadStatus) {
        perror("Failed to create printer thread");
        exit(1);
    };

    // threadStatus = pthread_create(&threads[3], NULL, console_output_thread, NULL);
    // printf("%d\n", threadStatus);

    // if(threadStatus) {
    //     perror("Failed to create printer thread");
    //     exit(1);
    // };

    // pthread_cancel(threads[0]);
    // pthread_exit(NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    // void *pItem = List_curr(input_list);
    // List_remove(input_list); 
    // char *message = pItem;

    // printf("input: %s\n", message);

    // while (true){
    //     sleep(1);
    // }

    return 0;
}