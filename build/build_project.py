#!/usr/bin/python2.7

import sys
import os


sys.path.insert(0, os.path.join('/data/chrome/tarball/chromium.25.0.1364.172/home/src_tarball/tarball/chromium/src', 'tools', 'gyp', 'pylib'))
import gyp


if __name__ == '__main__':
    args = sys.argv[1:]

    sys.exit(gyp.main(args))



