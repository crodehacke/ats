import matplotlib
import matplotlib.colors
import matplotlib.cm
import numpy as np

# black-zero jet is jet, but with the 0-value set to black, with an immediate jump to blue
def blackzerojet_cmap(data):
    blackzerojet_dict = {'blue': [[0.0, 0.0, 0.0],
                                  [0.0, 0.0, 0.5],
                                  [0.11, 1, 1],
                                  [0.34000000000000002, 1, 1],
                                  [0.65000000000000002, 0, 0],
                                  [1, 0, 0]],
                        'green': [[0.0, 0.0, 0.0],
                                  [0.0, 0.0, 0.0],
                                  [0.125, 0, 0],
                                  [0.375, 1, 1],
                                  [0.64000000000000001, 1, 1],
                                  [0.91000000000000003, 0, 0],
                                  [1, 0, 0]],
                          'red': [[0.0, 0.0, 0.0],
                                  [0.0, 0.0, 0.0],
                                  [0.34999999999999998, 0, 0],
                                  [0.66000000000000003, 1, 1],
                                  [0.89000000000000001, 1, 1],
                                  [1, 0.5, 0.5]]
                          }
    minval = data[np.where(data > 0.)[0]].min(); print(minval)
    maxval = data[np.where(data > 0.)[0]].max(); print(maxval)
    oneminval = .9*minval/maxval
    for color in ['blue', 'green', 'red']:
        for i in range(1,len(blackzerojet_dict[color])):
            blackzerojet_dict[color][i][0] = blackzerojet_dict[color][i][0]*(1-oneminval) + oneminval

    return matplotlib.colors.LinearSegmentedColormap('blackzerojet', blackzerojet_dict)

# ice color map
def ice_cmap():
    x = np.linspace(0,1,7)
    b = np.array([1,1,1,1,1,0.8,0.6])
    g = np.array([1,0.993,0.973,0.94,0.893,0.667,0.48])
    r = np.array([1,0.8,0.6,0.5,0.2,0.,0.])

    bb = np.array([x,b,b]).transpose()
    gg = np.array([x,g,g]).transpose()
    rr = np.array([x,r,r]).transpose()
    ice_dict = {'blue': bb, 'green': gg, 'red': rr}
    return matplotlib.colors.LinearSegmentedColormap('ice', ice_dict)

# water color map
def water_cmap():
    x = np.linspace(0,1,8)
    b = np.array([1.0, 1.0, 1.0, 1.0, 0.8, 0.6, 0.4, 0.2])
    g = np.array([1.0, 0.8, 0.6, 0.4, 0.2, 0.0, 0.0, 0.0])
    r = np.array([1.0, 0.7, 0.5, 0.3, 0.1, 0.0, 0.0, 0.0])

    bb = np.array([x,b,b]).transpose()
    gg = np.array([x,g,g]).transpose()
    rr = np.array([x,r,r]).transpose()
    water_dict = {'blue': bb, 'green': gg, 'red': rr}
    return matplotlib.colors.LinearSegmentedColormap('water', water_dict)

# water color map
def gas_cmap():
    x = np.linspace(0,1,8)
    r = np.array([1.0, 1.0, 1.0, 1.0, 0.8, 0.6, 0.4, 0.2])
    #    g = np.array([1.0, 0.8, 0.6, 0.4, 0.2, 0.0, 0.0, 0.0])
    b = np.array([1.0, 0.6, 0.4, 0.2, 0.0, 0.0, 0.0, 0.0])
    g = np.array([1.0, 0.6, 0.4, 0.2, 0.0, 0.0, 0.0, 0.0])

    bb = np.array([x,b,b]).transpose()
    gg = np.array([x,g,g]).transpose()
    rr = np.array([x,r,r]).transpose()
    gas_dict = {'blue': bb, 'green': gg, 'red': rr}
    return matplotlib.colors.LinearSegmentedColormap('gas', gas_dict)


# jet-by-index
def cm_mapper(vmin=0., vmax=1., cmap=matplotlib.cm.jet):
    """Factory for a Scalar Mappable, which gives a color based upon a scalar value.

    Typical Usage:
      >>> # plots 11 lines, with color scaled by index into jet
      >>> mapper = cm_mapper(vmin=0, vmax=10, cmap=matplotlib.cm.jet)
      >>> for i in range(11):
      ...     data = np.load('data_%03d.npy'%i)
      ...     plt.plot(x, data, color=mapper(i))
      ...
      >>> plt.show()
    """

    norm = matplotlib.colors.Normalize(vmin, vmax)
    sm = matplotlib.cm.ScalarMappable(norm, cmap)
    def mapper(value):
        return sm.to_rgba(value)
    return mapper


def float_list_type(mystring):
    """Convert string-form list of doubles into list of doubles."""
    colors = []
    for f in mystring.strip("(").strip(")").strip("[").strip("]").split(","):
        try:
            colors.append(float(f))
        except:
            colors.append(f)
    return colors


def desaturate(color, amount=0.4, is_hsv=False):
    if not is_hsv:
        hsv = matplotlib.colors.rgb_to_hsv(matplotlib.colors.to_rgb(color))
    else:
        hsv = color

    print(hsv)
    hsv[1] = max(0,hsv[1] - amount)
    return matplotlib.colors.hsv_to_rgb(hsv)

def darken(color, fraction=0.6):
    rgb = np.array(matplotlib.colors.to_rgb(color))
    return tuple(np.maximum(rgb - fraction*rgb,0))

def lighten(color, fraction=0.6):
    rgb = np.array(matplotlib.colors.to_rgb(color))
    return tuple(np.minimum(rgb + fraction*(1-rgb),1))
