import v3d
import time

class Interpolator:
    def get_interpolated_value(self, param, param_value_list, index_hint = None):
        """
        param_value_list is an ordered list of (parameter, value) pairs
        in which the parameter parameter is strictly increasing
        """
        raise NotImplementedError("Use a concrete derived class of Interpolator")


# TODO - create spline interpolator
# and quaternion interpolator for rotations
class LinearInterpolator(Interpolator):
    """
    Simple linear interpolator, so I can get things working quickly.
    Later we will use spline interpolation.
    """
    def get_interpolated_value(self, param, param_value_list, index_hint = None):
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
        val = val1 + alpha * (val2 - val1)
        # print param, p1, p2, val1, val2, alpha
        return val


class CameraPosition:
    def __init__(self, x_shift = 0, y_shift = 0, z_shift = 0,
                 zoom = 0):
        # for testing, camera position is just x_shift for now
        self.x_shift = x_shift
        self.y_shift = y_shift
        self.z_shift = z_shift
        self.zoom = zoom
        

class V3dMovieFrame:
    "Any single frame of a V3D movie"
    def __init__(self, camera_position = CameraPosition()):
        self.camera_position = camera_position


class V3dKeyFrame(V3dMovieFrame):
    def __init__(self, interval = 0, camera_position = CameraPosition()):
        self.interval = interval # in seconds from previous frame
        self.camera_position = camera_position
        self.interpolator = LinearInterpolator()


class V3dMovie:
    def __init__(self, view_control = None, frame_interval=1.0/24.0):
        self.frame_interval = frame_interval # seconds per frame, default 24 fps
        self.key_frames = []
        if view_control:
            self.view_control = view_control
        try:
            image = v3d.ImageWindow.current()
            self.view_control = image.getView3DControl()
        except:
            self.view_control = None

    def _setup_interpolation_lists(self):
        # Create list of key frame x shifts
        elapsed_time = 0.0
        self.x_shift_list = []
        self.y_shift_list = []
        self.z_shift_list = []
        self.zoom_list = []
        for key_frame in self.key_frames:
            elapsed_time += key_frame.interval
            self.x_shift_list.append([elapsed_time, key_frame.camera_position.x_shift],)
            self.y_shift_list.append([elapsed_time, key_frame.camera_position.y_shift],)
            self.z_shift_list.append([elapsed_time, key_frame.camera_position.z_shift],)
            self.zoom_list.append([elapsed_time, key_frame.camera_position.zoom],)        
        
    def create_movie(self):
        self._setup_interpolation_lists()
        for frame in self.generate_frames():
            start_clocktime = time.clock()
            print start_clocktime
            self.set_current_v3d_camera(frame.camera_position)
            # Are we playing too fast?
            real_time_deficit = self.frame_interval - (time.clock() - start_clocktime)
            if real_time_deficit > 0:
                time.sleep(real_time_deficit)
            
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
        zoom = interpolator.get_interpolated_value(
                elapsed_time,
                self.zoom_list, 
                frame_index_hint)
        camera_position = CameraPosition(x_shift = x_shift,
                                         y_shift = y_shift,
                                         z_shift = z_shift,
                                         zoom = zoom,
                                         ) # TODO
        return V3dMovieFrame(camera_position)
        
    def generate_frames(self):
        "Generator to produce each frame object of the movie, one by one"
        total_time = 0.0
        frame_index = 0
        for key_frame in self.key_frames:
            # also avoid too many frames from roundoff error
            frame_time = self.frame_interval * 1.01
            # First emit in-between frames
            while frame_time < key_frame.interval:
                # TODO interpolate
                yield self.interpolate_frame(total_time, frame_index, key_frame.interpolator)
                # yield V3dMovieFrame() # TODO interpolate
                frame_time += self.frame_interval
                total_time += self.frame_interval
            # Then emit the key frame
            yield key_frame
            total_time += self.frame_interval
            frame_index += 1
            
    def set_current_v3d_camera(self, camera_position):
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        if self.view_control:
            # print "Setting view control..."
            self.view_control.setXShift(int(camera_position.x_shift))
            self.view_control.setYShift(int(camera_position.y_shift))
            self.view_control.setZShift(int(camera_position.z_shift))
            self.view_control.setZoom(int(camera_position.zoom))
        
    def get_current_v3d_camera(self):
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        return CameraPosition(
                    x_shift = self.view_control.xShift(),
                    y_shift = self.view_control.yShift(),
                    z_shift = self.view_control.zShift(),
                    zoom = self.view_control.zoom(),
                    )
        
    def append_current_view(self, interval = 1.0):
        camera = self.get_current_v3d_camera()
        if len(self.key_frames) == 0:
            interval = 0.0
        self.key_frames.append(V3dKeyFrame(camera_position = camera, 
                                           interval = interval))


def test_movie():
    movie = V3dMovie()
    movie.key_frames.append(V3dKeyFrame(
            camera_position = CameraPosition(x_shift = 0.0)))
    movie.key_frames.append(V3dKeyFrame(
            interval=1.0,
            camera_position = CameraPosition(x_shift = 50.0)))
    movie.create_movie()


# Standard python technique for optionally running this file as
# a program instead of as a library.
if __name__ == 'main':
    print "There is no main"
    assert(False)
