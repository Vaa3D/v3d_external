#!/usr/bin/python

from pyplusplus import module_builder
from pyplusplus import function_transformers as FT
from pyplusplus.module_builder import call_policies
from pygccxml.declarations.matchers import access_type_matcher_t
import commands
import os

class V3DWrapper:
    def __init__(self):
        "Container for pyplusplus module builder for wrapping V3D"
        includes = []
        for path in ['.',
                       '/home/cmbruns/svn/v3d_cmake/v3d_main/basic_c_fun',
                       '/Users/brunsc/svn/v3d_cmake/v3d_main/basic_c_fun',
                       '/usr/include/qt4',
                       '/usr/include/qt4/QtCore',
                       '/usr/include/qt4/QtGui',
                       '/Library/Frameworks/QtCore.framework/Headers',
                       '/Library/Frameworks/QtGui.framework/Headers',
                     ]:
            if os.path.exists(path):
                includes.append(path)
        gccxml_executable = commands.getstatusoutput("which gccxml")[1]
        self.mb = module_builder.module_builder_t(
            files = ['wrappable_v3d.h',],
            gccxml_path=gccxml_executable,
            cflags=' --gccxml-cxxflags "-m32"',
            include_paths=includes
            )
        
    def wrap(self):
        self.mb.enum('ImagePixelType').include()
        self.mb.enum('TimePackType').include()
        self.mb.class_('V3D_GlobalSetting').include()
        self.mb.class_('NeuronTree').include()
        self.mb.class_('Image4DSimple').include()
        self.mb.class_('View3DControl').include()
        self.mb.class_('NeuronSWC').include()
        self.mb.class_('XYZ').include()
        self.mb.enum('PxLocationMarkerShape').include()
        self.mb.enum('PxLocationUsefulness').include()
        # self.mb.class_('RGB16i').include()
        # self.mb.class_('RGB32i').include()
        # self.mb.class_('RGB32f').include()
        self.wrap_Image4DSimple()
        self.wrap_TriviewControl()
        self.wrap_ImageWindow()
        self.wrap_LocationSimple()
        
    def wrap_LocationSimple(self):
        cls = self.mb.class_('LocationSimple')
        cls.include()
        for fn in cls.member_functions('getCoord'):
            fn.add_transformation( 
                    FT.output('xx'), 
                    FT.output('yy'), 
                    FT.output('zz') )
        
    def wrap_Image4DSimple(self):
        cls = self.mb.class_("Image4DSimple")
        cls.include()
        cls.member_functions( access_type_matcher_t( 'protected' ), 
                              allow_empty=True ).exclude()
        cls.variables( access_type_matcher_t( 'protected' ), 
                              allow_empty=True ).exclude()
        cls.member_function('getRawData').exclude() # raw pointer not useful in python
        cls.member_function('getRawDataAtChannel').exclude()
        # link errors
        cls.member_functions('loadImage').exclude()        
        cls.member_function('saveImage').exclude()        
        cls.member_function('createImage').exclude()        
        cls.member_function('createBlankImage').exclude()        
    
    def wrap_ImageWindow(self):
        cls = self.mb.class_('ImageWindow')
        cls.variables('handle').exclude()
        # get/set image
        fn1 = cls.member_function("getImage")
        fn1.call_policies = call_policies.return_internal_reference()
        fn2 = cls.member_function("setImage")
        fn2.call_policies = call_policies.with_custodian_and_ward(1,2)
        cls.add_property( 'image'
                     , cls.member_function( 'getImage' )
                     , cls.member_function( 'setImage' ) )
        # get/set name
        cls.add_property( 'name'
                     , cls.member_function( 'getName' )
                     , cls.member_function( 'setName' ) )
        # 3D window controls
        fn = cls.member_function("getView3DControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('view3DControl', fn)
        fn = cls.member_function("getLocalView3DControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('localView3DControl', fn)
        fn = cls.member_function("getTriviewControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('triViewControl', fn)
        
    def wrap_TriviewControl(self):
        tvc = self.mb.class_('TriviewControl')
        tvc.include()
        # getFocusLocation passes int refs as args for return.
        gfl = tvc.member_function('getFocusLocation')
        gfl.add_transformation( 
                FT.output('cx'), 
                FT.output('cy'), 
                FT.output('cz') )
        fn = tvc.member_function('getTriViewColorDispType')
        fn.add_transformation(FT.output('mytype'))
        
    def write_out(self, outputDir):
        self.mb.build_code_creator(module_name='v3d')
        self.mb.split_module(outputDir)

if __name__ == "__main__":
    v3dWrapper = V3DWrapper()
    v3dWrapper.wrap()
    v3dWrapper.write_out('generated_code')
