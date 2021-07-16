import sys, os
from distutils.core import setup, Extension
from subprocess import Popen, PIPE, check_output

def call(*cmd):
    cmd = Popen(cmd,
                stdout=PIPE, stderr=PIPE,
                universal_newlines=True)
    if cmd.wait() == 0:
        return cmd.returncode, cmd.stdout.read()
    else:
        return cmd.returncode, cmd.stderr.read()

def pkgconfig(package, **kw):
    pkg_version = package.replace('-', '_').upper() + '_VERSION'
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    pkgconf = os.getenv('PKG_CONFIG', 'pkg-config')
    status, result = call(pkgconf, '--libs', '--cflags', package)
    if status != 0:
        return status, result
    for token in result.split():
        kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])

    # allow version detection to be overridden using environment variables
    version = os.getenv(pkg_version)
    if not version:
        version = check_output([pkgconf, '--modversion', package],
                               universal_newlines=True).strip()
    pair = (pkg_version, version)
    defines = kw.setdefault('define_macros', [])
    if pair not in defines:
        defines.append(pair)
    return status, kw

def lib(*names, **kw):
    if '--version' in sys.argv:
        return {}
    results = []
    for name in names:
        status, result = pkgconfig(name, **kw)
        if status == 0:
            return result
        results.append(result)
    sys.stderr.write('Cannot find ' + ' or '.join(names) + ':\n\n'
                     + '\n'.join(results) + '\n')
    sys.exit(status)

version = '249'
defines = {'define_macros':[('PACKAGE_VERSION', '"{}"'.format(version))]}

_fsprg = Extension('systemd/_fsprg',
                     sources = ['systemd/_fsprg.c',
                                'systemd/util.c',
                                'systemd/fsprg.c'],
                     extra_compile_args=['-std=c99', '-Werror=implicit-function-declaration'],
                     **lib('libsystemd', 'libsystemd-journal', **defines))

_siphash24 = Extension('systemd/_siphash24',
                     sources = ['systemd/_siphash24.c',
                                'systemd/siphash24.c'
                               ],
                     extra_compile_args=['-std=c99' ,'-D_DEFAULT_SOURCE',  '-Werror=implicit-function-declaration'],
                     **lib('libsystemd', 'libsystemd-journal', **defines))

setup (name = 'systemd-python',
       version = version,
       description = 'Python interface for libsystemd',
       author_email = 'david@davidstrauss.net',
       maintainer = 'systemd developers',
       maintainer_email = 'systemd-devel@lists.freedesktop.org',
       url = 'https://github.com/systemd/python-systemd',
       license = 'LGPLv2+',
       classifiers = [
           'Programming Language :: Python :: 2',
           'Programming Language :: Python :: 3',
           'Topic :: Software Development :: Libraries :: Python Modules',
           'Topic :: System :: Logging',
           'License :: OSI Approved :: GNU Lesser General Public License v2 or later (LGPLv2+)',
           ],
       py_modules = ['systemd.fsprg'],
       ext_modules = [_fsprg, _siphash24])
