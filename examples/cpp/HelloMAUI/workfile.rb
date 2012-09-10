#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = ["."]
	@LIBRARIES = ["mautil", "maui"]
	@EXTRA_LINKFLAGS = " -heapsize 128 -stacksize 16" unless(USE_NEWLIB)
	@NAME = "HelloMAUI"
end

work.invoke
