#ifndef NANO_H
#define NANO_H

#define SAVEFILE "\\save.txt"
typedef struct scene scene;
scene* create(scene** data, char label[10], char* content, char* next);
void add(scene** data, scene* s);
char* defmsgbrd = "[e_bike2:Keep reading;e_bike2:Reply to this post]";
void get_scenes(scene** data);

#endif
