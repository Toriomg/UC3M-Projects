.text        
Integral_Matrix:
        # Inputs
        # -Integer
        # matrix addr. -> a0, M -> a1, p -> a2, q -> a3, r -> a4
        # Function registers:
        # s0 -> a, s1 -> b, s2 -> offset, s3 -> N, s4 -> word size = 4
        # s5 ->matrix addr, s6 -> M

        #push
        addi sp sp -44          # Modify the stack pointer
        sw s0 0(sp)
        sw s1 4(sp)
        sw s2 8(sp)
        sw s3 12(sp)
        sw s4 16(sp)
        sw s5 20(sp)
        sw s6 24(sp)
        sw s7 28(sp)
        sw s8 32(sp)
        sw s9 36(sp)
        sw ra 40(sp)


        li s3 100                # s3 <- N = 100
        li s4 4                 # s4 <- word size
        mv s5 a0                # s5 <- Matrix addr
        mv s6 a1                # s6 <- Size of the matrix
        mv s7 a2                # s7 <- p
        mv s8 a3                # s8 <- q
        mv s9 a4                # s9 <- r

        li s0 0                 # s0 <- counter 'a' to 0

loop_a:

        li s1 0                 # s1 <- counter 'b' to 0

loop_b:
        # Calculus of the pointer of matrix[a][b]
        # matrix_address offset: (a*M+b)*4
        li s2 0                 # s2 <- offset = 0. Set a value in offset
        mul s2 s0 s6            # offset = a*M. Move to row 'a' with size M(a0)
        add s2 s2 s1            # offset += b. Move to column 'b'
        mul s2 s2 s4            # offset *= 4  Take word size=4

        add s2 s2 s5            # Sum the offset(s2) to the matrix(s5) address to access matrix[a][b]

        bge s0 s1 else          # if a(s0) >= b(s1) go to 'else'
        # Compute integral
        # move to a registers the inputs(a,b,p,q,r,N)
        #p, q and r are already in the a register they should be
        mv a0 s0                # a0 <- a                  
        mv a1 s1                # a1 <- b
        mv a2 s7                # a2 <- p
        mv a3 s8                # a3 <- q
        mv a4 s9                # a4 <- r
        mv a5 s3                # a5 <- N

        jal ra Compute_Integral #Call function
        fsw fa0 0(s2)           #Save the result in matrix[a][b]
        j end_if
        
else:
        sw zero 0(s2)           # matrix[a][b] = 0		(when b<=a)
end_if:

		# fill all the columns of the same row (moving b)
        addi s1 s1 1            # counter b += 1
        blt s1 s6 loop_b        #if b <= M loop_b (columns)

		# move to the next row and do it again to fill all the matrix
        addi s0 s0 1            # counter a += 1
        blt s0 s6 loop_a        #if a <= M loop_a (rows)
        
        #pop
        lw s0 0(sp)
        lw s1 4(sp)
        lw s2 8(sp)
        lw s3 12(sp)
        lw s4 16(sp)
        lw s5 20(sp)
        lw s6 24(sp)
        lw s7 28(sp)
        lw s8 32(sp)
        lw s9 36(sp)
        lw ra 40(sp)
        addi sp sp 44           # Modify the stack pointer

        #end function
        jr ra

