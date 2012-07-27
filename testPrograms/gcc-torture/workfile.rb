#!/usr/bin/ruby

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')
require 'fileutils'
require 'settings.rb'
require 'skipped.rb'

BASE = SETTINGS[:base_path]

SP = Struct.new('SourcePath', :base, :path)

SETTINGS[:source_paths] = [
	#SP.new('c-c++-common', BASE + 'c-c++-common/'),
	SP.new('ieee/', BASE + 'gcc.c-torture/execute/ieee'),
	SP.new('compat/', BASE + 'gcc.c-torture/compat'),
	SP.new('', BASE + 'gcc.c-torture/execute'),
]


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

NEWLIB_NEEDS_HEAP = [
'920501-8.c',
'930513-1.c',
'fprintf-1.c',
'fprintf-chk-1.c',
'struct-ret-1.c',
'vfprintf-1.c',
'vfprintf-chk-1.c',
]

if(USE_NEWLIB)
	NEWLIB_NEEDS_HEAP.each do |n|
		NEEDS_HEAP << n
	end
end

class TTWork < PipeExeWork
	def initialize(f, name)
		super()
		@sourcefile = f
		@BUILDDIR_PREFIX = String.new(f.base)
		@EXTRA_INCLUDES = ['.'] if(!USE_NEWLIB)
		@EXTRA_SOURCEFILES = [
			"#{f.path}/#{name}",
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
		@EXTRA_EMUFLAGS = ' -noscreen -allowdivzero'
		@NAME = name
	end
	def define_cflags
		#puts 'define_cflags'
		include_dirs = @EXTRA_INCLUDES
		include_flags = include_dirs.collect {|dir| " -I \""+File.expand_path_fix(dir)+'"'}.join
		flags = ' -g -w'
		flags << ' -O2 -fomit-frame-pointer' if(CONFIG == "")
		flags << ' -ffloat-store -fno-inline' if(@sourcefile.base == 'ieee/')
		flags << include_flags
		@CFLAGS = flags
		@CPPFLAGS = flags

		@TARGET_PATH = @BUILDDIR + @NAME.ext('.moo')
	end
	def builddir; @BUILDDIR; end
end

SourceFile = Struct.new('SourceFile', :base, :path, :filename)

files = []
SETTINGS[:source_paths].each do |sp|
	pattern = sp.path + '/*.c'
	pattern.gsub!("\\", '/')
	puts pattern
	Dir.glob(pattern).sort.collect do |fn|
		files << SourceFile.new(sp.base, sp.path, fn)
	end
end
puts "#{files.count} files to test:"

builddir = nil
oldBase = nil

files.each do |f|
	filename = f.filename
	bn = File.basename(filename)
	if(SKIPPED.include?(bn))
		#puts "Skipped #{bn}"
		next
	end
	#puts bn

	builddir = nil if(f.base != oldBase)

	if(!builddir)
		work = TTWork.new(f, bn)
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

	if(SETTINGS[:strict_prerequisites])
		if(!work)
			work = TTWork.new(f, bn)
		end
		work.invoke
	end

	winTask = FileTask.new(work, winFile)
	winTask.prerequisites << FileTask.new(work, pfn)
	if(!winTask.needed?(false))
		#puts "#{bn} won"
		next
	end

	if(!work)
		work = TTWork.new(f, bn)
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
		raise if(SETTINGS[:stop_on_fail])
	end
end
