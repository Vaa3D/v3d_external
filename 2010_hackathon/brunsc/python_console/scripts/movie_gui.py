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

class MovieGui(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.ui = Ui_movie_dialog()
        self.ui.setupUi(self)
        self.movie = movie_maker.V3dMovie()
        self.connect(self.ui.addCurrentViewButton, QtCore.SIGNAL('clicked()'), 
               self.append_view)
        self.playButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply)
        self.playButton.setText("Play")
        iconPath = os.path.join(os.path.dirname(movie_maker.__file__), 'icons')
        self.playIcon = QtGui.QIcon(os.path.join(iconPath, "play.png"))
        self.recordIcon = QtGui.QIcon(os.path.join(iconPath, "record.png"))
        self.pauseIcon = QtGui.QIcon(os.path.join(iconPath, "pause.png"))
        self.reelIcon = QtGui.QIcon(os.path.join(iconPath, "film_reel.png"))
        self.skipBackIcon = QtGui.QIcon(os.path.join(iconPath, "skip_backward.png"))
        self.setWindowIcon(self.reelIcon)
        self.playButton.setIcon(self.playIcon)
        self.connect(self.playButton, QtCore.SIGNAL('clicked()'),
               self.on_play_pause_pressed)
        self.resetButton = self.ui.buttonBox.addButton('Reset', QtGui.QDialogButtonBox.ActionRole)
        self.resetButton.setIcon(self.skipBackIcon)
        self.resetButton.setEnabled(False)
        self.connect(self.resetButton, QtCore.SIGNAL('clicked()'),
                     self.on_reset_pressed)
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
        #
        self._enter_state('ready')

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
        self._enter_state('ready') # stop current animation

    # There are three states - ready, playing, and paused
    def _enter_state(self, state):
        # Ready to play from start
        self.play_state = state
        if 'ready' == state:
            self.play_generator = None
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self.resetButton.setEnabled(False)
            self.updateKeyFrameLabel() # enable play button?
        elif 'playing' == state:
            if None == self.play_generator:
                self.play_generator = self.movie.generate_play_frames()
            self.playButton.setText('Pause')
            self.playButton.setIcon(self.pauseIcon)
            self.resetButton.setEnabled(True)
        elif 'paused' == state:
            assert None != self.play_generator
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self.resetButton.setEnabled(True)
        else:
            assert(False)
        
    def on_play_pause_pressed(self):
        if ('ready' == self.play_state) or ('paused' == self.play_state): # play
            self._enter_state('playing')
            for n in self.play_generator:
                QtGui.QApplication.processEvents()
                if 'playing' != self.play_state:
                    return
            self._enter_state('ready')
        elif 'playing' == self.play_state:
            self._enter_state('paused')
        else: # stop
            assert(False)

    def on_reset_pressed(self):
        if 'ready' == self.play_state:
            pass # should not happen, but whatever...
        elif ('playing' == self.play_state) or ('paused' == self.play_state):
            self.movie.generate_play_frames().next()
            self._enter_state('ready')
        else:
            assert(False)
        
    def save(self):
        dir = QtGui.QFileDialog.getExistingDirectory(
                    self,
                    "Choose directory to save frame files in",
                    self.previous_save_dir)
        if os.path.exists(dir):
            self.previous_save_dir = dir
            for n in self.movie.generate_write_frames(directory=str(dir)):
                QtGui.QApplication.processEvents()
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
            self._enter_state('ready')

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

if __name__ == '__main__':
    movie_gui = MovieGui()
    movie_gui.show()
