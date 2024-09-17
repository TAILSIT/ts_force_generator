from dataclasses import dataclass
import numpy as np
# from scipy.interpolate import interp1d

@dataclass
class ODESettings :
    mass : float 
    grav : float 
    dt : float 
    endt : float

@dataclass
class InitialConditions :
    z0 : float = 0.
    v0 : float = 0.

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
def ode_eb( settings : ODESettings, ic : InitialConditions, func ):
    m = settings.mass
    dt = settings.dt
    A = np.array( [[1.0, 0.0], [0.0,   m]])
    M = np.array( [[0.0, 1.0], [0.0, 0.0]])
    AM = A/dt - M
    iAM = np.linalg.inv( AM )
    mg = m * settings.grav

    t = np.arange( 0, settings.endt, dt)
    z = np.zeros_like(t)
    v = np.zeros_like(t)
    a = np.zeros_like(t)

    # initial conditions
    z[0] = ic.z0
    v[0] = ic.v0
    a[0] = -settings.grav + func( t[0] )/settings.mass

    # time-stepping loop
    for i in range(1,len(t)):
        xim1 = np.array( [ z[i-1], v[i-1]])
        fi = func(t[i])
        h = np.array( [0, -mg + fi])
        rhs = np.dot( M, xim1) + h
        k = np.dot( iAM, rhs )
        xi = xim1 + k
        z[i] = xi[0]
        v[i] = xi[1]
        a[i] = -settings.grav + fi / m

    return t, z, v, a

def export_to_file( filename: str, t, z, v, a ):
    data = np.column_stack((t,z,v,a))
    np.savetxt( filename, data, fmt='%6e')

def load_forces( filename ):
    data = np.loadtxt( filename, dtype=float, comments='#')
    t = data[:,0]
    f = data[:,3]
    return t, f

def force( t : float ) -> float:
    if t <= .06 or t >= 0.25: 
        return 0.0
    else:
        return 9.81 * 6e-3

if __name__ == "__main__":

    #
    # COMPUTE MASS BY MEANS OF A CYLINDER AND GIVEN DENSITY
    r_cyl = 6.35e-3
    h_cyl = 6.35e-3
    rho_cyl = 7459.0
    mass = np.pi * r_cyl**2 * h_cyl * rho_cyl
    settings = ODESettings(mass=mass,grav=9.81,dt=5e-4,endt=3e-1)

    ic = InitialConditions(z0=0.0,v0=0.0)

    # tin, fin = load_forces( 'implicit_dt5e-3.dat')
    # fh = interp1d( tin, fin, kind='linear' )
    
    t, z, v, a = ode_eb( settings=settings, ic=ic, func=force )
    
    export_to_file( 'ode_eb.dat', t, z, v, a)
    