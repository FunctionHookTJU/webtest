#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "csapp.h"
#include "parse.h"

/* 解析URL函数实现 */
void parse_url(char* url, struct UrlData* u) {
    char *hostpose = strstr(url, "//");
    char *portpose = strstr(hostpose + 2, ":");
    
    strcpy(u->host, hostpose + 2);
    
    /* 程序调试暂且默认url中包含了host信息 */
    if (portpose != NULL) {
        int tmp;
        sscanf(portpose + 1, "%d%s", &tmp, u->path);
        sprintf(u->port, "%d", tmp);
        *portpose = '\0';
    } else {
        char *pathpose = strstr(hostpose + 2, "/");
        if (pathpose != NULL) {
            strcpy(u->path, pathpose);
            strcpy(u->port, "80");
            *pathpose = '\0';
        }
    }
    
    return;
}

/* 测试代码
int main() {
    char url[MAX];
    strcpy(url, "http://www.xy.com:8080/home.html");
    printf("%s\n", url);
    char *hostpose;
    struct UrlData u;
    parse_url(url, &u);
    printf("Host: %s\nPort: %s\nPath: %s\n", u.host, u.port, u.path);

    return 0;
}
*/