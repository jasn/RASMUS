#!/usr/bin/env python2
#
# Copyright 2014 wkhtmltopdf authors
# Copyright 2014 The pyRASMUS development team
#
# This file is part of pyRASMUS.
# 
# pyRASMUS is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with pyRASMUS.  If not, see <http:#www.gnu.org/licensgges/>

# --------------------------------------------------------------- CONFIGURATION


QT_SOURCE={
    'url':"http://download.qt-project.org/official_releases/qt/4.8/4.8.6/qt-everywhere-opensource-src-4.8.6.tar.gz",
    'sha1': "ddf9c20ca8309a116e0466c42984238009525da6"
}
    
LLVM_SOURCE={
    'url':"http://llvm.org/releases/3.4.2/llvm-3.4.2.src.tar.gz",
    'sha1': "c5287384d0b95ecb0fd7f024be2cdfb60cd94bc9"
}

BUILDERS = {
    'source-tarball':        'source_tarball',
    'msvc2008-win32':        'msvc',
    'msvc2008-win64':        'msvc',
    'msvc2010-win32':        'msvc',
    'msvc2010-win64':        'msvc',
    'msvc2012-win32':        'msvc',
    'msvc2012-win64':        'msvc',
    'msvc2013-win32':        'msvc',
    'msvc2013-win64':        'msvc',
    'msvc-winsdk71-win32':   'msvc_winsdk71',
    'msvc-winsdk71-win64':   'msvc_winsdk71',
    'setup-mingw-w64':       'setup_mingw64',
    'setup-schroot-centos5': 'setup_schroot',
    'setup-schroot-centos6': 'setup_schroot',
    'setup-schroot-wheezy':  'setup_schroot',
    'setup-schroot-trusty':  'setup_schroot',
    'setup-schroot-precise': 'setup_schroot',
    'update-all-schroots':   'update_schroot',
    'centos5-i386':          'linux_schroot',
    'centos5-amd64':         'linux_schroot',
    'centos6-i386':          'linux_schroot',
    'centos6-amd64':         'linux_schroot',
    'wheezy-i386':           'linux_schroot',
    'wheezy-amd64':          'linux_schroot',
    'trusty-i386':           'linux_schroot',
    'trusty-amd64':          'linux_schroot',
    'precise-i386':          'linux_schroot',
    'precise-amd64':         'linux_schroot',
    'mingw-w64-cross-win32': 'mingw64_cross',
    'mingw-w64-cross-win64': 'mingw64_cross',
    'posix-local':           'posix_local',
    'osx-cocoa-x86-64':      'osx',
    'osx-carbon-i386':       'osx'
}

QT_CONFIG = {
    'common' : [
        '-confirm-license',
        '-exceptions',
        '-fast',
        '-graphicssystem raster',
        '-no-3dnow',
        '-no-accessibility',
        '-no-declarative',
        '-no-gif',
        '-no-libjpeg'
        '-no-libmng'
        '-no-libmng',
        '-no-libtiff'
        '-no-libtiff',
        '-no-mmx',
        '-no-multimedia',
        '-no-opengl',
        '-no-openssl'
        '-no-phonon',
        '-no-phonon-backend',
        '-no-qt3support',
        '-no-script',
        '-no-scripttools',
        '-no-sql-ibase',
        '-no-sql-mysql',
        '-no-sql-odbc',
        '-no-sql-psql',
        '-no-sql-sqlite',
        '-no-sql-sqlite2',
        '-no-sse',
        '-no-sse2',
        '-no-stl',
        '-no-svg'
        '-no-webkit',
        '-no-xmlpatterns',
        '-nomake demos',
        '-nomake docs',
        '-nomake examples',
        '-nomake tests',
        '-nomake tools',
        '-nomake translations'
        '-opensource',
        '-release',
        '-static',
    ],

    'msvc': [
        '-mp',
        '-qt-style-windows',
        '-qt-style-cleanlooks',
        '-no-style-plastique',
        '-no-style-motif',
        '-no-style-cde',
    ],

    'posix': [
        '-silent',                  # perform a silent build
        '-xrender',                 # xrender support is required
        '-largefile',
        '-iconv',                   # iconv support is required for text codecs
        '-no-openssl',
        '-no-rpath',
        '-no-dbus',
        '-no-nis',
        '-no-pch',
        '-no-nas-sound',
        '-no-gstreamer',
        '-no-openvg'
        '-D ENABLE_VIDEO=0',        # required as otherwise gstreamer gets linked in
        '-no-openvg',
        '-no-audio-backend',
        '-no-sse3',
        '-no-ssse3',
        '-no-sse4.1',
        '-no-sse4.2',
        '-no-avx',
        '-no-neon'
    ],

    'mingw-w64-cross' : [
        '-silent',                  # perform a silent build
        '-no-reduce-exports',
        '-no-rpath',
        '-xplatform win32-g++-4.6'
    ],

    'osx': [
        '-silent',                  # perform a silent build
        '-no-framework',
        '-no-dwarf2',
        '-xrender',                 # xrender support is required
        '-largefile',
        '-no-rpath'
    ]
}


