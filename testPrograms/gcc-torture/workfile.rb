#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')
require 'fileutils'
require 'settings.rb'
require 'skipped.rb'

class TTWork < PipeExeWork
	def initialize(name)
		super()
		@EXTRA_INCLUDES = ['.']
		@EXTRA_SOURCEFILES = [
			"#{SETTINGS[:source_path]}/#{name}",
			'helpers/helpers.c',
		]
		@SPECIFIC_CFLAGS = {
			# avoid testing long longs, as they are not yet properly supported by MoSync.
			'conversion.c' => ' -U __GNUC__',
		}
		@EXTRA_EMUFLAGS = ' -noscreen'
		@NAME = name
	end
	def define_cflags
		#puts 'define_cflags'
		include_dirs = @EXTRA_INCLUDES
		include_flags = include_dirs.collect {|dir| " -I \""+File.expand_path_fix(dir)+'"'}.join
		flags = ' -g -w'
		flags << include_flags
		@CFLAGS = flags
		@CPPFLAGS = flags

		@TARGET_PATH = @BUILDDIR + @NAME.ext('.moo')
	end
	def builddir; @BUILDDIR; end
end

pattern = SETTINGS[:source_path] + '/*.c'
pattern.gsub!("\\", '/')
puts pattern
files = Dir.glob(pattern).sort
puts "#{files.count} files to test:"

files.each do |filename|
	bn = File.basename(filename)
	if(SKIPPED.include?(bn))
		puts "Skipped #{bn}"
		next
	end
	puts bn

	work = TTWork.new(bn)
	work.invoke

	ofn = work.builddir + bn.ext('.o')
	suffix = ''
	pfn = ofn.ext('.moo' + suffix)
	winFile = ofn.ext('.win' + suffix)
	failFile = ofn.ext('.fail' + suffix)
	logFile = ofn.ext('.log' + suffix)
	sldFile = ofn.ext('.sld' + suffix)
	force_rebuild = SETTINGS[:rebuild_failed] && File.exists?(failFile)

	if(force_rebuild)
		FileUtils.rm_f(ofn)
		FileUtils.rm_f(winFile)
		work.invoke
	end

	winTask = FileTask.new(work, winFile)
	winTask.prerequisites << FileTask.new(work, ofn)
	if(!winTask.needed?)
		puts "#{bn} won"
		next
	end

	begin
		work.run
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
	rescue
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		raise
	end
end
