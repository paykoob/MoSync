#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')

work = PipeExeWork.new
work.instance_eval do
	@SOURCES = []
	@EXTRA_SOURCEFILES = ['linpack.c']
	@EXTRA_CFLAGS = ' -Wno-float-equal -Wno-unreachable-code'
	@NAME = 'linpack'
end

work.invoke
