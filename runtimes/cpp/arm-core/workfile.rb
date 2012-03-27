#!/usr/bin/ruby

require File.expand_path('../../../rules/host.rb')
require File.expand_path('../../../rules/mosync_util.rb')
require File.expand_path('../../../rules/dll.rb')

work = DllWork.new
work.instance_eval do
	@SOURCES = ['.']
	@EXTRA_INCLUDES = ['.']
	@EXTRA_CFLAGS = ' -Wno-shadow -Wno-empty-body -Wno-float-equal'
	@SPECIFIC_CFLAGS = {
		'iwmmxt.c' => ' -Wno-sign-compare',
	}
	# buggy compiler
	if(@GCC_IS_V4 && @GCC_V4_SUB == 6 && CONFIG == 'release')
		#@SPECIFIC_CFLAGS['armemu32.c'] = ' -Wno-uninitialized'
	end

	@NAME = 'armcore'
	@INSTALLDIR = mosyncdir + '/bin'
end

work.invoke
