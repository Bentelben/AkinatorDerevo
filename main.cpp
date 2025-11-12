#include "akinator.h"

#include <string.h>
#include <assert.h>

int main() {
    derevo_t derevoObject = {};
    derevo_t *const derevo = &derevoObject;
    AkinatorInitialize(derevo);

    DerevoPushNode(derevo, &derevo->head, strdup("Неизвестно кто"));
    while (1) {
        printf("Загадай персонажа и ответь на вопрос\n");
        bool isCorrect = false;
        derevo_node_t **result = AkinatorRunGuessing(derevo, &isCorrect);
        if (!isCorrect) {
            AkinatorAppendQuestion(derevo, result);
            AkinatorSaveData(derevo);
        } else
            printf("Элементарно!\n");
        printf("\n\n");
    }

    DerevoFinalize(derevo);
}
