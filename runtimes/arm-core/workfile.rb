#!/usr/bin/ruby

require File.expand_path('../../rules/host.rb')
require File.expand_path('../../rules/mosync_util.rb')
require File.expand_path('../../rules/dll.rb')

work = DllWork.new
work.instance_eval do
	@SOURCES = ['.']
	@EXTRA_INCLUDES = ['.']
	@EXTRA_CFLAGS = ' -Wno-shadow -Wno-empty-body -Wno-float-equal'
	@SPECIFIC_CFLAGS = {
		'iwmmxt.c' => ' -Wno-sign-compare',
	}
	@NAME = 'armcore'
end

work.invoke
