#!/usr/bin/env python

import os

TOP = '.'
APPNAME = 'WireCell'
VERSION = os.popen("git describe --tags").read().strip()


def options(opt):
    opt.load("wcb")

    # this used in cfg/wscript_build
    opt.add_option('--install-config', type=str, default="",
                   help="Install configuration files for given experiment")

    # fixme: add to spdlog entry in wcb.py
    opt.add_option('--with-spdlog-static', type=str, default="yes",
                   help="Def is true, set to false if your spdlog is not compiled (not recomended)")

def configure(cfg):
    # get this into config.h
    cfg.define("WIRECELL_VERSION", VERSION)
    cfg.load("wcb")

    # fixme: should go into wcb.py
    cfg.find_program("jsonnet", var='JSONNET')

    # boost 1.59 uses auto_ptr and GCC 5 deprecates it vociferously.
    cfg.env.CXXFLAGS += ['-Wno-deprecated-declarations']
    cfg.env.CXXFLAGS += ['-Wall', '-Wno-unused-local-typedefs', '-Wno-unused-function']
    # cfg.env.CXXFLAGS += ['-Wpedantic', '-Werror']
    
    if cfg.options.with_spdlog_static.lower() in ("yes","on","true"):
        cfg.env.CXXFLAGS += ['-DSPDLOG_COMPILED_LIB=1']

    print("Configured version", VERSION)
    # print(cfg.env)

def build(bld):
    bld.load('wcb')
