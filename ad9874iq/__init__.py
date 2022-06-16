import os.path
__dir__ = os.path.split(os.path.abspath(os.path.realpath(__file__)))[0]
data_location = os.path.join(__dir__, "verilog")

def data_file(f):
    """Get absolute path for file inside ad9874iq."""
    fn = os.path.join(data_location, f)
    fn = os.path.abspath(fn)
    if not os.path.exists(fn):
        raise IOError("File {f} doesn't exist in ad9874iq".format(f))
    return fn

from .ad9874iq import AD9874IQ

__all__ = [AD9874IQ]
