#!/usr/bin/ruby

require File.expand_path('../../rules/exe.rb')
require File.expand_path('../../rules/mosync_util.rb')
require 'stringio'

require './stabdefs.rb'

class GenStabDefsH < MemoryGeneratedFileTask
	def initialize(work)
		super(work, 'build/stabdefs.h')
		io = StringIO.new
		first = true
		io.puts 'const char* stabName(unsigned char type);'
		io.puts
		io.puts '#define N_EXT 0x1'
		io.puts
		STABS.each do |s|
			if(s.name.include?('|'))
				raise hell if(!s.name.end_with?(' | N_EXT'))
				next
			end
			io.puts "#define #{s.name} #{s.type}"
		end
		@buf = io.string
	end
end

class GenStabDefsC < MemoryGeneratedFileTask
	def initialize(work)
		super(work, 'build/stabdefs.cpp')
		io = StringIO.new
		first = true
		io.puts '#include "stabdefs.h"'
		io.puts '#include <stdio.h>'
		io.puts '#include <stdlib.h>'
		io.puts
		io.puts 'const char* stabName(unsigned char type) {'
		io.puts "\tswitch(type) {"
		STABS.each do |s|
			io.puts "\t\tcase #{s.type}: return \"#{s.name}\";"
		end
		io.puts "\t\tdefault: printf(\"Unknown type 0x%02x\\n\", type); exit(1); return \"\";"
		io.puts "\t}"
		io.puts '}'
		@buf = io.string
	end
end

work = ExeWork.new
work.instance_eval do
	@SOURCES = ['.']
	@EXTRA_SOURCEFILES = [
		'../../runtimes/cpp/platforms/sdl/FileImpl.cpp',
		'../../runtimes/cpp/base/FileStream.cpp',
	]
	@PREREQUISITES = [
		GenStabDefsH.new(self),
	]
	@EXTRA_SOURCETASKS = [
		GenStabDefsC.new(self),
	]
	@EXTRA_INCLUDES = [
		'../../intlibs',
		'../../runtimes/cpp/base',
		'../../runtimes/cpp/platforms/sdl',
	]
	@LOCAL_LIBS = ['mosync_log_file']
	if(HOST == :linux)
		@LIBRARIES = ['pthread']
	end

	@NAME = 'elfStabSld'

	@INSTALLDIR = mosyncdir + '/bin'
end

work.invoke
