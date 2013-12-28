#ifndef NANO_H
#define NANO_H

#define SAVEFILE "\\save.txt"
typedef struct scene scene;
scene* create(scene** data, char label[10], char* content, char* next);
void add(scene** data, scene* s);
void get_scenes(scene** data);

#endif
