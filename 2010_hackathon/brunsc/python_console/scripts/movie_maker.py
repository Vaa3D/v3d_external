import v3d
import time

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
    def __init__(self, 
                 x_shift = 0, y_shift = 0, z_shift = 0,
                 x_rotation = 0, y_rotation = 0, z_rotation = 0,
                 zoom = 0,
                 x_cut = [None, None], y_cut = [None, None], z_cut = [None, None]):
        # for testing, camera position is just x_shift for now
        self.x_shift = x_shift
        self.y_shift = y_shift
        self.z_shift = z_shift
        self.x_rotation = x_rotation
        self.y_rotation = y_rotation
        self.z_rotation = z_rotation
        self.zoom = zoom
        self.x_cut = x_cut
        self.y_cut = y_cut
        self.z_cut = z_cut
        

class V3dMovieFrame:
    """
    Any single frame of a V3D movie.
    
    Includes both key frames and in-between frames
    """
    def __init__(self, camera_position = CameraPosition()):
        self.camera_position = camera_position


class V3dKeyFrame(V3dMovieFrame):
    def __init__(self, interval = 0, camera_position = CameraPosition()):
        self.interval = interval # in seconds from previous frame
        self.camera_position = camera_position
        self.interpolator = LinearInterpolator()


class V3dMovie:
    def __init__(self, seconds_per_frame=1.0/24.0):
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

    def _setup_interpolation_lists(self):
        # Create list of key frame x shifts
        elapsed_time = 0.0
        self.x_shift_list = []
        self.y_shift_list = []
        self.z_shift_list = []
        self.x_rotation_list = []
        self.y_rotation_list = []
        self.z_rotation_list = []
        self.zoom_list = []
        for key_frame in self.key_frames:
            elapsed_time += key_frame.interval
            self.x_shift_list.append([elapsed_time, key_frame.camera_position.x_shift],)
            self.y_shift_list.append([elapsed_time, key_frame.camera_position.y_shift],)
            self.z_shift_list.append([elapsed_time, key_frame.camera_position.z_shift],)
            self.x_rotation_list.append([elapsed_time, key_frame.camera_position.x_rotation],)
            self.y_rotation_list.append([elapsed_time, key_frame.camera_position.y_rotation],)
            self.z_rotation_list.append([elapsed_time, key_frame.camera_position.z_rotation],)
            self.zoom_list.append([elapsed_time, key_frame.camera_position.zoom],)        
        
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
                
    def write_frames(self):
        frame_number = 0
        for frame in self.generate_frames():
            frame_number += 1
            self.set_current_v3d_camera(frame.camera_position)
            file_name = "v3d_frame_%04d.bmp" % frame_number
            print file_name
            if self.image_window:
                self.image_window.screenShot3DWindow(file_name)
                
    def interpolate_frame(self, elapsed_time, frame_index_hint, interpolator):
        """
        Returns an in-between frame.
        frame_index_hint is the index of a nearby key frame
        """
        x_shift = interpolator.get_interpolated_value(
                elapsed_time,
                self.x_shift_list, 
                frame_index_hint)
        y_shift = interpolator.get_interpolated_value(
                elapsed_time,
                self.y_shift_list, 
                frame_index_hint)
        z_shift = interpolator.get_interpolated_value(
                elapsed_time,
                self.z_shift_list, 
                frame_index_hint)
        x_rotation = interpolator.get_interpolated_value(
                elapsed_time,
                self.x_rotation_list, 
                frame_index_hint,
                wrap = 360)
        y_rotation = interpolator.get_interpolated_value(
                elapsed_time,
                self.y_rotation_list, 
                frame_index_hint,
                wrap = 360)
        z_rotation = interpolator.get_interpolated_value(
                elapsed_time,
                self.z_rotation_list, 
                frame_index_hint,
                wrap = 360)
        zoom = interpolator.get_interpolated_value(
                elapsed_time,
                self.zoom_list, 
                frame_index_hint)
        camera_position = CameraPosition(x_shift = x_shift,
                                         y_shift = y_shift,
                                         z_shift = z_shift,
                                         x_rotation = x_rotation,
                                         y_rotation = y_rotation,
                                         z_rotation = z_rotation,
                                         zoom = zoom,
                                         ) # TODO
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
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        if self.view_control:
            # print "Setting view control..."
            # For some reason set[XYZ]Rotation() does an incremental change
            self.view_control.doAbsoluteRot(
                        camera_position.x_rotation,
                        camera_position.y_rotation,
                        camera_position.z_rotation)
            self.view_control.setXShift(camera_position.x_shift)
            self.view_control.setYShift(camera_position.y_shift)
            self.view_control.setZShift(camera_position.z_shift)            
            self.view_control.setZoom(camera_position.zoom)
        
    def get_current_v3d_camera(self):
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        # TODO absoluteRotPose() changes the view.  I don't think it should...
        vc = self.view_control
        vc.absoluteRotPose()
        
        return CameraPosition(
                    x_shift = vc.xShift(),
                    y_shift = vc.yShift(),
                    z_shift = vc.zShift(),
                    x_rotation = vc.xRot(),
                    y_rotation = vc.yRot(),
                    z_rotation = vc.zRot(),
                    zoom = vc.zoom(),
                    # TODO - cut parameters
                    )
        
    def append_current_view(self, interval = 2.0):
        camera = self.get_current_v3d_camera()
        if len(self.key_frames) == 0:
            interval = 0.0
        self.key_frames.append(V3dKeyFrame(camera_position = camera, 
                                           interval = interval))


# Standard python technique for optionally running this file as
# a program instead of as a library.
if __name__ == 'main':
    print "There is no main"
    assert(False)