CHROOT_SETUP  = {
    'wheezy': [
        ('debootstrap', 'wheezy', 'http://ftp.us.debian.org/debian/'),
        ('write_file', 'etc/apt/sources.list', """
deb http://ftp.debian.org/debian/ wheezy         main contrib non-free
deb http://ftp.debian.org/debian/ wheezy-updates main contrib non-free
deb http://security.debian.org/   wheezy/updates main contrib non-free"""),
        ('shell', 'apt-get update'),
        ('shell', 'apt-get dist-upgrade --assume-yes'),
        ('shell', 'apt-get install --assume-yes xz-utils libssl-dev libpng-dev libjpeg8-dev zlib1g-dev rubygems'),
        ('shell', 'apt-get install --assume-yes libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxrender-dev'),
        ('shell', 'gem install fpm ronn --no-ri --no-rdoc'),
        ('write_file', 'update.sh', 'apt-get update\napt-get dist-upgrade --assume-yes\n'),
        ('fpm_setup',  'fpm_package.sh'),
        ('schroot_conf', 'Debian Wheezy')
    ],

    'trusty': [
        ('debootstrap', 'trusty', 'http://archive.ubuntu.com/ubuntu/'),
        ('write_file', 'etc/apt/sources.list', """
deb http://archive.ubuntu.com/ubuntu/ trusty          main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ trusty-updates  main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ trusty-security main restricted universe multiverse"""),
        ('shell', 'apt-get update'),
        ('shell', 'apt-get dist-upgrade --assume-yes'),
        ('shell', 'apt-get install --assume-yes xz-utils libssl-dev libpng-dev libjpeg-turbo8-dev zlib1g-dev ruby-dev'),
        ('shell', 'apt-get install --assume-yes libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxrender-dev'),
        ('shell', 'gem install fpm ronn --no-ri --no-rdoc'),
        ('write_file', 'update.sh', 'apt-get update\napt-get dist-upgrade --assume-yes\n'),
        ('fpm_setup',  'fpm_package.sh'),
        ('schroot_conf', 'Ubuntu Trusty')
    ],

    'precise': [
        ('debootstrap', 'precise', 'http://archive.ubuntu.com/ubuntu/'),
        ('write_file', 'etc/apt/sources.list', """
deb http://archive.ubuntu.com/ubuntu/ precise          main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ precise-updates  main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ precise-security main restricted universe multiverse"""),
        ('shell', 'apt-get update'),
        ('shell', 'apt-get dist-upgrade --assume-yes'),
        ('shell', 'apt-get install --assume-yes xz-utils libssl-dev libpng-dev libjpeg8-dev zlib1g-dev rubygems'),
        ('shell', 'apt-get install --assume-yes libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxrender-dev'),
        ('shell', 'gem install fpm ronn --no-ri --no-rdoc'),
        ('write_file', 'update.sh', 'apt-get update\napt-get dist-upgrade --assume-yes\n'),
        ('fpm_setup',  'fpm_package.sh'),
        ('schroot_conf', 'Ubuntu Precise')
    ],

    'centos5': [
        ('rinse', 'centos-5'),
        ('download_file', 'http://centos.karan.org/el5/ruby187/kbs-el5-ruby187.repo', 'etc/yum.repos.d'),
        ('download_file', 'http://dl.fedoraproject.org/pub/epel/5/i386/epel-release-5-4.noarch.rpm', 'tmp'),
        ('shell', 'rpm -i /tmp/epel-release-5-4.noarch.rpm'),
        ('shell', 'yum update -y'),
        ('append_file:amd64', 'etc/yum.conf', 'exclude = *.i?86\n'),
        ('shell', 'yum install -y gcc gcc-c++ make diffutils perl ruby-devel rubygems rpm-build libffi-devel'),
        ('shell', 'yum install -y openssl-devel libX11-devel libXrender-devel libXext-devel fontconfig-devel freetype-devel libjpeg-devel libpng-devel zlib-devel'),
        ('shell', 'gem install fpm ronn --no-ri --no-rdoc'),
        ('write_file', 'update.sh', 'yum update -y\n'),
        ('fpm_setup',  'fpm_package.sh'),
        ('schroot_conf', 'CentOS 5')
    ],

    'centos6': [
        ('rinse', 'centos-6'),
        ('shell', 'yum update -y'),
        ('append_file:amd64', 'etc/yum.conf', 'exclude = *.i?86\n'),
        ('shell', 'yum install -y gcc gcc-c++ make diffutils perl ruby-devel rubygems rpm-build libffi-devel'),
        ('shell', 'yum install -y openssl-devel libX11-devel libXrender-devel libXext-devel fontconfig-devel freetype-devel libjpeg-devel libpng-devel zlib-devel'),
        ('shell', 'gem install fpm ronn --no-ri --no-rdoc'),
        ('write_file', 'update.sh', 'yum update -y\n'),
        ('fpm_setup',  'fpm_package.sh'),
        ('schroot_conf', 'CentOS 6')
    ]
}

