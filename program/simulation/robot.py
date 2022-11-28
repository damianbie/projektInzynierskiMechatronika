class Robot:
    color = 0, 127, 255
    def __init__(self, radius, speed = (10, 10)):
        self._speed         = speed
        self._isSimulating  = False
        self._tileSize      = (0, 0)
        self._pos           = (0, 0)
        self._radius        = radius
        self._startCords    = (0, 0)
        self._path          = []
        
    def setStartCords(self, startCords):
        self._startCords = startCords

    def draw(self, _wnd):
        s = 0

    def startSimulation(self):
        self._isSimulating = True
    
    def setTileSize(self, tileSize):
        self._tileSize = tileSize

    def setPath(self, path):
        self._path = path

    def update(self, timeStep):
        if self._isSimulating == False:
            return
        
        