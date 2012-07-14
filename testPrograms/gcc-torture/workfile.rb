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
'ipa-sra-2.c',
'pr15262-1.c',
'pr36765.c',
'pr41395-1.c',
'pr41395-2.c',
'pr41463.c',
'pr42614.c',
'pr43008.c',
'printf-1.c',
'printf-chk-1.c',
'va-arg-21.c',
'vprintf-1.c',
'vprintf-chk-1.c',
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
			# longlong to float conversion is not yet supported.
			'conversion.c' => ' -U __GNUC__',

			'fprintf-chk-1.c' => ' -ffreestanding',
			'vfprintf-chk-1.c' => ' -ffreestanding',
			'pr42833.c' => ' -D__INT_LEAST8_TYPE__=char -D__UINT_LEAST32_TYPE__=unsigned',
			'pr22493-1.c' => ' -fwrapv',
			'pr23047.c' => ' -fwrapv',
		}
		@EXTRA_EMUFLAGS = ' -noscreen'
		@NAME = name
	end
	def define_cflags
		#puts 'define_cflags'
		include_dirs = @EXTRA_INCLUDES
		include_flags = include_dirs.collect {|dir| " -I \""+File.expand_path_fix(dir)+'"'}.join
		flags = ' -g -w'
		flags << ' -O2 -fomit-frame-pointer' if(CONFIG == "")
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

builddir = nil

files.each do |filename|
	bn = File.basename(filename)
	if(SKIPPED.include?(bn))
		#puts "Skipped #{bn}"
		next
	end
	#puts bn

	if(!builddir)
		work = TTWork.new(bn)
		work.invoke
		builddir = work.builddir
	end

	ofn = builddir + bn.ext('.o')
	suffix = ''
	pfn = ofn.ext('.moo' + suffix)
	winFile = ofn.ext('.win' + suffix)
	failFile = ofn.ext('.fail' + suffix)
	logFile = ofn.ext('.log' + suffix)
	sldFile = ofn.ext('.sld' + suffix)
	force_rebuild = SETTINGS[:rebuild_failed] && File.exists?(failFile)

	winTask = FileTask.new(work, winFile)
	winTask.prerequisites << FileTask.new(work, pfn)
	if(!winTask.needed?(false))
		#puts "#{bn} won"
		next
	end

	if(!work)
		work = TTWork.new(bn)
	end

	if(force_rebuild)
		FileUtils.rm_f(ofn)
		FileUtils.rm_f(winFile)
	end

	begin
		FileUtils.rm_f(winFile)
		FileUtils.rm_f(failFile)
		work.invoke
		work.run
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
	rescue
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		raise
	end
end
