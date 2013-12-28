#include <stdlib.h>
#include <stdio.h>
#include "nano.h"
#include "scene.c"

struct scene {
	char label[10], choice[10];
	char* txt;
	char* nxt;
	int index, flag;
	scene* prev; scene* next;
};

typedef struct evalparser evalparser;
struct evalparser {
    char* nxt;
    int index;
};

typedef struct choiceparser choiceparser;
struct choiceparser {
    char* start;
    char label[10];
    char* txt;
    int offset;
    scene* now;
    evalparser* eval;
    void (*examine)(char*, choiceparser*);
    int (*choose)(choiceparser*, int);
    void (*dischoice)(choiceparser*);
    scene* choice;
};

scene* data2 = 0;
scene** data = &data2;
int* save = 0;

int nextword(char* txtclip) {
    int word = 0;
    while (*txtclip && (*txtclip != ' ')) {
        txtclip++;
        word++;
    }
    return word;
}

void say(char* txt) {
    int line = 0;
    while (*txt) {
        putchar(*txt);
        txt++;
        line++;
        if (line+nextword(txt) >= 79) {
            putchar('\n');
            line = 0;
        }
    }
    putchar('\n');
}

void end(scene* now, int retcode);

void add(scene** data, scene* s) {
    if (*data == 0) {
        *data = s;
        (*data)->prev = 0;
        (*data)->next = 0;
        return;
    }
    (*data)->next = s;
    s->prev = *data;
    s->next = 0;
    *data = (*data)->next;
}

scene* create(scene** data, char label[10], char* content, char* next) {
	scene* s = malloc(sizeof(scene));
	s->flag = 0;
	strcpy(s->label, label);
	s->txt = malloc(sizeof(char) * (strlen(content)+1));
	strcpy(s->txt, content);
	s->nxt = malloc(sizeof(char) * (strlen(next)+1));
	strcpy(s->nxt, next);
	return s;
}

void rew(scene** data) {
    while ((*data)->prev) {
		*data = (*data)->prev;
	}
}

void p_reset(char* s, choiceparser* p) {
    p->start = s+1;
}

void p_clip(char* s, choiceparser* p, char* dest) {
    int offset = (int)(s - p->start);
    memcpy(dest, p->start, offset);
    dest[offset] = 0;
}

void p_skip(char* s, choiceparser* p) {
}

void p_label(char* s, choiceparser* p);

int prog = 0;

void addsave(int choice) {
    if (prog==0) {
        save = malloc(sizeof(int)*2);
        save[prog] = choice;
    }
    else {
        int* save2 = malloc(sizeof(int) * (prog+2));
        int i;
        for (i=0; i<prog; i++) {
            save2[i] = save[i];
        }
        save2[prog]=choice;
        free0(save);
        save = save2;
    }
    save[prog+1]=0;
    prog++;
}

int defpause(scene* now, void (*spec)(scene*, char*)) {
	char paussi[3];
	fgets(paussi, 3, stdin);
	paussi[strlen(paussi)-1] = 0;
	spec(now, paussi);
	return atoi(paussi);
}

int maxsave() {
    int i=0;
    while(save[i]) {
        i++;
    }
    return i;
}

int (*pause)(scene* now, void (*spec)(scene*, char*)) = &defpause;

int emulpause(scene* now, void (*spec)(scene*, char*)) {
    int out = save[prog];
	printf("\tautochoice %d\n", out);
	prog++;

	if (prog>=maxsave()) {
        pause = &defpause;
    }

	return out;
}

int emulating() {
    return (int)pause == (int)emulpause;
}

int p_choose(choiceparser* p, int choice) {
    scene** c = &p->choice;
    rew(c);
    while (1) {
        if ((*c)->index==choice) {
            //p->gotonext = &p_gotochoice;
            strcpy(p->now->choice, p->choice->nxt);
            printf("\n");

            if (!emulating()) addsave(choice);

            return 0;
        }
        if (!(*c)->next) {
            break;
        }
        *c = (*c)->next;
    }
    printf("invalid choice\n>");
    return 1;
}

//int (*p_choose)(choiceparser* p, int choice) = &defpchoose;

