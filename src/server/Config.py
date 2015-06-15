import os, sys, json, collections

defaults=dict(
  DEFAULT=dict(
    ws_port='9001', # server
    http_port='9002', # server
    recent_project_history=5, # project
    recent_projects=[], # runtime
    win_compilers={ # export
        'g++': '/opt/mingw64/bin/x86_64-w64-mingw32-g++.exe',
    },
    nacl={
        'pepper_folder': '/home/csantos/workspace/nacl_sdk/pepper_41',
        'compiler': 'toolchain/linux_pnacl/bin/pnacl-clang++'
    }
  )
)

config = defaults

def _update(d, u):
    for k, v in u.iteritems():
        if isinstance(v, collections.Mapping):
            r = _update(d.get(k, {}), v)
            d[k] = r
        else:
            d[k] = u[k]
    return d

def _initialize():
    try:
        user_config = json.loads(open(os.path.expanduser('~/.laurc'), 'r').read())
    except IOError:
        user_config = {}
        pass
    config = _update(defaults, user_config)
    open(os.path.expanduser('~/.laurc'), 'w').write(json.dumps(config, indent=2))
    pass

def _sanitizeRequest(section, field):
    if not config.has_key(section):
        config[section] = dict()
        pass
    if not config[section].has_key(section):
        config[section][field] = config['DEFAULT'][field]
        pass
    pass

def get(section, field):
    _sanitizeRequest(section, field)
    return config[section][field]

def set(section, field, value):
    _sanitizeRequest(section, field)
    config[section][field] = value
    open(os.path.expanduser('~/.laurc'), 'w').write(json.dumps(config, indent=2))
    pass

_env={
    'install_location': os.path.dirname(os.path.realpath(sys.argv[0])),
    'asset_extensions': ['.vs', '.fs']
}
def env(section):
    return _env[section]

_initialize()
