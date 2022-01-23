#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int validation(const char* , const char*);
void writeToFileOne(FILE*);
void writeToFileTwo(FILE*);

int main( int argc, char* argv[] )
{

    if( argc!= 6 )
    {
        perror("Usage -d<int> -k<int> <path_to_file>");
        exit(EXIT_FAILURE);
    }
    int opt = 0;
    int d = 0;
    int k = 0;
    char *endPointer = NULL;
    while ((opt = getopt(argc, argv, "d:k:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                d = (int)strtol(optarg, &endPointer, 10);
                validation(optarg,endPointer);
                if (d != 1 && d != 2)
                {
                    perror("Parameter d must be 1 or 2!\n");
                    return -4;
                }
                break;
            case 'k':
                k = (int)strtol(optarg, &endPointer, 10);
                validation(optarg,endPointer);
                if (k <= 0)
                {
                    perror("Parameter k must be non-negative number!\n");
                    return -5;
                }
                break;
            default:
                perror("Wrong flags!\n");
        }
    }

    FILE* fileDescriptor1 = fopen(argv[optind],"w");
    if (!fileDescriptor1) { perror("open 1\n"); }
    FILE* fileDescriptor2 = fopen(argv[optind],"w");
    if (!fileDescriptor2) { perror("open 2\n"); }
    for (int i=0; i<k; ++i)
    {
        writeToFileOne(fileDescriptor1);
        if (d==2) { writeToFileTwo(fileDescriptor2); }
        else writeToFileTwo(fileDescriptor1);

    }
    if (fclose(fileDescriptor1) == -1)
    {
        perror("Error in close first descriptor\n");
        exit(EXIT_FAILURE);
    }
    if (fclose(fileDescriptor2) == -1)
    {
        perror("Error in close second descriptor\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int validation(const char* argument , const char* endPointer)
{
    if (*argument != '\0' && *endPointer == '\0') { return 1; }
    else { return 0; }
}

void writeToFileOne(FILE* file)
{
    static double number = 0.625;
    int element = 0;
    char buffer[1024] = {0};
    sprintf(buffer,"%0.3f\n",number);
    element = fwrite(buffer, 1,sizeof(buffer),file);
    if (element != sizeof(buffer))
    {
        perror("Error in fwrite!\n");
        exit(EXIT_FAILURE);
    }
    number += 0.625;
}
void writeToFileTwo(FILE* file)
{
    static short int number = 0;
    int element = 0;
    char buffer[1024] = {0};
    for (int i=0; i<19; ++i)
    {
        sprintf(buffer,"%d\n",number);
        element = fwrite(buffer, 1, sizeof(buffer), file);
        if (element != sizeof(buffer))
        {
            perror("Error in fwrite!\n");
            exit(EXIT_FAILURE);
        }
        number += 80;
    }
}