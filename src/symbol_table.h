#include <inttypes.h>

#define NEW_SYM_TABLE malloc(sizeof(symbolt))
#define LABEL_BUFFER 50

typedef struct symbolt_node *symbolt;
extern void addToTable(symbolt t, char* label, uint64_t value);
extern uint64_t find(symbolt t, char* label);
extern void firstPass(symbolt t, char** lines);