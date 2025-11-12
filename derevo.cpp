#include "derevo.h"

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void DerevoInitialize(
    derevo_t *const derevo,
    travesal_function_t const elementValueDumpingTravesalFunctionPointer,
    travesal_function_t const graphDataWritingTravesalFunctionPointer,
    travesal_function_t const nodeFreeingTravesalFunctionPointer
) {
    assert(derevo);

    derevo->elementValueDumpingTravesalFunctionPointer = elementValueDumpingTravesalFunctionPointer; 
    derevo->graphDataWritingTravesalFunctionPointer = graphDataWritingTravesalFunctionPointer;
    derevo->nodeFreeingTravesalFunctionPointer = nodeFreeingTravesalFunctionPointer;


    LoggerInitialize(&derevo->logger);
    LogEvent(derevo, "Intialize", "");
}

derevo_node_t** DerevoPushNode(derevo_t *const derevo, derevo_node_t **const destination, derevo_elem_t value) {
    assert(derevo);
    assert(destination);

    LogEvent(derevo, "Before PUSH", "");

    assert(*destination == NULL);
    *destination = (derevo_node_t *)calloc(1, sizeof(**destination));
    if (*destination == NULL)
        return NULL;
    
    (*destination)->value = value;
    (*destination)->left = NULL;
    (*destination)->right = NULL;

    LogEvent(derevo, "After PUSH", "");
    return destination;
}


static bool DerevoFreeNodePostorderTravesalFunction(derevo_node_t **const node, void *const args) {
    derevo_t const *derevo = (derevo_t *)args;
    if (derevo->nodeFreeingTravesalFunctionPointer != NULL)
        derevo->nodeFreeingTravesalFunctionPointer(node, NULL);
    free(*node);
    *node = NULL;
    return true;
}

static void DerevoFreeNode(derevo_t *const derevo, derevo_node_t **const node) {
    DerevoDoTravesal(
        node,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        DerevoFreeNodePostorderTravesalFunction, (void *)derevo
    );
}

void DerevoPopNode(derevo_t *const derevo, derevo_node_t **const node) {
    LogEvent(derevo, "Before POP", "");
    DerevoFreeNode(derevo, node);
    LogEvent(derevo, "After POP", "");
}

derevo_node_t **DerevoDoTravesal(
    derevo_node_t **const node,
    travesal_function_t const leftSelectorFunctionPointer,  void *const leftSelectorArgs,
    travesal_function_t const rightSelectorFunctionPointer, void *const rightSelectorArgs,
    travesal_function_t const preorderFunctionPointer,      void *const preorderArgs,
    travesal_function_t const inorderFunctionPointer,       void *const inorderArgs,
    travesal_function_t const postorderFunctionPointer,     void *const postorderArgs
) {
    if (*node == NULL)
        return node;

    if (preorderFunctionPointer != NULL && !preorderFunctionPointer(node, preorderArgs))
        return node;

    bool const shouldGoLeft = leftSelectorFunctionPointer == NULL ? (*node)->left != NULL : leftSelectorFunctionPointer(node, leftSelectorArgs);
    if (shouldGoLeft) {
        derevo_node_t **leftResult = DerevoDoTravesal(
            &(*node)->left,
            leftSelectorFunctionPointer, leftSelectorArgs,
            rightSelectorFunctionPointer, rightSelectorArgs,
            preorderFunctionPointer, preorderArgs,
            inorderFunctionPointer, inorderArgs,
            postorderFunctionPointer, postorderArgs
        );
        if (leftResult != NULL)
            return leftResult;
    }

    if (inorderFunctionPointer != NULL && !inorderFunctionPointer(node, inorderArgs))
        return node;

    bool const shouldGoRight = rightSelectorFunctionPointer == NULL ? (*node)->right != NULL : rightSelectorFunctionPointer(node, rightSelectorArgs);
    if (shouldGoRight) {
        derevo_node_t **rightResult = DerevoDoTravesal(
            &(*node)->right,
            leftSelectorFunctionPointer, leftSelectorArgs,
            rightSelectorFunctionPointer, rightSelectorArgs,
            preorderFunctionPointer, preorderArgs,
            inorderFunctionPointer, inorderArgs,
            postorderFunctionPointer, postorderArgs
        );
        if (rightResult != NULL)
            return rightResult;
    }

    if (postorderFunctionPointer != NULL && !postorderFunctionPointer(node, postorderArgs))
        return node;

    return NULL;
}

void DerevoFinalize(derevo_t *const derevo) {
    LogEvent(derevo, "Finalize", "");
    DerevoFreeNode(derevo, &derevo->head);
}


// Logging

static void WriteGraphData(derevo_t *const derevo, FILE *const file) {
    fprintf(file, "digraph {\n");
    DerevoDoTravesal(&derevo->head, NULL, NULL, NULL, NULL, derevo->graphDataWritingTravesalFunctionPointer, (void *)file, NULL, NULL, NULL, NULL);
    fprintf(file, "}");
}


struct derevo_dump_preorder_travesal_function_args {
    FILE *file;
    travesal_function_t elementValueDumpingTravesalFunctionPointer;
};

static bool DerevoDumpPreorderTravesalFunction(derevo_node_t **node, void *rawArgs) {
    derevo_dump_preorder_travesal_function_args *args = (derevo_dump_preorder_travesal_function_args *)rawArgs; 
    fprintf(args->file, "(");
    args->elementValueDumpingTravesalFunctionPointer(node, args->file);
    fprintf(args->file, " ");
    return true;
}

static bool DerevoDumpInorderTravesalFunction(derevo_node_t **node, void *args) {
    FILE *file = (FILE *)args;
    if ((*node)->left == NULL)
        fprintf(file, "() ");
    if ((*node)->right == NULL)
        fprintf(file, "()");
    return true;
}

static bool DerevoDumpPostorderTravesalFunction(derevo_node_t **, void *args) {
    FILE *file = (FILE *)args;
    fprintf(file, ")");
    return true;
}

void DerevoDump(derevo_t *const derevo, FILE *const file) {
    derevo_dump_preorder_travesal_function_args preorderArgs = {
        file,
        derevo->elementValueDumpingTravesalFunctionPointer
    };
    DerevoDoTravesal(
        &derevo->head, 
        NULL, NULL, 
        NULL, NULL, 
        DerevoDumpPreorderTravesalFunction, (void *)&preorderArgs, 
        DerevoDumpInorderTravesalFunction, (void *)file, 
        DerevoDumpPostorderTravesalFunction, (void *)file
   );
}

void LogEvent(derevo_t *const derevo, char const *const name, char const *const body) {
    static size_t id = 0;

    {
        char buffer[1024] = "";
        snprintf(buffer, 1024, "%s/graph_%zu.gv", derevo->logger.dirPath, id);
        FILE *file = fopen(buffer, "w");
        WriteGraphData(derevo, file);
        fclose(file);
    }

    {
        char buffer[2048] = "";
        snprintf(buffer, 2048, "dot -Tpng %s/graph_%zu.gv -o %s/photo_%zu.png", derevo->logger.dirPath, id, derevo->logger.dirPath, id);
        system(buffer);
    }

    FILE *file = LoggerStartEvent(derevo->logger, name);
    fprintf(file, "%s\n", body);

    DerevoDump(derevo, file);

    fprintf(file, "\n");
    
    fprintf(file, "<img src=\"%s/photo_%zu.png\">", derevo->logger.dirPath, id);
    LoggerEndEvent(derevo->logger, file);
    id++;
}

void DerevoLog(derevo_t *const derevo) {
    LogEvent(derevo, "log", "");
}
