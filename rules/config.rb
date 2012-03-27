# load local_config.rb, if it exists.
lc = "#{File.dirname(__FILE__)}/local_config.rb"
require lc if(File.exists?(lc))
