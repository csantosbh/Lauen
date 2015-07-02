import os, sys, json, collections

defaults={
  'server': {
    'ws_port': '9001', # server
    'http_port': '9002', # server
  },
  'project': {
    'recent_project_history': 5, # project
  },
  'runtime': {
    'recent_projects': [], # runtime
  },
  'export': {
    'win_compilers': { # export
        'g++': '/opt/mingw64/bin/x86_64-w64-mingw32-g++.exe',
    },
    'nacl': {
        'pepper_folder': '/home/csantos/workspace/nacl_sdk/pepper_41',
        'compiler': 'toolchain/linux_pnacl/bin/pnacl-clang++'
    },
    'third_party_folder': '/home/csantos/workspace/LauEngine/third_party'
  }
}

config = defaults

def setDefaults(defaults, preservedData):
    for k, v in preservedData.iteritems():
        if isinstance(v, collections.Mapping):
            r = setDefaults(defaults.get(k, {}), v)
            defaults[k] = r
        else:
            defaults[k] = preservedData[k]
    return defaults

def _initialize():
    try:
        user_config = json.loads(open(os.path.expanduser('~/.laurc'), 'r').read())
    except IOError:
        user_config = {}
        pass
    config = setDefaults(defaults, user_config)
    open(os.path.expanduser('~/.laurc'), 'w').write(json.dumps(config, indent=2))
    pass

def get(section, field):
    return config[section][field]

def set(section, field, value):
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
