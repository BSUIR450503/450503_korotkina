#include "Manager.h"

using namespace std;

int main(int argc, char *argv[])
{
    int *firstBlock = (int*)malloc(50 * sizeof(int));

    for (int i = 0; i < 50; i++)
    {
        firstBlock[i] = 1;
    }

    cout << "\nFirst block [50]:" << endl;

    for (int i = 0; i < 50; i++){
        cout << firstBlock[i];
    }

    int *secondBlock = (int*)malloc(10 * sizeof(int));

    cout << "\nSecond block [10]:" << endl;

    for (int i = 0; i < 10; i++)
    {
        secondBlock[i] = 2;
        cout << secondBlock[i];
    }

    secondBlock = (int*)realloc(secondBlock, 15 * sizeof(int));

    cout << "\nSecond block after realloc [15]:" << endl;

    for (int i = 0; i < 15; i++)
    {
        cout << secondBlock[i];
    }

    cout << "\nFirst block [10]:" << endl;

    for (int i = 0; i < 10; i++)
    {
        cout << firstBlock[i];
    }

    free (firstBlock);

    firstBlock = (int*)malloc(50 * sizeof(int));

    cout << "\nFirst block after free and malloc [80]:" << endl;

    for (int i = 0; i < 80; i++)
    {
        cout << firstBlock[i];
    }

    cout << endl;

    return 0;
}

