#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = ['src', 'src/endian']
	@EXTRA_INCLUDES = ['inc', '.']
	@LSTFILES = ['resources.lst']
	#@EXTRA_CPPFLAGS = ' -DBENCHMARK'
	@EXTRA_LINKFLAGS = ' -heapsize 386 -stacksize 16'
	@NAME = 'QuakeMDL'
end

work.invoke
