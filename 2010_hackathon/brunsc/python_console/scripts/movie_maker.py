import time
import os

class Interpolator:
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None):
        """
        param_value_list is an ordered list of (parameter, value) pairs
        in which the parameter parameter is strictly increasing.
        Wrap is the cycle value for cyclic systems.  For example, wrap = 360 for
        angles in degrees.
        """
        raise NotImplementedError("Use a concrete derived class of Interpolator")


# TODO - create spline interpolator
# and quaternion interpolator for rotations
class LinearInterpolator(Interpolator):
    """
    Simple linear interpolator, so I can get things working quickly.
    Later we will use spline interpolation.
    """
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None):
        # Make sure our lookup table is up to the challenge
        if len(param_value_list) < 1:
            raise LookupError("No key values to interpolate from")
        if param < param_value_list[0][0]:
            raise ValueError("interpolator parameter value too small")
        if param > param_value_list[-1][0]:
            print param, param_value_list[-1][0]
            raise ValueError
        # Search for bounding keys
        if index_hint != None:
            below_index = index_hint
        else:
            # TODO - binary search
            raise NotImplementedError()
        while param_value_list[below_index][0] < param:
            below_index += 1
        while param_value_list[below_index][0] > param:
            below_index -= 1
        p1 = param_value_list[below_index][0]
        if param == p1:
            return param_value_list[below_index][1] # exact key frame
        p2 = param_value_list[below_index + 1][0]
        
        alpha = (param - p1) / (p2 - p1)
        val1 = param_value_list[below_index][1]
        val2 = param_value_list[below_index + 1][1]
        dv = val2 - val1
        if wrap != None:
            while dv > wrap / 2.0:
                dv -= wrap
            while dv < -wrap / 2.0:
                dv += wrap
        val = val1 + alpha * dv
        # print param, p1, p2, val1, val2, alpha
        return val


# TODO - include rotation and clipping
#      - And maybe eventually list of displayed objects
#        perhaps that list should be in a different object
class CameraPosition:
    def __init__(self, **kw_params):
        for param_name in kw_params:
            setattr(self, param_name, kw_params[param_name])
        

class V3dMovieFrame:
    """
    Any single frame of a V3D movie.
    
    Includes both key frames and in-between frames
    """
    def __init__(self, camera_position):
        self.camera_position = camera_position


class V3dKeyFrame(V3dMovieFrame):
    def __init__(self, camera_position, interval = 0):
        V3dMovieFrame.__init__(self, camera_position)
        self.interval = interval # in seconds from previous frame
        self.interpolator = LinearInterpolator()


