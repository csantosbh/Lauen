from server import RPC

# TODO investigate the possibility of generating this dict automatically
_defaultComponents = {
    'transform': {
        'id': 0,
        'path': 'default_components/Transform.hpp',
        'full_class_name': 'lau::Transform'
    }
}

def getDefaultComponents(evData=None):
    return _defaultComponents

RPC.listen(getDefaultComponents)
