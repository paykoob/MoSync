#!/usr/bin/ruby

require 'FileUtils'
require './settings.rb'
require './skipped.rb'
require '../../rules/util.rb'
require '../../rules/config.rb'

BUILD_DIR = 'build'
MOSYNCDIR = ENV['MOSYNCDIR']
GCC_FLAGS = " -I. -I#{MOSYNCDIR}/include -DNO_TRAMPOLINES"
PIPE_FLAGS = " -datasize=#{2*1024*1024} -stacksize=#{512*1024} -heapsize=#{1024*1024}"
PIPE_LIBS = " #{MOSYNCDIR}/lib/pipe_debug/mastd.lib"

# SETTINGS[:source_path] - directory in which source files are stored.

# use Work to build the tests?
# build them directly with command-line gcc & pipe-tool. no dependency or flags handling this way, but it would be faster.

# compile all tests

FileUtils.mkdir_p(BUILD_DIR)

if(SETTINGS[:arm])
else
	sh "#{MOSYNCDIR}/bin/xgcc -g -S helpers/helpers.c -o build/helpers.s#{GCC_FLAGS}"
end

pattern = SETTINGS[:source_path] + '/*.c'
pattern.gsub!("\\", '/')
puts pattern
files = Dir.glob(pattern)
puts "#{files.count} files to test:"

def delete_if_empty(filename)
	if(!File.size?(filename))
		if(File.exists?(filename))
			FileUtils.rm(filename)
		end
	end
end

def link_and_test(ofn, dead_code, force_rebuild)
	suffix = dead_code ? 'e' : ''
	pfn = ofn.ext('.moo' + suffix)
	winFile = ofn.ext('.win' + suffix)
	failFile = ofn.ext('.fail' + suffix)
	logFile = ofn.ext('.log' + suffix)
	mdsFile = ofn.ext('.md.s')
	esFile = ofn.ext('.e.s')
	sldFile = ofn.ext('.sld' + suffix)
	stabsFile = ofn.ext('.stabs' + suffix)

	delete_if_empty(pfn)

	# link
	if(SETTINGS[:arm])
	else
		if(!File.exists?(pfn) || force_rebuild)
			if(dead_code)
				sh "pipe-tool#{PIPE_FLAGS} -elim -master-dump -B #{pfn} #{ofn} build/helpers.s#{PIPE_LIBS}"
				sh "pipe-tool#{PIPE_FLAGS} -sld=#{sldFile} -B #{pfn} rebuild.s"
			else
				sh "pipe-tool -master-dump -sld=#{sldFile} -stabs=#{stabsFile}#{PIPE_FLAGS} -B #{pfn} #{ofn} build/helpers.s#{PIPE_LIBS}"
			end
		end
		force_rebuild = true
		delete_if_empty(pfn)
		if(!File.exists?(pfn))
			error"Unknown link failure."
		end
	end

	# execute it, if not win already, or we rebuilt something.

	if((File.exists?(winFile) || !SETTINGS[:retry_failed]) && !force_rebuild)
		return force_rebuild
	end
	if(SETTINGS[:arm])
		cmd = "#{MOSYNCDIR}/bin/more -noscreen -arm -program #{ofn}"
	else
		cmd = "#{MOSYNCDIR}/bin/more -noscreen -program #{pfn} -sld #{sldFile}"
	end
	$stderr.puts cmd
	res = system(cmd)
	puts res
	if(res == true)	# success
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
		FileUtils.rm_f(logFile)
		FileUtils.rm_f(mdsFile)
		FileUtils.rm_f(esFile)
		FileUtils.rm_f(sldFile)
		FileUtils.rm_f(stabsFile)
	else	# failure
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		FileUtils.mv('log.txt', logFile) if(File.exists?('log.txt'))
		FileUtils.mv('_masterdump.s', mdsFile) if(File.exists?('_masterdump.s'))
		FileUtils.mv('rebuild.s', esFile) if(File.exists?('rebuild.s'))
		if(SETTINGS[:stop_on_fail])
			error "Stop on fail"
		end
	end
	return force_rebuild
end

BROKEN_ONES = [
'20090711-1.c',
'20111208-1.c',
'921110-1.c',
]

files.each do |filename|
	bn = File.basename(filename)
	if(SKIPPED.include?(bn))
		#puts "Skipped #{bn}"
		next
	end
	#puts bn

	ofn = BUILD_DIR + '/' + bn.ext('.s')
	force_rebuild |= SETTINGS[:rebuild_failed] && (File.exists?(ofn.ext('.fail')) || File.exists?(ofn.ext('.faile')))

	# compile
	if(!File.exists?(ofn) || force_rebuild)
		if(SETTINGS[:arm])
			cmd = "#{ARM_DRIVER_NAME} -g \"#{filename}\" -Ic:/mosync/include/newlib -DMAPIP"+
				" C:/mosync/lib/newlib_debug_arm-4.6.3/rescompiler.a"+
				" C:/mosync/lib/newlib_debug_arm-4.6.3/newlib.a"+
				" -nodefaultlibs -mfloat-abi=soft c:/mosync/arm-gcc/lib/gcc/arm-elf/4.6.3/libgcc.a"+
				" arm/default.c"+
				" -o #{ofn}"
			if(BROKEN_ONES.include?(bn))
				cmd << ' -include arm/sparse.h'
			else
				cmd << ' -include arm/default.h'
			end
			cmd << ' -save-temps' if(SETTINGS[:save_temps])
			sh cmd
		else
			if(bn == 'conversion.c')
				# avoid testing long longs, as they are not yet properly supported by MoSync.
				extra_flags = ' -U __GNUC__'
			end
			sh "#{MOSYNCDIR}/bin/xgcc -g -S \"#{filename}\" -o #{ofn}#{GCC_FLAGS}#{extra_flags}"
		end
		force_rebuild = true
	end

	force_rebuild = link_and_test(ofn, false, force_rebuild)
	link_and_test(ofn, true, force_rebuild) if(SETTINGS[:test_dce])
end
