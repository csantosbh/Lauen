from server import RPC

# TODO investigate the possibility of generating this dict automatically
# TODO if the "type" field is to be kept, then transform this into an array to avoid repeating it in the dict key
_defaultComponents = {
    'transform': {
        'id': 0,
        'type': 'transform', # TODO investigate the cost of dropping component string ids (which is what the type actually is)
        'path': 'default_components/Transform.hpp',
        'full_class_name': 'lau::Transform'
    }
}

def getDefaultComponents(evData=None):
    return _defaultComponents

RPC.listen(getDefaultComponents)
