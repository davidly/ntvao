
/* makeprg.c by bill buckels 1990 */
/* strips the 4 byte bloadable header
   from a manx C65 executable and outputs
   a same named file with an extension.

   default extension is PRG */


#include <stdio.h>
#include <io.h>

int bitcopy(char *name1, char *name2)
{
    FILE *fp,*fp2;
    unsigned char c;

    long target,count=0;
    if((fp=fopen(name1,"rb"))==NULL){
                                     perror("Sorry..");
                                     return -1;
                                     }
    fp2=fopen(name2,"wb");
    target=filelength(fileno(fp));


    while(count<target)
        {
        c=fgetc(fp);
        if(count>3)fputc(c,fp2);
        count++;
        }
    fclose(fp);
    fclose(fp2);

   return(0);
}


main(int argc, char *argv[])
{
    char buf[66];

    if(argc>1)
    {
		if (argc > 2)sprintf(buf,"%s.%s",argv[1],argv[2]);
		else sprintf(buf,"%s.PRG",argv[1]);

    	bitcopy(argv[1],buf);
    }
    else
    {
        printf(
        "Usage is \"makeprg [MANX C65 EXECUTABLE FILE- NO EXTENSION]\"\n"
        "Optional \"makeprg [MANX C65 EXECUTABLE FILE- NO EXTENSION] [EXTENSION]\"\n");
    }
    exit(0);
}

