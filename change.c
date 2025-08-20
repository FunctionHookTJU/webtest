#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "csapp.h"
#include "parse.h"
//typedef struct {
//    int rio_fd;                /* 文件描述符 */
//    int rio_cnt;               /* 缓冲区中未读字节数 */
//    char *rio_bufptr;          /* 下一个未读字节指针 */
//    char rio_buf[RIO_BUFSIZE]; /* 内部缓冲区 */
//} rio_t;

int main(){
    char url[MAXLINE];
    char new_httpdata[MAXLINE];
    strcpy(url,"http://www.xy.com:8080/home.html");
    struct UrlData u;
    parse_url(url,&u);
    printf("Host: %s\nPort: %s\nPath: %s\n",u.host,u.port,u.path);
    

    return 0;
}
void change_httpdata(rio_t *rio,struct UrlData *u,char * new_httpdata){
    static const char* Con_hdr = "Connection: close\r\n";
    static const char* Pcon_hdr = "Proxy-connection: close\r\n";
    char buf[MAXLINE+128];
    char Reqline[MAXLINE+128];//整合后的请求行
    char Host_hdr[MAXLINE+128], Cdata[MAXLINE+128];//分别为请求行，Host首部字段，和其他的数据信息 
    sprintf(Reqline,"GET %s HTTP/1.0\r\n",u->path);
    while(Rio_readlineb(rio,buf,MAXLINE)>0){
        /*读到空行就算结束，GET没有请求实体*/
        if(strcmp(buf,"\r\n")==0){
            strcat(Cdata,"\r\n");//strcat是字符串连接函数
            break;
        }
        else if(strncasecmp(buf,"Host:",5)==0){/*按字典序比较两个字符串的前 n 个字符，忽略大小写差异。*/
            strcpy(Host_hdr,buf);
        }
        else if(!strncasecmp(buf,"Connection:",11) && !strncasecmp(buf,"Proxy_Connection:",17) && !strncasecmp(buf,"User-agent:",11)){
            strcat(Cdata,buf);
        }
    }
    if(!strlen(Host_hdr)){
        /*如果hosthdr为空，说明该host被加载进请求的url中，我们格式读从url中读取解析的host*/
        sprintf(Host_hdr,"Host: %s\r\n",u->host);
    }
    sprintf(new_httpdata,"%s%s%s%s%s",Reqline,Host_hdr,Con_hdr,Pcon_hdr,Cdata);
    return;
}
void doit(int fd){
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char new_httpdata[MAXLINE], urltemp[MAXLINE];
    struct UrlData u;
    rio_t rio, server_rio;
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, url, version);
    strcpy(urltemp, url);   //赋值url副本以供读者写者使用，因为在解析url中，url可能改变 
    
    /*只接受GEI请求*/
    if (strcmp(method, "GET") != 0){
        printf ("The proxy can not handle this method: %s\n", method);
        return;
    }
    
    if (readcache(fd, urltemp) != 0)    //如果读者读取缓存成功的话，直接返回 
        return;

    parse_url(url, &u);     //解析url 
    change_httpdata(&rio, &u, new_httpdata);    //修改http数据，存入 new_httpdata中 
    
    int server_fd = Open_clientfd(u.host, u.port);
    size_t n;

    Rio_readinitb(&server_rio, server_fd);
    Rio_writen(server_fd, new_httpdata, strlen(new_httpdata));

    char cache[MAX_OBJECT_SIZE];
    int sum = 0;
    while((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0){
        Rio_writen(fd, buf, n);
        sum += n;
        strcat(cache, buf);
    }
    printf("proxy send %ld bytes to client\n", sum);
    if (sum < MAX_OBJECT_SIZE)
        writecache(cache, urltemp); //如果可以的话，读入缓存 
    close(server_fd);
    return;
}