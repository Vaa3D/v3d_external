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
        # Hack to get around defect in pyuic4
        _ql = QtGui.QLineEdit
        if not hasattr(_ql, 'setPlaceholderText'):
            def foo(x,y):
                pass
            _ql.setPlaceholderText = foo
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
        iconPath = os.path.join(os.path.dirname(movie_maker.__file__), 'icons')
        self.playIcon = QtGui.QIcon(os.path.join(iconPath, "play.png"))
        self.recordIcon = QtGui.QIcon(os.path.join(iconPath, "record.png"))
        self.pauseIcon = QtGui.QIcon(os.path.join(iconPath, "pause.png"))
        self.reelIcon = QtGui.QIcon(os.path.join(iconPath, "film_reel.png"))
        self.skipBackIcon = QtGui.QIcon(os.path.join(iconPath, "skip_backward.png"))
        self.skipAheadIcon = QtGui.QIcon(os.path.join(iconPath, "skip_ahead.png"))
        self.setWindowIcon(self.reelIcon)
        self.ui.frameCartoonLabel.hide() # its' just a placeholder
        self.beginningButton = self.ui.playButtonBox.button(QtGui.QDialogButtonBox.Reset)
        self.beginningButton.setText('Beginning')
        # self.beginningButton = self.ui.buttonBox.addButton('Beginning', QtGui.QDialogButtonBox.ActionRole)
        self.beginningButton.setIcon(self.skipBackIcon)
        self.beginningButton.setEnabled(False)
        self.connect(self.beginningButton, QtCore.SIGNAL('clicked()'),
                     self.on_beginning_pressed)
        # self.playButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply)
        # self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply).hide()
        self.playButton = self.ui.playButtonBox.addButton('Play', QtGui.QDialogButtonBox.ActionRole)
        self.playButton.setText("Play")
        self.playButton.setIcon(self.playIcon)
        self.connect(self.playButton, QtCore.SIGNAL('clicked()'),
               self.on_play_pause_pressed)
        self.endButton = self.ui.playButtonBox.addButton('End', QtGui.QDialogButtonBox.ActionRole)
        self.endButton.setIcon(self.skipAheadIcon)
        self.endButton.setEnabled(False)
        self.connect(self.endButton, QtCore.SIGNAL('clicked()'),
                     self.on_end_pressed)
        self.saveImagesButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Save)
        self.saveImagesButton.setText("Save images...")
        self.connect(self.saveImagesButton, QtCore.SIGNAL('clicked()'),
               self.save_images)
        self.saveParametersButton = self.ui.buttonBox.addButton('Save', QtGui.QDialogButtonBox.ApplyRole)
        self.connect(self.saveParametersButton, QtCore.SIGNAL('clicked()'),
               self.save_parameters)
        # deleteAllButton used to be a generic button
        self.ui.deleteAllButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Reset)
        self.ui.deleteAllButton.setText('Delete all frames')
        self.connect(self.ui.deleteAllButton, QtCore.SIGNAL('clicked()'),
               self.delete_all)
        self.previous_save_dir = ""
        self._updateFrameCount()
        # Frame interval
        interval_validator = QtGui.QDoubleValidator(0.00, 10000.0, 2, 
                self.ui.frameIntervalLineEdit)
        self.ui.frameIntervalLineEdit.setValidator(interval_validator)
        self.connect(self.ui.frameIntervalLineEdit, QtCore.SIGNAL('textChanged(QString)'),
                     self.update_frame_interval)
        self.frame_interval = 2.5
        self.ui.frameIntervalLineEdit.setText(str(self.frame_interval))
        self.loadButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Open)
        self.connect(self.loadButton, QtCore.SIGNAL('clicked()'),
                     self.load_parameters)
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
        self._updateFrameCount()
        self._enter_state('ready') # stop current animation

    # There are three states - ready, playing, and paused
    def _enter_state(self, state):
        # Ready to play from start
        self.play_state = state
        if 'ready' == state:
            self.play_generator = None
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self._updateFrameCount() # enable play button?
        elif 'playing' == state:
            if None == self.play_generator:
                self.play_generator = self.movie.generate_play_frames()
            self.playButton.setText('Pause')
            self.playButton.setIcon(self.pauseIcon)
            self.endButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
        elif 'paused' == state:
            assert None != self.play_generator
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self.endButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
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

    def on_beginning_pressed(self):
        self.movie.generate_play_frames().next()
        self._enter_state('ready')
        
    def on_end_pressed(self):
        self.movie.generate_final_frame_view()
        self._enter_state('ready')

    def save_images(self):
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

    def save_parameters(self):
        fname = QtGui.QFileDialog.getSaveFileName(
                    self,
                    "Choose file to save frame parameters in",
                    self.previous_save_dir,
                    "V3D movie files (*.vmv)")[0]
        if None == fname: return
        if len(fname) < 1: return
        fname = str(fname)
        # Note: Qt dialog already asks for confirmation.
        dir = os.path.dirname(fname)
        self.previous_save_dir = dir
        file_object = open(fname, 'w')
        self.movie.save_parameter_file(file_object)
        file_object.close()
        answer = QtGui.QMessageBox.information(self, "Parameters saved", 
             "Finished saving movie parameters file")
        self.show() # why does window get hidden in this method?

    def load_parameters(self):
        fname = QtGui.QFileDialog.getOpenFileName(
                        self,
                        "Open V3D movie parameters file",
                        self.previous_save_dir,
                        "V3D movie files (*.vmv)")[0]
        if None == fname: return
        if len(fname[0]) < 1: return
        if not os.path.exists(fname): 
            QtGui.QMessageBox.information(this, "No such file", "No such file")
            return
        file_object = open(fname, 'r')
        self.movie.load_parameter_file(file_object)
        self._updateFrameCount()
        self.show() # why does window get hidden in this method?
        
    def delete_all(self):
        answer = QtGui.QMessageBox.question(self, "Confirm clear movie", 
                 "Really erase all key-frames?", 
                 QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, 
                 QtGui.QMessageBox.No)
        if answer == QtGui.QMessageBox.Yes:
            self.movie.key_frames = []
            self._updateFrameCount()
            self._enter_state('ready')

    def _updateFrameCount(self):
        nframes = len(self.movie.key_frames)
        if nframes == 0:
            self.ui.keyFrameLabel.setText("No key frames added")
            self.playButton.setEnabled(False)
            self.beginningButton.setEnabled(False)
            self.endButton.setEnabled(False)
            self.saveImagesButton.setEnabled(False)
            self.saveParametersButton.setEnabled(False)
            self.ui.deleteAllButton.setEnabled(False)
            self.ui.frameIntervalLineEdit.setEnabled(False)
        elif nframes == 1:
            self.ui.keyFrameLabel.setText("One key frame added")
            self.playButton.setEnabled(False)
            self.beginningButton.setEnabled(True)
            self.endButton.setEnabled(True)
            self.saveImagesButton.setEnabled(False)
            self.saveParametersButton.setEnabled(True)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalLineEdit.setEnabled(True)
        else:
            self.ui.keyFrameLabel.setText("%d key frames added" % nframes)
            self.playButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
            self.endButton.setEnabled(True)
            self.saveImagesButton.setEnabled(True)
            self.saveParametersButton.setEnabled(True)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalLineEdit.setEnabled(True)

if __name__ == '__main__':
    movie_gui = MovieGui()
    movie_gui.show()
