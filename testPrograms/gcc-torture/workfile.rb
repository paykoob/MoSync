#!/usr/bin/ruby

require './settings.rb'

EXIT_ON_ERROR = false if(!SETTINGS[:stop_on_fail])

target = nil
if(ARGV.size > 0 && ARGV[0].end_with?('.c'))
	target = ARGV[0]
	ARGV.delete_at(0)
end

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')
require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_lib.rb')
require 'fileutils'
require './skipped.rb'
require './dejaGnu.rb'

if(target)
	puts "Target: #{target}"
	SETTINGS[:strict_prerequisites] = true
end

BASE = SETTINGS[:base_path]

SP = Struct.new('SourcePath', :base, :path, :mode, :defaultMode)

def dg(name, defaultMode)
	return SP.new(name + '/', BASE + name, :dejaGnu, defaultMode)
end

def dgSub(name, defaultMode)
	array = []
	#puts "Scanning #{BASE + name}..."
	Dir.foreach(BASE + name).each do |file|
		subPath = name + '/' + file
		path = BASE + subPath
		if(File.directory?(path) &&
			!file.start_with?('.') &&
			!SKIPPED_DIRS.include?(subPath))
			array << path
		end
	end
	return [dg(name, defaultMode)] +
		array.sort.collect do |dir|
		sp(dir.slice(BASE.length..-1)+'/', dir, :dejaGnu, defaultMode)
	end
end

def sp(base, path, mode = :run, defaultMode = :run)
	return SP.new(base, path, mode)
end

# allowed modes: run, compile, dejaGnu (parse the source file to find compile or run).
SETTINGS[:source_paths] =
[
	sp('ieee/', BASE + 'gcc.c-torture/execute/ieee'),
	sp('', BASE + 'gcc.c-torture/execute'),
	#dg('c-c++-common/dfp'),	# decimal floating point is not supported.
	dg('c-c++-common/torture', :run),
	dg('c-c++-common', :run),
	sp('compile/', BASE + 'gcc.c-torture/compile', :compile),
	sp('unsorted/', BASE + 'gcc.c-torture/unsorted', :compile),
	sp('compat/', BASE + 'gcc.c-torture/compat'),
]+
	dgSub('gcc.dg', :compile)+
	#dgSub('g++.dg', :run)+
	#dgSub('g++.old-deja', :run)+
[]

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
'arraynew.C',
'vbase1.C',
'20050527-1.c',
'alias-11.c',
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

class Libdir
	def self.set(dir)
		#p dir
		@libdir = dir
	end

	def self.get()
		#p @libdir
		@libdir
	end
end

mod = Module.new
mod.class_eval do
	def setup_pipe
		@EXTRA_SOURCEFILES = [
			'helpers/helpers.c',
		]
		@EXTRA_INCLUDES = ['.'] if(!USE_NEWLIB)
		@NAME = 'helpers'
		Libdir.set(@COMMON_BUILDDIR)
	end
	def mosync_libdir; 'build'; end
	def copyHeaders; end
end
MoSyncLib.invoke(mod)

mod = Module.new
mod.class_eval do
	def setup_pipe
		@EXTRA_SOURCEFILES = [
			'helpers/override_heap.c',
		]
		@NAME = 'override_heap'
	end
	def mosync_libdir; 'build'; end
	def copyHeaders; end
end
MoSyncLib.invoke(mod)

