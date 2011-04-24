import os
import re


env = Environment()
        
# Generate configure.h from configure.h to have a single source for
# software version whether we are running scons or automake
inp = open("configure.in")
for line in inp.readlines():
    m = re.search(r"AM_INIT_AUTOMAKE\(\$PACKAGE,\"(.*?)\"\)", line)
    if m:
        env['VER'] = m.group(1)
        break

# All purpose template filling routine
def template_fill(env, target, source):
    out = open(str(target[0]), "wb")
    inp = open(str(source[0]), "r")

    for line in inp.readlines():
        line = re.sub('@(.*?)@',
                      lambda x: env[x.group(1)],
                      line)
        out.write(line)
        
    out.close()
    inp.close()

# Generate configure.h
def create_version(env, target, source):
    out = open(str(target[0]), "wb")
    out.write("#define VERSION \"" + env['VER'] + "\"\n")
    out.close()

env.Command("config.h",
            ["SConstruct"],
            create_version)
    
if ARGUMENTS.get('mingw', 0):
    env['CC']='i686-pc-mingw32-gcc'
    env['SHCC']='i686-pc-mingw32-gcc'
    env['CXX']='i686-pc-mingw32-g++'
    env['SHCXX']='i686-pc-mingw32-g++'
    env['AR']='i686-pc-mingw32-ar'
    env['RANLIB']='i686-pc-mingw32-ranlib'
    env['ENV']['PKG_CONFIG_PATH'] = "/usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig"
    env['PKGCONFIG'] = "env PKG_CONFIG_PATH=/usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig:/usr/local/mingw32/lib/pkgconfig pkg-config"
    env['OBJSUFFIX']=".obj"
    env['SHLIBSUFFIX']=".dll"
    env['SHLIBPREFIX']=""
#    env['LIBSUFFIX']=".lib"
    env['PROGSUFFIX'] = ".exe"
    env['CROOT'] = "/home/dov/.wine/drive_c/"
    env['PREFIX'] = "\usr\i686-pc-mingw32\sys-root\mingw"
    env['DLLWRAP'] = "i686-pc-mingw32-dllwrap"
    env['DLLTOOL'] = "i686-pc-mingw32-dlltool"
    env['DLLWRAP_FLAGS'] = "--mno-cygwin --as=${AS} --export-all --driver-name ${CXX} --dll-tool-name ${DLLTOOL} -s"
    env.Append(CPPFLAGS= ['-mms-bitfields'])
    env['PKGGTKSOURCEVIEW'] = "gtksourceview-2.0"
    env['GLADESRC'] = "z:\\archive\\svnwork\\glade3"

    env.Command("gemtcl.wine.nsi",
                ["gemtcl.nsi.in",
                 "SConstruct",
                 "configure.in"
                ],
                template_fill
                )
    env.Command("gemtcl.rc",
                ["gemtcl.rc.in",
                 "SConstruct",
                 ],
                template_fill
                )
    
    env.Command("COPYING.dos",
                "COPYING",
                ["unix2dos < COPYING > COPYING.dos"])
    
    env.Command("InstallGemtcl-" + env['VER'] + ".exe",
                ["src",
                 "server-examples",
                 "gemtcl.wine.nsi"],
                ["makensis gemtcl.wine.nsi"])
    res = env.Command("gemtcl.res.o",
                      ["gemtcl.rc",
                       "misc/gemtcl-logo.ico"
                      ],
                      ["i686-pc-mingw32-windres gemtcl.rc gemtcl.res.o"])

    env.Append(CPPDEFINES=['GTKSOURCEVIEW1'],
               CPPPATH=["/usr/local/mingw32/include"],
               LIBPATH=["/usr/local/mingw32/lib"],
               LIBS = ['tcl85']
               )
else:
    # Posix by default
    env['PKGCONFIG'] = "pkg-config"
    env['LIBPATH'] = []
    env['PKGGTKSOURCEVIEW'] = "gtksourceview-2.0"
    env.Append(LIBS = ['tcl'])

env.Command("README",
            ["README.in",
             "SConstruct",
             ],
            template_fill
            )

print(env["PKGCONFIG"], "--cflags --libs gtk+-2.0 gthread-2.0 libpcre ", env["PKGGTKSOURCEVIEW"], "libglade-2.0 gnet-2.0")
env.ParseConfig("${PKGCONFIG} --cflags --libs gtk+-2.0 gthread-2.0 libpcre ${PKGGTKSOURCEVIEW} libglade-2.0 gnet-2.0")

SConscript(['src/SConscript',
            'doc/SConscript',
            'server-examples/SConscript'
            ],
           exports='env')
