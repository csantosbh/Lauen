import ConfigParser, os

config = ConfigParser.ConfigParser()
config.read('../config/config.cfg')

def get(section, field):
    return config.get(section, field)