class TTWork < PipeExeWork
	def initialize(f, name)
		super()
		@sourcepath = "#{f.sourcePath.path}/#{name}"
		@sourcefile = f
		@BUILDDIR_PREFIX = String.new(f.sourcePath.base)
		@BUILDDIR_PREFIX << 'rebuild_' if(defined?(MODE) && MODE == 'rebuild')
		@EXTRA_INCLUDES = ['.'] if(!USE_NEWLIB)
		@EXTRA_INCLUDES = [mosync_include + '/stlport'] if(USE_NEWLIB)
		@EXTRA_SOURCEFILES = [
			@sourcepath,
		]

		@EXTRA_OBJECTS = [
			FileTask.new(self, Libdir.get()+'helpers.o'),
		]
		unless(NEEDS_HEAP.include?(name) || f.sourcePath.mode == :dejaGnu)
			@EXTRA_OBJECTS << FileTask.new(self, Libdir.get()+'override_heap.o')
		end

		@SPECIFIC_CFLAGS = {
			# longlong to float conversion is not yet supported.
			'conversion.c' => ' -U __GNUC__',

			'fprintf-chk-1.c' => ' -ffreestanding',
			'vfprintf-chk-1.c' => ' -ffreestanding',
			'pr42833.c' => ' -D__INT_LEAST8_TYPE__=char -D__UINT_LEAST32_TYPE__=unsigned',
			'pr22493-1.c' => ' -fwrapv',
			'pr23047.c' => ' -fwrapv',
			'rbug.c' => ' -D__SPU__',
			'pr47141.c' => ' -D__UINTPTR_TYPE__=unsigned',
			'struct-layout-1_main.c' => ' -DSKIP_DECIMAL_FLOAT',
			'struct-layout-1_x.c' => ' -DSKIP_DECIMAL_FLOAT',
			'struct-layout-1_y.c' => ' -DSKIP_DECIMAL_FLOAT',
		}
		@EXTRA_LINKFLAGS = standardMemorySettings(16)
		@EXTRA_EMUFLAGS = ' -noscreen -allowdivzero'
		@NAME = name
	end
	def rebuildArgs
		# warning: this flag can expose or conceal bugs.
		return ['CONFIG=debug']
	end
	def define_cflags
		#puts 'define_cflags'
		include_dirs = @EXTRA_INCLUDES
		include_flags = include_dirs.collect {|dir| " -I \""+File.expand_path_fix(dir)+'"'}.join
		flags = ' -g -w -DSIGNAL_SUPPRESS'
		flags << ' -DNO_TRAMPOLINES -DNO_LABEL_VALUES'
		flags << ' -O2 -fomit-frame-pointer' if(CONFIG == "")
		flags << ' -ffloat-store -fno-inline' if(@sourcefile.sourcePath.base == 'ieee/')
		flags << include_flags
		@CFLAGS = flags + @EXTRA_CFLAGS
		@CPPFLAGS = flags + @EXTRA_CPPFLAGS

		@TARGET_PATH = @BUILDDIR + @NAME.ext('.moo')
	end
	def builddir; @BUILDDIR; end
	def compile
		setup if(!@CFLAGS)
		FileUtils.mkdir_p(@BUILDDIR)
		makeGccTask(FileTask.new(self, @sourcepath), '.o').invoke
	end
	def setMode
		return if(@mode)
		if(@sourcefile.sourcePath.mode == :dejaGnu)
			@EXTRA_CFLAGS = ''
			@mode = @sourcefile.sourcePath.defaultMode
			parseDejaGnu
			#puts "Mode #{@mode} for #{@NAME}"
		else
			@mode = @sourcefile.sourcePath.mode
		end
	end
	def shouldRun
		setMode
		return @mode == :run
	end
	def invoke
		begin
			setMode
			if(@mode == :run || @mode == :link)
				super
			elsif(@mode == :compile)
				compile
			elsif(@mode == :skip)
				puts "Skipped #{@sourcepath}"
				return
			elsif(@mode == :preprocess)
				@EXTRA_CFLAGS << ' -E'
				compile
			else
				raise "Unknown mode in #{@sourcepath}: #{@mode.inspect}"
			end
		rescue => e
			puts "#{@sourcepath}:#{@lineNum}:"
			p e
			exit(1) if(EXIT_ON_ERROR)
			raise
		end
	end

	include DejaGnu

	# scans a C file for { dg-do [compile|run] }
	def parseMode
		open(@sourcepath) do |file|
			file.each do |line|
				lineStrip = line.strip
				SKIP_LINES.each do |sl|
					if(lineStrip == sl)
						@mode = :skip
						return
					end
				end

				dgdo = '{ dg-do '
				ts = '{ target '
				xfails = '{ xfail '
				i = line.index(dgdo)
				e = nil
				if(i)
					e = ti = line.index(ts)
					if(ti)
						ti += ts.length
						te = line.index(' }', ti)
						raise "Bad dg-do line!" if(!te)
						target = line.slice(ti, te-ti).strip
						if(target == 'inttypes_types' ||
							target == 'fpic' ||
							target == 'native' ||
							target == '{ int32plus' ||
							target == 'c99_runtime' ||
							false)
							# do nothing
						elsif(target.start_with?('{ {') ||
							target == 'alpha*-*-* cris-*-* crisv32-*-* i?86-*-* mmix-*-* powerpc*-*-* rs6000-*-* x86_64-*-*' ||
							target == 'hppa*-*-*' ||
							target == '{ *-*-darwin*' ||
							target == 'm68k-*-* fido-*-* sparc-*-*' ||
							target == 'ia64-*-* i?86-*-* x86_64-*-*' ||
							target == 'arm*-*-pe*' ||
							target.start_with?('i?86-*-*') ||
							target.include?('pcc_bitfield_type_matters') ||
							target.start_with?('i?86-*-linux') ||
							target.start_with?('*-*-linux*') ||
							target.start_with?('powerpc*-*-*') ||
							target.start_with?('sh-*-*') ||
							target.start_with?('*-*-interix*') ||
							target.start_with?('*-*-solaris') ||
							target.start_with?('hppa*-*-hpux*') ||
							target.start_with?('*-*-darwin') ||
							false)
							@mode = :skip
							return
						else
							$stderr.puts "#{@sourcepath}:1: Unknown target: #{target}"
							exit(1)
						end
					end
					if(!e)
						# bad formatting; ignore
						e = line.index('{target')
					end
					if(!e)
						xi = line.index(xfails)
						if(xi)
							e = xi-1
						else
							e = line.index(' }', i)
						end
					end
					raise "Bad dg-do line!" if(!e)
					@mode = line.slice(i+dgdo.length, e-(i+dgdo.length)).strip.to_sym
					@mode = :compile if(@mode == :assemble)
					next
				end

				dgop = '{ dg-options "'
				i = line.index(dgop)
				if(i)
					i += dgop.length
					e = line.index('"', i)
					raise "Bad dg-options line!" if(!e)
					options = line.slice(i, e-i).split

					# because we don't use collect2, -frepo will not work.
					if(options.include?('-frepo') ||
						options.include?('-fprofile-arcs') ||
						options.include?('-fno-keep-inline-dllexport') ||
						true)
						@mode = :skip
						return
					end

					# fdump options are bugged for source files that are not in cwd.
					options.delete_if do |o| o.start_with?('-fdump-') end

					options = ' '+options.join(' ')

					ti = line.index(ts, e)
					#raise "Bad dg-options line in #{@sourcepath}: #{line}" if(!ti)
					if(ti)
						ti += ts.length
						te = line.index(' }', ti)
						raise "Bad dg-options line!" if(!te)
						target = line.slice(ti, te-ti)
						#puts "Found options \"#{options}\" for target #{target}"
						if(target == 'c')
							@EXTRA_CFLAGS = options
						end
						if(target == 'c++')
							@EXTRA_CPPFLAGS = options
						end
					else
						@EXTRA_CFLAGS = options
						@EXTRA_CPPFLAGS = options
					end
					next
				end

				dgop = '{ dg-additional-sources "'
				i = line.index(dgop)
				if(i)
					i += dgop.length
					e = line.index('"', i)
					raise "Bad dg-additional-sources line!" if(!e)
					line.slice(i, e-i).split.each do |a|
						@EXTRA_SOURCEFILES << File.dirname(@sourcepath)+'/'+a
					end
					next
				end

				dgret = '{ dg-require-effective-target '
				i = line.index(dgret)
				if(i)
					i += dgret.length
					e = line.index(' }', i)
					raise "Bad dg-require-effective-target line!" if(!e)
					t = line.slice(i, e-i).strip
					if(t == 'trampolines' ||
						t == 'sync_char_short' ||
						false)
						@mode = :skip
						return
					end
				end

				if(line.index('{ dg-error ') ||
					#line.index('{ dg-bogus ') ||
					line.index('throw ') ||
					false)
					@mode = :skip
					return
				end
			end
		end
		return nil
	end
	def mode
		return @mode if(@mode)
		m = @sourcefile.sourcePath.mode
		m = :run if(!m)
		return m
	end
