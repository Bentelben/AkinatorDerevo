#include "akinator.h"

#include "derevo.h"
#include "text_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static bool DumpValue(derevo_node_t **const node, void *const args) {
    FILE *const file = (FILE *)args;
    fprintf(file, "\"%s\"", (*node)->value);
    return true;
}

static bool WriteNodeGraphData(derevo_node_t **const node, void *const args) {
    FILE *const file = (FILE *)args;
    fprintf(file, "node_%p [shape=record, label=\"{%s?| { <left> да | <right> нет } }\"]\n", *node, (*node)->value);
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
    if (scanf("%d", &res) != 1) {
        ClearBuffer();
        res = 1; 
    }

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

static void SkipSpaces(char *const text, size_t *const index) {
    while (isspace(text[*index]))
        (*index)++;
}

static int LoadNode(derevo_t *derevo, char *const text, size_t *const index, derevo_node_t **destination) {
    assert(derevo);
    assert(text);
    assert(index);
    assert(destination);
    
    char buffer[2048];
    int bytesRead = 0;

    SkipSpaces(text, index);

    if (text[(*index)++] != '(')
        return 0;

    SkipSpaces(text, index);

    if (text[*index] == ')') {
        (*index)++;
        return 1;
    }

    if (sscanf(text + *index, "\"%[^\"]\"%n", buffer, &bytesRead) != 1)
        return 0;

    (*index) += (size_t)bytesRead;
    SkipSpaces(text, index);

    if (DerevoPushNode(derevo, destination, strdup(buffer)) == NULL)
        return 0;
    
    if (LoadNode(derevo, text, index, &(*destination)->left) != 1)
        return 0;

    SkipSpaces(text, index);

    if (LoadNode(derevo, text, index, &(*destination)->right) != 1)
        return 0;

    SkipSpaces(text, index);

    if (text[(*index)++] != ')')
        return 0;

    return 1;
}

int AkinatorLoadData(derevo_t *const derevo, char *const filename) {
    char *const text = ReadFile(filename);
    size_t index = 0;

    return LoadNode(derevo, text, &index, &derevo->head);
}

struct definition_travesal_args {
    bool isFound;
    derevo_node_t **needle;
    derevo_node_t *last;
};

static bool DefinitionPreorder(derevo_node_t **const node, void *const rawArgs) {
    definition_travesal_args *args = (definition_travesal_args *)rawArgs;
    if (node == args->needle)
        args->isFound = true;
    
    return true;
}

static bool DefinitionPostorder(derevo_node_t **const node, void *const rawArgs) {
    definition_travesal_args *args = (definition_travesal_args *)rawArgs;
    if (args->isFound) {
        if (args->last != NULL) {
            assert((*node)->left);
            assert((*node)->right);

            printf(" - ");
            if ((*node)->right == args->last)
                printf("не ");
            printf("%s\n", (*node)->value);
        }
        args->last = *node;
    }
    return true;
}

static bool DefinitionSelectorLeft(derevo_node_t **node, void *const args) {
    if ((*node)->left == NULL)
        return false;

    bool *isFound = (bool *)args;
    return !(*isFound);
}

static bool DefinitionSelectorRight(derevo_node_t **node, void *const args) {
    if ((*node)->right == NULL)
        return false;

    bool *isFound = (bool *)args;
    return !(*isFound);
}

void AkinatorGetDefinition(derevo_t *const derevo, derevo_node_t **const node) {
    printf("Определение %s:\n", (*node)->value);

    definition_travesal_args args = {
        false,
        node,
        NULL
    };

    DerevoDoTravesal(
        &derevo->head,
        DefinitionSelectorLeft, &args.isFound,
        DefinitionSelectorRight, &args.isFound,
        DefinitionPreorder, &args,
        NULL, NULL,
        DefinitionPostorder, &args
    );
    printf("\n");
}
