#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int validation( const char* , const char* );

int main( int argc, char* argv[] )
{

    int correctDParameters = 0;
    int correctFParameters = 0;
    int correctIntArguments = 0;
    int correctFloatArguments = 0;
    int integerSum = 0;
    double floatingSum = 0;
    int opt = 0;
    char *endPointer = NULL;
    int d = 0;
    double f = 0;
    while ((opt = getopt(argc, argv, "d:f:")) != -1)
    {
        switch(opt)
        {
            case 'd':

                d = (int)strtol(optarg, &endPointer, 10);
                printf("%d\n",d);
                if (validation(optarg,endPointer) == 0)
                {
                    perror("Incorrect argument after -d\n");
                    return -4;
                }
                if( errno == ERANGE ) {
                    perror("Number out of range@!@!\n");
                    return -3;
                }
                correctDParameters++;
                integerSum+=d;
                break;
            case 'f':
                f = strtod(optarg, &endPointer);
                printf("%f\n",f);
                if (validation(optarg,endPointer) == 0)
                {
                    perror("Incorrect parameter after -f\n");
                    return -4;
                }
                if( errno == ERANGE ) {
                    perror("Number out of range@!@!\n");
                    return -3;
                }
                correctFloatArguments++;
                floatingSum += f;
                break;
            default:
                perror("Incorrect flag\n!");

        }
    }
    while (optind < argc)
    {
        f = strtod(argv[optind], &endPointer);
        if (validation(argv[optind],endPointer) == 0)
        {
            perror("Incorrect argument!\n");
            return -5;
        }
        if (errno == ERANGE)
        {
            perror("Number out of range@!@!\n");
            return -3;
        }
        d = (int)strtol(argv[optind], &endPointer,10);
        if (errno == ERANGE)
        {
            perror("Number out of range@!@!\n");
            return -3;
        }
        if (f == d)
        {
            printf("%d\n",d);
            correctIntArguments++;
            integerSum += d;
        }
        else {

            printf("%f\n",f);
            correctFloatArguments++;
            floatingSum += f;
        }
        optind++;
    }
    printf("Srednia z calkowitych %d\n", integerSum/(correctIntArguments + correctDParameters));
    printf("Srednia ze zmienno %f\n", floatingSum/(correctFloatArguments + correctFParameters));
    return 0;
}

int validation( const char* argument, const char* endPointer )
{
    if( *argument != '\0' && *endPointer == '\0' ) {
        return 1;
    }
    else {
        return 0;
    }
}