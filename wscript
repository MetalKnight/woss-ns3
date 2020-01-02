## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os

from waflib import Options

def options(opt):
    opt.add_option('--with-woss-source',
                   help=('Path to WOSS source code, for WOSS Integration Framework support'),
                   dest='with_woss_source', default=None)
    opt.add_option('--with-woss-library',
                   help=('Path to WOSS library, for NS-3 WOSS Integration Framework support'),
                   dest='with_woss_lib', default=None)
    opt.add_option('--with-netcdf-install',
                   help=('Path to NetCDF legacy library install, for WOSS Integration Framework support'),
                   dest='with_netcdf_install', default=None)
    opt.add_option('--with-netcdf4-install',
                   help=('Path to NetCDF4 and HDF5 library install, for WOSS Integration Framework support'),
                   dest='with_netcdf4_install', default=None)

def configure(conf):

    config_error = "false"

    if Options.options.with_woss_source:
        conf.msg("Checking the given WOSS source code path", ("%s (given)" % Options.options.with_woss_source))
        if os.path.isdir(Options.options.with_woss_source):
            conf.env['WITH_WOSS_SOURCE'] = os.path.abspath(Options.options.with_woss_source)
        else:
            conf.msg("WOSS source code path", False)
            config_error = "true"
    else:
        conf.report_optional_feature("WOSS", "WOSS Integration Framework", False,
                                     "WOSS not enabled (see option --with-woss)")
        # Add this module to the list of modules that won't be built
        # if they are enabled.
        conf.env['MODULES_NOT_BUILT'].append('woss-ns3')
        return

    if Options.options.with_woss_lib:
        conf.msg("Checking the given WOSS library path", ("%s (given)" % Options.options.with_woss_lib))
        if os.path.isdir(Options.options.with_woss_lib):
            conf.env['WITH_WOSS_LIB'] = os.path.abspath(Options.options.with_woss_lib)
        else:
            conf.msg("WOSS library path", False)
            config_error = "true"
    else:
        conf.msg("Checking for WOSS library location", ("%s (guessed)" % '/usr/lib'))
        conf.env['WITH_WOSS_LIB'] = os.path.abspath('/usr/lib')

    if Options.options.with_netcdf4_install:
        conf.msg("Checking the given NetCDF4 and HDF5 install path", ("%s (given)" % Options.options.with_netcdf4_install))
        if (os.path.isdir(Options.options.with_netcdf4_install) and 
              os.path.isdir(Options.options.with_netcdf4_install + '/include') and 
              os.path.isdir(Options.options.with_netcdf4_install + '/lib')):
            conf.env['WITH_NETCDF4_SRC'] = os.path.abspath(Options.options.with_netcdf4_install + '/include')
            conf.env['WITH_NETCDF4_LIB'] = os.path.abspath(Options.options.with_netcdf4_install + '/lib')
        else:
            conf.msg("NetCDF4 and HDF5 install path", False)
            config_error = "true"
    elif Options.options.with_netcdf_install:
        conf.msg("Checking the given NetCDF legacy install path", ("%s (given)" % Options.options.with_netcdf_install))
        if (os.path.isdir(Options.options.with_netcdf_install) and 
              os.path.isdir(Options.options.with_netcdf_install + '/include') and 
              os.path.isdir(Options.options.with_netcdf_install + '/lib')):
            conf.env['WITH_NETCDF_SRC'] = os.path.abspath(Options.options.with_netcdf_install + '/include')
            conf.env['WITH_NETCDF_LIB'] = os.path.abspath(Options.options.with_netcdf_install + '/lib')
        else:
            conf.msg("NetCDF legacy install path", False)
            config_error = "true"
    else:
        conf.msg("NetCDF support", False)

    if config_error == "true":
        conf.report_optional_feature("WOSS", "WOSS Integration Framework", False,
                              "WOSS configuration error")

        # Add this module to the list of modules that won't be built
        # if they are enabled.
        conf.env['MODULES_NOT_BUILT'].append('woss-ns3')
        return

    if conf.env['WITH_WOSS_SOURCE']:

        for tmp in ['woss', 'woss/woss_def', 'woss/woss_db']:
            inc_dir = os.path.abspath(os.path.join(conf.env['WITH_WOSS_SOURCE'],tmp))
            if os.path.isdir(inc_dir):
                conf.msg("WOSS source code path is valid", ("%s " % inc_dir))
                conf.env.append_value('INCLUDES_WOSS', inc_dir)
            else:
                conf.msg("WOSS source code path is not valid", ("%s " % inc_dir))
                conf.report_optional_feature("WOSS", "WOSS Integration Framework", False,
                                     "WOSS configuration error")

                # Add this module to the list of modules that won't be built
                # if they are enabled.
                conf.env['MODULES_NOT_BUILT'].append('woss-ns3')
                return

        conf.env['LIBPATH_WOSS']=conf.env['WITH_WOSS_LIB']
        conf.env.append_value('NS3_MODULE_PATH', conf.env['LIBPATH_WOSS'])
        conf.env['LIB_WOSS'] = ['WOSS']
        conf.env['DEFINES_WOSS'] = ['WOSS_MULTITHREAD']
        conf.env.append_value('DEFINES_WOSS', 'NS3_WOSS_SUPPORT')

        conf.env['WOSS'] = conf.check(mandatory=True, lib='WOSS', define_name='WOSS', libpath=conf.env['WITH_WOSS_LIB'] , uselib_store='WOSS',
                                              msg="Checking the given WOSS library")

        if conf.env['WITH_NETCDF_SRC'] or conf.env['WITH_NETCDF4_SRC']:
            if conf.env['WITH_NETCDF_SRC']:
                conf.msg("NetCDF source code path", ("%s " % conf.env['WITH_NETCDF_SRC']))
    
                conf.env['DEFINES_NETCDF'] = ['WOSS_NETCDF_SUPPORT']
                conf.env['INCLUDES_NETCDF'] = conf.env['WITH_NETCDF_SRC']
                conf.env['LIBPATH_NETCDF'] = conf.env['WITH_NETCDF_LIB']
                conf.env.append_value('NS3_MODULE_PATH', conf.env['LIBPATH_NETCDF'])
                conf.env['LIB_NETCDF'] = ['netcdf_c++', 'netcdf']

                conf.env['NETCDF'] = conf.check(mandatory=True, lib='netcdf_c++ netcdf', libpath=conf.env['WITH_NETCDF_LIB'], define_name='NETCDF_CPP', uselib_store='NETCDF_CPP', msg="Checking the given NETCDF library")

                conf.report_optional_feature("WOSS", "WOSS Integration Framework", True,
                      "WOSS correctly configured")
            else:
                conf.msg("NetCDF4 and HDF5 source code path", ("%s " % conf.env['WITH_NETCDF4_SRC']))
    
                conf.env['DEFINES_NETCDF'] = ['WOSS_NETCDF_SUPPORT', 'WOSS_NETCDF4_SUPPORT']
                conf.env['INCLUDES_NETCDF'] = conf.env['WITH_NETCDF4_SRC']
                conf.env['LIBPATH_NETCDF'] = conf.env['WITH_NETCDF4_LIB']
                conf.env.append_value('NS3_MODULE_PATH', conf.env['LIBPATH_NETCDF'])
                conf.env['LIB_NETCDF'] = ['netcdf_c++4', 'netcdf', 'hdf5']

                conf.env['NETCDF'] = conf.check(mandatory=True, lib='netcdf_c++4 netcdf hdf5', libpath=conf.env['WITH_NETCDF4_LIB'], define_name='NETCDF_CPP', uselib_store='NETCDF_CPP', msg="Checking the given NETCDF4 and HDF5 libraries")

                conf.report_optional_feature("WOSS", "WOSS Integration Framework", True,
                      "WOSS correctly configured")
        else:
             conf.msg("Checking for NetCDF/NetCDF4 source location", False)


