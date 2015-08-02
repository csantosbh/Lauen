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
    'transform': {
        'id': 0,
        'type': 'transform', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Transform.hpp',
        'full_class_name': 'lau::Transform',
        'fields': {
            'position': [0,0,0],
            'scale': [1,1,1],
            'rotation': [0,0,0]
        }
    },
    'mesh': {
        'id': 1,
        'type': 'mesh', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Mesh.hpp',
        'full_class_name': 'lau::Mesh',
        'fields': {
            'mesh': 'default_components/primitive_meshes/Cube.lmf',
        }
    }
}

def getDefaultComponents(evData=None):
    return _defaultComponents

RPC.listen(getDefaultComponents)
