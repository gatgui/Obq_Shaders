# 2015-04-06 12.39 pm

import pymel.core as pm
import maya.cmds as cmds
import maya.mel as mel
import mtoa.ui.ae.templates as templates
#from mtoa.ui.ae.customShapeAttributes import CameraTemplate as CameraTemplate

ViewModeEnumOp = [
    (0, 'Center Camera'),
    (1, 'Left Camera'),
    (2, 'Right Camera'),
    (3, 'Stereo Camera <left-right>'),
    (4, 'Stereo Camera <down-up>')
]

def Obq_StereoLensCreateViewMode(attr):
    cmds.setUITemplate('attributeEditorPresetsTemplate', pushTemplate=True)
    cmds.attrEnumOptionMenuGrp('Obq_StereoLensViewMode', attribute=attr, label="View Mode", enumeratedItem=ViewModeEnumOp)    
    cmds.setUITemplate(popTemplate=True)

def Obq_StereoLensSetViewMode(attr):
    cmds.attrEnumOptionMenuGrp('Obq_StereoLensViewMode', edit=True, attribute=attr)

def Obq_StereoLensHelpURL():
    # Add the Obq_Shader docs URL to the Attribute Editor help menu
    ObqNodeType = 'Obq_StereoLens'
    ObqNodeHelpURL = 'http://s3aws.obliquefx.com/public/shaders/help_files/Obq_StereoLens.html'
    ObqHelpCommand = 'addAttributeEditorNodeHelp("' + ObqNodeType + '", "showHelp -absolute \\"' +ObqNodeHelpURL +'\\"");'
    mel.eval(ObqHelpCommand)

class Obq_StereoLensTemplate(templates.AttributeTemplate):
    def setup(self):
        self.beginLayout("Obq_StereoLens", collapse=False)

        Obq_StereoLensHelpURL()
        
        self.beginLayout("Main", collapse=False)
        
        self.beginLayout("View Mode", collapse=False)
        self.addCustom('aiViewMode', Obq_StereoLensCreateViewMode, Obq_StereoLensSetViewMode)
        self.endLayout()

        self.beginLayout("Cameras", collapse=False)
        self.addControl("aiLeftCamera", label="Left")
        self.addControl("aiRightCamera", label="Right")
        # Note this attribute might not exist
        # Error: MayaAttributeError: Maya Attribute does not exist (or is not unique):: u'perspShape.aiCameraStatus' # 
        #self.addControl("aiCameraStatus", label="Status")
        self.endLayout()
        
        
        self.beginLayout("Automatic Overscan", collapse=True)
        #self.addControl("aiUseOverscan", label="Enable use Overscan")
        #self.addControl("aiFilterSize", label="Filter Size")
        self.endLayout()

        self.beginLayout("Target Resolution", collapse=True)
        # self.addControl("aiTargetResolutionX", label="Width")
        # self.addControl("aiTargetResolutionY", label="Height")
        self.endLayout()

        self.beginLayout("Render Resolution", collapse=True)
        #self.addControl("aiRenderResolutionX", label="Width")
        #self.addControl("aiRenderResolutionY", label="Height")
        #self.addControl("aiUpdatePassResolution", label="Automatic update of pass output resolution")
        self.endLayout()

        self.beginLayout("Nuke Info", collapse=True)
        #self.addControl("aiLeftCropInfo", label="Left Crop")
        #self.addControl("aiRghtCropInfo", label="Right Crop")
        self.endLayout()
        
        self.endLayout() # End of Main Layout

        
        
        self.beginLayout("Depth of Field", collapse=False)
        self.addControl("aiUseDof", label="Enable DoF")
        
        self.beginLayout("Focus Distance", collapse=False)
        self.addControl("aiFocusDistance", label="Distance")
        self.addControl("aiFocusPlaneIsPlane", label="Focal plane is a plane")
        self.addControl("aiRecalculateDistanceForSideCameras", label="Recalculate focus distance for left and right cameras")
        self.endLayout()

        self.beginLayout("Aperture", collapse=False)
        self.addControl("aiApertureSize", label="Size")
        self.addControl("aiApertureAspectRatio", label="Aspect Ratio")
        self.addControl("aiUsePolygonalAperture", label="Polygonal Aperture")
        self.addControl("aiApertureBlades", label="Blades")
        self.addControl("aiApertureBladeCurvature", label="Blade Curvature")
        self.addControl("aiApertureRotation", label="Rotation")
        
        self.beginLayout("Bokeh Quality", collapse=False)
        self.addControl("aiBokehInvert", label="Invert")
        self.addControl("aiBokehBias", label="Bias")
        self.addControl("aiBokehGain", label="Gain")
        self.endLayout()
        
        self.endLayout() #End of Aperture Layout
        
        # self.beginLayout("Debug", collapse=False)
        # self.addControl("aiFilmbackX", label="Filmback X")
        # self.addControl("aiLeftCenterOffset", label="Left Optical Center Offset")
        # self.addControl("aiRightCenterOffset", label="Right Optical Center Offset")
        # self.addControl("aiTotalOverscanPixels", label="Total Pixel Overscan")
        # self.endLayout()

        self.endLayout()     # end Obq_StereoLens layout

        self.beginLayout("Options", collapse=True)
        self.addControl("aiUserOptions", label="User Options")
        self.endLayout()

templates.registerTranslatorUI(Obq_StereoLensTemplate, "camera", "Obq_StereoLens")
#templates.registerTranslatorUI(Obq_StereoLensTemplate, "stereoRigCamera", "Obq_StereoLens")
