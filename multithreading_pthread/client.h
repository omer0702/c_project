#ifndef CLIENT_H
#define CLIENT_H

#define PORT 8080
#define BUFFER_SIZE 1024

//הextern הוא בשביל להגדיר את המשתנים כגלובליים וכך נוכל לכגשת אליהם בקבצים אחרים
extern int sock;
extern int running;

void* receive_handle(void* arg);

#endif