int emulchoose(choiceparser* p, int choice) {
    scene** c = &p->choice;
    rew(c);
    while (1) {
        if ((*c)->index==choice) {
            //p->gotonext = &p_gotochoice;
            strcpy(p->now->choice, p->choice->nxt);
            printf("\n");
            return 0;
        }
        if (!(*c)->next) {
            break;
        }
        *c = (*c)->next;
    }
    return 1;
}

int e_defchoose(choiceparser* p, int choice) {
    if (!emulating()) addsave(-1);
    strcpy(p->now->choice, p->eval->nxt);
    return 0;
}
int p_defchoose(choiceparser* p, int choice) {
    if (!emulating()) addsave(-1);
    strcpy(p->now->choice, p->now->nxt);
    return 0;
}

void p_defdischoice(choiceparser* p) {
}

void p_dischoice(choiceparser* p) {
    scene** choice = &p->choice;
    rew(choice);
    printf("Choose: \n");
    int cn = 1;
    while (1) {
        printf("  %d:%s\n", cn, (*choice)->txt);
        (*choice)->index = cn;
        if (!((*choice)->next)) {
            break;
        }
        *choice = (*choice)->next;
        cn++;
    }
    printf(">");
}

void p_title(char* s, choiceparser* p) {
    if (*s == ';') {
        p->choice->txt = malloc(sizeof(char) * (s - p->start + 1));
        p_clip(s, p, p->choice->txt);
        p_reset(s, p);
        p->choice->next = malloc(sizeof(scene));
        p->choice->next->prev = p->choice;
        p->choice = p->choice->next;
        p->choice->txt = 0;
        p->choice->nxt = 0;
        p->choice->next = 0;
        p->examine = &p_label;
    }
    else if (*s == ']') {
        p->choice->txt = malloc(sizeof(char) * (s - p->start + 1));
        p_clip(s, p, p->choice->txt);
        p_reset(s, p);
        p->choice->next = 0;
        p->examine = &p_skip;
    }
}

void p_txt(char* s, choiceparser* p);

void p_label(char* s, choiceparser* p) {
    if (*s == ':') {
        p->choice->nxt = malloc(sizeof(char) * (s - p->start +1));
        p_clip(s, p, p->choice->nxt);
        p_reset(s, p);
        p->choose = &p_choose;
        p->dischoice = &p_dischoice;
        p->examine = &p_title;
    }
}

void p_txt(char* s, choiceparser* p) {
    if (*s == '[') {
        //p->txt[s - p->now->txt] = 0;
        p_reset(s, p);
        p->examine = &p_label;
    }
}

void p_init(choiceparser* p, scene* s) {
    p->now = s;
    p->txt = malloc(sizeof(char) * (strlen(s->txt)+1));
    strcpy(p->txt, s->txt);
    p->examine = &p_txt;
    p->choose = &p_defchoose;
    p->dischoice = &p_defdischoice;
    p->choice = malloc(sizeof(scene));
    p->choice->txt = 0;
    p->choice->nxt = 0;
    p->choice->prev = 0;
    p->choice->next = 0;
}

void s_quit(scene* now, char* paussi) {
	if (paussi[0]=='q') {
	    printf("bye\n");
	    end(now, 0);
	}
}

void s_skip(scene* now, char* paussi) {
}

scene* txtlabelchar(char* label);

int e_eval(char* label, int cond) {
    scene* target = txtlabelchar(label);
    return target->flag==cond;
}

void e_label(char* sub, evalparser* e) {
    char* start = sub;
    while (*sub) {
        if (*sub=='=') {
            int offset = (int)(sub-start);
            char* label = malloc(sizeof(char) * (offset+1));
            memcpy(label, start, offset);
            label[offset] = 0;
            int cond = *(sub+1)-'0';

            if (e_eval(label, cond)) {
                start = sub+3;
                sub = start;
                int par = 1;
                do {
                    sub++;
                    if (*sub=='{') {
                        par++;
                    }
                    if (*sub=='}') {
                        par--;
                    }
                } while (par);
                offset = (int)(sub-start);
                memcpy(e->nxt+strlen(e->nxt), start, offset);
                e->index+=offset;
                e->nxt[e->index]=0;
            }

            free0(label);
            return;
        }
        else {
            sub++;
        }
    }
}

