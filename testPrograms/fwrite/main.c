/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include <ma.h>
#include <conprint.h>
#include <maassert.h>
#include <MAFS/File.h>
#include "MAHeaders.h"

static void testfwrite(void)
{
  FILE * pFile;
  char c;

  pFile=fopen("alphabet.txt","wt");
	MAASSERT(pFile);
  for (c = 'A' ; c <= 'Z' ; c++) {
    putc (c , pFile);
  }
  fclose (pFile);
}

static void testfopen(void) {
	FILE *f;
	int i;
	setCurrentFileSystem(RES_BUNDLE, 0);
	f = fopen("mafs.data", "rb");
	MAASSERT(f);

	for(i = 0; i < 10; i++) {
		int j;
		/*int ret =*/ fread(&j, sizeof(int), 1, f);
		if(i == j) {
			;//int a = 2;
		}
	}
}

static void testGetData(void) {
	MAFS_FILE_DATA fd;
	int res;

	setCurrentFileSystem(RES_BUNDLE, 0);
	res = MAFS_getFileData(&fd, "mafs.data");
	printf("gfd: %i\n", res);
	MAASSERT(res > 0);
	printf("h %i, o %i, s %i\n", fd.h, fd.offset, fd.size);
}

int MAMain(void) {
	testGetData();
	testfopen();
	testfwrite();
	return 0;
}