def build(bld):
    if 'woss-ns3' in bld.env['MODULES_NOT_BUILT']:
        return

    module = bld.create_ns3_module('woss-ns3', ['network', 'energy', 'mobility', 'uan'])
    module.source = [
        'model/definitions/woss-location.cc',
        'model/definitions/woss-time-reference.cc',
        'model/definitions/woss-random-generator.cc',
        'model/woss-prop-model.cc',
        'model/woss-channel.cc',
        'model/woss-position-allocator.cc',
        'model/woss-waypoint-mobility-model.cc',
        'helper/woss-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('woss')
    module_test.source = [
        'test/woss-test.cc',
        ]
    headers = bld(features='ns3header')
    headers.module = 'woss-ns3'
    headers.source = [
        'model/definitions/woss-location.h',
        'model/definitions/woss-time-reference.h',
        'model/definitions/woss-random-generator.h',
        'model/woss-prop-model.h',
        'model/woss-channel.h',
        'model/woss-position-allocator.h',
        'model/woss-waypoint-mobility-model.h',
        'helper/woss-helper.h',
           ]

    if bld.env['WOSS']:
        if bld.env['NETCDF']:
            module.cxxflags = ['-Wno-unused-variable']
            module.cppflags = ['-Wno-unused-variable']
            module.use.extend(['WOSS', 'NETCDF'])
            module_test.use.extend(['WOSS', 'NETCDF'])
        else:
            module.use.extend(['WOSS'])
            module_test.use.extend(['WOSS'])

    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')

