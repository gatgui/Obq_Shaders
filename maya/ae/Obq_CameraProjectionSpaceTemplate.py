# 2016-12-18 5.46 AM

import pymel.core as pm
import maya.cmds as cmds
import maya.mel as mel
import mtoa.utils as utils
import mtoa.ui.ae.utils as aeUtils
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

def Obq_CameraProjectionSpaceHelpURL():
	# Add the Obq_Shader docs URL to the Attribute Editor help menu
	ObqNodeType = 'Obq_CameraProjectionSpace'
	ObqNodeHelpURL = 'https://github.com/madesjardins/Obq_Shaders/wiki/Obq_CameraProjectionSpace'
	ObqHelpCommand = 'addAttributeEditorNodeHelp("' + ObqNodeType + '", "showHelp -absolute \\"' +ObqNodeHelpURL +'\\"");'
	mel.eval(ObqHelpCommand)

class AEObq_CameraProjectionSpaceTemplate(ShaderAETemplate):
	convertToMayaStyle = True
	
	def setup(self):

		#self.addSwatch()

		self.beginScrollLayout()

		self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

		#pm.picture(image='Obq_shader_header.png', parent="AttrEdObq_CameraProjectionSpaceFormLayout")
		Obq_CameraProjectionSpaceHelpURL()

		self.beginLayout("Render Camera", collapse=False )
		self.addControl("useRenderCamera", label="Use Render Camera")
		self.endLayout()

		self.beginLayout("Other Camera", collapse=False )
		self.addControl("projectionCamera", label="Camera")
		self.addControl("cameraAspectRatio", label="Camera Aspect Ratio")
		self.endLayout()

		# include/call base class/node attributes
		pm.mel.AEdependNodeTemplate(self.nodeName)

		# Hide the NormalCamera and HardwareColor Extra Attributes
		self.suppress('normalCamera')
		self.suppress('hardwareColor')

		self.addExtraControls()
		self.endScrollLayout()
