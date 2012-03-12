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

/******************************************************************************/
/* MoSync GDB Stub                                                            */
/* (c) 2007 Mobile Sorcery                                                    */
/* This is used to compile the GDB debug version of the mosync runtime.       */
/******************************************************************************/

#define DEBUGGING_MODE

#include <stdexcept>
#include "config_platform.h"
#include <helpers/helpers.h>
#include <helpers/attribute.h>
#include <base/base_errors.h>

using namespace MoSyncError;

#ifdef GDB_DEBUG

#include "config_platform.h"
#include "GdbStub.h"
#include <net/net.h>
#include "ThreadPoolImpl.h"
#include <mostl/vector.h>

#include "fastevents.h"
#include "sdl_syscall.h"

#define CORE mCore

// Map from integer to a hexadecimal ASCII character.
static const char hexChars[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

class GdbStubInt : public GdbStub {
public:
	GdbStubInt(Core::VMCore* core, CpuType);

	void setupDebugConnection();
	void closeDebugConnection();

	// we arrive here when a trap occurs in the mosync program
	void exceptionHandler(int exception);
	void exitHandler(int exception);
	bool waitForRemote();	//returns true if stub has quit.

private:
	// Connections from and to the GDB host.
	TcpServer server;
	TcpConnection *connection;

	void runControl();
	static int sRunControl(void*);
	bool mQuit, mExitPacketSent;
	MoSyncThread mControlThread;

	// CpuType stuff
	const int mNregs;

	//waitForRemote()
	MoSyncSemaphore mExecSem;

	// Unprotected queue
	enum MessageType {
		eInput, eException, eExit, eTerminate
	};
	struct MESSAGE {
		MessageType type;
		int code;
	};
	class MessageQueue {
	private:
		static const int QUEUE_SIZE = 4;
		MESSAGE mArray[QUEUE_SIZE];
		int mWP, mRP;
	public:
		MessageQueue();
		MESSAGE pop();
		void push(MESSAGE);
	} mMessageQueue;

	// Internal messages
	MoSyncSemaphore mMessageSem;
	MoSyncSemaphore mMessageMutex;

	MESSAGE getMessage();
	void putMessage(MESSAGE);
	void handleMessage();

	// handlers
	void sendExceptionPacket(int code);
	void sendExitPacket(int code);
	void sendTerminationPacket(int code);

	void runRead();
	static int sRunRead(void*);
	MoSyncThread mReadThread;
	MoSyncSemaphore mReadSem;
	MoSyncSemaphore mReadThreadSem;

	void handleInput();
	void handleAck();
	void handleNack();
	int handlePacket(int pos);	//returns new pos, or <0 if packet was incomplete

	// Used to store the input data and output data.
	//char mInputBuffer[1024*10];	//todo:make variable length
	mostd::vector<char> mInputBuffer;
	int mInputPos;
	char* mInputPtr;	//legacy

	mostd::vector<char> outputBuffer;
	char *curOutputBuffer;

	bool mForceNextPacket;

	// Acks
	char getAck();

	bool mWaitingForAck;

	// Reference to the core, used to retrieve and write memory/registers and such.
	Core::VMCore *mCore;

	char tempBuffer[1024];

	template<typename type>
	const char* convertDataTypeToString(type b) {
		char* ret = tempBuffer;
		for(int i = (sizeof(type)<<3)-4; i >= 0; i-=4) {
			*ret++ = (hexChars[((b>>i)&0xf)]);
		}
		*ret++ = 0;
		return tempBuffer;
	}

	int hexToNum(char c);

	template<typename type>
	type getDataTypeFromString(char*& b) {
		type ret = 0;
		for(int i = (sizeof(type)*2)-1; i >= 0; i-=1) {
			ret |= ((type)(hexToNum(*b)))<<(i*4);
			b++;
		}
		return ret;
	}

	template<typename type>
	type getBoundedDataTypeFromInput(char bound) {
		const char *start = mInputPtr;
		while(*mInputPtr!=bound) mInputPtr++;
		const char *end = mInputPtr;
		mInputPtr++;
		type ret = 0;
		type n = 0;
		while(start!=end && (n=hexToNum(*start))!=-1) {
			ret<<=4;
			ret|=n;
			start++;
		}
		return ret;
	}


	void appendOut(const char *what);
	void appendOut(char what);
	void checkAndResize(int len);

	template<typename type>
	void appendDataTypeToOutput(type d) {
		appendOut(convertDataTypeToString<type>(d));
	}

	template<typename type>
	type getDataTypeFromInput() {
		return getDataTypeFromString<type>(mInputPtr);
		//mInputPtr += ((sizeof(type))-1)<<1;
	}

	void clearOutputBuffer();

	void putDebugChar(char c);
	void putDebugChars(char *c, int len);

	void transmissionNAK();
	void transmissionACK();

	// Required commands follows:
	bool readRegisters();
	bool writeRegisters();
	bool readMemory();
	bool writeMemory();
	bool continueExec();
	bool stepExec();

	/**
	 * Terminates the program (relatively) gracefully.
	 *
	 * Note:
	 * This function must try to get the main thread out of the
	 * kernel, e.g. if it is waiting idefinitley for a semaphore,
	 * otherwise MoRE will not receive any signals and will hang
	 * on Unix like systems if mdb dies.
	 *
	 * This function does not call exit directly, but implicitly
	 * makes the other threads stop. It relies on the VM loop to
	 * exit when waitForRemote returns false.
	 */
	bool quit();

	// Optional commands follows:
	bool lastSignal();
	bool readRegister();
	bool writeRegister();
	bool killRequest();
	bool toggleDebug();
	bool reset();
	bool search();
	bool generalSet();
	bool generalQuery();
	bool sectionOffsetsQuery();
	bool consoleOutput();
	bool defaultResponse();

	bool appendOK();

	// Select and execute command:
	bool executeCommand();
	void putPacket(bool force = false);

	// Read and execute command and send reply.
	bool doPacket();
};


GdbStub* GdbStub::create(Core::VMCore* core, CpuType type) {
	return new GdbStubInt(core, type);
}

static int nregs(GdbStub::CpuType t) {
	switch(t) {
	case GdbStub::Mapip: return 128;
	case GdbStub::Arm: return 16;
	default: DEBIG_PHAT_ERROR;
	}
}

GdbStubInt::GdbStubInt(Core::VMCore *core, CpuType type)
	: mNregs(nregs(type))
{
	outputBuffer.resize(1024);
	curOutputBuffer = outputBuffer.begin();
	mInputBuffer.resize(1024);
	mCore = core;
	mWaitingForAck = false;
	mQuit = false;
	mExitPacketSent = false;
	mForceNextPacket = false;
	mMessageMutex.post();	//leave it open so exactly one thread can get in
}

void GdbStub::setupDebugConnection() { ((GdbStubInt*)this)->setupDebugConnection(); }
void GdbStub::closeDebugConnection() { ((GdbStubInt*)this)->closeDebugConnection(); }

void GdbStub::exceptionHandler(int exception) { ((GdbStubInt*)this)->exceptionHandler(exception); }
void GdbStub::exitHandler(int code) { ((GdbStubInt*)this)->exitHandler(code); }
bool GdbStub::waitForRemote() { return ((GdbStubInt*)this)->waitForRemote(); }


int GdbStubInt::hexToNum(char c) {
	if(c>='0'&&c<='9') return c-'0';
	else if(c>='a'&&c<='f') return c+10-'a';
	else if(c>='A'&&c<='F') return c+10-'A';
	else return -1;
}

void GdbStubInt::checkAndResize(int len) {
	size_t curIndex = (size_t)(curOutputBuffer-outputBuffer.begin());
	size_t newSize = (len+curIndex)+1; // +1 for the null terminator...
	if(newSize>(outputBuffer.size())) {
		//BIG_PHAT_ERROR(ERR_INTERNAL);
		size_t betterSize = (outputBuffer.size()+2)*2;
		if(betterSize>newSize) newSize=betterSize;
		outputBuffer.resize(newSize);
		curOutputBuffer = outputBuffer.begin()+curIndex;
	}
}

void GdbStubInt::appendOut(const char *what) {
	checkAndResize(strlen(what));
	curOutputBuffer += sprintf(curOutputBuffer, "%s", what);
}

void GdbStubInt::appendOut(char what) {
	checkAndResize(1);
	*curOutputBuffer++ = what;
	*curOutputBuffer = 0;
}

void GdbStubInt::clearOutputBuffer() {
	curOutputBuffer = outputBuffer.begin();
	checkAndResize(1024);
	appendOut('$');

}

void GdbStubInt::setupDebugConnection() {
	connection = NULL;
	int res = server.open(50000);
	MYASSERT(res > 0, ERR_GDB_SERVER_OPEN);

	res = server.accept(connection);
	DEBUG_ASSERT(res > 0);

	mControlThread.start(sRunControl, this);
	mReadThread.start(sRunRead, this);
}

int GdbStubInt::sRunControl(void* arg) {
	GdbStubInt* s = (GdbStubInt*)arg;
	s->runControl();
	return 0;
}
void GdbStubInt::runControl() {
	LOGD("runControl()\n");
	while(!mQuit) {
		try {
			handleMessage();
		} catch(std::exception& e) {
			LOG("Caught exception: %s\n", e.what());
			mQuit = true;
		}
	}
	mExecSem.post();
}

void GdbStubInt::handleMessage() {
	MESSAGE m = getMessage();	//blocking
	switch(m.type) {
	case eInput:
		//LOG("MESSAGE input\n");
		handleInput();
		mReadSem.post();
		break;
	case eException:
		LOGD("MESSAGE exception\n");
		sendExceptionPacket(m.code);
		break;
	case eExit:
		LOG("MESSAGE exit\n");
		sendExitPacket(m.code);
		break;
	case eTerminate:
		LOG("MESSAGE terminate\n");
		if(!mExitPacketSent)
			sendTerminationPacket(m.code);
		mQuit = true;
		break;
	default:
		DEBIG_PHAT_ERROR;
	}
}

GdbStubInt::MESSAGE GdbStubInt::getMessage() {
	MESSAGE m;
	mMessageSem.wait();	//wait for a message to arrive in the queue
	mMessageMutex.wait();	//get the lock on the queue
	{
		m = mMessageQueue.pop();
	}
	mMessageMutex.post();	//release the lock on the queue
	return m;
}

void GdbStubInt::putMessage(MESSAGE m) {
	//LOG("putMessage %i\n", m.type);
	mMessageMutex.wait();	//get the lock on the queue
	{
		mMessageQueue.push(m);
	}
	mMessageMutex.post();	//release the lock on the queue
	mMessageSem.post();	//signal getMessage()
}


GdbStubInt::MessageQueue::MessageQueue() {
	mWP = mRP = 0;
}
GdbStubInt::MESSAGE GdbStubInt::MessageQueue::pop() {
	DEBUG_ASSERT(mWP != mRP);
	MESSAGE m = mArray[mRP];
	mRP++;
	if(mRP == QUEUE_SIZE)
		mRP = 0;
	return m;
}
void GdbStubInt::MessageQueue::push(MESSAGE m) {
	int oldWP = mWP;
	mWP++;
	if(mWP == QUEUE_SIZE)
		mWP = 0;
	DEBUG_ASSERT(mWP != mRP);
	mArray[oldWP] = m;
}


//wait until the remote has requested that execution be started/resumed.
bool GdbStubInt::waitForRemote() {
	if(!mQuit) {
		if(!mControlThread.isCurrent()) {
			mExecSem.wait();
		}
	}
	return mQuit;
}

// we arrive here when a trap occurs in the mosync program
void GdbStubInt::exceptionHandler(int code) {
	MESSAGE m;
	m.type = eException;
	m.code = code;
	putMessage(m);
}
void GdbStubInt::sendExceptionPacket(int code) {
	clearOutputBuffer();
	appendOut('S');
	appendOut(hexChars[(code>>4)&0xf]);
	appendOut(hexChars[(code)&0xf]);
	putPacket();
}

void GdbStubInt::exitHandler(int code) {
	MESSAGE m;
	m.type = eExit;
	m.code = code;
	putMessage(m);
}
void GdbStubInt::sendExitPacket(int code) {
	clearOutputBuffer();
	appendOut('W');
	appendOut(hexChars[(code>>4)&0xf]);
	appendOut(hexChars[(code)&0xf]);
	putPacket();
	mExitPacketSent = true;
}

void GdbStubInt::sendTerminationPacket(int code) {
	clearOutputBuffer();
	appendOut('X');
	appendOut(hexChars[(code>>4)&0xf]);
	appendOut(hexChars[(code)&0xf]);
	putPacket();
}

void GdbStubInt::closeDebugConnection() {
	{
		MESSAGE m;
		m.type = eTerminate;
		m.code = 0;
		putMessage(m);
	}
	mControlThread.join();

	server.close();
	if(connection) {
		connection->close();
		mReadSem.post();
		mReadThread.join();
	}
}

int GdbStubInt::sRunRead(void* arg) {
	GdbStubInt* s = (GdbStubInt*)arg;
	s->runRead();
	return 0;
}
void GdbStubInt::runRead() {
	mInputPos = 0;
	while(!mQuit) {
		if(mInputPos>=(int)mInputBuffer.size()) {
			mInputBuffer.resize((mInputBuffer.size()+1)*2);
		}
		int res = connection->read(mInputBuffer.begin() + mInputPos,
			(mInputBuffer.size() - mInputPos)-1);
		//LOG("GDB read %i\n", res);
		if(res <= 0) {
			LOG("connection->read error %i\n", res);

			/* If there is a connection error, then either mdb has crashed, been
			 * killed by a signal or gotten -gdb-exit. In either case we should
			 * quit since we cannot restore the debug session. */
			mQuit = true;
			break;
		}
		mInputPos += res;
		mInputBuffer[mInputPos] = 0;
		MESSAGE m;
		m.type = eInput;
		putMessage(m);
		mReadSem.wait();	//wait until this packet has been handled before reading more
		//LOG("GDB read wait complete\n");
	}
	mExecSem.post();
}

void GdbStubInt::handleInput() {
	int pos = 0;
	while(pos < mInputPos) {
		switch(mInputBuffer[pos]) {
		case '+':
			handleAck();
			pos++;
			break;
		case '-':
			handleNack();
			pos++;
			break;
		case '$':
			pos = handlePacket(pos + 1);
			break;
		case 0x03:
			LOG("Stub recieved interrupt signal\n");
			mCore->mGdbSignal = eInterrupt;
			{
				SDL_UserEvent event = { FE_INTERRUPT, 0, NULL, NULL };
				FE_PushEvent((SDL_Event*)&event);
			}
			pos++;
			break;
		default:
			DEBIG_PHAT_ERROR;
		}
		if(pos < 0) {
			return;
		}
	}
	//now we've handled everything, and can reset the input buffer.
	mInputPos = 0;
}

void GdbStubInt::handleAck() {
	LOGD("GDB ACK was sent from the client.\n");
	if(mWaitingForAck) {
		mWaitingForAck = false;
	} else {
		// seems GNU gdb does this.
		//DEBIG_PHAT_ERROR;
	}
}

char GdbStubInt::getAck() {
	DEBUG_ASSERT(!mWaitingForAck);
	mWaitingForAck = true;
	while(!mQuit && mWaitingForAck) {
		handleMessage();
	}
	return '+';
}

void GCCATTRIB(noreturn) GdbStubInt::handleNack() {
	LOG("GDB NACK was sent from the client.\n");
	DEBIG_PHAT_ERROR;
}

int GdbStubInt::handlePacket(int pos) {
	int begin = pos;
	uint myChecksum = 0;
	while(pos < mInputPos && mInputBuffer[pos] != '#') {
		myChecksum += (byte)mInputBuffer[pos];
		pos++;
	}
	myChecksum &= 0xFF;

	//make sure we got a whole packet
	if(mInputBuffer[pos] != '#')
		return -1;
	//including the checksum
	pos++;
	if(pos + 2 > mInputPos)
		return -1;

	//check the checksum
	uint theirChecksum;
	int res = sscanf(mInputBuffer.begin() + pos, "%x", &theirChecksum);
	pos += 2;
	if(res != 1 || myChecksum != theirChecksum) {
		transmissionNAK();
		return pos;
	}

	//parse the packet
	mInputPtr = mInputBuffer.begin() + begin;
	doPacket();
	return pos;
}


void GdbStubInt::putDebugChar(char c) {
	//LOG("putDebugChar(%c)\n", c);
	int res = connection->write(&c, 1);
	if(res <= 0) {
		LOG("connection->write error %i\n", res);
		throw std::logic_error("gdbConn write error");
	}
}

void GdbStubInt::putDebugChars(char *c, int len) {
	int res = connection->write(c, len);
	if(res <= 0) {
		LOG("connection->write error %i\n", res);
		throw std::logic_error("gdbConn write error");
	}
}

void GdbStubInt::transmissionNAK() {
	LOG("GDB transmission: NAK\n");
	putDebugChar('-');
}

void GdbStubInt::transmissionACK() {
	LOGD("GDB transmission: ACK\n");
	putDebugChar('+');
}

// Required commands follows:
bool GdbStubInt::readRegisters() {
	for(int i = 0; i < mNregs; i++) {
		appendDataTypeToOutput<int>(mCore->regs[i]);
	}
	appendDataTypeToOutput<int>(Core::GetIp(mCore));
	return true;
}

bool GdbStubInt::writeRegisters() {
	int i;
	for(i = 0; i < mNregs; i++) {
		mCore->regs[i] = getDataTypeFromInput<int>();
	}
	Core::SetIp(mCore, getDataTypeFromInput<int>());
	return true;
}

bool GdbStubInt::readMemory() {
	int address = getBoundedDataTypeFromInput<int>(',');
	int length = getBoundedDataTypeFromInput<int>(0);

	byte *src;
	int size;
	if(address >= DATA_MEMORY_START && address < INSTRUCTION_MEMORY_START) {
		size = mCore->DATA_SEGMENT_SIZE;
		src = (byte*)mCore->mem_ds;
	} else {
		size = mCore->CODE_SEGMENT_SIZE;
		src = (byte*)mCore->mem_cs;
	}
	address &= ADDRESS_MASK;
	if(address >= size || address + length > size) {
		LOG("bad address: 0x%x + 0x%x\n", address, length);
		return false;
	}
	src += address;

	for(int i = 0; i < length; i++) {
		appendDataTypeToOutput<byte>(src[i]);
	}
	return true;
}

bool GdbStubInt::writeMemory() {
	int address = getBoundedDataTypeFromInput<int>(',');
	int length = getBoundedDataTypeFromInput<int>(':');

	byte *dst;
	int size;
	if(address>=DATA_MEMORY_START&&address<INSTRUCTION_MEMORY_START) {
		size = mCore->DATA_SEGMENT_SIZE;
		dst = (byte*)mCore->mem_ds;
	} else {
		size = mCore->CODE_SEGMENT_SIZE;
		dst = (byte*)mCore->mem_cs;
	}
	address&=ADDRESS_MASK;

	if(address >= size || address + length > size) {
		LOG("bad address: 0x%x + 0x%x\n", address, length);
		return false;
	}

	for(int i = 0; i < length; i++) {
		dst[address+i] = getDataTypeFromInput<byte>();
	}
	appendOut("OK");

	return true;
}

bool GdbStubInt::continueExec() {
	int address = getBoundedDataTypeFromInput<int>(0);
	if(address) {
		Core::SetIp(mCore, address);
	}
	mCore->mGdbSignal = eNone;
	mExecSem.post();
	return true;
}

bool GdbStubInt::stepExec() {
	int address = getBoundedDataTypeFromInput<int>(0);
	if(address) {
		Core::SetIp(mCore, address);
	}
	mCore->mGdbSignal = eStep;
	mExecSem.post();
	return true;
}

bool GdbStubInt::quit() {
	mQuit = true;
	return true;
}

// Optional commands follows:
bool GdbStubInt::lastSignal() {
	appendOut("S00");
	return true;
}

bool GdbStubInt::readRegister() {
	return false;
}

bool GdbStubInt::writeRegister() {
	return false;
}

bool GdbStubInt::killRequest() {
	return false;
}

bool GdbStubInt::toggleDebug() {
	return false;
}

bool GdbStubInt::reset() {
	return false;
}

bool GdbStubInt::search() {
	return false;
}

bool GdbStubInt::generalSet() {
	return false;
}

bool GdbStubInt::generalQuery() {
	if(strncmp(mInputPtr, "Offsets", sizeof("Offsets")-1) == 0) {
		return sectionOffsetsQuery();
	}
	mForceNextPacket = true;
	return true;
}

bool GdbStubInt::sectionOffsetsQuery() {
	appendOut("TextSeg=0");
	return true;
}

bool GdbStubInt::consoleOutput() {
	return false;
}

bool GdbStubInt::defaultResponse() {
	appendOut("NO");
	mForceNextPacket = true;
	return true;
}

bool GdbStubInt::appendOK() {
	appendOut("OK");
	return true;
}


// Select and execute command:
bool GdbStubInt::executeCommand() {
	char instruction = *mInputPtr;
	mInputPtr++;

	LOGD("GDB instruction '%c' was sent from the client.\n", instruction);

	switch(instruction) {

		// required
		case 'g': return readRegisters();
		case 'G': return writeRegisters();
		case 'm': return readMemory();
		case 'M': return writeMemory();
		case 'c': return continueExec();
		case 's': return stepExec();
		case 'e': return quit();

			// optional;

		case 'H': return appendOK();
		case 'q': return generalQuery();
		case '?': return lastSignal();

#if 0
		case 'p': return readRegister();
		case 'P': return writeRegister();
		case 'k': return killRequest();
		case 'd': return toggleDebug();
		case 'r': return reset();
		case 't': return search();
			case 'Q' return generalSetQuery();
			case 'O' return consoleOutput();
#endif	//0

		default:
			return defaultResponse();
	}
}

void GdbStubInt::putPacket(bool force) {
	DEBUG_ASSERT(!mWaitingForAck);
	int calculatedChecksum = 0;
	const char *cur = outputBuffer.begin() + 1;
	*curOutputBuffer = 0;
	int len = 0;
	while(*cur) {
		len++;
		calculatedChecksum += (byte)(*cur++);
	}
	if(len == 0 && !force)
		return;

	appendOut('#');
	appendOut(hexChars[(calculatedChecksum>>4)&0xf]);
	appendOut(hexChars[(calculatedChecksum)&0xf]);
	*curOutputBuffer = 0;

	LOGD("GDB transmission: \"%s\"\n", outputBuffer.begin());
	mWaitingForAck = true;
	putDebugChars(outputBuffer.begin(), curOutputBuffer - outputBuffer.begin());
}

// Read and execute command and send reply.
bool GdbStubInt::doPacket() {
	clearOutputBuffer();

	if(executeCommand() == false) {
		// command arguments or command were wrong somehow.
		transmissionNAK();
		return false;
	}

	transmissionACK();
	putPacket(mForceNextPacket);
	mForceNextPacket = false;

	return true;
}

#endif	//GDB_DEBUG
