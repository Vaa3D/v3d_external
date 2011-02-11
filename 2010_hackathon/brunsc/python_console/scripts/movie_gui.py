import movie_maker
import os

# User needs to install either PyQt4 or PySide to get GUI
try:
    from PyQt4 import QtGui, QtCore
except ImportError:
    from PySide import QtGui, QtCore

class MovieGui(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.movie = movie_maker.V3dMovie()
        self.setGeometry(300, 300, 250, 150)
        self.setWindowTitle('V3D Keyframe Movie Maker')
        self.add_buttons()
        self.previous_save_dir = ""
        
    def add_buttons(self):
        intro = QtGui.QLabel("""
            Adjust the V3D '3D View' to the orientation you want to see
            in your movie.  Then press 'Add current view'.  Keep changing
            the view and pressing 'Add current view' until you have 
            included all of the key frames of your movie.  Press 'Play'
            to preview your movie.  After you are satisfied with your 
            movie, press 'Save' to write your movie frames to disk.  
            Converting frames to a movie file is an exercise left to 
            the user...
            """)
        append = QtGui.QPushButton('1) Add current view')
        append.setToolTip("Press to add the current V3D 3D view to this movie")
        self.connect(append, QtCore.SIGNAL('clicked()'), 
                     self.append_view)
        play = QtGui.QPushButton('2) Play movie')
        play.setToolTip("Press to animate your movie without saving")
        self.connect(play, QtCore.SIGNAL('clicked()'),
                     self.play)
        save = QtGui.QPushButton('3) Save frames to disk...')
        save.setToolTip("Press to save your movie frames to disk")
        self.connect(save, QtCore.SIGNAL('clicked()'),
                     self.save)
        vbox = QtGui.QVBoxLayout()
        vbox.addStretch(1)
        vbox.addWidget(intro)
        vbox.addWidget(append)
        vbox.addWidget(play)
        vbox.addWidget(save)
        self.setLayout(vbox)
        
    def append_view(self):
        # Perhaps there was no 3D viewer when the MovieGui was launched
        try:
            self.movie.append_current_view()
        except ValueError:
            self.movie = movie_maker.V3dMovie()
            self.movie.append_current_view()

    def play(self):
        self.movie.play()
        
    def save(self):
        dir = QtGui.QFileDialog.getExistingDirectory(
                    self,
                    "Choose directory to save frame files in",
                    self.previous_save_dir)
        if os.path.exists(dir):
            self.previous_save_dir = dir
            self.movie.write_frames(directory=str(dir))

if __name__ == '__main__':
    movie_gui = MovieGui()
    movie_gui.show()
