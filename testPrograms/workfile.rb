#!/usr/bin/ruby

raise "usage: workfile <name> [options]" unless(ARGV[0])

# fetch name of program to build.
name = ARGV[0]
# normalize ARGV, so the Work can parse it.
ARGV.delete_at(0)

require File.expand_path('../rules/mosync_exe.rb')
require File.expand_path('../rules/util.rb')

default_const(:USE_ARM, false)

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = [name]
	@EXTRA_SOURCEFILES = [name]
	@NAME = name
	@EXTRA_INCLUDES = ['.']
	#@EXTRA_CFLAGS = ' -save-temps'
	@LIBRARIES = ['mautil', 'mtxml', 'sdl']
	@EXTRA_LINKFLAGS = ' -datasize=1024000 -heapsize=386000 -stacksize=64000' if(!USE_ARM)
	@PACK_PARAMETERS = ' --s60v3uid E1234512 --debug --permissions "Internet Access"'
end

work.invoke
