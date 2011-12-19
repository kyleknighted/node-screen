from os import chdir, system
from os.path import abspath

LIBVNCSERVER = 'deps/libvncserver/'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

  chdir(LIBVNCSERVER)
  system('./autogen.sh && ./configure')
  chdir('../../')
  conf.env.append_value('LIBPATH_VNC', abspath('%s/libvncserver/.libs' % LIBVNCSERVER))
  conf.env.append_value('CPPPATH_VNC', LIBVNCSERVER)

def build(bld):
  system('make -C %s' % LIBVNCSERVER)
 
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.cxxflags = [ '-g', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE', '-Wall' ]
  obj.source = 'binding.cc'
  obj.target = 'binding'
  obj.uselib = 'VNC'

#def shutdown(bld):
  #system('make -C %s clean' % LIBVNCSERVER)
