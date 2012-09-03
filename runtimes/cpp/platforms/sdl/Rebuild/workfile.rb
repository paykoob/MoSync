#!/usr/bin/ruby

require File.expand_path('../shared_work.rb')
require File.expand_path('../../../../../rules/mosync_util.rb')

work = MoSyncExe.new
class << work
	include SdlCommon
end
work.instance_eval do
	setup_common

	BD = '../../../../..'
	@SOURCES = []
	@EXTRA_SOURCEFILES = [
		'rebuilt.cpp',
		"#{BD}/intlibs/helpers/intutil.cpp",
		'../mosynclib/main.cpp',
		]
	if(defined?(REBUILD_CPP))
		@EXTRA_SOURCEFILES << REBUILD_CPP
		@TARGETDIR = File.dirname(REBUILD_CPP)

		def run
			Dir.chdir(@TARGETDIR)
			cmd = @TARGET.to_s
			cmd << " -resource #{RESOURCE}" if(defined?(RESOURCE))
			sh cmd
		end

	else
		@EXTRA_SOURCEFILES << 'rebuild.build.cpp'
	end
	@EXTRA_INCLUDES += ['.', '../../..']
	@EXTRA_CPPFLAGS = ' -D_USE_REBUILDER_'
	@SPECIFIC_CFLAGS = {
		'rebuild.build.cpp' => " -I \"#{mosyncdir}/include-rebuild\""+
			' -Wno-unused-label -Wno-unused-but-set-variable -Wno-return-type -Wno-unused-function'+
			' -Wno-error=uninitialized'+
			' -Wno-unused-parameter -Wno-float-equal -fno-exceptions -Wno-error=suggest-attribute=noreturn'+
			'',
	}
	if(HOST == :win32)
		@EXTRA_LINKFLAGS = ' -mwindows'
	end

	@LOCAL_LIBS = ["mosync_sdl"] + @LOCAL_LIBS

	@NAME = "Rebuild"

	setup
end

work.invoke
