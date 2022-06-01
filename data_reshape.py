import numpy as np
import pylab

d=np.fromfile('afhba.0.log',dtype=np.int16)
t=d.reshape(-1,d.size//(d.size//160)) #nsamples

x=np.arange(0,d.size//160)*0.00002

#print(t[0,0])
pylab.plot(x,t[:,0]*0.000305,'b-',linewidth=1.5)
pylab.show()
