
typedef struct symbolt *symbolt;
extern void addToTable(symbolt t, char* label, int value);
extern int find(symbolt t, char* label);