#Compute_integral function obtained from exercise1
Compute_Integral:
		#push
		addi sp, sp, -56
        sw ra, 0(sp)
        sw s0, 4(sp)
        sw s1, 8(sp)
        sw s2, 12(sp)
        sw s3, 16(sp)
        sw s4, 20(sp)
        sw s5, 24(sp)
        fsw fs0, 28(sp)
        fsw fs1, 32(sp)
        fsw fs2, 36(sp)
        fsw fs3, 40(sp)
        fsw fs4, 44(sp)
        fsw fs5, 48(sp)
        fsw fs6, 52(sp)
        

		#pass the values to float type to be able to work with them to calculate h(float type)
        fcvt.s.w fs0, a0		# fs0 (float) <- a0 (int): float(a)
        fcvt.s.w fs1, a1		# fs1 (float) <- a1 (int): float(b)
		fcvt.s.w fs2, a5		# fs2 (float) <- a5 (int): float(N)
        
        fsub.s fs3, fs1, fs0				# b-a
        fdiv.s fs3, fs3, fs2				# fs3 <- h=(b-a)/N
        
        #store p,q,r,N into 's registers' to store the value each interation (a,b are not needed to be stored, h=(b-a)/N is constant)
        mv s0, a2							# s0 <- p
        mv s1, a3							# s1 <- q
        mv s2, a4							# s2 <- r
        mv s3, a5							# s3 <- N
        
        li s4, 0				# s4 <- n = 0
        
        li s5, 0				# s5 <- result = 0
        fcvt.s.w fs4, s5		# fs4 <- result
        
while:	
		bge s4, s3, end			# if n>=N, end
        
		fcvt.s.w fs5, s4		# fs5 <- float(n)
		fmul.s fs5, fs5, fs3	# n*h
        fadd.s fs6, fs5, fs0	# fs6 <- x = n*h + a

		#move arguments of 'f function'
       	mv a0, s0				# a0 <- p
        mv a1, s1				# a1 <- q
        mv a2, s2				# a2 <- r
        fmv.s fa0, fs6			# fa0 <- x

        #call f
        jal ra, f				# jump to f function: fa0 <- output (f)
        
        fadd.s fs4, fs4, fa0	# result += f
        addi s4, s4, 1			# n += 1
        
        j while
        
end:	
		fmul.s fs4, fs4, fs3	# result = result * h
        fmv.s fa0, fs4			# fa0 <- final output (result)
        
        #pop
        lw ra, 0(sp)
        lw s0, 4(sp)
        lw s1, 8(sp)
        lw s2, 12(sp)
        lw s3, 16(sp)
        lw s4, 20(sp)
        lw s5, 24(sp)
        flw fs0, 28(sp)
        flw fs1, 32(sp)
        flw fs2, 36(sp)
        flw fs3, 40(sp)
        flw fs4, 44(sp)
        flw fs5, 48(sp)
        flw fs6, 52(sp)
        addi sp, sp, 56

        jr ra                   # End function
        
        
f:
		#push
        addi sp, sp, -28
        sw ra, 0(sp)
        fsw fs0, 4(sp)
        fsw fs1, 8(sp)
        fsw fs2, 12(sp)
        fsw fs3, 16(sp)
        fsw fs4, 20(sp)
        fsw fs5, 24(sp)
        
        #move arguments to float type to operate
        fcvt.s.w fs0, a0		# fs0 <- float(p)
        fcvt.s.w fs1, a1		# fs1 <- float(q)
        fcvt.s.w fs2, a2		# fs2 <- float(r)
		fmv.s fs3, fa0			# fs3 <- x
        
        #move the arguments for pow()
    	li a0, 2				# a0 <- 2
        fmv.s fa0, fs3			# fa0 <- x
        jal ra pow				# call pow: fa0 <- output = x^2
        
        fmv.s fs4, fa0			# f = fs4 <- x^2
        fmul.s fs4, fs4, fs0	# fs4 = p*x^2
        fmul.s fs5, fs1, fs3	# fs5 <- q*x
        fadd.s fs4, fs4, fs5	# fs4 = pq^2 + qx
        fadd.s fs4, fs4, fs2	# f = px^2 + qx + r
        
        fmv.s fa0, fs4			# return f
        
        #pop
        lw ra, 0(sp)
        flw fs0, 4(sp)
        flw fs1, 8(sp)
        flw fs2, 12(sp)
        flw fs3, 16(sp)
        flw fs4, 20(sp)
        flw fs5, 24(sp)
        addi sp, sp, 28
        
        jr ra						# end f
		
