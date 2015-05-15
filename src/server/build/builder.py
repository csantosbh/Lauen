from server import Event

# TODO: Move the build tools to a separate file
cxx_flags=''
link_flags='-rdynamic -lglfw3 -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp'
cxx_compiler='g++'

def build_game(event_msg):
    from subprocess import call
    call(cxx_compiler + ' ' + link_flags + ' assets/standard/main.cpp -o game', shell=True)
    pass

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

Event.listen('build', build_game)