class V3dMovie:
    def __init__(self, seconds_per_frame=1.0/24.0):
        import v3d
        self.seconds_per_frame = seconds_per_frame # seconds per frame, default 24 fps
        self.key_frames = []
        self.image_window = None
        self.view_control = None
        try:
            self.image_window = v3d.ImageWindow.current()
        except:
            self.image_window = None
        try:
            self.view_control = self.image_window.getView3DControl()
        except:
            self.view_control = None
        # interpolation_param_names are names of View3DControl getter/getter methods
        self.view_control_param_names = {
                                        'xShift' : 'setXShift', 
                                        'yShift' : 'setYShift', 
                                        'zShift' : 'setZShift',
                                        'xRot' : 'setXRotation', 
                                        'yRot' : 'setYRotation', 
                                        'zRot' : 'setZRotation',
                                        'zoom' : 'setZoom',
                                        'xCut0' : 'setXCut0', 
                                        'xCut1' : 'setXCut1',
                                        'yCut0' : 'setYCut0', 
                                        'yCut1' : 'setYCut1',
                                        'zCut0' : 'setZCut0', 
                                        'zCut1' : 'setZCut1'}

    def _setup_interpolation_lists(self):
        # Create list of key frame x shifts
        self.interpolation_list = {}
        elapsed_time = 0.0
        for param_name in self.view_control_param_names:
            self.interpolation_list[param_name] = []
        for key_frame in self.key_frames:
            elapsed_time += key_frame.interval
            for param_name in self.view_control_param_names:
                self.interpolation_list[param_name].append([elapsed_time, 
                            getattr(key_frame.camera_position, param_name)],)
        
    def play(self):
        self.image_window.open3DWindow()
        frame_number = 0
        for frame in self.generate_frames():
            frame_number += 1
            start_clocktime = time.clock()
            # TODO -for some reason printing is required for the 3D viewer to update
            print "frame %d at %f seconds" % (frame_number, start_clocktime)
            self.set_current_v3d_camera(frame.camera_position)
            # Are we playing too fast?
            real_time_deficit = self.seconds_per_frame - (time.clock() - start_clocktime)
            if real_time_deficit > 0:
                time.sleep(real_time_deficit)

    def _frame_name(self, dir, root, num):
        fname = "%s_frame_%05d.BMP" % (root, num)
        return os.path.join(dir, fname)

    def write_frames(self, directory, file_root=None):
        """
        Writes movie to disk, with one BMP file per movie frame.
        
        Parameters:
          directory -- directory path where frames will be saved
        
          file_root -- base name for frame files.  For example, a file_root
            of "foo" would result in frame files like "foo_frame_00001.BMP",
            "foo_frame_00002.BMP", etc.  If file_root is not specified,
            the file name is based on the name of the V3D image.
        """
        # Make sure the file_root name is OK
        if file_root == None:
            file_root = self.image_window.name
            file_root = os.path.split(file_root)[-1] # remove directory name
            file_root = os.path.splitext(file_root)[0] # remove file extension
        if len(file_root) < 1:
            file_root = "v3d"
        # Modify file name if such frames already exist
        file_root0 = file_root
        file_root_ver = 2
        while os.path.exists(self._frame_name(directory, file_root, 1)):
            file_root = "%s_%d" % (file_root0, file_root_ver)
            file_root_ver += 1
        # Actually write the frames
        frame_number = 0
        for frame in self.generate_frames():
            frame_number += 1
            self.set_current_v3d_camera(frame.camera_position)
            file_name = self._frame_name(directory, file_root, frame_number)
            # print "Writing frame image file %s" % file_name
            if self.image_window:
                self.image_window.screenShot3DWindow(file_name[0:-4]) # strip off ".BMP"
                
    def interpolate_frame(self, elapsed_time, frame_index_hint, interpolator):
        """
        Returns an in-between frame.
        
        elapsed_time -- the amount of time between the start of the movie and the desired frame
        
        frame_index_hint is the index of a nearby key frame
        
        interpolator -- Interpolator object that can compute the frame parameters
        """
        camera_position = CameraPosition()
        for param_name in self.view_control_param_names:
            wrap = None
            # Rotation angles need to be interpolated in a Z system
            if 'Rot' in param_name:
                wrap = 360
            interp_val = interpolator.get_interpolated_value(
                elapsed_time,
                self.interpolation_list[param_name],
                frame_index_hint,
                wrap = wrap)
            setattr(camera_position, param_name, interp_val)
        return V3dMovieFrame(camera_position)
        
    def generate_frames(self):
        "Generator to produce each frame object of the movie, one by one"
        self._setup_interpolation_lists()
        total_time = 0.0
        frame_index = 0
        for key_frame in self.key_frames:
            # also avoid too many frames from roundoff error
            frame_time = self.seconds_per_frame * 1.01
            # First emit in-between frames
            while frame_time < key_frame.interval:
                # TODO interpolate
                yield self.interpolate_frame(total_time, frame_index, key_frame.interpolator)
                # yield V3dMovieFrame() # TODO interpolate
                frame_time += self.seconds_per_frame
                total_time += self.seconds_per_frame
            # Then emit the key frame
            yield key_frame
            total_time += self.seconds_per_frame
            frame_index += 1
            
    def set_current_v3d_camera(self, camera_position):
        vc = self.view_control
        if not vc:
            raise ValueError("No V3D window is attached")
        # print "Setting view control..."
        for getter_name in self.view_control_param_names:
            setter_name = self.view_control_param_names[getter_name]
            if 'Rot' in getter_name:
                continue # rotation is handled specially, below
            val = getattr(camera_position, getter_name)
            if 'Cut' in getter_name:
                val = int(val) # Cut methods take integer arguments
            fn = getattr(vc, setter_name)
            fn(val) # set parameter in V3D view_control
            # print "Setting parameter %s to %s" % (getter_name, val)
        # For some reason set[XYZ]Rotation() does an incremental change        
        self.view_control.doAbsoluteRot(
                    camera_position.xRot,
                    camera_position.yRot,
                    camera_position.zRot)
        
    def get_current_v3d_camera(self):
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        # TODO absoluteRotPose() changes the view slightly.  I don't think it should...
        vc = self.view_control
        vc.absoluteRotPose()
        camera = CameraPosition()
        for param_name in self.view_control_param_names:
            val = getattr(self.view_control, param_name)()
            setattr(camera, param_name, val)
            # print "Parameter %s = %s" % (param_name, val)
        return camera
        
    def append_current_view(self, interval = 2.0):
        camera = self.get_current_v3d_camera()
        if len(self.key_frames) == 0:
            interval = 0.0
        self.key_frames.append(V3dKeyFrame(camera_position = camera, 
                                           interval = interval))


# Standard python technique for optionally running this file as
# a program instead of as a library.
# print "__name__ = %s" % __name__
if __name__ == '__main__':
    import PyQt4