int e_txt(char* s, evalparser* e) {
    int out = 0;
    int qlock = 0;
    int par = 0;
    char* start = s;
    while (*s) {
        if (*s=='?' && !par) {
            char* sub = s+1;
            e_label(sub, e);
            out = 1;
            qlock=1;
        }
        else if (*s=='{') {
            qlock=0;
            par++;
        }
        else if (*s=='}') {
            par--;
        }
        else {
            if (!qlock && !par) {
                e->nxt[e->index]=*s;
                e->index++;
                e->nxt[e->index]=0;
            }
        }
        s++;
    }

    return out;
}

int eval(char** s, evalparser* e) {
    int out = 0;
    char* e_buf = 0;
    e->index = 0;

    while (e_txt(*s, e)) {
        free0(e_buf);
        e_buf = malloc(sizeof(char) * (strlen(e->nxt)+1));
        strcpy(e_buf, e->nxt);
        *s = e_buf;

        e->index = 0;
        e->nxt[0]=0;

        out = 1;
    }

    free0(e_buf);
    *s = e->nxt;

    return out;
}

void parse(scene** now) {
    evalparser e;
    e.nxt = malloc(sizeof(char) * (strlen((*now)->nxt)+1));
    e.nxt[0] = 0;

    choiceparser p;
    p_init(&p, *now);
    p.eval = &e;

    char* s = (*now)->nxt;
    if (eval(&s, &e)) {
        p.choose = &e_defchoose;
    }

    while(*s) {
        p.examine(s, &p);
        s++;
    }

    say(p.txt);

    p.dischoice(&p);

    while( p.choose(&p, pause(*now, &s_quit)) )
        ;

    (*now)->flag++;
    free0(e.nxt);
    free0(p.txt);
    freeall(p.choice);
}

void free0 (void* p) {
    if (p) free(p);
}

void freeall(scene* start) {
    scene* uf = start;
    scene** unfree = &uf;
    rew(unfree);

    while ((*unfree)->next) {
        *unfree = (*unfree)->next;
        free0((*unfree)->prev->txt);
        free0((*unfree)->prev->nxt);
        free0((*unfree)->prev);
    }
    free0((*unfree)->txt);
    free0((*unfree)->nxt);
    free0(*unfree);
}

void savestate() {
    if (emulating() || !save || !save[0]) return;

    int i = 0;
    FILE* fr;
    fr = fopen(SAVEFILE, "w");
    if (!fr) {
        printf("Can't open %s for saving state", SAVEFILE);
        return;
    }

    while (save[i]) {
        fprintf(fr, "%d\n", save[i]);
        i++;
    }
    fclose(fr);
}

void end(scene* now, int retcode) {
    freeall(now);
	savestate();
	pause(now, &s_skip);
    exit(retcode);
}

scene* txtlabelchar(char* label) {
    scene* search2 = *data;
    scene** search = &search2;
    rew(search);
    while (strcmp(label, (*search)->label)) {
        if ((*search)->next == 0) {
            printf("scenario %s not written yet\n", label);
            end(*search, 1);
        }
        *search = (*search)->next;
    }
    return *search;
}

scene* txtlabel(scene* labeler) {
    scene* search2 = labeler;
    scene** search = &search2;
    rew(search);
    while (strcmp(labeler->choice, (*search)->label)) {
        if ((*search)->next == 0) {
            printf("scenario %s not written yet\n", labeler->choice);
            end(*search, 1);
        }
        *search = (*search)->next;
    }
    return *search;
}

void read(scene** now) {
	do {
	    parse(now);
		if (strlen((*now)->choice)==0) {
            end(*now, 0);
		}
		*now = txtlabel(*now);
	} while ((*now)->choice);
}

void emulate() {
    pause = &emulpause;
    //p_choose = &emulchoose;
}

int main(int argc, char* argv[]) {
    get_scenes(data);

    if (argc>1) {
        FILE* fr;
        fr = fopen(SAVEFILE, "r");
        if (!fr) {
            printf("Can't open %s for loading savestate", SAVEFILE);
            return 1;
        }

        printf("emulating\n");
        int i=0; char l[20];
        save = malloc(sizeof(int)*80);
        while(fgets(l, 20, fr) != NULL) {
            sscanf(l, "%d", &save[i]);
            i++;
        }
        save[i]=0;
        /*i=0;
        printf("scanned: ");
        while (save[i]) {
            printf("%d", save[i]);
            i++;
        }
        printf("\n");*/
        fclose(fr);

        emulate();
    }

    rew(data);
    read(data);
    freeall(*data);
    return 0;
}
