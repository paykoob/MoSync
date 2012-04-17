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

#include <MAUtil/Moblet.h>
#include <conprint.h>

using namespace MAUtil;

static void dumpInet4Addr(MAHandle h) {
	MAConnAddr addr;
	int res = maConnGetAddr(h, &addr);
	printf("maConnGetAddr res: %i\n", res);
	printf("family: %i\n", addr.family);
	int b = addr.inet4.addr;
	printf("%i.%i.%i.%i:%i\n",
		(b >> 24) & 0xff,
		(b >> 16) & 0xff,
		(b >> 8) & 0xff,
		(b) & 0xff,
		addr.inet4.port);
	maConnClose(h);
}

class MyMoblet : public Moblet, ConnListener {
public:
	MyMoblet() {
		printf("Hello World!\n");
		MAConnAddr addr;
		addr.family = CONN_FAMILY_BT;
		int res = maConnGetAddr(HANDLE_LOCAL, &addr);
		printf("maConnGetAddr res: %i\n", res);
		byte* a = addr.bt.addr.a;
		printf("%02x%02x%02x%02x%02x%02x\n", a[0], a[1], a[2], a[3], a[4], a[5]);

		MAHandle h = maConnect("socket://");
		printf("socket server: %i\n", h);
		if(h > 0) {
			dumpInet4Addr(h);
		}

		h = maConnect("socket://modev.mine.nu:5001");
		printf("socket conn: %i\n", h);
		if(h > 0) {
			setConnListener(h, this);
		}
	}

	void connEvent(const MAConnEventData& data) {
		printf("conn result: %i\n", data.result);
		if(data.result >= 0) {
			dumpInet4Addr(data.handle);
		}
	}

	void keyPressEvent(int keyCode) {
		if(keyCode == MAK_0)
			close();
	}
};

extern "C" int MAMain() {
	Moblet::run(new MyMoblet());
	return 0;
};
