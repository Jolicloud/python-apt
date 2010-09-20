import os
import unittest



if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    print os.getcwd()

    for path in os.listdir('.'):
        if path.endswith('.py'):
            exec 'from %s import *' % path[:-3]
    unittest.main()