DEPENDENT_LIBS = {}

EXCLUDE_SRC_TARBALL = [
    'qt/config.profiles*',
    'qt/demos*',
    'qt/dist*',
    'qt/doc*',
    'qt/examples*',
    'qt/imports*',
    'qt/templates*',
    'qt/tests*',
    'qt/translations*',
    'qt/util*',
    'qt/lib/fonts*',
    'qt/src/3rdparty/*ChangeLog*',
    'qt/src/3rdparty/ce-compat*',
    'qt/src/3rdparty/clucene*',
    'qt/src/3rdparty/fonts*',
    'qt/src/3rdparty/freetype*',
    'qt/src/3rdparty/javascriptcore*',
    'qt/src/3rdparty/libgq*',
    'qt/src/3rdparty/libmng*',
    'qt/src/3rdparty/libtiff*',
    'qt/src/3rdparty/patches*',
    'qt/src/3rdparty/phonon*',
    'qt/src/3rdparty/pixman*',
    'qt/src/3rdparty/powervr*',
    'qt/src/3rdparty/ptmalloc*',
    'qt/src/3rdparty/s60*',
    'qt/src/3rdparty/wayland*'
]



import os, sys, platform, subprocess, shutil, re, fnmatch, multiprocessing, urllib, hashlib, tarfile

from os.path import exists

CPU_COUNT = max(2, multiprocessing.cpu_count()-1)   # leave one CPU free

def rchop(s, e):
    if s.endswith(e):
        return s[:-len(e)]
    return s

