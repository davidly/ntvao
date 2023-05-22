 /* converts apple to IBM text files */
 /* or vice versa                    */
 /* defaults to apple to IBM         */
 /* by bill buckels 1990             */
 /* revised somewhat 2008 to display usage and handle unix text
    and added some minor fix-ups like autodetecting input files etc. */

 #include <stdio.h>
 #include <io.h>
 #include <stdlib.h>


 FILE *infile, *outfile;
 char iname[66], oname[66];

char *usage = "Usage : \"APPIBTEXT [infile] [outfile] [FORMAT-optional]\"\n"
              "        OUTPUT FORMAT = \"A\" for APPLE\n"
              "                        \"I\" for IBM\n"
              "                        \"U\" for UNIX\n"
              "        Default is IBM\n";



crlf(char c)
{

	 switch(c) {

	  case 'U': fputc(10,outfile); break;
	  case 'A': fputc(13,outfile); break;
	  case 'I': fputc(13,outfile);
	            fputc(10,outfile); break;
	}
}

 main(int argc, char **argv)
 {
     long target, count=0;
     char c='I';
     int d, e;


     iname[0] = oname[0] = 0;

     /* check command line args for legal args */
     /* if not print the usage and split       */

     printf("AppIBtxt(C) Copyright Bill Buckels 1990-2008\nAll rights reserved.");

     if(argc > 3)
     {
        c= toupper(argv[3][0]);
        if (c != 'A' && c != 'U')c = 'I';
     }
     else printf(usage);

     if (argc < 2) {
		 printf("Infile  :");gets(iname);
	 }
	 else strcpy(iname, argv[1]);

     if (argc < 3) {
		 if (iname[0] != 0) {
		 	printf("Outfile :");gets(oname);
		 }
	 }
	 else strcpy(oname, argv[2]);

     if (iname[0] == 0 || oname[0] == 0) c = 0;


     switch(c)
     {
         /* if we are converting from apple to IBM then  */
         /* replace carriage returns with CR + line feed */
         case 'I' :
         /* if we are going back to apple then remove the line feeds */
         case 'A' :
         /* if going to unix use line-feeds only */
         case 'U':

                   if((infile = fopen(iname,"rb"))==NULL)
                   {
                     perror(iname);
                     break;
                   }
                   if((outfile = fopen(oname,"wb"))==NULL)
                   {
                     perror(oname);
                     fclose(infile);
                     break;
                   }

                   target = filelength(fileno(infile));

                   printf("Infile     < %s\n",iname);
                   printf("Outfile    < %s\n",oname);
                   printf("           < Count > %ld",target);

                   e = 0;
                   while(count<target)
                   {
                     d=fgetc(infile);
                     count++;
                     if(!(count%10))printf("\r%ld",count);
                     if (d == 13) crlf(c);
					 else if (d==10) {
						 /* if previous character was a carriage return
						    we are reading an MS-DOS text file
						    so skip the line feed */
						 if (e!=13)crlf(c);
					 }
					 else fputc(d,outfile);
					 e = d;

                     }
                     fclose(infile);
                     fclose(outfile);
                     printf("\rDone!                                  \n");
                     break;
        }


     exit(0);

 }

