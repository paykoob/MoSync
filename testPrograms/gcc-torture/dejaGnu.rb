# support routines for handling dejaGnu tests

EFFECTIVE_TARGETS = [
	'double64plus',
	'int32plus',
	'lto',
	'ptr32plus',
	'stdint_types',
]

INEFFECTIVE_TARGETS = [
	'fpic',
	'ilp32',
	'lp64',
	'sync_char_short',
	'sync_int_long',
	'trampolines',
]

ACCEPTABLE_TARGETS = [
	'c99_runtime',
	'init_priority',
	'int32plus',
	'inttypes_types',
	'lto',
	'native',
	'nonpic',
	'*-*-*gnu*',
	'!',
]

UNACCEPTABLE_TARGETS = [
	'||',
	'&&',
	'default_packed',
	'fpic',
	'ilp32',
	'lp64',
	'pcc_bitfield_type_matters',
	'No Inf support',
	'consts are shorts, not longs',
	'SPU float rounds towards zero',
	'*-*-cygwin*',
	'*-*-darwin',
	'*-*-darwin*',
	'*-*-interix*',
	'*-*-linux*',
	'*-*-mingw*',
	'*-*-solaris',
	'*-*-solaris*',
	'*-*-solaris2.1[0-9]*',
	'*-*-solaris2.[89]*',
	'*-*-vxworks*',
	'*-*-darwin[912]*',
	'alpha*-*-*',
	'alpha*-*-linux*',
	'alpha*-dec-osf*',
	'alpha*-dec-osf5*',
	'arm*-*-*',
	'arm*-*-*eabi',
	'arm*-*-pe*',
	'avr-*-*',
	'cris-*-*',
	'crisv32-*-*',
	'fido-*-*',
	'h8300-*-*',
	'h8300*-*-*',
	'hppa*-*-hpux*',
	'hppa*-*-*',
	'i?86-*-*',
	'i?86-*-darwin',
	'i?86-*-darwin*',
	'i?86-*-linux',
	'i?86-*-linux*',
	'i?86-*-netware',
	'i686-*-*',
	'i?86-pc-cygwin',
	'ia64-*-*',
	'ia64-*-hpux11.*',
	'ia64-*-linux*',
	'm32c-*-*',
	'm68k-*-*',
	'mips*-*-*',
	'mips-*-linux-*',
	'-mflip-mips16',
	'mmix-*-*',
	'-msx*',
	'pdp11-*-*',
	'powerpc*-*-*',
	'powerpc*-*-darwin*',
	'powerpc*-*-linux*',
	'powerpc-ibm-aix*',
	'rs6000-*-*',
	's390*-*-*',
	's390*-*-linux*',
	'sh-*-*',
	'sh[1234ble]*-*-*',
	'sh2a*-*-*',
	'sparc-*-*',
	'sparc*-*-*',
	'sparc*-*-linux*',
	'spu-*-*',
	'vax-*-*',
	'x86_64-*-*',
	'x86_64-*-linux*',
]

class Object
	def is_an?(c)
		is_a?(c)
	end
end

