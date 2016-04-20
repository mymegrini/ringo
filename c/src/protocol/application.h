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
    void (*app)(char *); // action to do, take the message as argument
} application;



void sendappmessage_all(char *type, char *format, ...);


#endif /* APPLICATION_H */
