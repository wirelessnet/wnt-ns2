#ifndef ns_distance_h
#define ns_distance_h

#include "stdio.h"
#include <stdlib.h>


int convert(FILE *out,int id,int x,int y);
int writeFile(FILE *out, double value);
char* transform(int tmp);

int convert(FILE *out,int id,int x,int y){
	fputs(transform(id),out);
	putc(' ',out);
	fputs(transform(x),out);	
	fputs(transform(y),out);	
	//writeFile(out,x);	
	//writeFile(out,y);
    putc('\n',out);
	
	return 0;
}
char* transform(int tmp){
	char *a="";
	sprintf(a, "%d", tmp);
	return a;
}
/*int writeFile(FILE *o, double value){
	char *string;
	int dec, sign;
	int ndig = 10;
	string = ecvt(value, ndig, &dec, &sign);
	
	if(sign==1) fputs("-",o);
	if(dec==0)putc('0',o);
	for(int i=0; string[i]!='\0';i++){
       if(dec==i){ 
				fputs(".",o);
				putc(string[i],o);
		}
       else putc(string[i],o);
	}	
	fputs(" ",o);
	
	return 0;

}*/
#endif