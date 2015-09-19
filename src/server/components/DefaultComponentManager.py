from server import RPC

# Auxiliary functions
def DefaultFieldValue(typename):
    if typename == 'int' or typename == 'float':
        return 0
    elif typename == 'v3f':
        return [0,0,0]
    elif typename == 'v2f':
        return [0,0]
    pass

# TODO investigate the possibility of generating this dict automatically
# TODO if the "type" field is to be kept, then transform this into an array to avoid repeating it in the dict key
_defaultComponents = {
    'camera': {
        'id': 0,
        'type': 'camera', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Camera.hpp',
        'fields': {
            'near': 1.0,
            'far': 1000.0,
            'width': 1.0,
            'priority': 0.0
        }
    },
    'mesh': {
        'id': 1,
        'type': 'mesh', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Mesh.hpp',
        'fields': {
            'mesh': 'default_assets/default_components/primitive_meshes/Cube.lmf',
        }
    },
    'mesh_renderer': {
        'id': 2,
        'type': 'mesh_renderer', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/MeshRenderer.hpp',
        'fields': { }
    },
    'light': {
        'id': 3,
        'type': 'light', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Light.hpp',
        'fields': {
            'color': 0xffffff
        }
    }
}

def getDefaultComponents(evData=None):
    return _defaultComponents

RPC.listen(getDefaultComponents)
