-- Lua (keep this comment, it is an indication for editor's 'run' command)

if console_gui ~= nil then
    console_gui.show()
end    

B = NL.create_vector(2)
B[0] = 5.0
B[1] = 6.0

M = NL.create_matrix(2,2)

M.add_coefficient(0,0,1.0)
M.add_coefficient(0,1,2.0)
M.add_coefficient(1,0,3.0)
M.add_coefficient(1,1,4.0)

print('')
print('========= test matrix factorization =======')
 
Minv = M.factorize('SuperLU_perm')

X = gom.create({classname='OGF::NL::Vector',size=2})
Minv.mult(B,X)

Bcheck = NL.create_vector(2)

M.mult(X,Bcheck)
print(Bcheck[0])
print(Bcheck[1])

print('========= test matrix solve direct =======')

M.solve_direct({b=B,x=X,factorization='SuperLU_perm'})
M.mult(X,Bcheck)
print(Bcheck[0])
print(Bcheck[1])

print('========= test matrix solve iterative =======')

M.solve_iterative({b=B,x=X,solver='BiCGSTAB',precond='None',max_iter=1000,eps=0.001})
M.mult(X,Bcheck)
print(Bcheck[0])
print(Bcheck[1])




M = NL.create_matrix(2,2)

M.add_coefficient(0,0,1.0)
M.add_coefficient(0,1,2.0)
M.add_coefficient(1,0,2.0)
M.add_coefficient(1,1,3.0)

print('========= test matrix solve symmetric (iterative) =======')

M.solve_symmetric({b=B,x=X})
M.mult(X,Bcheck)
print(Bcheck[0])
print(Bcheck[1])

print('========= test matrix solve symmetric (direct) =======')

M.solve_symmetric({b=B,x=X,direct_solver=true})
M.mult(X,Bcheck)
print(Bcheck[0])
print(Bcheck[1])

