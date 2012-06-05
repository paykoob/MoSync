
require "#{File.dirname(__FILE__)}/util.rb"
require "#{File.dirname(__FILE__)}/mosync_util.rb"

# load local_config.rb, if it exists.
lc = "#{File.dirname(__FILE__)}/local_config.rb"
require lc if(File.exists?(lc))

default_const(:USE_GCC_VERSION_IN_BUILDDIR_NAME, false)
default_const(:GCC_PIPE_EXTRA_FLAGS, '')
default_const(:PRINT_FLAG_CHANGES, false)
default_const(:PRINT_GCC_VERSION_INFO, false)
default_const(:GCC_DRIVER_NAME, mosyncdir + "/bin/xgcc")
default_const(:DEPEND_ON_GCC, false)
default_const(:USE_GNU_BINUTILS, false)
