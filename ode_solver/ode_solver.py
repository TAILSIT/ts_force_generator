import numpy as np
from scipy.integrate import solve_ivp
from functools import partial
import argparse

# IMPLICIT EULER
# WE JUST SOLVE: m z'' = -m g + f(t,z)
# 
# EXPRESSED AS 1st ORDER SYSTEM:
# | 1   0 | | z' |    | 0   1 | | z |   |     0   |
# |       |.|    | =  |       |.|   | + |         |
# | 0   m | | v' |    | 0   0 | | v |   | -mg + f |
#
#  ---> A x' = M x + h
#
def ode_system( t, y, m, g, f):
    z, v = y
    dzdt = v
    dvdt = -g + f(t,z)/m
    return [dzdt, dvdt]

def export_to_file( filename: str, t, z, v, f ):
    header = "t z v f=m z''"
    data = np.column_stack((t,z,v,f))
    np.savetxt( filename, data, header=header, fmt='%6e')

def force( t: float, z: float, scale:float ) -> float:
    zz = np.abs(z)
    args = np.array( [20.0-1000.0*zz, 25.0-1000.0*zz, 85.0-1000.0*zz])
    exps = np.exp( args )
    eval = 2.0/(1.0+exps[0]) - 1.0/(1.0+exps[1]) - 1.0/(1.0+exps[2])
    return eval * scale

if __name__ == "__main__":
    # 
    # argument parser 
    parser = argparse.ArgumentParser(description="simple ode solver / illustrate force of a falling magnet")
    parser.add_argument( "ode_method", type=str, help="The ode method") # e.g. 'RK45' or 'BDF'
    parser.add_argument( "dt", type=float, help="The time step size")
    #
    # parse arguments
    args = parser.parse_args()
    #
    # COMPUTE MASS BY MEANS OF A CYLINDER AND GIVEN DENSITY
    r_cyl = 6.35e-3
    h_cyl = 6.35e-3
    rho_cyl = 7459.0
    mass = np.pi * r_cyl**2 * h_cyl * rho_cyl

    y0 = [0.0,0.0]
    
    t_span = (0,3e-1)
    dt = args.dt

    grav = 9.81
    mg = mass * grav
    fz = partial( force, scale=mg )
    time_steps = np.arange(0,t_span[1],dt)
    
    sol = solve_ivp( ode_system, t_span, y0, args=(mass, grav, fz), t_eval=time_steps, method=args.ode_method)

    t = sol.t
    z = sol.y[0]
    v = sol.y[1]

    f = np.zeros_like(t)

    for i, ti in enumerate(t):
        f[i] = -mg + fz(ti,z[i])
    
    fname = f"ode_{args.ode_method}_dt{args.dt:.0e}.dat"
    export_to_file( fname, t, z, v, f )
    