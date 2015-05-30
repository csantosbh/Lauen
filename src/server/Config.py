import os, sys, json, collections

defaults=dict(
  DEFAULT=dict(
    port='8080',
    recent_project_history=5,
    recent_projects=[],
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
    'install_location': os.path.dirname(os.path.realpath(sys.argv[0]))
}
def env(section):
    return _env[section]

_initialize()
