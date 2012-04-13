#!/usr/bin/ruby

require File.expand_path('../../rules/mosync_lib.rb')

#src/audio/alsa src/audio/arts src/audio/baudio src/audio/bsd src/audio/dart src/audio/dc src/audio/dma src/audio/dmedia src/audio/dsp src/audio/esd src/audio/macosx src/audio/macrom src/audio/mint src/audio/mme src/audio/nas src/audio/nds src/audio/nto src/audio/paudio src/audio/pulse src/audio/sun src/audio/symbian src/audio/ums src/audio/windib src/audio/windx5 src/cdrom/aix src/cdrom/beos src/cdrom/bsdi src/cdrom/dc src/cdrom/freebsd src/cdrom/linux src/cdrom/macos src/cdrom/macosx src/cdrom/mint src/cdrom/openbsd src/cdrom/os2 src/cdrom/osf src/cdrom/qnx src/cdrom/win32 src/hermes src/loadso/beos src/loadso/dlopen src/loadso/macos src/loadso/macosx src/loadso/mint src/loadso/os2 src/loadso/win32 src/main/beos src/main/macos src/main/macosx src/main/qtopia src/main/symbian src/main/win32 src/thread/beos src/thread/dc src/thread/irix src/thread/os2 src/thread/pth src/thread/pthread src/thread/riscos src/thread/symbian src/thread/win32 src/timer/beos src/timer/dc src/timer/macos src/timer/mint src/timer/nds src/timer/os2 src/timer/riscos src/timer/symbian src/timer/unix src/timer/win32 src/timer/wince src/video/aalib src/video/ataricommon src/video/bwindow src/video/caca src/video/dc src/video/dga src/video/directfb src/video/fbcon src/video/gapi src/video/gem src/video/ggi src/video/ipod src/video/maccommon src/video/macdsp src/video/macrom src/video/nanox src/video/nds src/video/os2fslib src/video/photon src/video/picogui src/video/ps2gs src/video/ps3 src/video/qtopia src/video/quartz src/video/riscos src/video/svga src/video/symbian src/video/vgl src/video/wincommon src/video/windib src/video/windx5 src/video/wscons src/video/x11 src/video/xbios src/video/Xext src/joystick/beos src/joystick/bsd src/joystick/darwin src/joystick/dc src/joystick/linux src/joystick/macos src/joystick/mint src/joystick/nds src/joystick/os2 src/joystick/riscos src/joystick/win32 src/video/dummy test/checkkeys.c test/graywin.c test/loopwave.c test/testbitmap.c test/testblitspeed.c test/testcdrom.c test/testcursor.c test/testdyngl.c test/testerror.c test/testfile.c test/testgl.c test/testhread.c test/testiconv.c test/testjoystick.c test/testkeys.c test/testloadso.c test/testlock.c test/testoverlay.c test/testoverlay2.c test/testpalette.c test/testplatform.c test/testsem.c test/testsprite.c test/testtimer.c test/testver.c test/testvidinfo.c test/testwin.c test/threadwin.c test/torturethread.c test/testgamma.c test/testalpha.c test/testwm.c main.c unimplemented src/audio/dummy +src/timer/dummy/SDL_systimer.c src/timer/dummy
mod = Module.new
mod.class_eval do
	def setup_pipe
		@SOURCES = ['src', 'src/thread/generic'] + Dir['src/*/'] + Dir['src/*/mosync/']
		@SOURCES << 'src/stdlib' if(!USE_NEWLIB)

		@HEADER_DIRS = ['include']
		@EXTRA_INCLUDES = ['include']
		@EXTRA_INCLUDES << 'src/stdlib' if(!USE_NEWLIB)
		@EXTRA_CFLAGS = ' -Wno-undef -Wno-shadow'
		@EXTRA_CFLAGS << ' -Wno-unreachable-code' if(!@GCC_IS_V4)
		@SPECIFIC_CFLAGS = {
			'SDL_iconv.c' => ' -Wno-sign-compare',
			'SDL_systimer.c' => ' -Wno-missing-prototypes -Wno-missing-declarations',
			'SDL_joystick.c' => ' -Wno-missing-prototypes -Wno-missing-declarations',
			'SDL_cdrom.c' => ' -Wno-missing-prototypes -Wno-missing-declarations',
			'SDL_fatal.c' => ' -Wno-missing-prototypes -Wno-missing-declarations',
			'SDL_gamma.c' => ' -Wno-float-equal',
			'SDL_bmp.c' => ' -Wno-unused-but-set-variable',
			'SDL_blit_N.c' => ' -Wno-sign-compare',
		}
		@IGNORED_FILES = [
			'SDL_joystick.c',
			'SDL_cdrom.c',
			'SDL_.c',
		]

		@INSTALL_INCDIR = 'SDL'
		@NAME = 'sdl'
	end
end

MoSyncLib.invoke(mod)
