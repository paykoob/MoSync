#include "maheap.h"
#include "mastack.h"
#include "mavsprintf.h"

struct MA_STACK_FRAME* nextFrame(struct MA_STACK_FRAME* frame) {
	char* bp = (char*)(frame->_nextFrame);
	if(!bp)
		return NULL;
	else
		return (struct MA_STACK_FRAME*)(bp - 8);
}

static MAHandle sDumpFile = 0;
static void dumpStack(int req, int block, void* address);

extern void (*gDumpStack)(int req, int block, void* address);

void initStackDump(void) {
	// Mosync Heap Stack Dump.
	sDumpFile = maFileOpen("/mhsd.bin", MA_ACCESS_READ_WRITE);
	lprintfln("initStackDump maFileOpen: %i", sDumpFile);
	if(sDumpFile > 0) {
		// todo: better error handling
		int res = maFileExists(sDumpFile);
		if(res == 0)
			res = maFileCreate(sDumpFile);
		else if(res == 1)
			res = maFileTruncate(sDumpFile, 0);
		if(res < 0) {
			maFileClose(sDumpFile);
			sDumpFile = res;
			lprintfln("initStackDump error: %i", sDumpFile);
			return;
		}
		maFileWrite(sDumpFile, "MHSD", 4);	// magic header, identifies this file type.
		gDumpStack = &dumpStack;
	}
}

/* Dump format:
struct Dump {
	uint timeStamp;	// maGetMilliSecondCount()
	int requestedSize;	// number of bytes requested for malloc(). -1 for free().
	uint blockSize;	// number of bytes actually allocated/freed. 0 on malloc error.
	uint address;	// address of allocated block. 0 on malloc error.
	uint nFrames;	// number of stack frames

	struct Frame {
		uint address;
	} frames[nFrames];
};
*/
static void dumpStack(int req, int block, void* address) {
	int count;
	MA_STACK_FRAME* frame;
	if(sDumpFile <= 0)
		return;

	count = maGetMilliSecondCount();
	maFileWrite(sDumpFile, &count, sizeof(int));

	maFileWrite(sDumpFile, &req, sizeof(int));
	maFileWrite(sDumpFile, &block, sizeof(int));
	maFileWrite(sDumpFile, &address, sizeof(int));

	frame = getStackTop();
	count = 0;
	while(frame) {
		count++;
		frame = nextFrame(frame);
	}
	maFileWrite(sDumpFile, &count, sizeof(int));
	frame = getStackTop();
	while(frame) {
		maFileWrite(sDumpFile, &frame->retAddr, sizeof(int));
		frame = nextFrame(frame);
	}
}
