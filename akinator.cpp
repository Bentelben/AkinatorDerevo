#include "akinator.h"

#include "derevo.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static bool DumpValue(derevo_node_t **const node, void *const args) {
    FILE *const file = (FILE *)args;
    fprintf(file, "\"%s\"", (*node)->value);
    return true;
}

static bool WriteNodeGraphData(derevo_node_t **const node, void *const args) {
    FILE *const file = (FILE *)args;
    fprintf(file, "node_%p [shape=record, label=\"{ node %p | value = \\\"%s\\\" | { <left> да | <right> нет } }\"]", *node, *node, (*node)->value);
    if ((*node)->left)
        fprintf(file, "node_%p:<left> -> node_%p\n", *node, (*node)->left);
    if ((*node)->right)
        fprintf(file, "node_%p:<right> -> node_%p\n", *node, (*node)->right);
    return true;
}

static bool FreeNode(derevo_node_t **const node, void *const) {
    free((*node)->value);
    return true;
}

void AkinatorInitialize(derevo_t *const derevo) {
    DerevoInitialize(derevo, DumpValue, WriteNodeGraphData, FreeNode);
}

static bool GuessingLeftSelector(derevo_node_t **, void *args) {
    return *(bool *)args;
}

static bool GuessingRightSelector(derevo_node_t **, void *args) {
    return !*(bool *)args;
}

static bool GuessingPreorder(derevo_node_t **node, void *args) {
    printf("Вопрос: %s?\n", (*node)->value);
    printf("1. Да\n2. Нет\n");
    int res = 2;
    scanf("%d", &res); // TODO check

    bool *const answer = (bool *)args;
    *answer = res == 1;
    if (*answer && (*node)->left == NULL)
        return false;

    if (!*answer && (*node)->right == NULL)
        return false;

    return true;
}

derevo_node_t **AkinatorRunGuessing(derevo_t *const derevo, bool *const answer) {
    return DerevoDoTravesal(
        &derevo->head,
        GuessingLeftSelector, (void *)answer,
        GuessingRightSelector, (void *)answer,
        GuessingPreorder, (void *)answer,
        NULL, NULL,
        NULL, NULL
    );
}

static void ClearBuffer() {
    scanf("%*[^\n]");
    getchar();
}

static char *ReadLine() {
    char *buffer = NULL;
    size_t n = 0;
    ssize_t charsRead = getline(&buffer, &n, stdin);
    buffer[charsRead-1] = '\0';
    return buffer;
}

derevo_node_t **AkinatorAppendQuestion(derevo_t *const derevo, derevo_node_t **const destination) {
    assert(destination);
    LogEvent(derevo, "Before akinator push", "");
    derevo_node_t *const currentAnswerNode = *destination;

    printf("Проклятье! Позволь узнать что же это было?\nЭто был...\n");
    ClearBuffer();
    char *newAnswerString = ReadLine();
    printf("Хмм... Чем же отличается `%s` от `%s`\n`%s` он...\n", currentAnswerNode->value, newAnswerString, newAnswerString);
    char *newQuestionString = ReadLine();

    derevo_node_t *const newAnswerNode = *DerevoPushNode(derevo, &((*destination)->left), newAnswerString);
    if (newAnswerNode == NULL)
        return NULL;
    derevo_node_t *const newQuestionNode = *DerevoPushNode(derevo, &((*destination)->right), newQuestionString);
    if (newQuestionNode == NULL)
        return NULL;

    newQuestionNode->left = newAnswerNode;
    newQuestionNode->right = currentAnswerNode;

    currentAnswerNode->left = NULL;
    currentAnswerNode->right = NULL;


    *destination = newQuestionNode;
    LogEvent(derevo, "After akinator push", "");
    return &newQuestionNode->left;
}

void AkinatorSaveData(derevo_t *const derevo) {
    char buffer[1024] = "";
    sprintf(buffer, "%s/data.txt", derevo->logger.dirPath);
    FILE *const file = fopen(buffer, "w");
    DerevoDump(derevo, file);
    fclose(file);
}

// TODO reading tree from file
