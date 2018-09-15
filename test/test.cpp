#include <stdio.h>
#include "test_run.h"

int main(int argc, char** argv)
{
    return thread_utils::tests::test_run() ? 0 : 1;
}