module DejaGnu
# parse @sourcefile.
# set @mode.
# optionally, set @EXTRA_SOURCEFILES, @EXTRA_CFLAGS, @EXTRA_CPPFLAGS
def parseDejaGnu
	open(@sourcepath) do |file|
		file.each do |line|
			# skip non-comment lines.
			start = line.index('//')
			if(!start)
				start = line.index('/*')
			end
			next if(!start)
			# check to see if it's a dg directive.
			# assuming that any comment containing '{' must be dg-formatted.
			start = line.index('{', start)
			next if(!start)

			# not gonna work
			#@tokens = line.slice(start..-1).split(/[ {}]/)

			@line = line
			@pos = start
			a = tokenize()

			# parse the results.
			#p a
			case(a[0])
			when 'dg-do' then
				# mode
				@mode = a[1].to_sym
				@mode = :compile if(@mode == :assemble)
				# options
				a[2..-1].each do |o|
					if(o.is_an?(Array))
						case(o[0])
						when 'target'
							# scan all targets. one acceptable is enough.
							acceptable = isAcceptableTarget?(o[1..-1])
							if(!acceptable)
								@mode = :skip
								return
							end
						else
							raise "Unknown option in array: #{o.inspect}"
						end
					else
						raise "Unknown option: #{o.inspect}"
					end
				end
			when 'dg-require-effective-target' then
				a[1..-1].each do |ret|
					if(!isEffectiveTarget?(ret))
						@mode = :skip
						return
					end
				end
			when 'dg-skip-if'
				a[1..-1].each do |ret|
					if(ret != '' && ret != '*' && ret != [''] && ret != ['*'] &&
						isAcceptableTarget?(ret))
						@mode = :skip
						return
					end
				end
			when 'dg-additional-sources'
				a[1..-1].each do |as|
					@EXTRA_SOURCEFILES << File.dirname(@sourcepath)+'/'+as
				end
			when 'dg-options' then
				raise "Invalid option" if(!a[1].is_a?(String))
				# if we don't have a target, set both flags.
				# if target is c, set c flags.
				# if target is c++, set c++ flags.
				# if target is something else, set both flags if the target is acceptable
				options = ' '+a[1]
				raise hell if(a[3])
				if(options.include?('-fprofile') ||
					options.include?('-fexceptions') ||
					options.include?('-ftrapv') ||
					options.include?('-fdump'))
					@mode = :skip
					return
				end
				ta = a[2]
				if(ta)
					raise "error" if(!ta.is_an?(Array))
					raise hell if(ta[0] != 'target')
					pure = true
					c = false
					cpp = false
					acceptable = false
					ta[1..-1].each do |t|
						if(t == 'c')
							c = true
						elsif(t == 'c++')
							cpp = true
						else
							pure = false
							acceptable |= isAcceptableTarget?(t)
						end
					end
					if(pure)
						if(c)
							@EXTRA_CFLAGS = options
						elsif(cpp)
							@EXTRA_CPPFLAGS = options
						else
							raise hell
						end
					elsif(acceptable)
						@EXTRA_CFLAGS = options
						@EXTRA_CPPFLAGS = options
					end
				else # no target
					@EXTRA_CFLAGS = options
					@EXTRA_CPPFLAGS = options
				end
			when 'dg-require-named-sections',
				'dg-require-ifunc',
				'dg-require-profiling',
				'dg-require-dll',
				'dg-xfail-if',
				'dg-error'
				@mode = :skip
				return
			when 'dg-final',
				'dg-bogus',
				'dg-timeout-factor',
				'dg-message',
				'dg-prune-output',
				'dg-excess-errors',
				'dg-require-alias',
				'dg-require-ascii-locale',
				'dg-require-weak',
				'dg-require-visibility',
				'dg-add-options',
				'xxdg-warning',
				'dg-warning'
				# ignored
			else
				p line
				raise "Unknown directive: #{a[0].inspect}"
			end
		end
	end
end

def isEffectiveTarget?(ret)
	if(INEFFECTIVE_TARGETS.include?(ret))
		return false
	end
	if(EFFECTIVE_TARGETS.include?(ret))
		return true
	end
	raise "Unknown require-effective-target: #{ret.inspect}"
end

def isAcceptableTarget?(t)
	if(t.is_an?(Array))
		acceptable = false
		t.each do |tt|
			acceptable |= isAcceptableTarget?(tt)
		end
		return acceptable
	end
	if(ACCEPTABLE_TARGETS.include?(t))
		return true
	end
	if(UNACCEPTABLE_TARGETS.include?(t))
		return false
	end
	raise "Unknown target: #{t.inspect}"
end

def char
	@line[@pos,1]
end

def skipWhitespace
	while(char == ' ')
		@pos += 1
	end
end

# returns an array or a string, or nil.
# raises exceptions on error.
def tokenize()
	skipWhitespace
	if(char == '{')
		@pos += 1
		array = []
		while(1)
			skipWhitespace
			if(char == '}')
				break
			end
			array << tokenize()
		end
		@pos += 1
		return array
	end

	if(char == '"')
		@pos += 1
		si = @line.index('"', @pos) - 1
		n = si+2
	else
		si = @line.index(/[ }]/, @pos) - 1
		n = si+1
	end
	s = @line.slice(@pos..si)
	@pos = n
	return s
end

end
