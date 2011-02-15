import movie_maker
import os

# User needs to install either PyQt4 or PySide to get GUI
try:
    from PySide import QtGui, QtCore
    from ui_movie_maker_dialog_pyside import Ui_movie_dialog
    # import PySide as PyQt4
except ImportError:
    try:
        from PyQt4 import QtGui, QtCore
        from ui_movie_maker_dialog_pyqt4 import Ui_movie_dialog
    except ImportError:
        print """
To use V3D Movie GUI, you must install either PySide or PyQt4.
    http://www.pyside.org/
      OR
    http://www.riverbankcomputing.co.uk/software/pyqt/download
            """
        raise

class MovieGui2(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.ui = Ui_movie_dialog()
        self.ui.setupUi(self)
        self.movie = movie_maker.V3dMovie()
        self.connect(self.ui.addCurrentViewButton, QtCore.SIGNAL('clicked()'), 
               self.append_view)
        self.playButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply)
        self.playButton.setText("Play")
        self.connect(self.playButton, QtCore.SIGNAL('clicked()'),
               self.play)
        self.saveButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Save)
        self.saveButton.setText("Save Frames...")
        self.connect(self.saveButton, QtCore.SIGNAL('clicked()'),
               self.save)
        self.connect(self.ui.deleteAllButton, QtCore.SIGNAL('clicked()'),
               self.delete_all)
        self.previous_save_dir = ""
        self.updateKeyFrameLabel()
        # Frame interval
        interval_validator = QtGui.QDoubleValidator(0.00, 10000.0, 2, 
                self.ui.frameIntervalLineEdit)
        self.ui.frameIntervalLineEdit.setValidator(interval_validator)
        self.connect(self.ui.frameIntervalLineEdit, QtCore.SIGNAL('textChanged(QString)'),
                     self.update_frame_interval)
        self.frame_interval = 2.5
        self.ui.frameIntervalLineEdit.setText(str(self.frame_interval))

    def update_frame_interval(self, value_text):
        self.frame_interval = float(value_text)
        
    def append_view(self):
        # Perhaps there was no 3D viewer when the MovieGui was launched
        try:
            self.movie.append_current_view(interval=self.frame_interval)
        except ValueError:
            self.movie = movie_maker.V3dMovie()
            self.movie.append_current_view(interval=self.frame_interval)
        self.updateKeyFrameLabel()

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
            answer = QtGui.QMessageBox.information(self, "Frames saved", 
                 "Finished writing movie frames")
        self.show() # why does window get hidden in this method?

    def delete_all(self):
        answer = QtGui.QMessageBox.question(self, "Confirm clear movie", 
                 "Really erase all key-frames?", 
                 QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, 
                 QtGui.QMessageBox.No)
        if answer == QtGui.QMessageBox.Yes:
            self.movie.key_frames = []
            self.updateKeyFrameLabel()

    def updateKeyFrameLabel(self):
        nframes = len(self.movie.key_frames)
        if nframes == 0:
            self.ui.keyFrameLabel.setText("No key frames added")
            self.playButton.setEnabled(False)
            self.saveButton.setEnabled(False)
            self.ui.deleteAllButton.setEnabled(False)
            self.ui.frameIntervalLineEdit.setEnabled(False)
        elif nframes == 1:
            self.ui.keyFrameLabel.setText("One key frame added")
            self.playButton.setEnabled(False)
            self.saveButton.setEnabled(False)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalLineEdit.setEnabled(True)
        else:
            self.ui.keyFrameLabel.setText("%d key frames added" % nframes)
            self.playButton.setEnabled(True)
            self.saveButton.setEnabled(True)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalLineEdit.setEnabled(True)

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
    movie_gui = MovieGui2()
    movie_gui.show()
