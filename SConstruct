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


env.Command("configure.h",
            ["SConstruct"],
            create_version)
    
if ARGUMENTS.get('mingw', 0):
    env['CC']='/usr/local/mingw32/bin/mingw32-gcc'
    env['CXX']='/usr/local/mingw32/bin/mingw32-g++'
    env['AR']='/usr/local/mingw32/bin/mingw32-ar'
    env['RANLIB']='/usr/local/mingw32/bin/mingw32-ranlib'
    env['PKGCONFIG'] = "PKG_CONFIG_PATH=/usr/local/mingw32/lib/pkgconfig pkg-config"
    env['OBJSUFFIX']=".obj"
    env['PROGSUFFIX'] = ".exe"
    env['CROOT'] = "z:\\dosc"
    env['PREFIX'] = "z:\\usr\\local\\mingw32"
    env['PKGGTKSOURCEVIEW'] = "gtksourceview-1.0"

    env.Command("gemtcl.wine.nsi",
                ["gemtcl.nsi.in",
                 "SConstruct"
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
                ["src/gemtcl.exe", "gemtcl.wine.nsi"],
                ["wine \"${CROOT}\Program Files\NSIS\makensis.exe\" gemtcl.wine.nsi"])
    res = env.Command("gemtcl.res.o",
                      ["gemtcl.rc",
                       "gemtcl-logo.ico"
                      ],
                      ["/usr/local/mingw32/bin/mingw32-windres gemtcl.rc gemtcl.res.o"])

    env.Append(CPPDEFINES=['GTKSOURCEVIEW1'],
               CPPPATH=["/usr/local/mingw32/include"],
               LIBPATH=["/usr/local/mingw32/lib"],
               LIBS = ['tcl84']
               )
else:
    # Posix by default
    env['PKGCONFIG'] = "pkg-config"
    env['LIBPATH'] = []
    env['PKGGTKSOURCEVIEW'] = "gtksourceview-2.0"
    env.Append(LIBS = ['tcl'])

env.ParseConfig("${PKGCONFIG} --cflags --libs gtk+-2.0 gthread-2.0 libpcre ${PKGGTKSOURCEVIEW} libglade-2.0 gnet-2.0")

SConscript(['src/SConscript'],
           exports='env')
