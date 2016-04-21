#ifndef APPLICATION_H
#define APPLICATION_H


////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

/***
 * list of supported applications and there actions
 */
typedef struct application {
    char id[9];
    char desc[512];
    void (*app)(char *, char *, int); // action to do, take the message, content and lookup_flag
} application;



void sendappmessage_all(char *type, char *format, ...);


#endif /* APPLICATION_H */
