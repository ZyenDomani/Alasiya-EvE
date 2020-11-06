package main

import (
	"fmt"
	"github.com/sbinet/go-python"
)

// No longer used.  Using a real decorator causes an issue with injection
var decos = `
__patches__ = {}
class liveupdate(object):
		def __init__(self, type, className, method):
				global __patches__
				__patches__[className + "." + method] = True
				self.className = className
				self.method = method

		def __call__(self, f):
				def wrapper(*args):
						f(*args)
				return wrapper
`

// MakeDevTools generates a devtools.raw file given a devtools.py
//
// It uses a hack to replace all instances of hexex::<filename> with a hex encoded bytecode representation of the code
func MakeDevTools() {
	// The following is Python code that performs hexex replacement in our devtools.raw generation.  Very hacky but hey.
	ret := python.PyRun_SimpleString(`
import marshal
import os
import types

execfile("devtools.py")

k = 0
co_consts = list(Bootstrap.func_code.co_consts)

for i in Bootstrap.func_code.co_consts:
    ext = None
    fname = None
    if not isinstance(i, basestring):
        k = k + 1
        continue
    print i.split("::")
    if i.split("::")[0] == "hexex":
        ext = os.path.splitext(i)[1]
        fname = i.split("::")[1]
    else:
        k = k + 1
        continue
    print ext
    if ext == ".py":
        f = open(fname, "rb")
        c = f.read()
        code = compile(c, fname, 'exec')
        cc = marshal.dumps(code).encode("hex")
        co_consts[k] = cc
        f.close()
    k = k + 1

c = Bootstrap.func_code

BootstrapMod = types.CodeType(c.co_argcount,
                    # c.co_kwonlyargcount,  Add this in Python3
                    c.co_nlocals,
                    c.co_stacksize,
                    c.co_flags,
                    c.co_code,
                    tuple(co_consts),
                    c.co_names,
                    c.co_varnames,
                    c.co_filename,
                    c.co_name,
                    c.co_firstlineno,
                    c.co_lnotab,   # In general, You should adjust this
                    c.co_freevars,
                    c.co_cellvars)

print "Dumping co_consts"
print BootstrapMod.co_consts

#cobj = compile(Bootstrap, "devtools", "exec")
d = {"Bootstrap": (BootstrapMod, ("some", "defaults"))}
f = open("devtools.raw", "wb")
c = marshal.dump(d, f)
f.close()

`)

	if ret == 0 {
		fmt.Printf("   > Successfully wrote devtools.raw\n")
	} else {
		fmt.Printf("   [Error] Couldn't write out devtools.raw\n")
	}

}
