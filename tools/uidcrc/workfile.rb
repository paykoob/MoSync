#!/usr/bin/ruby

require File.expand_path('../../rules/exe.rb')
require File.expand_path('../../rules/mosync_util.rb')

work = ExeWork.new
work.instance_eval do
	@SOURCES = ["."]
	@EXTRA_INCLUDES = ["./inc"]
	@NAME = "uidcrc"
	@INSTALLDIR = mosyncdir + '/bin'
	setup
end

work.invoke
