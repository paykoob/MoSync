#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')
require 'fileutils'
require 'settings.rb'
require 'skipped.rb'

NEEDS_HEAP = [
'20000914-1.c',
'20020406-1.c',
'20000914-1.c',
'20001203-2.c',
'20020406-1.c',
'20051113-1.c',
'20071018-1.c',
'20071120-1.c',
'920810-1.c',
'941014-2.c',
'960521-1.c',
'990628-1.c',
'comp-goto-1.c',
'pr15262-1.c',
'pr36765.c',
'pr41395-1.c',
'pr41395-2.c',
'pr41463.c',
'pr42614.c',
'pr43008.c',
'va-arg-21.c',
]

class TTWork < PipeExeWork
	def initialize(name)
		super()
		@EXTRA_INCLUDES = ['.']
		@EXTRA_SOURCEFILES = [
			"#{SETTINGS[:source_path]}/#{name}",
			'helpers/helpers.c',
		]
		@EXTRA_SOURCEFILES << 'helpers/override_heap.c' unless(NEEDS_HEAP.include?(name))
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
		#puts "Skipped #{bn}"
		next
	end
	#puts bn

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
	winTask.prerequisites << FileTask.new(work, pfn)
	if(!winTask.needed?)
		#puts "#{bn} won"
		next
	end

	begin
		FileUtils.rm_f(winFile)
		FileUtils.rm_f(failFile)
		work.run
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
	rescue
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		raise
	end
end
