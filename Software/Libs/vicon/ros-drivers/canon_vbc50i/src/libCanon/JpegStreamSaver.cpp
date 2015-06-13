#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "canon_vbc50i/libCanon/JpegStreamSaver.h"

bool JpegStreamSaver::record(const char * buffer, unsigned int size)
{
	char cstr[16],dstr[16];
	FILE * fp;
	std::string s;
	while (1) {
		sprintf(dstr,"-%08d",counter/1000);
		sprintf(cstr,"/%03d.jpg",counter%1000);
		s = directory+ "/" + base + dstr;
		mkdir(s.c_str(),0775);
		s += cstr;
		fp = fopen(s.c_str(),"r");
		if (fp == NULL) break;
		fprintf(stderr,"Warning: skipping '%s'\n",s.c_str());
		fclose(fp);
		counter += 1;
		if (counter == 0) /* loop */ {
			counter -= 1;
			return false;
		}
	}
	fp = fopen(s.c_str(),"w");
	unsigned int rs = fwrite(buffer,size,1,fp);
	fclose(fp);
	counter += 1;
	return (rs == 1);
}

