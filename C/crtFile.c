#include<stdio.h>

int main(int argc,char* argv[])
{
    FILE *fp=NULL;
    int i=0;
    if(argc<2)
    {
        fprintf(stderr,"ERROR:Please Provide Filename\n");
        return -1;
    }
    for(i=1;i<argc;i++)
    {
        if(fp=fopen(argv[i],"r")){
        fprintf(stderr,"ERROR:%s ALREADY EXIST",argv[i]);
        return -1;
        }
        fp=fopen(argv[i],"w");
        if(fp==NULL)
        {
            fprintf(stderr,"ERROR:%s creation unsuccessful\n",argv[i]);
            return -1;
        }
        fclose(fp);
    }
    return 0;
}

/**
 * 
 * This file is command utility for my laptop which creates file if not already 
 * present.
 * I has less background in programming back then
 */