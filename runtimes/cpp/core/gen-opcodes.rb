#!/usr/bin/ruby

# This program generates numeric values of opcodes in the Mapip2 arch.
# Outputs header files with enums in the following languages:
# CGEN
# C
# custom binutils description

Opcode = Struct.new('Opcode', :name, :mnemonic, :operands)

def o(n,m,o)
	Opcode.new(n,m,o)
end

OPCODES = [
	o(:nop, 'nop', []),	# NOP must be the first one, number 0.
	o(:ldr, 'ld', [:rd, :rs]),
	o(:ldi, 'ld', [:rd, :imm]),
	o(:ldw, 'ld', [:rd, :adaddr]),
	o(:ldb, 'ld.h', [:rd, :adaddr]),
	o(:ldh, 'ld.b', [:rd, :adaddr]),
	o(:stw, 'ld', [:adaddr, :rs]),
	o(:stb, 'ld.b', [:adaddr, :rs]),
	o(:sth, 'ld.h', [:adaddr, :rs]),
	o(:jpr, 'jp', [:rd]),
	o(:jpi, 'jp', [:aiaddr]),
	o(:callr, 'call', [:rd]),
	o(:calli, 'call', [:aiaddr]),
	o(:syscall, 'syscall', [:imm8]),
	o(:ret, 'ret', []),
	o(:push, 'push', [:rd, :rs]),
	o(:pop, 'pop', [:rd, :rs]),
	o(:not, 'not', [:rd, :rs]),
	o(:neg, 'neg', [:rd, :rs]),
	o(:xh, 'xh', [:rd, :rs]),
	o(:xb, 'xb', [:rd, :rs]),
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

JC_OPCODES.each do |op|
	OPCODES << o(('jc_' + op.to_s).to_sym, 'jc '+op.to_s, [:rd, :rs, :riaddr])
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

ARITH_OPCODES.each do |op|
	OPCODES << o(op, op.to_s, [:rd, :rs])
	i = op.to_s + 'i'
	OPCODES << o(i.to_sym, op.to_s, [:rd, :imm])
end

puts "Count: #{OPCODES.size} opcodes"


REGISTERS = [
:zr,:sp,:ra,:fp,
:s0,:s1,:s2,:s3,:s4,:s5,:s6,:s7,
:p0,:p1,:p2,:p3,
:g0,:g1,:g2,:g3,:g4,:g5,:g6,:g7,
:g8,:g9,:g10,:g11,:g12,:g13,
:r0,:r1,
]


mode = ARGV[0]
filename = ARGV[1]

puts "Mode: #{mode}"
puts "Filename: #{filename}"

def nodePart(name, children_or_opcode)
	name = name.gsub('.', '_')
	name = name.gsub(' ', '_')
	if(children_or_opcode.is_a? Hash)
		return "#{name}, ARRAY_SIZE(#{name})"
	else
		return "NULL, OP_#{children_or_opcode.to_s.upcase}"
	end
end

def outputParseNode(file, prefix, op)
	prefix = prefix.gsub('.', '_')
	prefix = prefix.gsub(' ', '_')
	if(op.is_a? Hash)
		op.each do |operand, children|
			outputParseNode(file, prefix + '_' + operand.to_s, children)
		end
		file.puts "static const mapip2_parse_node _#{prefix}[] = {"
		op.each do |operand, children_or_opcode|
			c = nodePart("_#{prefix}_#{operand}", children_or_opcode)
			file.puts "\t{ #{operand.to_s.upcase}, #{c} },"
		end
		file.puts '};'
	end
end

if(mode == 'cgen')
	open(filename, 'w') do |file|
		i = 0
		file.puts '(define-normal-insn-enum opcodes "opcodes" () OP_ f-op ('
		OPCODES.each do |op|
			file.puts "(\"#{op.name.to_s.upcase}\" #{i})"
			i += 1
		end
		file.puts '))'
	end
elsif(mode == 'ccore')
	open(filename, 'w') do |file|
		i = 0
		file.puts '#ifndef GEN_OPCODES_H'
		file.puts '#define GEN_OPCODES_H'
		file.puts
		OPCODES.each do |op|
			file.puts "#define OP_#{op.name.to_s.upcase} #{i}"
			i += 1
		end
		file.puts
		file.puts 'enum {'
		REGISTERS.each do |r|
			file.puts "\tREG_#{r},"
		end
		file.puts '};'
		file.puts
		file.puts '#endif	//GEN_OPCODES_H'
	end
elsif(mode == 'binutils/desc')
	open(filename, 'w') do |file|
		file.puts '#ifndef _MAPIP2_DESC_H'
		file.puts "#error Don't include this file unless you know what you're doing."
		file.puts '#endif'
		file.puts
		# plain opcode table
		file.puts 'static const mapip2_insn mapip2_insns[] = {'
		OPCODES.each do |op|
			file.puts "{ OP_#{op.name.to_s.upcase}, \"#{op.mnemonic}\", { "+
				op.operands.collect { |r| r.to_s.upcase + ', ' }.join + "END } },"
		end
		file.puts '};'
		file.puts
		# parse tree
		# eg: 'ld' => [
		#	rd => [rs, imm, adaddr],
		#	adaddr => [rs],
		# ]
		tree = {}
		OPCODES.each do |op|
			node = tree[op.mnemonic]
			node = {} if(!node)
			n0 = node
			n2 = node
			nr = nil
			op.operands.each do |r|
				n1 = n0[r]
				n1 = {} if(!n1)
				n0[r] = n1
				n2 = n0
				nr = r
				n0 = n1
			end
			if(nr)
				n2[nr] = op.name
			else
				node = op.name
			end
			tree[op.mnemonic] = node
		end
		p tree
		puts "Mnemonics: #{tree.size}"
		tree.each do |m, op|
			outputParseNode(file, m.to_s, op)
		end
		# todo: use gperf to make a perfect hash table instead of an array.
		file.puts 'static const mapip2_mnemonic mapip2_mnemonics[] = {'
		tree.each do |m, op|
			file.puts "{ \"#{m}\", #{nodePart("_#{m}", op)} },"
		end
		file.puts '};'
		file.puts
		file.puts 'static const char* const mapip2_register_names[] = {'
		REGISTERS.each do |r|
			file.puts "\t\"#{r}\","
		end
		file.puts '};'
	end
else
	raise "Invalid mode: #{mode}"
end
