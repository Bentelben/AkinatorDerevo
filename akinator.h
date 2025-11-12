#ifndef AKINATOR_H
#define AKINATOR_H

#include "derevo.h"

void AkinatorInitialize(derevo_t *derevo);
derevo_node_t **AkinatorRunGuessing(derevo_t *derevo, bool *answer);
derevo_node_t **AkinatorAppendQuestion(derevo_t *derevo, derevo_node_t **destination);
void AkinatorSaveData(derevo_t *derevo);
int AkinatorLoadData(derevo_t *derevo, char *filename);

#endif
