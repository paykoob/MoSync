#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = ['.', 'GUI', 'Application']
	#@EXTRA_CPPFLAGS = " -Wno-shadow"
	@LIBRARIES = ['mautil', 'maui', 'Facebook', 'yajl', 'nativeui']
	@EXTRA_LINKFLAGS = ' -heapsize 386 -stacksize 64'
	@NAME = 'FacebookDemo'
end

work.invoke
