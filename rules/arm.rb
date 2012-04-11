
module MoSyncArmGccMod
	def gcc; ARM_DRIVER_NAME; end
	def host_cppflags; '' end
	def host_flags
		flags = ''
		flags << ' -fno-exceptions -mfloat-abi=soft'
		flags << ' -DUSE_NEWLIB' if(USE_NEWLIB)
		#flags << ' -D_POSIX_SOURCE'	#avoid silly bsd functions
		return flags
	end
	def set_defaults
		default(:BUILDDIR_PREFIX, "")
		default(:COMMOM_BUILDDDIR_PREFIX, "")
		if(USE_NEWLIB)
			@BUILDDIR_PREFIX << "newlib_"
			@COMMOM_BUILDDDIR_PREFIX << "newlib_"
		end
		super
	end
end
