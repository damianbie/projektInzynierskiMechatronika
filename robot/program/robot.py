import sys
import socket
import pygame as pg
import pygame_gui as pgg

class App:
    def __init__(self):
        pg.init()
        self._res = (800, 400)
        self._wnd = pg.display.set_mode(self._res)
        pg.display.set_caption("RobocikKontroller")
        
        self._robotIp           = "192.168.0.102"
        self._robotPort         = "3333"
        self._robotSocket       = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._robotSocketFlag   = False
        
        self._guiManager        = pgg.UIManager(self._res)
        self._uiControlsPanel   = pgg.elements.UIPanel(relative_rect=pg.Rect((0, 0), (400, 400)), manager=self._guiManager, starting_layer_height=0)
        
        self._btnLeft           = pgg.elements.UIButton(relative_rect=pg.Rect((25, 60), (120, 40)), manager=self._guiManager, text="lewo", container=self._uiControlsPanel)
        self._btnForward        = pgg.elements.UIButton(relative_rect=pg.Rect((145, 10), (120, 40)), manager=self._guiManager, text="Do przodu", container=self._uiControlsPanel)
        self._btnRight          = pgg.elements.UIButton(relative_rect=pg.Rect((245, 60), (120, 40)), manager=self._guiManager, text="prawo", container=self._uiControlsPanel)
        self._btnBackward       = pgg.elements.UIButton(relative_rect=pg.Rect((135, 110), (120, 40)), manager=self._guiManager, text="Do tyłu", container=self._uiControlsPanel)
        self._btnStop           = pgg.elements.UIButton(relative_rect=pg.Rect((157, 60), (80, 40)), manager=self._guiManager, text="Stop", container=self._uiControlsPanel)
        
        self._textRobotIp       = pgg.elements.UITextEntryBox(relative_rect=pg.Rect((20, 200), (340, 40),), manager=self._guiManager, container=self._uiControlsPanel, initial_text=self._robotIp)
        self._textRobotPort     = pgg.elements.UITextEntryBox(relative_rect=pg.Rect((20, 280), (340, 40),), manager=self._guiManager, container=self._uiControlsPanel, initial_text=self._robotPort)
        self._btnConnect        = pgg.elements.UIButton(relative_rect=pg.Rect((20, 330), (200, 40)), manager=self._guiManager, text="Polacz robota", container=self._uiControlsPanel)
        
        
        self._uiParamsPanel     = pgg.elements.UIPanel(relative_rect=pg.Rect((400, 0), (400, 400)), manager=self._guiManager, starting_layer_height=0)

            
    def _connectWithRobot(self):
        print("connecting with robot...")
        self._robotIp   = self._textRobotIp.get_text()
        self._robotPort = self._textRobotPort.get_text()
        
        if self._robotSocketFlag:
            self._robotSocket.close()
            
        try:
            self._robotSocketFlag = True
            self._robotSocket.connect((self._robotIp, int(self._robotPort)))
        except Exception as err:
            print("Nie mozna polaczyc z robotem!!")
            return
        
        self._sendCommandToRobot("setMode manual")
        print("Podlaczono!!!")
           
        
    def _sendCommandToRobot(self, cmd):
        if self._robotSocketFlag is False:
            print("Robot nie podlaczony!!")
        
        
        try:
            self._robotSocket.send(str.encode(cmd))
        except OSError as err:
            print("Robot nie podlaczony!!!")
            self._robotSocket.close()
            self._robotSocketFlag = False
            return
        
    def _processEvents(self, ev):
        if ev.type == pg.QUIT:
            sys.exit()
        elif ev.type == pg.KEYDOWN and self._textRobotIp.is_focused is not True and self._textRobotPort.is_focused is not True:
            if ev.unicode == 'a':
                self._sendCommandToRobot("turnLeft")
            elif ev.unicode == 'd':
                self._sendCommandToRobot("turnRight")
            elif ev.unicode == 'w':
                self._sendCommandToRobot("forward")
            elif ev.unicode == 's':
                self._sendCommandToRobot("backwards")
        elif ev.type == pg.KEYUP and self._textRobotIp.is_focused is not True and self._textRobotPort.is_focused is not True:
            self._sendCommandToRobot("noMovment")
        
    def _processGUI(self, ev):
        if ev.type == pgg.UI_BUTTON_PRESSED:
            if ev.ui_element == self._btnLeft:
                self._sendCommandToRobot("turnLeft")
            elif ev.ui_element == self._btnRight:
                self._sendCommandToRobot("turnRight")
            elif ev.ui_element == self._btnForward:
                self._sendCommandToRobot("forward")
            elif ev.ui_element == self._btnBackward:
                self._sendCommandToRobot("backwards")
            elif ev.ui_element == self._btnStop:
                self._sendCommandToRobot("noMovment")
            elif ev.ui_element == self._btnConnect:
                self._connectWithRobot()

    def main(self):
        clock = pg.time.Clock()
        while True:
            deltaTime = clock.tick(60)/1000
            
            for ev in pg.event.get():
                self._processEvents(ev)
                self._guiManager.process_events(ev)
                self._processGUI(ev)
            
            self._wnd.fill((255,255,255))
    
            self._guiManager.update(deltaTime)        
            self._guiManager.draw_ui(self._wnd)
            pg.display.update()
                

if __name__ == "__main__":
    app = App()
    app.main()
        