def message(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

def error(msg):
    message(msg+'\n')
    sys.exit(1)

def shell(cmd):
    ret = os.system(cmd)
    if ret != 0:
        error("%s\ncommand failed: exit code %d" % (cmd, ret))

def get_output(*cmd):
    try:
        return subprocess.check_output(cmd, stderr=subprocess.STDOUT).strip()
    except:
        return None

def rmdir(path):
    if exists(path):
        if platform.system() == 'Windows':
            shell('attrib -R %s\* /S' % path)
        shutil.rmtree(path)

def mkdir_p(path):
    if not exists(path):
        os.makedirs(path)

def get_version(basedir):
    mkdir_p(basedir)
    text = open(os.path.join(basedir, '..', 'VERSION'), 'r').read()
    if '-' not in text:
        return (text, text)
    version = text[:text.index('-')]
    os.chdir(os.path.join(basedir, '..'))
    hash = get_output('git', 'rev-parse', '--short', 'HEAD')
    if not hash:
        return (text, version)
    return ('%s-%s' % (version, hash), version)

def qt_config(key, *opts):
    input, output = [], []
    input.extend(QT_CONFIG['common'])
    input.extend(QT_CONFIG[key])
    input.extend(opts)
    cfg = os.environ.get('RASMUS_QT_CONFIG')
    if cfg:
        input.extend(cfg.split())
    for arg in input:
        if not arg.startswith('remove:-'):
            output.append(arg)
        elif arg[1+arg.index(':'):] in output:
            output.remove(arg[1+arg.index(':'):])
    return ' '.join(output)

def fpm_setup(cfg):
    input, output = {}, ''
    input.update(FPM_SETUP['common'])
    input.update(FPM_SETUP[cfg])
    for key in input:
        if type(input[key]) is list:
            for val in input[key]:
                output += ' %s "%s"' % (key, val)
        else:
            output += ' %s "%s"' % (key, input[key])
    return output, input

def download_file(url, sha1, dir):
    name = url.split('/')[-1]
    loc  = os.path.join(dir, name)
    if os.path.exists(loc):
        hash = hashlib.sha1(open(loc, 'rb').read()).hexdigest()
        if hash == sha1:
            return loc
        os.remove(loc)
        message('Checksum mismatch for %s, re-downloading.\n' % name)
    def hook(cnt, bs, total):
        pct = int(cnt*bs*100/total)
        message("\rDownloading: %s [%d%%]" % (name, pct))
    urllib.urlretrieve(url, loc, reporthook=hook)
    message("\r")
    hash = hashlib.sha1(open(loc, 'rb').read()).hexdigest()
    if hash != sha1:
        os.remove(loc)
        error('Checksum mismatch for %s, aborting.' % name)
    message("\rDownloaded: %s [checksum OK]\n" % name)
    return loc

def download_tarball(url, sha1, dir, name):
    loc = download_file(url, sha1, dir)
    tar = tarfile.open(loc)
    sub = tar.getnames()[0]
    if '/' in sub:
        sub = sub[:sub.index('/')]
    src = os.path.join(dir, sub)
    tgt = os.path.join(dir, name)
    rmdir(src)
    tar.extractall(dir)
    rmdir(tgt)
    os.rename(src, tgt)
    return tgt

def _is_compiled(dst, loc):
    present = True
    for name in loc['result']:
        present = present and exists(os.path.join(dst, name))
    return present

def check_running_on_debian():
    if not sys.platform.startswith('linux') or not exists('/etc/apt/sources.list'):
        error('This can only be run on a Debian/Ubuntu distribution, aborting.')

    if os.geteuid() != 0:
        error('This script must be run as root.')

    if platform.architecture()[0] == '64bit' and 'amd64' not in ARCH:
        ARCH.insert(0, 'amd64')

PACKAGE_NAME = re.compile(r'ii\s+(.+?)\s+.*')
def install_packages(*names):
    lines = get_output('dpkg-query', '--list').split('\n')
    avail = [PACKAGE_NAME.match(line).group(1) for line in lines if PACKAGE_NAME.match(line)]
    inst  = [name for name in names if name in avail]

    if len(inst) != len(names):
        shell('apt-get update')
        shell('apt-get install --assume-yes %s' % (' '.join(names)))

# --------------------------------------------------------------- Linux chroot

ARCH = ['i386']

def check_setup_schroot(config):
    check_running_on_debian()
    login = os.environ.get('SUDO_USER') or get_output('logname')
    if not login or login == 'root':
        error('Unable to determine the login for which schroot access is to be given.')

def build_setup_schroot(config, basedir):
    install_packages('git', 'debootstrap', 'schroot', 'rinse', 'debian-archive-keyring')
    os.environ['HOME'] = '/tmp' # workaround bug in gem when home directory doesn't exist

    login  = os.environ.get('SUDO_USER') or get_output('logname')
    chroot = config[1+config.rindex('-'):]
    for arch in ARCH:
        message('******************* %s-%s\n' % (chroot, arch))
        base_dir = os.environ.get('RASMUS_CHROOT') or '/var/chroot'
        root_dir = os.path.join(base_dir, 'rasmus-%s-%s' % (chroot, arch))
        rmdir(root_dir)
        mkdir_p(root_dir)
        for command in CHROOT_SETUP[chroot]:
            # handle architecture-specific commands
            name = command[0]
            if ':' in name:
                if name[1+name.rindex(':'):] != arch:
                    continue
                else:
                    name = name[:name.rindex(':')]

            # handle commands
            if name == 'debootstrap':
                shell('debootstrap --arch=%(arch)s --variant=buildd %(distro)s %(dir)s %(url)s' % {
                    'arch': arch, 'dir': root_dir, 'distro': command[1], 'url': command[2] })
            elif name == 'rinse':
                cmd = (arch == 'i386' and 'linux32 rinse' or 'rinse')
                shell('%s --arch %s --distribution %s --directory %s' % (cmd, arch, command[1], root_dir))
            elif name == 'shell':
                cmd = (arch == 'i386' and 'linux32 chroot' or 'chroot')
                shell('%s %s %s' % (cmd, root_dir, command[1]))
            elif name == 'write_file':
                open(os.path.join(root_dir, command[1]), 'w').write(command[2].strip())
            elif name == 'append_file':
                open(os.path.join(root_dir, command[1]), 'a').write(command[2].strip())
            elif name == 'download_file':
                name = command[1].split('/')[-1]
                loc  = os.path.join(root_dir, command[2], name)
                if exists(loc): os.remove(loc)
                def hook(cnt, bs, total):
                    pct = int(cnt*bs*100/total)
                    message("\rDownloading: %s [%d%%]" % (name, pct))
                urllib.urlretrieve(command[1], loc, reporthook=hook)
                message("\rDownloaded: %s%s\n" % (name, ' '*10))
            elif name == 'fpm_setup':
                args, cfg = fpm_setup(chroot)
                cmd = '#!/bin/sh\nXZ_OPT=-9 fpm --force %s --package ../%s-$1_linux-%s-$2.%s .\n'
                loc = os.path.join(root_dir, command[1])
                open(loc, 'w').write(cmd % (args, cfg['--name'], chroot, cfg['-t']))
                shell('chmod a+x %s' % loc)
            elif name == 'schroot_conf':
                cfg = open('/etc/schroot/chroot.d/rasmus-%s-%s' % (chroot, arch), 'w')
                cfg.write('[rasmus-%s-%s]\n' % (chroot, arch))
                cfg.write('type=directory\ndirectory=%s/\n' % root_dir)
                cfg.write('description=%s %s for rasmus\n' % (command[1], arch))
                cfg.write('users=%s\nroot-users=root\n' % login)
                if arch == 'i386' and 'amd64' in ARCH:
                    cfg.write('personality=linux32\n')
                cfg.close()

def check_update_schroot(config):
    check_running_on_debian()
    if not get_output('schroot', '--list'):
        error('Unable to determine the list of available schroots.')

def build_update_schroot(config, basedir):
    for name in get_output('schroot', '--list').split('\n'):
        message('******************* %s\n' % name[name.index('rasmus-'):])
        shell('schroot -c %s -- /bin/bash /update.sh' % name[name.index('rasmus-'):])

def check_setup_mingw64(config):
    check_running_on_debian()

def build_setup_mingw64(config, basedir):
    install_packages('build-essential', 'mingw-w64', 'nsis')

def check_source_tarball(config):
    if not get_output('git', 'rev-parse', '--short', 'HEAD'):
        error("This can only be run inside a git checkout.")

    if not exists(os.path.join(os.getcwd(), 'qt', '.git')):
        error("Please initialize and download the Qt submodule before running this.")

def _filter_tar(info):
    name = info.name[1+info.name.index('/'):]
    if name.endswith('.git') or [p for p in EXCLUDE_SRC_TARBALL if fnmatch.fnmatch(name, p)]:
        return None

    info.uid   = info.gid   = 1000
    info.uname = info.gname = 'rasmus'
    return info

def build_source_tarball(config, basedir):
    version, simple_version = get_version(basedir)
    root_dir = os.path.realpath(os.path.join(basedir, '..'))
    os.chdir(os.path.join(root_dir, 'qt'))
    shell('git clean -fdx')
    shell('git reset --hard HEAD')
    os.chdir(root_dir)
    shell('git clean -fdx')
    shell('git reset --hard HEAD')
    shell('git submodule update')
    open('VERSION', 'w').write(version)
    with tarfile.open('rasmus-%s.tar.bz2' % version, 'w:bz2') as tar:
        tar.add('.', 'rasmus-%s/' % version, filter=_filter_tar)
    shell('git reset --hard HEAD')

# --------------------------------------------------------------- MSVC (2008-2013)

MSVC_LOCATION = {
    'msvc2008': 'VS90COMNTOOLS',
    'msvc2010': 'VS100COMNTOOLS',
    'msvc2012': 'VS110COMNTOOLS',
    'msvc2013': 'VS120COMNTOOLS'
}

def check_msvc(config):
    version, arch = rchop(config, '-dbg').split('-')
    env_var = MSVC_LOCATION[version]
    if not env_var in os.environ:
        error("%s does not seem to be installed." % version)

    vcdir = os.path.join(os.environ[env_var], '..', '..', 'VC')
    if not exists(os.path.join(vcdir, 'vcvarsall.bat')):
        error("%s: unable to find vcvarsall.bat" % version)

    if arch == 'win32' and not exists(os.path.join(vcdir, 'bin', 'cl.exe')):
        error("%s: unable to find the x86 compiler" % version)

    if arch == 'win64' and not exists(os.path.join(vcdir, 'bin', 'amd64', 'cl.exe')) \
                       and not exists(os.path.join(vcdir, 'bin', 'x86_amd64', 'cl.exe')):
        error("%s: unable to find the amd64 compiler" % version)

def build_msvc(config, basedir):
    msvc, arch = rchop(config, '-dbg').split('-')
    vcdir = os.path.join(os.environ[MSVC_LOCATION[msvc]], '..', '..', 'VC')
    vcarg = 'x86'
    if arch == 'win64':
        if exists(os.path.join(vcdir, 'bin', 'amd64', 'cl.exe')):
            vcarg = 'amd64'
        else:
            vcarg = 'x86_amd64'

    python = sys.executable
    process = subprocess.Popen('("%s" %s>nul)&&"%s" -c "import os, sys; sys.stdout.write(repr(dict(os.environ)))"' % (
        os.path.join(vcdir, 'vcvarsall.bat'), vcarg, python), stdout=subprocess.PIPE, shell=True)
    stdout, _ = process.communicate()
    exitcode = process.wait()
    if exitcode != 0:
        error("%s: unable to initialize the environment" % msvc)

    os.environ.update(eval(stdout.strip()))

    build_msvc_common(config, basedir)

# --------------------------------------------------------------- MSVC via Windows SDK 7.1

def check_msvc_winsdk71(config):
    for pfile in ['ProgramFiles(x86)', 'ProgramFiles']:
        if pfile in os.environ and exists(os.path.join(os.environ[pfile], 'Microsoft SDKs', 'Windows', 'v7.1', 'Bin', 'SetEnv.cmd')):
            return
    error("Unable to detect the location of Windows SDK 7.1")

def build_msvc_winsdk71(config, basedir):
    arch = config[config.rindex('-'):]
    setenv = None
    for pfile in ['ProgramFiles(x86)', 'ProgramFiles']:
        if not pfile in os.environ:
            continue
        setenv = os.path.join(os.environ[pfile], 'Microsoft SDKs', 'Windows', 'v7.1', 'Bin', 'SetEnv.cmd')

    mode = debug and '/Debug' or '/Release'
    if arch == 'win64':
        args = '/2008 /x64 %s' % mode
    else:
        args = '/2008 /x86 %s' % mode

    python = sys.executable
    process = subprocess.Popen('("%s" %s>nul)&&"%s" -c "import os, sys; sys.stdout.write(repr(dict(os.environ)))"' % (
        setenv, args, python), stdout=subprocess.PIPE, shell=True)
    stdout, _ = process.communicate()
    exitcode = process.wait()
    if exitcode != 0:
        error("unable to initialize the environment for Windows SDK 7.1")

    os.environ.update(eval(stdout.strip()))

    build_msvc_common(config, basedir)

def build_msvc_common(config, basedir):
    version, simple_version = get_version(basedir)

    libdir = os.path.join(basedir, config, 'deplibs')
    qtdir  = os.path.join(basedir, config, 'qt')
    mkdir_p(qtdir)

    configure_args = qt_config('msvc',
        '-I %s\\include' % libdir,
        '-L %s\\lib' % libdir,
        'OPENSSL_LIBS="-L%s\\\\lib -lssleay32 -llibeay32 -lUser32 -lAdvapi32 -lGdi32 -lCrypt32"' % libdir.replace('\\', '\\\\'))

    os.chdir(qtdir)
    if not exists('is_configured'):
        shell('%s\\..\\qt\\configure.exe %s' % (basedir, configure_args))
        open('is_configured', 'w').write('')
    shell('nmake')

    appdir = os.path.join(basedir, config, 'app')
    mkdir_p(appdir)
    os.chdir(appdir)
    rmdir('bin')
    mkdir_p('bin')

    os.environ['RASMUS_VERSION'] = version

    shell('%s\\bin\\qmake %s\\..\\rasmus.pro' % (qtdir, basedir))
    shell('nmake')

    found = False
    for pfile in ['ProgramFiles(x86)', 'ProgramFiles']:
        if not pfile in os.environ or not exists(os.path.join(os.environ[pfile], 'NSIS', 'makensis.exe')):
            continue
        found = True

        makensis = os.path.join(os.environ[pfile], 'NSIS', 'makensis.exe')
        os.chdir(os.path.join(basedir, '..'))
        shell('"%s" /DVERSION=%s /DSIMPLE_VERSION=%s /DTARGET=%s rasmus.nsi' % \
                (makensis, version, simple_version, config))

    if not found:
        message("\n\nCould not build installer as NSIS was not found.\n")

# ------------------------------------------------ MinGW-W64 Cross Environment

MINGW_W64_PREFIX = {
    'mingw-w64-cross-win32' : 'i686-w64-mingw32',
    'mingw-w64-cross-win64' : 'x86_64-w64-mingw32',
}

def check_mingw64_cross(config):
    shell('%s-gcc --version' % MINGW_W64_PREFIX[rchop(config, '-dbg')])

def build_mingw64_cross(config, basedir):
    version, simple_version = get_version(basedir)

    libdir = os.path.join(basedir, config, 'deplibs')
    qtdir  = os.path.join(basedir, config, 'qt')

    configure_args = qt_config('mingw-w64-cross',
        '--prefix=%s'   % qtdir,
        '-I %s/include' % libdir,
        '-L %s/lib'     % libdir,
        '-device-option CROSS_COMPILE=%s-' % MINGW_W64_PREFIX[rchop(config, '-dbg')])

    os.environ['OPENSSL_LIBS'] = '-lssl -lcrypto -L %s/lib -lws2_32 -lgdi32 -lcrypt32' % libdir

    mkdir_p(qtdir)
    os.chdir(qtdir)

    if not exists('is_configured'):
        for var in ['CFLAGS', 'CXXFLAGS']:
            os.environ[var] = '-w'
        shell('%s/../qt/configure %s' % (basedir, configure_args))
        shell('touch is_configured')
    shell('make -j%d' % CPU_COUNT)

    appdir = os.path.join(basedir, config, 'app')
    mkdir_p(appdir)
    os.chdir(appdir)
    shell('rm -f bin/*')

    # set up cross compiling prefix correctly
    os.environ['RASMUS_VERSION'] = version
    shell('%s/bin/qmake -set CROSS_COMPILE %s-' % (qtdir, MINGW_W64_PREFIX[rchop(config, '-dbg')]))
    shell('%s/bin/qmake -spec win32-g++-4.6 %s/../rasmus.pro' % (qtdir, basedir))
    shell('make')
    shutil.copy('bin/librasmus0.a', 'bin/rasmus.lib')

    os.chdir(os.path.join(basedir, '..'))
    shell('makensis -DVERSION=%s -DSIMPLE_VERSION=%s -DTARGET=%s rasmus.nsi' % \
            (version, simple_version, config))

# -------------------------------------------------- Linux schroot environment

def check_linux_schroot(config):
    shell('schroot -c rasmus-%s -- gcc --version' % rchop(config, '-dbg'))

def build_linux_schroot(config, basedir):
    version, simple_version = get_version(basedir)

    dir    = os.path.join(basedir, config)
    script = os.path.join(dir, 'build.sh')
    dist   = os.path.join(dir, 'dist')

    mkdir_p(dir)
    rmdir(dist)

    configure_args = qt_config('posix', '--prefix=%s' % os.path.join(dir, 'qt'))

    lines = ['#!/bin/bash']
    lines.append('# start of autogenerated build script')
    lines.append('mkdir -p app qt')
    lines.append('cd qt')
    if config == 'centos5-i386':
        lines.append('export CFLAGS="-march=i486 -w"')
        lines.append('export CXXFLAGS="-march=i486 -w"')
    else:
        for var in ['CFLAGS', 'CXXFLAGS']:
            lines.append('export %s="-w"' % var)
    lines.append('if [ ! -f is_configured ]; then')
    lines.append('  ../../../qt/configure %s || exit 1' % configure_args)
    lines.append('  touch is_configured')
    lines.append('fi')
    lines.append('if ! make -j%d -q; then\n  make -j%d || exit 1\nfi' % (CPU_COUNT, CPU_COUNT))
    lines.append('cd ../app')
    lines.append('rm -f bin/*')
    lines.append('export RASMUS_VERSION=%s' % version)
    lines.append('../qt/bin/qmake ../../../rasmus.pro')
    lines.append('make install INSTALL_ROOT=%s || exit 1' % dist)
    lines.append('cd ..')
    lines.append('/fpm_package.sh %s %s' % (version, config[1+config.index('-'):]))
    lines.append('# end of build script')

    open(script, 'w').write('\n'.join(lines))
    os.chdir(dir)
    shell('chmod +x build.sh')
    shell('schroot -c rasmus-%s -- ./build.sh' % rchop(config, '-dbg'))


# -------------------------------------------------- POSIX local environment

def check_posix_local(config):
    pass

def build_posix_local(config, basedir):
    version, simple_version = get_version(basedir)
    app    = os.path.join(basedir, config, 'rasmus')
    qt     = os.path.join(basedir, config, 'qt')
    llvm     = os.path.join(basedir, config, 'llvm')
    make   = get_output('which gmake') and 'gmake' or 'make'

    mkdir_p(llvm)
    os.chdir(llvm)
    if not exists('is_configured'):
        shell('cmake ../../llvm -DLLVM_INCLUDE_DOCS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_TOOLS=ON -DLLVM_BUILD_TOOLS=ON -DLLVM_ENABLE_ZLIB=OFF -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86')
        shell('touch is_configured')

    if subprocess.call([make, '-j%d' % CPU_COUNT]):
        shell('%s -j%d' % (make, CPU_COUNT))

    mkdir_p(qt)
    os.chdir(qt)
    if not exists('is_configured'):
        shell('../../qt/configure %s' % qt_config('posix', '--prefix=%s' % qt))
        shell('touch is_configured')

    if subprocess.call([make, '-j%d' % CPU_COUNT]):
        shell('%s -j%d' % (make, CPU_COUNT))

    mkdir_p(app)
    os.chdir(app)
    shell('rm -f bin/*')
    os.environ['RASMUS_VERSION'] = version
    shell('cmake ../../.. -DCMAKE_BUILD_TYPE=Release -DRASMUS_TESTS=OFF -DLLVM_CONFIG_EXECUTABLE=%s/bin/llvm-config -DQT_QMAKE_EXECUTABLE=%s/bin/qmake'%(llvm, qt))

    shell('%s -j%d' % (make, CPU_COUNT))

    # os.chdir(basedir)
    # shell('tar -c -v -f ../rasmus-%s_local-%s.tar rasmus-%s/' % (version, platform.node(), version))
    # shell('xz --compress --force --verbose -9 ../rasmus-%s_local-%s.tar' % (version, platform.node()))

# --------------------------------------------------------------- OS X

def check_osx(config):
    if not platform.system() == 'Darwin' or not platform.mac_ver()[0]:
        error('This can only be run on a OS X system!')

    if not get_output('xcode-select', '--print-path'):
        error('Xcode is not installed, aborting.')

    if not get_output('which', 'fpm'):
        error('Please install fpm by running "sudo gem install fpm --no-ri --no-rdoc"')

def build_osx(config, basedir):
    version, simple_version = get_version(basedir)
    build_deplibs(config, basedir)

    osxver    = platform.mac_ver()[0][:platform.mac_ver()[0].rindex('.')]
    framework = config.split('-')[1]
    if osxver == '10.6':
        osxcfg = '-%s -platform macx-g++42' % framework
    else:
        osxcfg = '-%s -platform unsupported/macx-clang' % framework

    flags = ''
    if framework == 'carbon' and osxver != '10.6':
        for item in ['CFLAGS', 'CXXFLAGS']:
            flags += '"QMAKE_%s += %s" ' % (item, '-fvisibility=hidden -fvisibility-inlines-hidden')

    def get_dir(name):
        return os.path.join(basedir, config, name)

    configure_args = qt_config('osx', osxcfg,
        '--prefix=%s'   % get_dir('qt'),
        '-I %s/include' % get_dir('deplibs'),
        '-L %s/lib'     % get_dir('deplibs'))

    rmdir(get_dir('dist'))
    rmdir(get_dir('pkg'))
    mkdir_p(get_dir('qt'))
    mkdir_p(get_dir('app'))
    mkdir_p(get_dir('pkg'))

    os.chdir(get_dir('qt'))
    if not exists('is_configured'):
        shell('../../../qt/configure %s' % configure_args)
        shell('touch is_configured')

    shell('make -j%d' % CPU_COUNT)

    os.chdir(get_dir('app'))
    shell('rm -f bin/*')
    os.environ['RASMUS_VERSION'] = version
    shell('../qt/bin/qmake %s ../../../rasmus.pro' % flags)
    shell('make -j%d' % CPU_COUNT)

    if osxver not in ['10.6', '10.7']:
        for item in ['rasmus', 'rasmus', 'librasmus.%s.dylib' % simple_version]:
            shell(' '.join([
                'install_name_tool', '-change',
                '/System/Library/Frameworks/CoreText.framework/Versions/A/CoreText',
                '/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/CoreText.framework/C7oreText',
                'bin/'+item]))

    shell('make install INSTALL_ROOT=%s' % get_dir('dist'))

    def _osx_tar(info):
        info.uid   = info.gid   = 0
        info.uname = 'root'
        info.gname = 'wheel'
        return info

    # create tarball for application and copy xz
    os.chdir(get_dir('dist'))
    with tarfile.open('../pkg/app.tar', 'w') as tar:
        tar.add('.', './', filter=_osx_tar)
    xz = os.path.join(get_dir('deplibs'), 'bin', 'xz')
    shell('%s --compress --force --verbose -9 ../pkg/app.tar' % xz)
    shutil.copy(xz, '../pkg/')

    args, cfg = fpm_setup('osx')
    with open('../pkg/uninstall-rasmus', 'w') as f:
        os.chmod('../pkg/uninstall-rasmus', 0o755)
        f.write("""#!/bin/bash
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as sudo uninstall-rasmus" 1>&2
   exit 1
fi
cd /usr/local
if which pkgutil >/dev/null; then
    pkgutil --forget %s.%s
fi
""" % (cfg['--osxpkg-identifier-prefix'], cfg['--name']))
        for root, dirs, files in os.walk(get_dir('dist')):
            for file in files:
                f.write('echo REMOVE /usr/local/%(name)s && rm -f %(name)s\n' % { 'name': os.path.relpath(os.path.join(root, file)) })
        f.write('echo REMOVE /usr/local/include/rasmus && rm -df /usr/local/include/rasmus\n')
        f.write('echo REMOVE /usr/local/bin/uninstall-rasmus && rm -f /usr/local/bin/uninstall-rasmus')

    open('../extract.sh', 'w').write("""#!/bin/bash
TGTDIR=/usr/local
BASEDIR=%s
cd $BASEDIR
./xz --decompress app.tar.xz
cd $TGTDIR
tar xf $BASEDIR/app.tar
mv $BASEDIR/uninstall-rasmus $TGTDIR/bin
rm -fr $BASEDIR
""" % cfg['--prefix'])

    os.chdir(os.path.join(basedir, config))
    shell('fpm --force %s --package ../%s-%s_%s.pkg .' % (args.replace('$1', version), cfg['--name'], version, config))

# --------------------------------------------------------------- command line

def usage(exit_code=2):
    message("Usage: scripts/build.py <target> [-clean] [-debug]\n\nThe supported targets are:\n")
    opts = list(BUILDERS.keys())
    opts.sort()
    for opt in opts:
        message('* %s\n' % opt)
    sys.exit(exit_code)

def main():
    rootdir = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..'))
    basedir = os.path.join(rootdir, 'static-build')

    if len(sys.argv) == 1:
        usage(0)

    config = sys.argv[1]
    if config not in BUILDERS:
        usage()

    for arg in sys.argv[2:]:
        if not arg in ['-clean', '-debug']:
            usage()

    final_config = config
    if '-debug' in sys.argv[2:]:
        final_config += '-dbg'
        QT_CONFIG['common'].extend(['remove:-release', 'remove:-webkit', '-debug', '-webkit-debug'])

    if '-clean' in sys.argv[2:]:
        rmdir(os.path.join(basedir, config))

    mkdir_p(basedir)
    if not os.path.isdir(os.path.join(basedir, "qt")):
        download_tarball(QT_SOURCE['url'], QT_SOURCE['sha1'], basedir, "qt")

    if not os.path.isdir(os.path.join(basedir, "llvm")):
        download_tarball(LLVM_SOURCE['url'], LLVM_SOURCE['sha1'], basedir, "llvm")

    os.chdir(rootdir)
    globals()['check_%s' % BUILDERS[config]](final_config)
    globals()['build_%s' % BUILDERS[config]](final_config, basedir)

if __name__ == '__main__':
    main()
