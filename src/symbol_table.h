#define NEW_SYM_TABLE malloc(sizeof(symbolt))
#define LABEL_BUFFER 50

typedef struct symbolt_node *symbolt;
extern void addToTable(symbolt t, char* label, int value);
extern int find(symbolt t, char* label);
extern void firstPass(symbolt t, char** lines);