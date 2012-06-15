#!/usr/bin/ruby

# This program generates numeric values of opcodes in the Mapip2 arch.
# Outputs header files with enums in the following languages:
# CGEN
# C

OPCODES = [
	:nop,	# NOP must be the first one, number 0.
	:ldr,
	:ldi,
	:ldw,
	:ldb,
	:ldh,
	:stw,
	:stb,
	:sth,
	:jpr,
	:jpi,
	:callr,
	:calli,
	:syscall,
	:ret,
	:push,
	:pop,
	:not,
	:neg,
	:xh,
	:xb,
]

JC_OPCODES = [
	:eq,
	:ne,
	:ge,
	:gt,
	:le,
	:lt,
	:ltu,
	:geu,
	:gtu,
	:leu,
]

JC_OPCODES.each do |o|
	OPCODES << ('jc_' + o.to_s).to_sym
end

ARITH_OPCODES = [
	:div,
	:divu,
	:mul,
	:add,
	:sub,
	:and,
	:or,
	:xor,
	:sll,
	:sra,
	:srl,
]

ARITH_OPCODES.each do |o|
	OPCODES << o
	OPCODES << (o.to_s + 'i').to_sym
end

puts "Count: #{OPCODES.size} opcodes"

mode = ARGV[0]
filename = ARGV[1]

puts "Mode: #{mode}"
puts "Filename: #{filename}"

if(mode == 'cgen')
	open(filename, 'w') do |cgen|
		i = 0
		cgen.puts '(define-normal-insn-enum opcodes "opcodes" () OP_ f-op ('
		OPCODES.each do |o|
			cgen.puts "(\"#{o.to_s.upcase}\" #{i})"
			i += 1
		end
		cgen.puts '))'
	end
#elsif(mode == 'ccore')
else
	raise "Invalid mode: #{mode}"
end
