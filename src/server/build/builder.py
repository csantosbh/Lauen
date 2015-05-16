from server import Event
from server import Project

# TODO: Move the build tools to a separate file
cxx_flags=''
link_flags='-rdynamic -lglfw3 -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp'
cxx_compiler='g++'

def build_game(event_msg):
    from subprocess import call
    # TODO: Put the project folder in its own module
    project_folder = Project.getProjectFolder()
    call(cxx_compiler + ' ' + project_folder+'/assets/default_assets/main.cpp -o '+project_folder+'/game' + ' ' + link_flags, shell=True)
    pass

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

Event.listen('build', build_game)