end

SourceFile = Struct.new('SourceFile', :sourcePath, :filename)

files = []
SETTINGS[:source_paths].each do |sp|
	pattern = sp.path + '/*.[cC]'
	pattern.gsub!("\\", '/')
	#puts pattern
	Dir.glob(pattern).sort.collect do |fn|
		files << SourceFile.new(sp, fn)
	end
end
puts "#{files.count} files to test:"

builddir = nil
oldBase = nil
targetFound = false

def matchRegexp(tName)
	SKIPPED_REGEXP.each do |r|
		return true if(r.match(tName))
	end
	return false
end

files.each do |f|
	sp = f.sourcePath
	filename = f.filename
	bn = File.basename(filename)
	#p sp, bn
	tName = sp.base + bn
	if(SKIPPED.include?(tName) || matchRegexp(tName))
		#puts "Skipped #{bn}"
		next
	end
	if(target)
		next if(target != (tName))
		targetFound = true
	end
	#puts bn

	builddir = nil if(sp.base != oldBase)

	if(!builddir)
		work = TTWork.new(f, bn)
		work.setup
		builddir = work.builddir
		oldBase = sp.base
	end

	next if(!builddir)

	ofn = builddir + bn.ext('.o')
	suffix = ''
	pfn = ofn.ext('.moo' + suffix)
	winFile = ofn.ext('.win' + suffix)
	failFile = ofn.ext('.fail' + suffix)
	logFile = ofn.ext('.log' + suffix)
	sldFile = ofn.ext('.sld' + suffix)
	force_rebuild = SETTINGS[:rebuild_failed] && File.exists?(failFile)

	next if(!SETTINGS[:retry_failed] && File.exists?(failFile))

	if(SETTINGS[:strict_prerequisites])
		if(!work)
			work = TTWork.new(f, bn)
		end
		work.invoke
	end

	if(force_rebuild)
		FileUtils.rm_f(ofn)
		FileUtils.rm_f(pfn)
		FileUtils.rm_f(winFile)
	end

	winTask = FileTask.new(work, winFile)
	#winTask.prerequisites << FileTask.new(work, ofn)
	if(!winTask.needed?(false))
		#puts "#{bn} won"
		next
	end

	if(!work)
		work = TTWork.new(f, bn)
	end

	begin
		FileUtils.mkdir_p(builddir)
		FileUtils.rm_f(winFile)
		FileUtils.touch(failFile)
		if(work.mode == :compile)
			work.compile
		else
			work.invoke
			work.run if(work.mode == :run)
		end
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
	rescue
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		raise if(SETTINGS[:stop_on_fail])
	end
end

if(target && !targetFound)
	puts "Error: target not found!"
	exit(1)
end
