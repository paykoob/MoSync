#!/usr/bin/ruby

require File.expand_path('../../rules/mosync_lib.rb')

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
